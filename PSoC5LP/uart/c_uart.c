/*! @file
  @brief
  UART class for Cypress PSoC5LP

  <pre>
  Copyright (C) 2018-2020 Kyushu Institute of Technology.
  Copyright (C) 2018-2020 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  (Usage)
  Hardware configration.

   1. Use PSoC Creator, place "Communication > UART" device.
   2. Open a configure dialog.
   3. Make sure the name is "UART_1".
   4. Set the baud rate and other parameters.
   5. Change to the "Advanced" tab and check "RX - ON Byte Received" and "TX - On TX Complete".
   6. Close this dialog.
   7. Place two "System > Interrupt" devices.
   8. Connect to tx_interrupt and rx_interrupt of UART.
   9. Change the names to "isr_UART_1_Tx" and "isr_UART_1_Rx" respectively.


  C program (main.c)

    #include "c_uart.h"
    mrbc_init_class_uart(0);

    If necessary, define maximum string length in the UART_SIZE_RXFIFO macro.
    The default is 128 bytes.


  mruby program

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

  </pre>
*/


#include "vm_config.h"
#include <stdint.h>
#include <project.h>	// auto generated by PSoC Creator.

#include "uart2.h"
#include "mrubyc.h"


//================================================================
/*! UART用設定
*/
#if !defined(MRBC_NUM_UART)
# define MRBC_NUM_UART 1
#endif

UART_HANDLE uh[MRBC_NUM_UART];

#if MRBC_NUM_UART >= 1	// use boost? the following are enough in this project.
UART_ISR( &uh[0], UART_1 );
#endif
#if MRBC_NUM_UART >= 2
UART_ISR( &uh[1], UART_2 );
#endif
#if MRBC_NUM_UART >= 3
UART_ISR( &uh[2], UART_3 );
#endif
#if MRBC_NUM_UART >= 4
#error "MRBC_NUM_UART >= 4"
#endif



//================================================================
/*! UART constructor

  $uart = UART.new		# automatic assign
  $uart = UART.new( num )	# 1 origin.
*/
static void c_uart_new(mrbc_vm *vm, mrbc_value v[], int argc)
{
  static int uart_counter = 0;
  int uart_num;

  if( argc == 0 ) {
    uart_num = 0;
  } else if( v[1].tt == MRBC_TT_FIXNUM ) {
    uart_num = v[1].i;
  } else {
    goto ERROR_RETURN;
  }

  if( uart_num == 0 ) {
    if( uart_counter >= MRBC_NUM_UART ) goto ERROR_RETURN;
    uart_num = uart_counter++;
  } else {
    if( --uart_num >= MRBC_NUM_UART ) goto ERROR_RETURN;
  }

  *v = mrbc_instance_new(vm, v->cls, sizeof(UART_HANDLE *));
  *((UART_HANDLE **)v->instance->data) = &uh[uart_num];
  return;

 ERROR_RETURN:
  SET_NIL_RETURN();
}


//================================================================
/*! read

  s = $uart.read(n)

  @param  n		Number of bytes receive.
  @return String	Received data.
  @return Nil		Not enough receive length.
*/
static void c_uart_read(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value ret;
  UART_HANDLE *handle = *(UART_HANDLE **)v->instance->data;
  int need_length = GET_INT_ARG(1);

  if( uart_bytes_available(handle) < need_length ) {
    ret = mrbc_nil_value();
    goto DONE;
  }

  char *buf = mrbc_alloc( vm, need_length + 1 );
  uart_read( handle, buf, need_length );

  ret = mrbc_string_new_alloc( vm, buf, need_length );

 DONE:
  SET_RETURN(ret);
}


//================================================================
/*! write

  $uart.write(s)

  @param  s	  Write data.
*/
static void c_uart_write(mrbc_vm *vm, mrbc_value v[], int argc)
{
  UART_HANDLE *handle = *(UART_HANDLE **)v->instance->data;

  switch( v[1].tt ) {
  case MRBC_TT_STRING: {
    int n = uart_write( handle,
			mrbc_string_cstr(&v[1]), mrbc_string_size(&v[1]) );
    SET_INT_RETURN(n);
  } break;

  default:
    SET_NIL_RETURN();
    break;
  }
}


//================================================================
/*! gets

  s = $uart.gets()
  @return String	Received string.
  @return Nil		Not enough receive length.
*/
static void c_uart_gets(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_value ret;
  UART_HANDLE *handle = *(UART_HANDLE **)v->instance->data;

  if( !uart_can_read_line( handle ) ) {
    ret = mrbc_nil_value();
    goto DONE;
  }

  int len = uart_bytes_available( handle ) + 1;
  char *buf = mrbc_alloc( vm, len );

  len = uart_gets( handle, buf, len );
  mrbc_realloc( vm, buf, len );

  ret = mrbc_string_new_alloc( vm, buf, len );

 DONE:
  SET_RETURN(ret);
}


//================================================================
/*! clear_tx_buffer

  $uart.clear_tx_buffer()
*/
static void c_uart_clear_tx_buffer(mrbc_vm *vm, mrbc_value v[], int argc)
{
  UART_HANDLE *handle = *(UART_HANDLE **)v->instance->data;
  uart_clear_tx_buffer( handle );
}


//================================================================
/*! clear_rx_buffer

  $uart.clear_rx_buffer()
*/
static void c_uart_clear_rx_buffer(mrbc_vm *vm, mrbc_value v[], int argc)
{
  UART_HANDLE *handle = *(UART_HANDLE **)v->instance->data;
  uart_clear_rx_buffer( handle );
}


//================================================================
/*! initialize
*/
void mrbc_init_class_uart(struct VM *vm)
{
  // start physical device
#if MRBC_NUM_UART >= 1
  uart_init( &uh[0], UART_1 );
#endif
#if MRBC_NUM_UART >= 2
  uart_init( &uh[1], UART_2 );
#endif
#if MRBC_NUM_UART >= 3
  uart_init( &uh[2], UART_3 );
#endif

  // define class and methods.
  mrbc_class *uart;
  uart = mrbc_define_class(0, "UART",	mrbc_class_object);
  mrbc_define_method(0, uart, "new",	c_uart_new);
  mrbc_define_method(0, uart, "read",	c_uart_read);
  mrbc_define_method(0, uart, "write",	c_uart_write);
  mrbc_define_method(0, uart, "gets",	c_uart_gets);
  mrbc_define_method(0, uart, "puts",	c_uart_write);
  mrbc_define_method(0, uart, "clear_tx_buffer", c_uart_clear_tx_buffer);
  mrbc_define_method(0, uart, "clear_rx_buffer", c_uart_clear_rx_buffer);
}
