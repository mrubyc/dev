# PSoC5LP SPI class

## Usage

### Copy the following 4 files and add to project.
 * c_spi.h
 * c_spi.c
 * spi_m2.h
 * spi_m2.c


### Hardware configration.

   1. Use PSoC Creator, place "Communication > SPI >
        SPI Master Full Duplex mode Macro" device.
   2. Open a configure dialog.
   3. Make sure the name is "SPIM_1".
   4. Set the Mode, Shift Direction, and Bit Rate.
   5. Change to the "Advanced" tab and check as follows.
       *	Enable Tx Internal Interrupt
       *	Interrupt On Byte/Word Transfer Complete
       * 	Enable Rx Internal Interrupt
       *	Interrupt On Rx FIFO Not Empty

   6. Close this dialog.
   7. Place "Ports and Pins > Digital Output Pin"
   8. Connect to SPIM_1's ss pin.
   9. Add following lines to the auto-generated "cyapicallbacks.h".
```
#define SPIM_1_TX_ISR_ENTRY_CALLBACK
void SPIM_1_TX_ISR_EntryCallback(void);
#define SPIM_1_RX_ISR_ENTRY_CALLBACK
void SPIM_1_RX_ISR_EntryCallback(void);
```

### More SPI devices?

Place the following SPI device and the Digital Output Pin connected to ss, and add the interrupt setting to “cyapicallbacks.h”.

Define pre-processor macro MRBC_NUM_SPI=n. (n=1..3)


### C program (main.c)

```
#include "c_spi.h"
mrbc_init_class_spi(0);
```


### mruby program

```
# create object
spi = SPI.new()	# first device
spi = SPI.new(1)	# first device
spi = SPI.new(2)	# second device

# transfer (normally used)
#  sending 0xf2 and then send 0x00 * 6bytes.
#  receive 7 bytes but returns the last 6 bytes.
ret = spi.transfer( [0xf2], 6 )

# only write
#  send 2 bytes of 0x2c and 0x0a.
spi.write( 0x2c, 0x0a )

# only read
#  sending 0x00 * 2 bytes, then receive 2 bytes and return.
ret = spi.read( 2 )
```
