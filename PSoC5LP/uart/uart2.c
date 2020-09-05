/*! @file
  @brief
  UART wrapper for PSoC5LP. Multi component version.

  @version 1.2
  @date 2020/09/05 16:20:25

  <pre>
  Copyright (C) 2016-2020 Shimane IT Open-Innovation Center.
  All Rights Reserved.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/


/***** System headers *******************************************************/
#include <project.h>

/***** Local headers ********************************************************/
#include "uart2.h"

/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
int uart_check_timeout(void);
void uart_stop_timeout(void);


/***** Global variables *****************************************************/
/***** Local variables ******************************************************/

/***** Interrupt functions **************************************************/

//================================================================
/*! Tx interrupt handler.

  @internal
  @param  uh            Pointer of UART_HANDLE.
  @note
    Don't use this directry. Use UART_ISR macro.
*/
void uart_isr_tx(UART_HANDLE *uh)
{
  // clear Tx status register and check simply.
  if( !(uh->ReadTxStatus() & uh->TX_STS_FIFO_EMPTY) ) return;

  uint16_t n = uh->size_txbuf - uh->tx_rd;
  if( n > 4 ) n = 4;	// 4 = Hardware FIFO size for PSoC5LP UART module

  for( ; n > 0; n-- ) {
    uh->WriteTxData( uh->p_txbuf[uh->tx_rd++] );
  }

  if( uh->tx_rd >= uh->size_txbuf ) uh->flag_tx_finished = 1;
}


//================================================================
/*! Rx interrupt handler.

  @internal
  @param  uh            Pointer of UART_HANDLE.
  @note
    Don't use this directry. Use UART_ISR macro.
*/
void uart_isr_rx(UART_HANDLE *uh)
{
  int sts = uh->ReadRxStatus();

  for(; sts != 0; sts = uh->ReadRxStatus()) {
    if( sts & uh->RX_STS_FIFO_NOTEMPTY ) {
      uh->rxfifo[uh->rx_wr++] = uh->ReadRxData();

      // check rollover write index.
      if( uh->rx_wr < sizeof(uh->rxfifo)) {
        if( uh->rx_wr == uh->rx_rd ) {
          --uh->rx_wr;   // buffer full
	  uh->rx_overflow = 1;
        }
      }
      else {
        if( uh->rx_rd == 0 ) {
          --uh->rx_wr;   // buffer full
	  uh->rx_overflow = 1;
        }
        else {
          uh->rx_wr = 0; // roll over.
        }
      }
    }

    // and any more check other status?
  }
}


/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! initialize

  @internal
  @param  uh            Pointer of UART_HANDLE.
  @note
    Don't use this directry. Use uart_init macro.
*/
void uart_init_m(UART_HANDLE *uh,
                 uint8_t      tx_sts_fifo_empty,
                 uint8_t      rx_sts_fifo_notempty,
                 void        *Start,
                 void        *Stop,
                 void        *ClearTxBuffer,
                 void        *ClearRxBuffer,
                 void        *ReadTxStatus,
                 void        *ReadRxStatus,
                 void        *WriteTxData,
                 void        *ReadRxData)
{
  *uh = (UART_HANDLE){
    .p_txbuf          = 0,
    .size_txbuf       = 0,
    .tx_rd            = 0,
    .flag_tx_finished = 1,
    .mode             = 0,
    .rx_overflow      = 0,
    .delimiter        = '\n',
    .rx_rd            = 0,
    .rx_wr            = 0,

    .TX_STS_FIFO_EMPTY    = tx_sts_fifo_empty,
    .RX_STS_FIFO_NOTEMPTY = rx_sts_fifo_notempty,
    .Start                = Start,
    .Stop                 = Stop,
    .ClearTxBuffer        = ClearTxBuffer,
    .ClearRxBuffer        = ClearRxBuffer,
    .ReadRxStatus         = ReadRxStatus,
    .ReadTxStatus         = ReadTxStatus,
    .WriteTxData          = WriteTxData,
    .ReadRxData           = ReadRxData,
  };

  uh->Start();
  if( uh->ClearTxBuffer ) uh->ClearTxBuffer();
  if( uh->ClearRxBuffer ) uh->ClearRxBuffer();
}


//================================================================
/*! Clear transmit buffer.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
*/
void uart_clear_tx_buffer(UART_HANDLE *uh)
{
  uh->ClearTxBuffer();
}


//================================================================
/*! Clear receive buffer.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
*/
void uart_clear_rx_buffer(UART_HANDLE *uh)
{
  uh->ClearRxBuffer();

  uint8 interrupts = CyEnterCriticalSection();
  uh->rx_rd = 0;
  uh->rx_wr = 0;
  uh->rx_overflow = 0;
  CyExitCriticalSection( interrupts );
}


//================================================================
/*! Send out binary data.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @param  buffer        Pointer of buffer.
  @param  size          Size of buffer.
  @return               Size of transmitted.
*/
int uart_write(UART_HANDLE *uh, const void *buffer, size_t size)
{
  if( !uh->flag_tx_finished ) return 0;  // TODO: or -1 ??
  if( size == 0 ) return 0;

  uh->p_txbuf          = buffer;
  uh->size_txbuf       = size;
  uh->tx_rd            = 1;
  uh->flag_tx_finished = 0;

  uh->WriteTxData( *(uint8_t *)buffer );	// send first byte.
  if( uh->mode & UART_WRITE_NONBLOCK ) return 0;

  do {
    CyPmAltAct(PM_ALT_ACT_TIME_NONE, PM_ALT_ACT_SRC_PICU);
#ifdef UART_CHECK_TIMEOUT
    if( uart_check_timeout()) {
      uart_stop_timeout();
      uh->flag_tx_finished = 1;
      return -1;
    }
#endif
  } while( !uh->flag_tx_finished );

#ifdef UART_CHECK_TIMEOUT
  uart_stop_timeout();
#endif
  return uh->tx_rd;
}


//================================================================
/*! Receive binary data.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @param  buffer        Pointer of buffer.
  @param  size          Size of buffer.
  @return int           Num of received bytes.

  @note                 If no data received, it blocks execution.
*/
int uart_read(UART_HANDLE *uh, void *buffer, size_t size)
{
  // wait for data.
  while( !uart_is_readable(uh) ) {
    CyPmAltAct(PM_ALT_ACT_TIME_NONE, PM_ALT_ACT_SRC_PICU);
#ifdef UART_CHECK_TIMEOUT
    if( uart_check_timeout()) {
      uart_stop_timeout();
      return -1;
    }
#endif
  }

  // copy fifo to buffer
  uint8_t *buf = buffer;
  size_t   cnt = size;
  uint16_t rx_rd;

  do {
    rx_rd = uh->rx_rd;
    *buf++ = uh->rxfifo[rx_rd++];
    if( rx_rd >= sizeof(uh->rxfifo) ) rx_rd = 0;
    uh->rx_rd = rx_rd;
  } while( --cnt != 0 && rx_rd != uh->rx_wr );

#ifdef UART_CHECK_TIMEOUT
  uart_stop_timeout();
#endif
  return size - cnt;
}


//================================================================
/*! Receive string.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @param  buf           Pointer of buffer.
  @param  size          Size of buffer.
  @return int           Num of received bytes.

  @note                 If no data received, it blocks execution.
*/
int uart_gets(UART_HANDLE *uh, char *buf, size_t size)
{
  size_t cnt = size - 1;

  while( 1 ) {
    // wait for data.
    if( !uart_is_readable(uh) ) {
      CyPmAltAct(PM_ALT_ACT_TIME_NONE, PM_ALT_ACT_SRC_PICU);
#ifdef UART_CHECK_TIMEOUT
      if( uart_check_timeout()) {
        uart_stop_timeout();
        *buf = '\0';
        return -1;
      }
#endif
      continue;
    }

    // copy fifo to buffer
    uint16_t rx_rd = uh->rx_rd;
    int ch = (*buf++ = uh->rxfifo[rx_rd++]);
    if( rx_rd >= sizeof(uh->rxfifo)) rx_rd = 0;
    uh->rx_rd = rx_rd;

    if( --cnt == 0 ) break;
    if( ch == uh->delimiter ) break;
  }
  *buf = '\0';

#ifdef UART_CHECK_TIMEOUT
  uart_stop_timeout();
#endif
  return size - cnt - 1;
}


//================================================================
/*! Receive binary block data.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @param  buffer        Pointer of buffer.
  @param  size          Size of buffer.
  @return int           Num of received bytes.
*/
int uart_read_block(UART_HANDLE *uh, void *buffer, size_t size)
{
  uint8 *buf = buffer;
  size_t cnt = size;

  while( cnt > 0 ) {
    int n = uart_read(uh, buf, cnt);
    if( n < 0 ) return n;
    cnt -= n;
    buf += n;
  }

  return size;
}


//================================================================
/*! Receive binary data. (non block)

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @param  buffer        Pointer of buffer.
  @param  size          Size of buffer.
  @return int           Num of received bytes.
*/
int uart_read_nonblock(UART_HANDLE *uh, void *buffer, size_t size)
{
  int n = uart_bytes_available(uh);
  if( n > size ) n = size;
  if( n != 0 ) uart_read(uh, buffer, n);

  return n;
}


//================================================================
/*! check data length can be read.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @return int           result (bytes)
*/
int uart_bytes_available(UART_HANDLE *uh)
{
  uint16_t rx_wr = uh->rx_wr;

  if( uh->rx_rd <= rx_wr ) {
    return rx_wr - uh->rx_rd;
  }
  else {
    return sizeof(uh->rxfifo) - uh->rx_rd + rx_wr;
  }
}


//================================================================
/*! check data can be read a line.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @return int           string length.
  @note
   If RX-FIFO buffer is full, return -1.
*/
int uart_can_read_line(UART_HANDLE *uh)
{
  uint16_t idx   = uh->rx_rd;
  uint16_t rx_wr = uh->rx_wr;

  if( uh->rx_overflow ) return -1;

  while( idx != rx_wr ) {
    if( uh->rxfifo[idx++] == uh->delimiter ) {
      if( uh->rx_rd < idx ) {
	return idx - uh->rx_rd;
      } else {
	return sizeof(uh->rxfifo) - uh->rx_rd + idx;
      }
    }
    if( idx >= sizeof(uh->rxfifo)) idx = 0;
  }

  return 0;
}
