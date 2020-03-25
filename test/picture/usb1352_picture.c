#include "../../src/usb1352.h"

#include <time.h>

#define SIZE 190

typedef struct test_frame
{
	uint8_t type;
	uint16_t seq;
	uint8_t tx_addr;
	uint8_t rx_addr;
	uint8_t length;
	char payload[SIZE];
} test_frame;

void* tx_thread(void* dev)
{
	usb1352_dev* p_dev = (usb1352_dev*)dev;

	test_frame frame;
	FILE* img;
	int count;
	uint16_t seq;
	
	int start;


	while (1)
	{
		scanf("%d", &start);

		if (start == 123)
		{
			frame.type = 1;
			frame.seq = seq;
			usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &frame);
			seq = 0;
			printf("start\n");
			img = fopen("wakgood_slave.jpg", "rb");
		}
		else if (start == 1234)
		{
			printf("p_dev count: %d\n", p_dev->count);
			continue;
		}
		else
		{
			continue;
		}

		usleep(1000000);

		while (1)
		{
			count = fread(frame.payload, 1, SIZE, img);

			if (count < SIZE)
			{
				if (feof(img) != 0)
				{
					frame.type = 3;
					frame.seq = seq;
					frame.length = count;
					usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &frame);
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
				usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &frame);
				seq += 1;
			}
		}
		printf("send count: %d\n", seq);
	}
}

void* rx_thread(void* dev)
{
	usb1352_dev* p_dev = (usb1352_dev*)dev;

	test_frame frame;

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
		}
		else if (frame.type == 3)
		{
			fwrite(frame.payload, 1, frame.length, img);
			printf("done\n");
			fclose(img);

			clock_gettime(CLOCK_REALTIME, &end);

			result.tv_sec = end.tv_sec - start.tv_sec;
			result.tv_nsec = end.tv_nsec - start.tv_nsec;

			count += 1;
			printf("recv count: %d\n", count);
			printf("%ldsec %ldnsec\n", result.tv_sec, result.tv_nsec);
		}
		else if (frame.type == 2)
		{
			printf("cur seq: %d\n", frame.seq);
			count += 1;
			fwrite(frame.payload, 1, frame.length, img);
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

	pthread_create(&tx_thread_t, NULL, tx_thread, my_dev);
	pthread_create(&rx_thread_t, NULL, rx_thread, my_dev);

	while (1);
}
