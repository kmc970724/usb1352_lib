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
	int size;

	long long int elapsed_time_int;
	double elapsed_time_double;
	double data_rate;

	while (1)
	{
		usb1352_spi_data_receive(p_dev, sizeof(test_frame), &frame);

		if (frame.type == 1)
		{
			printf("start\n");
			img = fopen("wakgood_master.jpg", "wb");

			count = 0;
			size = 0;
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
			size += frame.length;
			printf("recv count: %d\n", count);
			printf("%ldsec %ldnsec\n", result.tv_sec, result.tv_nsec);
			printf("size: %d\n", size * 8);

			elapsed_time_int = (long long int)result.tv_sec * 1000000000L + result.tv_nsec;
			elapsed_time_double = ((double)elapsed_time_int) / 1000000000L;
			elapsed_time_double -= 1;

			printf("elapsed sec: %.2lf (int: %ld\n", elapsed_time_double, elapsed_time_int);
			data_rate = size * 8 / elapsed_time_double;
			printf("BPS: %.2lf\n", data_rate);

		}
		else if (frame.type == 2)
		{
//			printf("cur seq: %d\n", frame.seq);
			count += 1;
			size += frame.length;
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

	printf("init complete\n");

	pthread_create(&tx_thread_t, NULL, tx_thread, my_dev);
	pthread_create(&rx_thread_t, NULL, rx_thread, my_dev);

	while (1);
}
