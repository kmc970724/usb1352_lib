#include "../src/usb1352.h"

#include <time.h>
#include <errno.h>

int count;

typedef struct test_frame
{
	uint8_t type;
	uint8_t seq;
	uint8_t tx_addr;
	uint8_t rx_addr;
	uint8_t length;
	char payload[16];
} test_frame;

pthread_cond_t transfer_cond;
pthread_mutex_t transfer_mutex;

void* tx_thread(void* dev)
{
	usb1352_dev* p_dev = (usb1352_dev*)dev;
	
	test_frame frame;

	uint8_t seq = 0;

	struct timespec transfer_clock;

	while (1)
	{
		frame.type = 1;
		frame.length = strlen("Test");
		frame.seq = seq;
		strcpy(frame.payload, "Test");
		usb1352_spi_data_transfer(p_dev, sizeof(test_frame), &frame);
		seq = (seq + 1) % 0xFF;

		usleep(10000);
	}
}

void* rx_thread(void* dev)
{
	usb1352_dev* p_dev = (usb1352_dev*)dev;

	count = 0;

	test_frame rx_frame;
	test_frame ack_frame;

	while (1)
	{
		usb1352_spi_data_receive(p_dev, sizeof(test_frame), &rx_frame);
		if (rx_frame.seq == 0xF0)
		{
			printf("Test\n");
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
