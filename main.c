//
// Write ROM file (*.BIN) into SPI Serial flash IS25LP256 by Raspberry PI 4B
// Structure of IS25LP256 memory
//    Total byte 33554432 (256Mbit, 32MB)
//    Total memory address range (4byte=32bit) 0x00000000 - 0x1FFFFFFF
//      * Real usage by FPGA (Artix7 100T) is 3.64MB (1/8 of total memory) 
//    Total number of block 512 (64KB/block, 16sector/block)
//    Total number of sector 8192 (4KB/sector)
//    JEDEC ID: 9D-60 19
//    Unique ID: different for individual chip
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <gpiod.h>        // GPIO control using libgpiod Library
#include <wiringPiSPI.h>  // SPI control using WiringPi Library
#include "IS25LP256.h"    // Custom made library for SPI Flash operation through SPI0 channel

#define SPI_CHANNEL 0   // /dev/spidev0.0

#define GPIO_CHIP "gpiochip0"
#define GPIO_14_ROM_UPDATE_EN	14		// ROM_UPDATE_EN
#define GPIO_07_SLEEP_EN	07		// SLEEP_EN


#define FILENAME "./SMI_v2.2_240613_1xSPI.bin"		// Binaray file name to be written to SPI Flash memory

#define SPI_MODE  0          // SPI mode among 0, 1, 2 or 3
#define SPI_DEVICE "/dev/spidev0.0"  // SPI channel 0
#define SPI_SPEED_HZ 10000000	// SPI clock speed at 10MHz
#define CHUNK_SIZE 256			// unit amount per write operation
#define SECTOR_SIZE 4096    // unit amount of one sector


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


//
// Pause and wait for space bar
//
void wait_for_space() {
    struct termios oldt, newt;
    int ch;

    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);

    // Copy settings to newt and modify them
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo

    // Apply the new settings to the terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Wait for the space bar
    printf("Press the space bar to continue...\n");
    do {
        ch = getchar();
    } while (ch != ' ');

    // Restore the original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}




//
// Main program
//    1. Read ROM binary file
//    2. Read JEDEC ID of Flash memeory
//    3. Pause and wait for space bar
//
int main() {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;
  
    uint8_t jedc[3];      // JEDEC-ID (3byte, MF7-MF0 ID15-ID8 ID7-ID0)
    uint8_t buf[CHUNK_SIZE];     // acquired data, 256byte
    uint8_t wdata[CHUNK_SIZE];   // data to be written, 256byte (Maximum 256byte by Input Page Write command)
    uint8_t i;            // general variable, unsigned 8bit
    uint16_t ii;          // general variable, unsigned 16bit
    uint16_t n;           // return value or number of data read
  
    ssize_t bytes_read;
  
    int start_addr=0x00;    // start address for write
  
    uint16_t s_sect_no=start_addr>>12;  // start sector number for Input Page Write
    uint16_t s_blk32_no=start_addr>>15;  // start block32 number for Input Page Write
    uint16_t s_blk64_no=start_addr>>16;  // start block64 number for Input Page Write

    uint32_t s_addr=start_addr;         // assign start address at 32bit variable


    // Open binary file for reading
    FILE* binaryFile = fopen(FILENAME, "rb");
    if (binaryFile == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Get the file size (Check manually if file is open normally)
    size_t fileSize;
    fseek(binaryFile, 0, SEEK_END);
    fileSize = ftell(binaryFile);
    printf ("File size: %d\n",fileSize);
    fseek(binaryFile, 0, SEEK_SET);

    // Open GPIO chip
    chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return 1;
    }





    // Get GPIO 07 line.
    // GPIO_07_SLEEP_EN (Active high)
    line = gpiod_chip_get_line(chip, GPIO_07_SLEEP_EN);
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

    // Test of GPIO 07 (High - Enable Sleep --> Clock power down)
    gpiod_line_set_value(line, 1);
    printf("Sleep Enabled!\n\n");

    usleep(100000);  //sleep 0.1sec

    // Test of GPIO 07 (Low - Disable Sleep --> Clock operation)
    gpiod_line_set_value(line, 0);
    printf("Sleep Disabled!\n\n");






    // Get GPIO 14 line.
    // GPIO_14_ROM_UPDATE_EN (Active high)
    line = gpiod_chip_get_line(chip, GPIO_14_ROM_UPDATE_EN);
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

    // Test of GPIO 14 (Low - Disable ROM Update)
    gpiod_line_set_value(line, 0); // Set line low (0V)
    printf("SPI Bypass Disabled!\n\n");

    usleep(100000);  //sleep 0.1sec

    // Test of GPIO 14 (High - Enable ROM Update)
    gpiod_line_set_value(line, 1); // Set line high (3.3V)
    printf("SPI Bypass Enabled!\n\n");
	
	
	
		
	
	
	
	
	
	
    printf("Setup SPI0 channel\n");
  
    // Start SPI channel 0 with pre-defined speed
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
      printf("%02X ",jedc[i]);
    }
    printf("\n");
    
    // Unique ID 획득 (16 byte, every memory chip has a distinct or unique value)
    IS25LP256_readUniqieID(buf);
    printf("Unique ID : ");
    for (i=0; i< 16; i++) {
      printf("%02X ",buf[i]);
    }
    printf("\n");
  
    // Read current stored data
    // 256 byte from address s_addr
    memset(buf,0,256);  // clear temporary buffer
    n =  IS25LP256_read(s_addr, buf, 256);
    printf("Read Data: n=%d\n",n);
    dump(buf,256);
  
    printf("We will start to erase 4MB...\n");
    wait_for_space(); // Program waits here for space bar press
    printf("Erase 4MB is started...\n\n");

//  Erase All. It takes about 1 minute.
//    n = IS25LP256_eraseAll(true);
//    printf("Erase All: n=%d\n",n);

    // Erase first 4MB using Block(64kB) erase for 64 times.
    // It is because the size of binary file is 3,825,788 byte (3.64MB) which is less than 4MB.
    // It takes about 8 sec.
    for (ii=0; ii<64; ii++){
      n = IS25LP256_erase64Block(ii, true);
    }
  
    // Check if erase is done
    memset(buf,0,256);  // clear temporary buffer
    n =  IS25LP256_read (0, buf, 256);
    dump(buf,256);
  
    printf("Erase 4MB is done!!!\n\n");
  
    wait_for_space(); // Program waits here for space bar press

  
    // write BIN file in SPI Flash memory
    ssize_t read_bytes;
    uint32_t flash_address = 0; // Start address in SPI Flash where data will be written
    uint16_t sector_no = 0;
    uint16_t int_addr = 0;  //internal address within a sector (total 4096byte)
    uint8_t k=0;
  
    while ((read_bytes = fread(buf, 1, CHUNK_SIZE, binaryFile)) > 0) {

//      dump(buf,256);
      
      if (int_addr == SECTOR_SIZE){
        int_addr=0;  //initialize int_addr
      }

      sector_no = flash_address>>12;
      
      n = IS25LP256_pageWrite(sector_no, int_addr, buf, CHUNK_SIZE);

      if (flash_address%0x40000==0){
        printf("sector no = %04x  int_addr = %04x read bytes = %d  write bytes = %d\n", sector_no, int_addr, read_bytes, n-4);
      }

      // increase internal address within Sector(4KB)
      int_addr += CHUNK_SIZE;
      
      // increase flash address (full address)
      flash_address += read_bytes;

    }

    printf("Write is done!!!\n\n");
    wait_for_space(); // Program waits here for space bar press
 
  
    // Read current stored data
    // 512 byte from address 0x00
    memset(buf,0,256);  // clear temporary buffer
    n =  IS25LP256_read(0, buf, 256);
    printf("Read Data: n=%d\n",n);
    dump(buf,256);

    memset(buf,0,256);  // clear temporary buffer
    n =  IS25LP256_read(256, buf, 256);
    printf("Read Data: n=%d\n",n);
    dump(buf,256);
  

    // Get fron Status Register1
    buf[0] = IS25LP256_readStatusReg();
    printf("Status Register: %X\n",buf[0]);

    // Disable SPI0 Bypass lines
    gpiod_line_set_value(line, 0); // Set FLASH_EN (GPIO 14) line low (0V)
    printf("SPI Bypass Disabled!\n\n");
  
    return 0;
}
