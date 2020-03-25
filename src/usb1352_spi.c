#include "usb1352.h"

void* usb1352_spi_intr_thread(void* dev);
void* usb1352_spi_thread(void* dev);

void usb1352_spi_init(usb1352_dev* p_dev)
{
	sub1ghz_spi_frame tx_frame;
	sub1ghz_spi_frame rx_frame;

	pthread_t intr_thread_t;
	pthread_t spi_thread_t;

	FT_OpenEx((PVOID)(uintptr_t)(p_dev->loc_id), FT_OPEN_BY_LOCATION, &(p_dev->ft_handle));
	FT4222_SPIMaster_Init(p_dev->ft_handle, SPI_IO_SINGLE, CLK_DIV_32, CLK_IDLE_HIGH, CLK_TRAILING, SLAVE_SELECT(0));

	tx_frame.fc.frameType = SPI_DUMMY;
	tx_frame.fc.cmdType = SPI_NONE;
	tx_frame.fc.cmdSubType = SPI_NONE;

	p_dev->count = 0;

	while (1)
	{
		if (!(usb1352_spi_rw(p_dev, &tx_frame, &rx_frame)))
		{
			continue;
		}

		if (rx_frame.sync1 == SPI_SYNC_BYTE1 && rx_frame.sync2 == SPI_SYNC_BYTE2)
		{
			break;
		}
	}
	
	pthread_mutex_init(&p_dev->usb1352_rx_mutex, NULL);
	pthread_cond_init(&p_dev->usb1352_rx_cond, NULL);

	pthread_create(&intr_thread_t, NULL, usb1352_spi_intr_thread, p_dev);
//	pthread_create(&spi_thread_t, NULL, usb1352_spi_thread, p_dev);
}

uint8_t usb1352_spi_rw(usb1352_dev* p_dev, sub1ghz_spi_frame* tx_frame, sub1ghz_spi_frame* rx_frame)
{
	uint16_t bytes_to_transceive;
	uint16_t bytes_transceived;
	FT_STATUS ft_status;

	bytes_to_transceive = sizeof(sub1ghz_spi_frame);

	tx_frame->sync1 = SPI_SYNC_BYTE1;
	tx_frame->sync2 = SPI_SYNC_BYTE2;

	pthread_mutex_lock(&(p_dev->usb1352_spi_mutex));
	ft_status = FT4222_SPIMaster_SingleReadWrite(p_dev->ft_handle, (uint8_t*)rx_frame, (uint8_t*)tx_frame, bytes_to_transceive, &bytes_transceived, TRUE);
	pthread_mutex_unlock(&(p_dev->usb1352_spi_mutex));

	if (bytes_to_transceive == bytes_transceived && ft_status == FT4222_OK)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void usb1352_spi_data_transfer(usb1352_dev* p_dev, uint8_t size, void* payload)
{
	sub1ghz_spi_frame tx_frame, rx_frame;

	tx_frame.fc.frameType = SPI_DATA_TRANSFER_REQ;
	tx_frame.fc.cmdType = SPI_DATA_CMD_COMMON;
	tx_frame.fc.cmdSubType = SPI_DATA_CMD_COMMON_NORMAL;
	tx_frame.length = size;
	memcpy(tx_frame.payload, payload, size);
	
//	usb1352_queue_insert(p_dev->usb1352_tx_queue, &tx_frame);

	while (1)
	{
		if (usb1352_spi_rw(p_dev, &tx_frame, &rx_frame) == TRUE)
		{
			break;
		}
		else
		{
			usleep(1000);
		}
	}

//	if (!(usb1352_spi_rw(p_dev, &tx_frame, &rx_frame)))
//	{
//		return;
//	}

	if (rx_frame.sync1 == SPI_SYNC_BYTE1 && rx_frame.sync2 == SPI_SYNC_BYTE2)
	{
		if (rx_frame.fc.frameType == SPI_INTERRUPT)
		{
			if (rx_frame.fc.cmdType == SPI_INTR_NORMAL)
			{
				if (rx_frame.fc.cmdSubType == SPI_INTR_NORMAL_DATA)
				{
					pthread_mutex_lock(&p_dev->usb1352_queue_mutex);
					usb1352_queue_insert(p_dev->usb1352_rx_queue, &rx_frame);
					pthread_mutex_unlock(&p_dev->usb1352_queue_mutex);
				}
			}
		}

		if (rx_frame.rfc.queue_size < 3)
		{
			while (rx_frame.rfc.queue_size < 3)
			{
				memset(&tx_frame, 0, sizeof(sub1ghz_spi_frame));
				if (!(usb1352_spi_rw(p_dev, &tx_frame, &rx_frame) == TRUE))
				{
					continue;
				}

				if (rx_frame.fc.frameType == SPI_INTERRUPT)
				{
					if (rx_frame.fc.cmdType == SPI_INTR_NORMAL)
					{
						if (rx_frame.fc.cmdSubType == SPI_INTR_NORMAL_DATA)
						{
							pthread_mutex_lock(&p_dev->usb1352_queue_mutex);
							usb1352_queue_insert(p_dev->usb1352_rx_queue, &rx_frame);
							pthread_mutex_unlock(&p_dev->usb1352_queue_mutex);
						}
					}
				}
				usleep(1000);
			}
		}
	}
	p_dev->count += 1;
}

void usb1352_spi_data_receive(usb1352_dev* p_dev, uint8_t size, void* payload)
{
	sub1ghz_spi_frame rx_frame;

	if (!(usb1352_queue_empty(p_dev->usb1352_rx_queue)))
	{
		memcpy(&rx_frame, &(p_dev->usb1352_rx_queue->buffer[p_dev->usb1352_rx_queue->front]), sizeof(sub1ghz_spi_frame));
		pthread_mutex_lock(&p_dev->usb1352_queue_mutex);
		usb1352_queue_remove(p_dev->usb1352_rx_queue);
		pthread_mutex_unlock(&p_dev->usb1352_queue_mutex);

		memcpy(payload, rx_frame.payload, size);
	}
	else
	{
		memset(payload, 0, size);
	}
}

/*
void* usb1352_spi_thread(void* dev)
{
	usb1352_dev* p_dev = (usb1352_dev*)dev;

	sub1ghz_spi_frame tx_frame;
	sub1ghz_spi_frame rx_frame;

	while (1)
	{
		if (!(usb1352_queue_empty(p_dev->usb1352_tx_queue)))
		{
			memcpy(&tx_frame, &(p_dev->usb1352_tx_queue->buffer[p_dev->usb1352_tx_queue->front]), sizeof(sub1ghz_spi_frame));
			if (!(usb1352_spi_rw(p_dev, &tx_frame, &rx_frame)))
			{
				continue;
			}
			usb1352_queue_remove(p_dev->usb1352_tx_queue);
			p_dev->count += 1;

			if (rx_frame.sync1 == SPI_SYNC_BYTE1 && rx_frame.sync2 == SPI_SYNC_BYTE2)
			{
				if (rx_frame.fc.frameType == SPI_INTERRUPT)
				{
					if (rx_frame.fc.cmdType == SPI_INTR_NORMAL)
					{
						if (rx_frame.fc.cmdSubType == SPI_INTR_NORMAL_DATA)
						{
							pthread_mutex_lock(&p_dev->usb1352_queue_mutex);
							usb1352_queue_insert(p_dev->usb1352_rx_queue, &rx_frame);
							pthread_cond_signal(&p_dev->usb1352_rx_cond);
							pthread_mutex_unlock(&p_dev->usb1352_queue_mutex);
						}
					}
				}
			}
			if (rx_frame.rfc.queue_size < 3)
			{
				while (rx_frame.rfc.queue_size < 3)
				{
					memset(&tx_frame, 0, sizeof(sub1ghz_spi_frame));
					if (!(usb1352_spi_rw(p_dev, &tx_frame, &rx_frame)))
					{
						continue;
					}

					if (rx_frame.fc.frameType == SPI_INTERRUPT)
					{
						if (rx_frame.fc.cmdType == SPI_INTR_NORMAL)
						{
							if (rx_frame.fc.cmdSubType == SPI_INTR_NORMAL_DATA)
							{
								pthread_mutex_lock(&p_dev->usb1352_queue_mutex);
								usb1352_queue_insert(p_dev->usb1352_rx_queue, &rx_frame);
								pthread_mutex_unlock(&p_dev->usb1352_queue_mutex);
							}
						}
					}
					usleep(1000);
				}
			}
		}
	}
}
*/

void* usb1352_spi_intr_thread(void* dev)
{
	FT_HANDLE intr_handle;
	DWORD loc_id;
	EVENT_HANDLE event_handle;
	GPIO_Dir gpio_dir[4];
	BOOL value;
	usb1352_dev* p_dev = (usb1352_dev*)dev;

	FT_OpenEx((PVOID)(uintptr_t)(p_dev->loc_id + 1), FT_OPEN_BY_LOCATION, &intr_handle);

	pthread_mutex_init(&(event_handle.eMutex), NULL);
	pthread_cond_init(&(event_handle.eCondVar), NULL);

	FT_SetEventNotification(intr_handle, FT_EVENT_RXCHAR, (PVOID)&event_handle);

	gpio_dir[0] = GPIO_OUTPUT; gpio_dir[1] = GPIO_OUTPUT;
	gpio_dir[2] = GPIO_OUTPUT; gpio_dir[3] = GPIO_INPUT;

	FT4222_GPIO_Init(intr_handle, gpio_dir);
	FT4222_SetWakeUpInterrupt(intr_handle, TRUE);
	FT4222_SetInterruptTrigger(intr_handle, GPIO_TRIGGER_FALLING);

	FT4222_GPIO_Write(intr_handle, (GPIO_Port)GPIO_PORT0, 1);
	FT4222_GPIO_Write(intr_handle, (GPIO_Port)GPIO_PORT1, 1);
	FT4222_GPIO_Write(intr_handle, (GPIO_Port)GPIO_PORT2, 1);
	FT4222_GPIO_Write(intr_handle, (GPIO_Port)GPIO_PORT3, 1);

	sub1ghz_spi_frame dummy_data;
	sub1ghz_spi_frame rx_frame;

	while (1)
	{
		pthread_cond_wait(&(event_handle.eCondVar), &(event_handle.eMutex));
		FT4222_GPIO_Read(intr_handle, (GPIO_Port)GPIO_PORT3, &value);

		if (value == TRUE)
		{
			dummy_data.fc.frameType = SPI_DUMMY;
			dummy_data.fc.cmdType = SPI_NONE;
			dummy_data.fc.cmdSubType = SPI_NONE;

			while (1)
			{
				if (usb1352_spi_rw(p_dev, &dummy_data, &rx_frame) == TRUE)
				{
					break;
				}
				else
				{
					usleep(1000);
				}
			}

			if (rx_frame.sync1 == SPI_SYNC_BYTE1 && rx_frame.sync2 == SPI_SYNC_BYTE2)
			{
				if (rx_frame.fc.frameType == SPI_INTERRUPT)
				{
					if (rx_frame.fc.cmdType == SPI_INTR_NORMAL)
					{
						if (rx_frame.fc.cmdSubType == SPI_INTR_NORMAL_DATA)
						{
							pthread_mutex_lock(&p_dev->usb1352_queue_mutex);
							usb1352_queue_insert(p_dev->usb1352_rx_queue, &rx_frame);
							pthread_mutex_unlock(&p_dev->usb1352_queue_mutex);
						}
					}
				}
			}

//			usb1352_queue_insert(p_dev->usb1352_tx_queue, &dummy_data);
		}
	}
}
