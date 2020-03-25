#ifndef __USB1352_CORE_H__
#define __USB1352_CORE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "include/ft4222/ftd2xx.h"
#include "include/ft4222/libft4222.h"
#include "include/ft4222/WinTypes.h"

#include "include/cc1352/frame.h"
#include "include/cc1352/protocol.h"

#define SLAVE_SELECT(x) (1 << (x))

typedef struct usb1352_queue
{
	sub1ghz_spi_frame* buffer;
	uint8_t front;
	uint8_t end;
	uint8_t size;
} usb1352_queue;

typedef struct usb1352_dev
{
	FT_HANDLE ft_handle;
	DWORD loc_id;

	int count;

	usb1352_queue* usb1352_tx_queue;
	usb1352_queue* usb1352_rx_queue;

	pthread_mutex_t usb1352_spi_mutex;
	pthread_mutex_t usb1352_queue_mutex;

	pthread_cond_t usb1352_rx_cond;
	pthread_mutex_t usb1352_rx_mutex;
} usb1352_dev;

// QUEUE

#define MAX_QUEUE_SIZE 512

void usb1352_queue_init(usb1352_queue* queue);
uint8_t usb1352_queue_empty(usb1352_queue* queue);
uint8_t usb1352_queue_full(usb1352_queue* queue);
void usb1352_queue_insert(usb1352_queue* queue, sub1ghz_spi_frame* data);
void usb1352_queue_remove(usb1352_queue* queue);

// SPI

void usb1352_spi_init(usb1352_dev* p_dev);
uint8_t usb1352_spi_rw(usb1352_dev* p_dev, sub1ghz_spi_frame* tx_frame, sub1ghz_spi_frame* rx_frame);
void usb1352_spi_data_transfer(usb1352_dev* p_dev, uint8_t size, void* payload);
void usb1352_spi_data_receive(usb1352_dev* p_dev, uint8_t size, void* payload);

// CORE

usb1352_dev* usb1352_init(void);

#endif
