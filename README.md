# Update Flash memory of FPGA by Raspberry PI CM4

Access, erase and write ROM file for flash Memory IS25LP256 from RaspberryPi CM4.
Updated ROM will be effected on next power on.
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

# Build & Run
```
git clone https://github.com/bsooho/SPIFlashUpdate
cd SPIFlashUpdate
sudo make
sudo ./main
```

---

# ISSI Flash memory information

|Device|# of Bytes|Address range|# of 4K-Sectors|# of 32K-Blocks|# of 64K-Blocks|JEDEC ID|
|:---|:---|:---|:---|:---|:---|:---|
|IS25LP256|32M|0x1FFFFFF|8192|1024|512|9D-60-19|

- Normal 80MHz  clock operation   
- Upto 166MHz clock operation

---
