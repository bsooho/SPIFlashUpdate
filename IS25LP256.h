//#include <arduino.h>
//#include <SPI.h>

// 플래시 메모리 IS25LP256 사용 시작
void IS25LP256_begin(uint8_t cs);

// 상태 레지스터 1의 값 가져오기
uint8_t IS25LP256_readStatusReg1(void);

// 상태 레지스터 2의 값 가져오기
uint8_t IS25LP256_readStatusReg2();

// JEDEC ID(Manufacture, Memory Type,Capacity) 취득
void IS25LP256_readManufacturer(uint8_t* d);

// Unique ID 취득
void IS25LP256_readUniqieID(uint8_t* d);

// 쓰기 등 처리 중 확인
bool IS25LP256_IsBusy(void);

// 파워 다운 지정 
void IS25LP256_powerDown(void);

// 쓰기 권한 설정
void IS25LP256_WriteEnable(void);

// 쓰기 금지 설정
void IS25LP256_WriteDisable(void);

// 데이터 읽기
uint16_t IS25LP256_read(uint32_t addr,uint8_t *buf,uint16_t n);

// 고속 데이터 읽기
uint16_t IS25LP256_fastread(uint32_t addr,uint8_t *buf,uint16_t n);

// 섹터 단위 지우기
bool  IS25LP256_eraseSector(uint16_t sect_no, bool flgwait);

// 64KB 블록 단위 지우기
bool  IS25LP256_erase64Block(uint16_t blk_no, bool flgwait);

// 32KB 블록 단위 지우기
bool  IS25LP256_erase32Block(uint16_t blk_no, bool flgwait);

// 전체 영역 지우기
bool  IS25LP256_eraseAll(bool flgwait);

// 데이터 쓰기
uint16_t IS25LP256_pageWrite(uint16_t sect_no, uint16_t inaddr, uint8_t* data, uint16_t n);
