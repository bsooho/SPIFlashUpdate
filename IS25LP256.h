//#include <arduino.h>
//#include <SPI.h>

// Begin of flash memory operation by specify SPI channel number.
// For example 0, 1, or 2, usually 0 is used.
void IS25LP256_begin(uint8_t cs);

// Read status register
uint8_t IS25LP256_readStatusReg(void);

// Read JEDEC ID(Manufacture, Memory Type,Capacity)
void IS25LP256_readManufacturer(uint8_t* d);

// Read Unique ID of the memory
void IS25LP256_readUniqieID(uint8_t* d);

// Check if memory is uder operation, used during write process
bool IS25LP256_IsBusy(void);

// Set power down 
void IS25LP256_powerDown(void);

// Set write enable
void IS25LP256_WriteEnable(void);

// Set write disable
void IS25LP256_WriteDisable(void);

// Read data
uint16_t IS25LP256_read(uint32_t addr,uint8_t *buf,uint16_t n);

// Fast read data
uint16_t IS25LP256_fastread(uint32_t addr,uint8_t *buf,uint16_t n);

// Erase by sector
bool  IS25LP256_eraseSector(uint16_t sect_no, bool flgwait);

// Erase by 64KB block
bool  IS25LP256_erase64Block(uint16_t blk64_no, bool flgwait);

// Erase by 32KB block
bool  IS25LP256_erase32Block(uint16_t blk32_no, bool flgwait);

// Erase all (Entire of memory to '1')
bool  IS25LP256_eraseAll(bool flgwait);

// Write data
uint16_t IS25LP256_pageWrite(uint16_t sect_no, uint16_t inaddr, uint8_t* data, uint16_t n);
