#ifndef __FRAME_H__
#define __FRAME_H__

#include <stdint.h>

#define SPI_BUF_SIZE 200

#define TRUE 	1
#define FALSE 	0

typedef struct spi_fc_type
{
	uint8_t frameType: 	3;
	uint8_t cmdType: 	2;
	uint8_t cmdSubType: 3;
} spi_fc_type;

typedef struct spi_rfc
{
	uint8_t queue_size;
} spi_rfc;

typedef struct sub1ghz_spi_frame
{
	uint8_t 	sync1;
	uint8_t 	sync2;
	spi_fc_type fc;
	spi_rfc		rfc;
	uint8_t		tx_seq;
	uint8_t 	rx_seq;
	uint8_t 	length;
	char		payload[SPI_BUF_SIZE];
} sub1ghz_spi_frame;

typedef struct rfParams
{
	uint8_t data_rate;
	uint8_t center_freq;
	uint8_t tx_power;
} rfParams;

typedef enum { rf10dBm, rf11dBm, rf12dBm, rf13dBm, rf14dBm, rf_tx_power_cnt } rf_tx_power_t;
typedef enum { rf917MHz, rf918MHz, rf919MHz, rf920MHz, rf921MHz, rf922MHz, rf923MHz, rf_center_freq_cnt } rf_center_freq_t;
typedef enum { rf50kbps, rf200kbps, rf300kbps, rf400kbps, rf500kbps, rf_data_rate_cnt } rf_data_rate_t;

#endif
