/*! @file
  @brief
  SPI master convenience library for PSoC5LP. Multi component version.

  @version 1.0
  @date 2019/09/12 13:45:40
  @note This version supports up to 4 interfaces.

<pre>
  Copyright (C) 2019 Shimane IT Open-Innovation Center.
  All Rights Reserved.

  This file is distributed under BSD 3-Clause License.
</pre>
*/


/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdint.h>

/***** Local headers ********************************************************/
#include "spi_m2.h"

/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/

//================================================================
/*! Intterrupt callback on byte transfer complete.
*/
void spi_tx_isr(SPI_HANDLE *spih)
{
  if( spih->send_n < spih->send_size ) {
    spih->WriteTxData( *spih->send_data++ );
    ++spih->send_n;
    return;
  }

  if( spih->send_n < spih->send_total ) {
    spih->WriteTxData( 0 );
    ++spih->send_n;
  }
}


//================================================================
/*! Intterrupt callback on Rx FIFO not empty.
*/
void spi_rx_isr(SPI_HANDLE *spih)
{
  do {
    int data = spih->ReadRxData();

    if( spih->recv_n < spih->recv_size &&
	spih->recv_n++ >= 0 ) {
      *spih->recv_data++ = data;
    }
  } while( spih->GetRxBufferSize() != 0 );
}


/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! initialize
  @internal
  @param  spih		pointer to SPI_HANDLE
  @note
    Don't use this directry. Use spi_init macro.
*/
void spi_init_m(SPI_HANDLE *spih,
		uint8_t sts_spi_idle,
		uint8_t fifo_size,
		void *Start,
//		void *Stop,
		void *EnableTxInt,
		void *EnableRxInt,
		void *DisableTxInt,
		void *DisableRxInt,
		void *ReadTxStatus,
//		void *ReadRxStatus,
		void *WriteTxData,
		void *ReadRxData,
		void *GetRxBufferSize,
//		void *GetTxBufferSize,
		void *ClearFIFO)
{
  spih->STS_SPI_IDLE = sts_spi_idle;
  spih->FIFO_SIZE = fifo_size;
  spih->Start = Start;
//spih->Stop = Stop;
  spih->EnableTxInt = EnableTxInt;
  spih->EnableRxInt = EnableRxInt;
  spih->DisableTxInt = DisableTxInt;
  spih->DisableRxInt = DisableRxInt;
  spih->ReadTxStatus = ReadTxStatus;
//spih->ReadRxStatus = ReadRxStatus;
  spih->WriteTxData = WriteTxData;
  spih->ReadRxData = ReadRxData;
  spih->GetRxBufferSize = GetRxBufferSize;
//spih->GetTxBufferSize = GetTxBufferSize;
  spih->ClearFIFO = ClearFIFO;

  spih->Start();
}



//================================================================
/*! Perform SPI data transfer. (send and receive)

  @param  spih		pointer to SPI_HANDLE
  @param  send_buf	pointer to send data buffer. or NULL.
  @param  send_size	send data size (bytes).
  @param  recv_buf	pointer to receive data buffer. or NULL.
  @param  recv_size	receive data size (bytes).
  @param  flag_include	if this flag true, including receive data when sending data
*/
void spi_transfer(SPI_HANDLE *spih, void *send_buf, int send_size,
		  void *recv_buf, int recv_size, int flag_include)
{
  spi_wait_done(spih);

  spih->DisableTxInt();
  spih->DisableRxInt();
  spih->ClearFIFO();

  spih->send_data = send_buf;
  spih->send_size = send_size;
  if( flag_include ) {
    spih->send_total = send_size > recv_size ? send_size : recv_size;
  } else {
    spih->send_total = send_size + recv_size;
  }
  spih->send_n = 0;

  spih->recv_data = recv_buf;
  spih->recv_size = recv_buf ? recv_size : 0;
  spih->recv_n = flag_include ? 0 : -send_size;

  // send SPI_n_FIFO_SIZE (maybe 4) byte continuously.
  while( spih->send_n < spih->send_size ) {
    spih->WriteTxData( *spih->send_data++ );
    if( ++spih->send_n >= spih->FIFO_SIZE ) goto DONE;
  }
  while( spih->send_n < spih->send_total ) {
    spih->WriteTxData( 0 );
    if( ++spih->send_n >= spih->FIFO_SIZE ) goto DONE;
  }

 DONE:
  spih->EnableTxInt();
  spih->EnableRxInt();
}
