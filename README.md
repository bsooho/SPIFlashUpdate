# Raspberry PI CM4 - Write FPGA ROM file to Flash memeory of IS25LP256 through Artix7

Access and write SPI Flash Memory IS25LP256 for Artix7 from RaspberryPi 4B and/or CM4

Based on the below code.
https://github.com/nopnop2002/Raspberry-W25Q64.git
---

# Software requirement
- WiringPi Library
  ```
  git clone https://github.com/WiringPi/WiringPi
  cd WiringPi
  ./build
  ```
- libgpiod Library
  ```
  sudo apt-get install libgpiod2
  sudo apt-get install libgpiod-dev
  dpkg -l libgpiod2
  ```

 This project uses the wiringPiSPISetup() function to initialize SPI, and use /dev/spidev0.0.
 GPIO is handeled by gpiod

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
