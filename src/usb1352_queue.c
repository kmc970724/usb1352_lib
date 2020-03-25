#include "usb1352.h"

void usb1352_queue_init(usb1352_queue* queue)
{
	queue->buffer = (sub1ghz_spi_frame*)malloc(sizeof(sub1ghz_spi_frame) * MAX_QUEUE_SIZE);

	queue->front = 0;
	queue->end = 0;
	queue->size = 0;
}

uint8_t usb1352_queue_empty(usb1352_queue* queue)
{
	if (queue->size == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

uint8_t usb1352_queue_full(usb1352_queue* queue)
{
	if (queue->size == MAX_QUEUE_SIZE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void usb1352_queue_insert(usb1352_queue* queue, sub1ghz_spi_frame* data)
{
	memcpy(&queue->buffer[queue->end], data, sizeof(sub1ghz_spi_frame));
	queue->size = (queue->size + 1) % MAX_QUEUE_SIZE;
	queue->end = (queue->end + 1) % MAX_QUEUE_SIZE;
}

void usb1352_queue_remove(usb1352_queue* queue)
{
	queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
	queue->size -= 1;
}
