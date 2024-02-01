//
// SPI Serial flash 메모리 IS25LP256 동작검증 프로그램
// IS25LP256의 메모리 영역 구조
//    총 바이트 수 33554432 (256Mbit)
//    메모리 공간 32비트 주소 지정 0x00000000 - 0x1FFFFFFF
//    총 블록 수 512 (64KB/블록, 16섹터/블록)
//    총 섹터 수 8192 (4KB/섹터)
//    JEDEC ID: 9D-6019
//

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wiringPiSPI.h>
#include "IS25LP256.h"

#define SPI_CHANNEL 0   // /dev/spidev0.0 사용
//#define SPI_CHANNEL 1 // /dev/spidev0.1 사용

//#define START_ADDR  0  // 사용할 메모리의 시작 주소 지정
//#define START_ADDR  0x10000  // Block(64kB) 1번 = Block(32kB) 2번 = Sector 16번의 시작 주소에 해당함함

//
// 쓰기 데이터 덤프 목록 보여주기
// dt(in) : 데이터 저장 시작 주소 (포인터)
// n(in)  : 표시할 데이터 개수
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
  for (i=0;i<16;i++) vsum[i]=0;  //vsum 모두 0으로 초기화
  
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
    uint8_t jedc[3];      // JEDEC-ID (3byte, MF7-MF0 ID15-ID8 ID7-ID0)
    uint8_t buf[256];     // 취득 데이터, 256byte
    uint8_t wdata[26];    // 데이터 쓰기, 26byte???
    uint8_t i;            // 범용 변수
    uint16_t n;           // 취득 데이터 수
    uint16_t sect_no;     // sector number
    uint16_t blk32_no;    // block(32kB) number
    uint16_t blk64_no;    // block(64kB) number

    int start_addr=0x10000;    // start address
  
    uint16_t s_sect_no=start_addr>>12;  // start sector and page write number
    uint32_t s_addr=start_addr;       // start address for 32bit variable
  
    // SPI channel 0을 2MHz로 시작
    // Start SPI channel 0 with 2MHz
    if (wiringPiSPISetup(SPI_CHANNEL, 2000000) < 0) {
      printf("SPISetup failed:\n");
    }
    
    // 플래시 메모리 사용 시작
    // (채널 번호 지정. 대부분 0번 사용함)
    IS25LP256_begin(SPI_CHANNEL);
    
    // JEDEC ID 획득 (3byte)
    IS25LP256_readManufacturer(jedc);
    printf("JEDEC ID : ");
    for (i=0; i< 3; i++) {
      printf("%x ",jedc[i]);
    }
    printf("\n");
    
    // Unique ID 획득 (16byte)
    IS25LP256_readUniqieID(buf);
    printf("Unique ID : ");
    for (i=0; i< 16; i++) {
      printf("%x ",buf[i]);
    }
    printf("\n");
    
    // 현재 저장되어 있는 데이터 읽기
    // 주소 START_ADDR부터 256바이트 가져오기
    memset(buf,0,256);  // 임시 버퍼 클리어
    n =  IS25LP256_read(s_addr, buf, 256);
    printf("Read Data: n=%d\n",n);
    dump(buf,256);

    // 현재 저장되어 있는 데이터 고속 읽기
    // 주소 START_ADDR에서 256바이트까지 획득
    memset(buf,0,256);
    n =  IS25LP256_fastread(s_addr, buf, 256);
    printf("Fast Read Data: n=%d\n",n);
    dump(buf,256);

    // 섹터 단위 삭제, 256byte 단위로 테스트하므로 4kB 즉 4096byte만 지워도 됨
    // 입력할 주소는 Sector No.이므로, 주소를 12bit 오른으로 밀어야 함.
    // Erase data by Sector
    n = IS25LP256_eraseSector(s_sect_no,true);
    printf("Erase Sector(0): n=%d\n",n);
    memset(buf,0,256);  // 임시 버퍼 클리어
    n =  IS25LP256_read (s_addr, buf, 256);
    dump(buf,256);
 
    // 데이터 쓰기 테스트 START_ADDR+10부터 A~Z 쓰기
    for (i=0; i < 26; i++) {
      wdata[i]='A'+i; // 쓸 데이터 생성, A-Z, 총 26개
    }
    n =  IS25LP256_pageWrite(s_sect_no, 10, wdata, 26);
    printf("page_write(%08x,10,d,26): n=%d\n",start_addr,n);

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
    printf("page_write(%08x,0,d,10): n=%d\n",start_addr,n);

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

    return 0;
}
