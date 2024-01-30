# Raspberry PI 4 Model B - IS25LP256

SPI Flash Memory IS25LP256 Access Library for RaspberryPi 4B

I ported from here.

https://github.com/nopnop2002/Raspberry-W25Q64.git
---

# Software requirement
- WiringPi Library

 This project uses the wiringPiSPISetup() function to initialize SPI, and use /dev/spidev0.0.

---

# Build
```
git clone https://github.com/bsooho/RPi-IS25LP256
cd RPi-IS25LP256
sudo make
```

---

# API

// Start Flash  
void IS25LP256_begin(uint8_t spich);  

// Get status register  
uint8_t IS25LP256_readStatusReg(void);  

// Get JEDEC ID(Manufacture, Memory Type,Capacity)  
void IS25LP256_readManufacturer(uint8_t* jedc);  

// Get Unique ID(Winbond only)  
void IS25LP256_readUniqieID(uint8_t* id);  

// Check busy  
bool IS25LP256_IsBusy(void);  

// Set power down mode  
void IS25LP256_powerDown(void);  

// Set write enable  
void IS25LP256_WriteEnable(void);  

// Set write disable  
void IS25LP256_WriteDisable(void);  

// Read data from memory  
uint16_t IS25LP256_read(uint32_t addr,uint8_t *buf,uint16_t n);

// First read data from memory  
uint16_t IS25LP256_fastread(uint32_t addr,uint8_t *buf,uint16_t n);  

// Erase data by Sector  
bool IS25LP256_eraseSector(uint16_t sect_no, bool flgwait);  

// Erase data by block(64KB)  
bool IS25LP256_erase64Block(uint16_t blk64_no, bool flgwait);  

// Erase data by Block(32KB)  
bool IS25LP256_erase32Block(uint16_t blk32_no, bool flgwait);  

// Erase all data  
bool IS25LP256_eraseAll(bool flgwait);  

// Write data to memory  
uint16_t IS25LP256_pageWrite(uint16_t sect_no, uint16_t inaddr, uint8_t* data, uint16_t n);  

---

# Wireing  

---

# ISSI

|Device|# of Bytes|Address range|# of 4K-Sectors|# of 32K-Blocks|# of 64K-Blocks|JEDEC ID|
|:---|:---|:---|:---|:---|:---|:---|
|IS25LP256|32M|0x1FFFFFF|8192|1024|512|9D-60-19|

- Normal 80MHz  clock operation   
- Upto 166MHz clock operation  

## IS25LP256   
![IS25LP256](https://user-images.githubusercontent.com/6020549/81263674-0fe0f680-907b-11ea-83dc-f806963e34ae.jpg)

---
