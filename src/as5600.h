#ifndef AS5600_H
#define AS5600_H

#define AS5600_ADDR 0x36

/* 
0 0 MD ML MH 0 0 0

8 -  magnet too strong (MH)
16 - magnet too weak (ML)
32 - magnet detected (MD)
*/
#define AS5600_STATUS 0x08

/*
12-bit angle with 10-LSB hysteresis at the limit of the 360 degree range.
ANGLE_L : bits 0 - 7
ANGLE_H : bits 8 - 11
*/
#define AS5600_ANGLE_H 0x0C
#define AS5600_ANGLE_L 0x0D

#endif