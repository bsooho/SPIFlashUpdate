# Raspberry PI 4 Model B - Flash IS25LP256 through Artix7

Access and write SPI Flash Memory IS25LP256 for Artix7 from RaspberryPi 4B

I ported from here.

https://github.com/nopnop2002/Raspberry-W25Q64.git
---

# Software requirement
- WiringPi Library

 This project uses the wiringPiSPISetup() function to initialize SPI, and use /dev/spidev0.0.

---

# Build
```
git clone https://github.com/bsooho/A7FlashWrite
cd A7FlashWrite
sudo make
```

---

# Wiring  

---

# ISSI Flash memory information

|Device|# of Bytes|Address range|# of 4K-Sectors|# of 32K-Blocks|# of 64K-Blocks|JEDEC ID|
|:---|:---|:---|:---|:---|:---|:---|
|IS25LP256|32M|0x1FFFFFF|8192|1024|512|9D-60-19|

- Normal 80MHz  clock operation   
- Upto 166MHz clock operation  

## IS25LP256   

---
