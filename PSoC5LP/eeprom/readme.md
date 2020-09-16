# PSoC5LP EEPROM class

 * This is EEPROM read/write class for mruby/c
 * Only PSoC5LP internal EEPROM device are supported.
 * Only string read/write available in this version.


## Usage
 1. Launch PSoC Creator.
 2. Place 'EEPROM' device.
 3. Make sure name is 'EEPROM_1'.
 4. Copy c_eeprom.h and c_eeprom.c files to project folder.
 5. Add this (c_eeprom.c) file to PSoC Creator.
 6. Add below to main.c.
```
    #include "c_eeprom.h"
    mrbc_init_class_eeprom(0);	// needs to be after mrbc_init()
```


## mruby program

```
# write to device
EEPROM.write( address, "DATA" )

# read from device
s = EEPROM.read( address, byte_length )
```
