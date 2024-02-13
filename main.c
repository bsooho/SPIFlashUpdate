//
// Write ROM file (*.BIN) into SPI Serial flash IS25LP256 by Raspberry PI 4B
// Structure of IS25LP256 memory
//    Total byte 33554432 (256Mbit)
//    Total memory address range (4byte=32bit) 0x00000000 - 0x1FFFFFFF
//       Real usage
//    Total number of block 512 (64KB/block, 16sector/block)
//    Total number of sector 8192 (4KB/sector)
//    JEDEC ID: 9D-6019
//    Unique ID: different for individual chip
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <gpiod.h>        // GPIO control using GPIOD Library
#include <wiringPiSPI.h>  // SPI control using WiringPi Library
#include "IS25LP256.h"

#define SPI_CHANNEL 0   // /dev/spidev0.0 사용
//#define SPI_CHANNEL 1 // /dev/spidev0.1 사용

#define GPIO_CHIP "gpiochip0"
#define GPIO_PIN 14

#define FILENAME "./FLASH_EN.bin"		// File name to be written to SPI Flash memory

#define SPI_MODE  0          // SPI mode among 0, 1, 2 or 3
#define SPI_DEVICE "/dev/spidev0.0"  // SPI channel 0
#define SPI_SPEED_HZ 2000000	// SPI clock speed at 2MHz
#define CHUNK_SIZE 256			// unit amount per write operation

//
// Dump and show data (256 byte, similar with HEX editor mode)
// dt(in) : start address of data (pointer)
// n(in)  : number of data
//
void dump(uint8_t *dt, uint32_t n) {
  uint16_t clm = 0;
  uint8_t data;
  uint8_t sum;          // horizontal sum
  uint8_t vsum[16];     // vertical sum
  uint8_t total =0;
  uint32_t saddr =0;    // start address
  uint32_t eaddr =n-1;  // end address
  
  printf("----------------------------------------------------------\n");
  uint16_t i;
  for (i=0;i<16;i++) vsum[i]=0;  //initilize vsum with 0
  
  uint32_t addr;
  for (addr = saddr; addr <= eaddr; addr++) {
    data = dt[addr];
    if (clm == 0) {
      sum =0;
      printf("%05x: ",addr);
    }

    sum+=data;
    vsum[addr % 16]+=data;
    
    printf("%02x ",data);
    clm++;
    if (clm == 16) {
      printf("|%02x \n",sum);
      clm = 0;
    }
  }
  printf("----------------------------------------------------------\n");
  printf("       ");
  for (i=0; i<16;i++) {
    total+=vsum[i];
    printf("%02x ",vsum[i]);
  }
  printf("|%02x \n\n",total);
}

int main() {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;
  
    uint8_t jedc[3];      // JEDEC-ID (3byte, MF7-MF0 ID15-ID8 ID7-ID0)
    uint8_t buf[CHUNK_SIZE];     // acquired data, 256byte
    uint8_t wdata[CHUNK_SIZE];   // data to be written, 256byte (Maximum 256byte by Input Page Write command)
    uint8_t i;            // general variable
    uint16_t n;           // return value or number of data read
    uint16_t sect_no;     // sector number
    uint16_t blk32_no;    // block(32kB) number
    uint16_t blk64_no;    // block(64kB) number

  //  int spi_fd;
    int file_fd;
  
    ssize_t bytes_read;
  
    int start_addr=0xFF0000;    // start address for write
  
    uint16_t s_sect_no=start_addr>>12;  // start sector number for Input Page Write
    uint16_t s_blk32_no=start_addr>>15;  // start block32 number for Input Page Write
    uint16_t s_blk64_no=start_addr>>16;  // start block64 number for Input Page Write

    uint32_t s_addr=start_addr;         // assign start address at 32bit variable

    size_t fileSize;

    // Open binary file for reading
    FILE* binaryFile = fopen(FILENAME, "rb");
    if (binaryFile == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Get the file size
    fseek(binaryFile, 0, SEEK_END);
    fileSize = ftell(binaryFile);
    printf ("File size: %d\n",fileSize);
    fseek(binaryFile, 0, SEEK_SET);

    // Allocate memory for the file content
    uint8_t *fileContent = (uint8_t *)malloc(fileSize);
    if (fileContent == NULL) {
        perror("Memory allocation error");
        fclose(binaryFile);
        return 1;
    }
  
/*
    // Open binary file
    file_fd = open(FILENAME, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening binary file");
        close(spi_fd);
        return 1;
    }
*/
  
  /* Below is not required since WiringPiSPI library is used in this code.
  
    // Open SPI device
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device");
        return 1;
    }
  
    // Configure SPI settings (you may need to adjust according to your Flash memory specifications)
    uint8_t mode = SPI_MODE_0;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Error setting SPI mode");
        close(spi_fd);
        close(file_fd);
        return 1;
    }
  
    uint32_t speed = SPI_SPEED_HZ;
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error setting SPI speed");
        close(spi_fd);
        close(file_fd);
        return 1;
    }
*/

    // Open GPIO chip
    chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return 1;
    }

    // Get GPIO line
    line = gpiod_chip_get_line(chip, GPIO_PIN);
    if (!line) {
        perror("Failed to get GPIO line");
        gpiod_chip_close(chip);
        return 1;
    }

    // Request GPIO line
    ret = gpiod_line_request_output(line, "gpio-control", 0);
    if (ret < 0) {
        perror("Failed to request GPIO line");
        gpiod_chip_close(chip);
        return 1;
    }

    gpiod_line_set_value(line, 0); // Set line low (0V)
    printf("SPI Bypass Disabled!\n\n");

    usleep(500000);  //sleep 0.5sec

    gpiod_line_set_value(line, 1); // Set line high (3.3V)
    printf("SPI Bypass Enabled!\n\n");


    printf("Setup SPI0 channel\n");
  
    // Start SPI channel 0 with 2MHz speed
    if (wiringPiSPISetupMode(SPI_CHANNEL, SPI_SPEED_HZ, SPI_MODE) < 0) {
      printf("SPISetup failed:\n");
      return 1;
    }




  
    // Begin of flash memory
    IS25LP256_begin(SPI_CHANNEL);

    // Read JEDEC ID (It must be 9d 60 19 (3 byte))
    IS25LP256_readManufacturer(jedc);
    printf("\nJEDEC ID : ");
    for (i=0; i< 3; i++) {
      printf("%x ",jedc[i]);
    }
    printf("\n");
    
    // Unique ID 획득 (16 byte, every memory chip has a distinct or unique value)
    IS25LP256_readUniqieID(buf);
    printf("Unique ID : ");
    for (i=0; i< 16; i++) {
      printf("%x ",buf[i]);
    }
    printf("\n");
  
    // Read current stored data
    // 256 byte from address s_addr
    memset(buf,0,256);  // 임시 버퍼 클리어
    n =  IS25LP256_read(s_addr, buf, 256);
    printf("Read Data: n=%d\n",n);
    dump(buf,256);

    // 섹터 단위 삭제, 256byte 단위로 테스트하므로 4kB 즉 4096byte만 지워도 됨
    // 입력할 주소는 Sector No.이므로, 주소를 12bit 오른으로 밀어야 함.
    // Erase data by Sector
    n = IS25LP256_eraseSector(s_sect_no,true);
    printf("Erase Sector(%04x): n=%d\n",s_sect_no,n);
    memset(buf,0,256);  // 임시 버퍼 클리어
    n =  IS25LP256_read (s_addr, buf, 256);
    dump(buf,256);
 
    // 데이터 쓰기 테스트 START_ADDR+10부터 A~Z 쓰기
    for (i=0; i < 26; i++) {
      wdata[i]='A'+i; // 쓸 데이터 생성, A-Z, 총 26개
    }
    n =  IS25LP256_pageWrite(s_sect_no, 10, wdata, 26);
    printf("page_write(%08x,10,d,26): n=%d\n",s_addr,n);

    // 데이터 읽기 (주소 0부터 256바이트 데이터 가져오기)
    // Read 256 byte data from Address=0
    memset(buf,0,256);  // 임시 버퍼 클리어
    n =  IS25LP256_read(s_addr, buf, 256);
    printf("Read Data: n=%d\n",n);
    dump(buf,256);

    // 데이터 쓰기 테스트
    // Write data to Sector=0 Address=0
    for (i=0; i < 10; i++) {
      wdata[i]='0'+i; // 0-9     
    }  
    n =  IS25LP256_pageWrite(s_sect_no, 0, wdata, 10);
    printf("page_write(%08x,0,d,10): n=%d\n",s_addr,n);

    // 고속 데이터 읽기(주소 0에서 256바이트 가져 오기)
    // First read 256 byte data from Address=0
    memset(buf,0,256);  // 임시 버퍼 클리어
    n =  IS25LP256_fastread(s_addr,buf, 256);
    printf("Fast Read Data: n=%d\n",n);
    dump(buf,256);

    // 상태 레지스터 가져오기
    // Get fron Status Register1
    buf[0] = IS25LP256_readStatusReg();
    printf("Status Register: %x\n",buf[0]);

    // Disable SPI0 Bypass lines
    gpiod_line_set_value(line, 0); // Set line low (V)
    printf("SPI Bypass Disabled!\n\n");
  
    return 0;
}
