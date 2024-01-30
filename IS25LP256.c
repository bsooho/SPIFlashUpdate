//
// Flash Memory IS25LP256 Access Library for RaspberryPi
//

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "IS25LP256.h"

#define CMD_NORD              0x03    // Normal Read Mode
#define CMD_FRD               0x0B    // Fast Read Mode
#define CMD_PP                0x02    // Input Page Program
#define CMD_SER               0x20    // Sector Erase
#define CMD_BER32             0x52    // Block Erase 32Kbyte
#define CMD_BER64             0xD8    // Block Erase 64Kbyte
#define CMD_CER               0xC7    // Chip Erase
#define CMD_WREN              0x06    // Write Enable
#define CMD_WRDI              0x04    // Write Disable
#define CMD_RDSR              0x05    // Read Status Register
#define CMD_DP                0xB9    // Deep Power Down

#define CMD_RDJDID            0x9F    // Read JEDEC ID
#define CMD_RDMDID            0x90    // Read Manufacturer & Device ID, 미사용
#define CMD_RDUID             0x4B    // Read Unique ID

#define SR1_BUSY_MASK	0x01
#define SR1_WEN_MASK	0x02


#define UNUSED(a) ((void)(a))

static uint8_t _spich;

void spcDump(char *id,int rc, uint8_t *data,int len) {
    int i;
    printf("[%s] = %d\n",id,rc);
    for(i=0;i<len;i++) {
      printf("%0x ",data[i]);
      if ( (i % 10) == 9) printf("\n");
    }
    printf("\n");
}

//
// 플래시 메모리 IS25LP256 사용 시작
// 
void IS25LP256_begin(uint8_t spich) {
    _spich = spich;
}

//
// 상태 레지스터의 값 가져오기
// 반환 값: 상태 레지스터의 값
//
uint8_t IS25LP256_readStatusReg(void) {
  unsigned char data[2];
  int rc;
  UNUSED(rc);
  data[0] = CMD_RDSR;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("readStatusReg",rc,data,2);
  return data[1];
}

//
// JEDEC ID(Manufacture, Memory Type, Capacity)를 취득한다.
// d(out) : Manufacture, Memory Type, Capacity의 바이트 수를 저장한다.
//
void IS25LP256_readManufacturer(uint8_t* d) {
  unsigned char data[4];
  int rc;
  UNUSED(rc);
  memset(data,0,sizeof(data));
  data[0] = CMD_RDJDID;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("readManufacturer",rc,data,4);
  memcpy(d,&data[1],3);
}

//
// Unique ID 가져오기
// d(out): Unique ID 8바이트를 반환합니다.
//
void IS25LP256_readUniqieID(uint8_t* d) {
  unsigned char data[13];
  int rc;
  UNUSED(rc);
  memset(data,0,sizeof(data));
  data[0] = CMD_RDUID;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("readUniqieID",rc,data,13);
  memcpy(d,&data[5],8);
}

//
// 쓰기 등 처리 중 체크
// 반환값: true: 쓰기 중, false: 유휴 상태
//
bool IS25LP256_IsBusy(void) {
  unsigned char data[2];
  int rc;
  UNUSED(rc);
  data[0] = CMD_RDSR;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("IsBusy",rc,data,2);
  uint8_t r1;
  r1 = data[1];
  if(r1 & SR1_BUSY_MASK) return true;
  return false;
}

//
// 파워 다운 지정
//
void IS25LP256_powerDown(void) {
  unsigned char data[1];
  int rc;
  UNUSED(rc);
  data[0] = CMD_DP;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("powerDown",rc,data,1);
}

//
// 쓰기 권한 설정
//
void IS25LP256_WriteEnable(void) {
  unsigned char data[1];
  int rc;
  UNUSED(rc);
  data[0] = CMD_WREN;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("WriteEnable",rc,data,1);
}

//
// 쓰기 금지 설정
//
void IS25LP256_WriteDisable(void) {
  unsigned char data[1];
  int rc;
  UNUSED(rc);
  data[0] = CMD_WRDI;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("WriteDisable",rc,data,1);
}

//
// 데이터 불러오기
// addr(in): 읽기 시작 주소 (24비트 0x000000 - 0xFFFFFFFF)
// n(in): 읽기 데이터 수
//
uint16_t IS25LP256_read(uint32_t addr,uint8_t *buf,uint16_t n){ 
  unsigned char *data;
  int rc;

  data = (unsigned char*)malloc(n+4);
  data[0] = CMD_NORD;
  data[1] = (addr>>16) & 0xFF;     // A23-A16
  data[2] = (addr>>8) & 0xFF;      // A15-A08
  data[3] = addr & 0xFF;           // A07-A00
  rc = wiringPiSPIDataRW (_spich,data,n+4);    //Data out on Byte4
  //spcDump("read",rc,data,rc);
  memcpy(buf,&data[4],n);
  free(data);
  return rc-4;
}

//
// 고속 데이터 읽기
// addr(in): 읽기 시작 주소 (24비트 0x000000 - 0xFFFFFFFF)
// n(in): 읽기 데이터 수
//
uint16_t IS25LP256_fastread(uint32_t addr,uint8_t *buf,uint16_t n) {
  unsigned char *data;
  int rc;

  data = (unsigned char*)malloc(n+5);
  data[0] = CMD_FRD;
  data[1] = (addr>>16) & 0xFF;     // A23-A16
  data[2] = (addr>>8) & 0xFF;      // A15-A08
  data[3] = addr & 0xFF;           // A07-A00
  data[4] = 0;                     // Dummy byte
  rc = wiringPiSPIDataRW (_spich,data,n+5);    //Data out on Byte5
  //spcDump("fastread",rc,data,rc);
  memcpy(buf,&data[5],n);
  free(data);
  return rc-5;
}

//
// 섹터 단위 지우기(4kb 공간 단위로 데이터 지우기)
// sect_no(in) 섹터 번호(0 - 4095)
// flgwait(in) true: 처리 대기 false: 대기 없음
// 반환값: true:정상 종료 false:실패
// 추가: 데이터시트에는 지우기에 보통 30ms, 최대 400ms가 걸린다고 명시되어 있다.
// 주소 24비트 중 상위 12비트가 섹터 번호에 해당한다.
// 하위 12비트는 섹터 내 주소가 된다.
//
bool IS25LP256_eraseSector(uint16_t sect_no, bool flgwait) {
  unsigned char data[4];
  int rc;
  UNUSED(rc);
  uint32_t addr = sect_no;
  addr<<=12;

  IS25LP256_WriteEnable();
  data[0] = CMD_SER;
  data[1] = (addr>>16) & 0xff;
  data[2] = (addr>>8) & 0xff;
  data[3] = addr & 0xff;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
 
  // 처리 대기
  while(IS25LP256_IsBusy() & flgwait) {
    delay(10);
  }
  return true;
}

//
// 64KB 블록 단위 지우기(64kb 공간 단위로 데이터 지우기)
// blk_no(in) 블록 번호(0 - 255)
// flgwait(in) true: 처리 대기 false: 대기 없음
// 반환값: true:정상 종료 false:실패
// 보충: 데이터시트에는 지우기에 보통 150ms, 최대 1000ms가 걸린다고 명시되어 있다.
// 주소 24비트 중 상위 8비트가 블록 번호에 해당한다.
// 하위 16비트는 블록 내 주소가 된다.
//
bool IS25LP256_erase64Block(uint16_t blk_no, bool flgwait) {
  unsigned char data[4];
  int rc;
  UNUSED(rc);
  uint32_t addr = blk_no;
  addr<<=16;

  // 쓰기 권한 설정
  IS25LP256_WriteEnable();

  data[0] = CMD_BER64;
  data[1] = (addr>>16) & 0xff;
  data[2] = (addr>>8) & 0xff;
  data[3] = addr & 0xff;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
 
  // 처리 대기
  while(IS25LP256_IsBusy() & flgwait) {
    delay(50);
  }
  return true;
}

//
// 32KB 블록 단위 지우기(32kb 공간 단위로 데이터 지우기)
// blk_no(in) 블록 번호(0 - 511)
// flgwait(in) true:처리 대기 false:대기 없음
// 반환값: true:정상 종료 false:실패
// 추가: 데이터시트에는 지우기에 보통 120ms, 최대 800ms가 걸린다고 명시되어 있다.
// 주소 24비트 중 상위 9비트가 블록 번호에 해당한다.
// 하위 15 비트는 블록 내 주소가 된다.
//
bool IS25LP256_erase32Block(uint16_t blk_no, bool flgwait) {
  unsigned char data[4];
  int rc;
  UNUSED(rc);
  uint32_t addr = blk_no;
  addr<<=15;

  // 쓰기 권한 설정
  IS25LP256_WriteEnable();  

  data[0] = CMD_BER32;
  data[1] = (addr>>16) & 0xff;
  data[2] = (addr>>8) & 0xff;
  data[3] = addr & 0xff;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
 
  // 처리 대기
  while(IS25LP256_IsBusy() & flgwait) {
    delay(50);
  }
  return true;
}

//
// 전체 영역 지우기
// flgwait(in) true:처리 대기 false:대기 없음
// 반환값: true:정상 종료 false:실패
// 추가: 데이터시트에는 지우는데 보통 15s, 최대 30s가 걸린다고 명시되어 있다.
//
bool IS25LP256_eraseAll(bool flgwait) {
  unsigned char data[1];
  int rc;
  UNUSED(rc);

  // 쓰기 권한 설정
  IS25LP256_WriteEnable();  

  data[0] = CMD_CER;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));

  // 처리 대기
  while(IS25LP256_IsBusy() & flgwait) {
    delay(500);
  }
  return true;
}

//
// 데이터 쓰기
// sect_no(in) : 섹터 번호(0x000 - 0xFFF) 
// inaddr(in) : 섹터 내 주소(0x000 - 0xFFF)
// data(in) : 쓰기 데이터 저장 주소
// n(in) : 쓰기 바이트 수(0~256)
//
uint16_t IS25LP256_pageWrite(uint16_t sect_no, uint16_t inaddr, uint8_t* buf, uint16_t n) {
  if (n > 256) return 0;
  unsigned char *data;
  int rc;

  uint32_t addr = sect_no;
  addr<<=12;
  addr += inaddr;

  // 쓰기 권한 설정
  IS25LP256_WriteEnable();  
  if (IS25LP256_IsBusy()) return 0;  

  data = (unsigned char*)malloc(n+4);
  data[0] = CMD_PP;
  data[1] = (addr>>16) & 0xff;
  data[2] = (addr>>8) & 0xff;
  data[3] = addr & 0xFF;
  memcpy(&data[4],buf,n);
  rc = wiringPiSPIDataRW (_spich,data,n+4);
  //spcDump("pageWrite",rc,buf,n);

  // 처리 대기
  while(IS25LP256_IsBusy()) ;
  free(data);
  return rc;
}
