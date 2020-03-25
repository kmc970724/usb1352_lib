#include "usb1352.h"

usb1352_dev* usb1352_init(void)
{
	FT_DEVICE_LIST_INFO_NODE* usb_dev;
	DWORD dev_count;
	usb1352_dev* p_dev;

	FT_CreateDeviceInfoList(&dev_count);

	if (dev_count == 0)
	{
		printf("Device not found.\n");
		exit(0);
	}

	usb_dev = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * dev_count);
	FT_GetDeviceInfoList(usb_dev, &dev_count);

	p_dev = (usb1352_dev*)malloc(sizeof(usb1352_dev));

	p_dev->usb1352_tx_queue = (usb1352_queue*)malloc(sizeof(usb1352_queue));
	p_dev->usb1352_rx_queue = (usb1352_queue*)malloc(sizeof(usb1352_queue));

	usb1352_queue_init(p_dev->usb1352_tx_queue);
	usb1352_queue_init(p_dev->usb1352_rx_queue);

	for (int i = 0; i < dev_count; i++)
	{
		p_dev->loc_id = usb_dev[i].LocId;
		pthread_mutex_init(&(p_dev->usb1352_spi_mutex), NULL);
		pthread_mutex_init(&(p_dev->usb1352_queue_mutex), NULL);
		usb1352_spi_init(p_dev);
		break;
	}

	return p_dev;
}

