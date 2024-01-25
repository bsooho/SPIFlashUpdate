//#include <arduino.h>
//#include <SPI.h>

// フラッシュメモリ IS25LP256の利用開始
void IS25LP256_begin(uint8_t cs);
// ステータスレジスタ1の値取得
uint8_t IS25LP256_readStatusReg1(void);
// ステータスレジスタ2の値取得
uint8_t IS25LP256_readStatusReg2();
// JEDEC ID(Manufacture, Memory Type,Capacity)の取得
void IS25LP256_readManufacturer(uint8_t* d);
// Unique IDの取得
void IS25LP256_readUniqieID(uint8_t* d);
// 書込み等の処理中チェック
bool IS25LP256_IsBusy(void);
// パワーダウン指定 
void IS25LP256_powerDown(void);
// 書込み許可設定
void IS25LP256_WriteEnable(void);
// 書込み禁止設定
void IS25LP256_WriteDisable(void);
// データの読み込み
uint16_t IS25LP256_read(uint32_t addr,uint8_t *buf,uint16_t n);
// 高速データの読み込み
uint16_t IS25LP256_fastread(uint32_t addr,uint8_t *buf,uint16_t n);
// セクタ単位消去
bool  IS25LP256_eraseSector(uint16_t sect_no, bool flgwait);
// 64KBブロック単位消去
bool  IS25LP256_erase64Block(uint16_t blk_no, bool flgwait);
// 32KBブロック単位消去
bool  IS25LP256_erase32Block(uint16_t blk_no, bool flgwait);
// 全領域の消去
bool  IS25LP256_eraseAll(bool flgwait);
// データの書き込み
uint16_t IS25LP256_pageWrite(uint16_t sect_no, uint16_t inaddr, uint8_t* data, uint16_t n);

