# PSoC5LP UART class

## Usage
### Copy the following 4 files and add to project.
 * c_uart.h
 * c_uart.c
 * uart2.h
 * uart2.c

### Hardware configration.

1. Use PSoC Creator, place "Communication > UART" device.
2. Open a Configure dialog.
3. Make sure the name is "UART_1".
4. Set the baud rate and other parameters.
5. Change to the "Advanced" tab and check "RX - ON Byte Received" and "TX - On TX Complete".
6. Click OK button to close dialog.
7. Place two "System > Interrupt" devices.
8. Connect to tx_interrupt and rx_interrupt of UART.
9. Change the names to "isr_UART_1_Tx" and "isr_UART_1_Rx" respectively.


### C program (main.c)

```
#include "c_uart.h"
mrbc_init_class_uart(0);
```
If necessary, define maximum string length in the UART_SIZE_RXFIFO macro.
The default is 128 bytes.


### mruby program

```
uart = UART.new()

# String read
#  In the current specification, Nil is returned when a string is not received.
s = uart.gets()

# String write
uart.puts("STRING\r\n")  # alias for UART#write()

# Binary read
s = uart.read(n)    # read n bytes.

# Binary write
uart.write("STRING\r\n")

# Flush buffer
uart.clear_tx_buffer()
uart.clear_rx_buffer()
```
