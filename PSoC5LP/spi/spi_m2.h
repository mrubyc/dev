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
#ifndef	PSOC5_SPIMWRAP_H_
#define	PSOC5_SPIMWRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/***** System headers *******************************************************/
#include <stdint.h>


/***** Local headers ********************************************************/
/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
//! Convenience macro to define the interrupt handler.
#define SPI_ISR(spih, NAME)			\
  void NAME ## _TX_ISR_EntryCallback(void) {	\
    spi_tx_isr(spih);				\
  }						\
  void NAME ## _RX_ISR_EntryCallback(void) {	\
    spi_rx_isr(spih);				\
  }

//! Initializer macro for SPI Master
#define spi_init(spih, NAME)			\
  spi_init_m( spih,				\
	      NAME ## _STS_SPI_IDLE,		\
	      NAME ## _FIFO_SIZE,		\
	      NAME ## _Start,			\
	      NAME ## _EnableTxInt,		\
	      NAME ## _EnableRxInt,		\
	      NAME ## _DisableTxInt,		\
	      NAME ## _DisableRxInt,		\
	      NAME ## _ReadTxStatus,		\
	      NAME ## _WriteTxData,		\
	      NAME ## _ReadRxData,		\
	      NAME ## _GetRxBufferSize,		\
	      NAME ## _ClearFIFO)


/***** Typedefs *************************************************************/
//================================================================
/*! SPI handle.
*/
typedef struct SPI_HANDLE {
  uint8_t *send_data;
  int send_size;
  int send_total;
  int send_n;

  uint8_t *recv_data;
  int recv_size;
  int recv_n;

  // constant table
  uint8_t STS_SPI_IDLE;
  uint8_t FIFO_SIZE;

  // function table
  void (*Start)(void);
//void (*Stop)(void);
  void (*EnableTxInt)(void);
  void (*EnableRxInt)(void);
  void (*DisableTxInt)(void);
  void (*DisableRxInt)(void);
  uint8_t (*ReadTxStatus)(void);
//uint8_t (*ReadRxStatus)(void);
  void (*WriteTxData)(uint8_t);
  uint8_t (*ReadRxData)(void);
  uint8_t (*GetRxBufferSize)(void);
//uint8_t (*GetTxBufferSize)(void);
  void (*ClearFIFO)(void);

} SPI_HANDLE;


/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
void spi_tx_isr(SPI_HANDLE *spih);
void spi_rx_isr(SPI_HANDLE *spih);
void spi_init_m(SPI_HANDLE *spih,
		uint8_t sts_spi_idle,
		uint8_t fifo_size,
		void *Start,
		void *EnableTxInt,
		void *EnableRxInt,
		void *DisableTxInt,
		void *DisableRxInt,
		void *ReadTxStatus,
		void *WriteTxData,
		void *ReadRxData,
		void *GetRxBufferSize,
		void *ClearFIFO);
void spi_transfer(SPI_HANDLE *spih,
		  void *send_buf,
		  int send_size,
		  void *recv_buf,
		  int recv_size,
		  int flag_include);

/***** Inline functions *****************************************************/
//================================================================
/*! Wait for SPI transfer to done.

  @param  spih		pointer to SPI_HANDLE
*/
static inline void spi_wait_done(const SPI_HANDLE *spih)
{
  while( !(spih->ReadTxStatus() & spih->STS_SPI_IDLE))
    ;
}


//================================================================
/*! Is an SPI transfer in progress?

  @param  spih		pointer to SPI_HANDLE
  @return int	true or false
*/
static inline int spi_is_transfer(const SPI_HANDLE *spih)
{
  return !(spih->ReadTxStatus() & spih->STS_SPI_IDLE);
}


#ifdef __cplusplus
}
#endif
#endif
