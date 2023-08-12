# GenericI2CDemo


Setup:
```bash
git clone
git submodule update --init --recursive
make && make flash
```

Connections:
```
+--------+
|  A168  |                  +--------+
|        |                  | USBasp |
|    PB0 +-> LED -> GND     |        |
|    PD0 +------------------+ TX     |
|    PD1 +------------------+ RX     |
|    PB1 +-> BUTTON -> GND  +--------+
+--------+
```