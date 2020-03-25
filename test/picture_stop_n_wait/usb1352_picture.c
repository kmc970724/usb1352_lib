#include "../../src/usb1352.h"

#include <time.h>
#include <errno.h>

typedef struct test_frame
{
	uint8_t type;
	uint16_t seq;
	uint8_t tx_addr;
	uint8_t rx_addr;
	uint8_t length;
	char payload[190];
} test_frame;

pthread_cond_t transfer_cond;
pthread_mutex_t transfer_mutex;

void* tx_thread(void* dev)
{
	usb1352_dev* p_dev = (usb1352_dev*)dev;

	test_frame frame;
	FILE* img;
	int count;
	uint16_t seq;
	
	int start;

	struct timespec transfer_clock;

	img = fopen("wakgood_slave.jpg", "rb");

	scanf("%d", &start);

	if (start == 123)
	{
		frame.type = 1;
		frame.seq = seq;
		usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &frame);
		seq = 0;
		printf("start\n");
	}

	usleep(1000000);

	while (1)
	{
		count = fread(frame.payload, 1, 190, img);

		if (count < 190)
		{
			if (feof(img) != 0)
			{
				frame.type = 3;
				frame.seq = seq;
				frame.length = count;
				while (1)
				{
					clock_gettime(CLOCK_REALTIME, &transfer_clock);
					transfer_clock.tv_sec += 1;

					usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &frame);
					if (pthread_cond_timedwait(&transfer_cond, &transfer_mutex, &transfer_clock) != ETIMEDOUT)
					{
						break;
					}
				}
				seq += 1;
				printf("done\n");
				break;
			}
		}
		else
		{
			frame.type = 2;
			frame.seq = seq;
			frame.length = count;
			while (1)
			{
				clock_gettime(CLOCK_REALTIME, &transfer_clock);
				transfer_clock.tv_sec += 1;

				usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &frame);
				if (pthread_cond_timedwait(&transfer_cond, &transfer_mutex, &transfer_clock) != ETIMEDOUT)
				{
					break;
				}
			}
			seq += 1;
		}
	}
	printf("send count: %d\n", seq);
}

void* rx_thread(void* dev)
{
	usb1352_dev* p_dev = (usb1352_dev*)dev;

	test_frame frame, ack_frame;

	struct timespec start, end, result;

	FILE* img;
	uint16_t count;

	while (1)
	{
		usb1352_spi_data_receive(p_dev, sizeof(test_frame), &frame);

		if (frame.type == 1)
		{
			printf("start\n");
			img = fopen("wakgood_master.jpg", "wb");

			count = 0;
			clock_gettime(CLOCK_REALTIME, &start);

			ack_frame.type = 4;
			usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &ack_frame);
		}
		else if (frame.type == 3)
		{
			if (frame.seq == count)
			{
				fwrite(frame.payload, 1, frame.length, img);
				printf("done\n");
				fclose(img);

				clock_gettime(CLOCK_REALTIME, &end);

				result.tv_sec = end.tv_sec - start.tv_sec;
				result.tv_nsec = end.tv_nsec - start.tv_nsec;

				count += 1;
				printf("recv count: %d\n", count);
				printf("p_dev count: %d\n", p_dev->count);
				printf("%ldsec %ldnsec\n", result.tv_sec, result.tv_nsec);

				ack_frame.type = 4;
				usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &ack_frame);
			}
		}
		else if (frame.type == 2)
		{
//			printf("cur seq: %d\n", frame.seq);

			if (frame.seq == count)
			{
				count += 1;
				fwrite(frame.payload, 1, frame.length, img);

				ack_frame.type = 4;
				usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &ack_frame);
			}
		}
		else if (frame.type == 4)
		{
			pthread_cond_signal(&transfer_cond);
		}

		usleep(1000);
	}
}

int main(void)
{
	usb1352_dev* my_dev;

	pthread_t tx_thread_t;
	pthread_t rx_thread_t;

	my_dev = usb1352_init();

	pthread_cond_init(&transfer_cond, NULL);
	pthread_mutex_init(&transfer_mutex, NULL);

	pthread_create(&tx_thread_t, NULL, tx_thread, my_dev);
	pthread_create(&rx_thread_t, NULL, rx_thread, my_dev);

	while (1);
}
