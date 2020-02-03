/*
 * RF24HALChibios.h
 *
 *  Created on: 26 лют. 2018 р.
 *      Author: Bogdan
 */

#ifndef NRF24_RF24HALCHIBIOS_CONFIG_H_
#define NRF24_RF24HALCHIBIOS_CONFIG_H_

#include <hal.h>

#define RFM7x_USE_SPI SPID1
#define RFM7x_USE_SPI_CNF				\
	SPI_CR1_BR_2 | SPI_CR1_BR_1,		\
	0

#define RFM7x_CSN_PORT IOPORT2
#define RFM7x_CSN_PAD (1)
#define RFM7x_CE_PORT IOPORT2
#define RFM7x_CE_PAD (0)

#define RFM7x_CSN_LOW palClearPad(RFM7x_CSN_PORT, RFM7x_CSN_PAD)
#define RFM7x_CSN_HI  palSetPad(RFM7x_CSN_PORT, RFM7x_CSN_PAD)

#define RFM7x_CE_LOW  palClearPad(RFM7x_CE_PORT, RFM7x_CE_PAD)
#define RFM7x_CE_HI   palSetPad(RFM7x_CE_PORT, RFM7x_CE_PAD)

#endif /* NRF24_RF24HALCHIBIOS_CONFIG_H_ */
