# GenericI2CDemo

Demo application demonstrating the usage of the [GenericI2C](https://github.com/mateuszbugaj/GenericI2C) library with devices like:
- Another AVR microcontroller
- MPU6000 motion sensor
- AS5600 magnetic encoder

Connections:
```
+--------+                              +--------+
| A168   |                  +---------+ | A168   |
| MASTER |                  |USB-UART | | SLAVE  |
|    PB0 +-> LED -> GND     |Converter| |    PB0 +-> LED -> GND
|    PD0 +------------------+ TX      | |        |
|    PD1 +------------------+ RX      | |        |
|    PB1 +-> BUTTON -> GND  +---------+ |        |
|    PD7 +--SCL(OUT)          SLC(OUT)--+ PD7    |
|    PD5 +--SCL(IN)            SCL(IN)--+ PD5    |
|    PD6 +--SDA(OUT)          SDA(OUT)--+ PD6    |
|    PB7 +--SDA(IN)            SDA(IN)--+ PB7    |
|    PB2 +-> GND (ROLE)                 |        |
|    PC5 +------+                       +--------+
|    PC4 +----+ |
+--------+    | |
              | |
+---------+   | |
| Encoder |   | |
|     CLK +---+ |
|      DT +-----+
+---------+

+----------+
| MPU-6000 |
|      SCL +--SCL
|      SDA +--SDA
+----------+

+--------+
| AS5600 |
|    SCL +--SCL
|    SDA +--SDA
+--------+
```

Setup:

```bash
git clone ...
git submodule update --init --recursive
make && make flash
```
