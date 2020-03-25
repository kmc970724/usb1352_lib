#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define SPI_NONE 0 

#define SPI_SYNC_BYTE1                  0xAA
#define SPI_SYNC_BYTE2                  0x7E

/* FrameControl.FrameType */
#define SPI_DATA_TRANSFER_REQ           0b001
#define SPI_DATA_TRANSFER_RES           0b010

#define SPI_COMMAND_REQ                 0b011
#define SPI_COMMAND_RES                 0b100

#define SPI_INTERRUPT                   0b101

#define SPI_DUMMY                       0b111

/* FrameControl.CmdType */
#define SPI_DATA_CMD_COMMON             0b01
#define SPI_DATA_CMD_LEAD               0b10
#define SPI_DATA_CMD_ISOCHRONOUS        0b11

#define SPI_CMD_COMMON                  0b01
#define SPI_CMD_PARAMS                  0b10
#define SPI_CMD_RFSTATE                 0b11

#define SPI_INTR_NORMAL                 0b01
#define SPI_INTR_PARAMS                 0b10
#define SPI_INTR_STATUS					0b11

/* FrameControl.CmdSubType*/
#define SPI_CMD_COMMON_RESET            0b001
#define SPI_CMD_COMMON_RXQUEUE_SIZE		0b010

// REQ
#define SPI_DATA_CMD_COMMON_NORMAL      0b001
#define SPI_DATA_CMD_LEAD_START         0b001
#define SPI_DATA_CMD_LEAD_DATA          0b010
#define SPI_DATA_CMD_LEAD_END           0b011
#define SPI_DATA_CMD_ISOCHRONOUS_DATA   0b001

// RES
#define SPI_DATA_CMD_DONE               0b001
#define SPI_DATA_CMD_QUEUE_FULL         0b010

#define SPI_CMD_PARAMS_TXPOWER          0b001
#define SPI_CMD_PARAMS_CENTERFREQ       0b010
#define SPI_CMD_PARAMS_DATARATE         0b011
#define SPI_CMD_PARAMS_ALLSET           0b100

// RF State Cmd
#define SPI_CMD_RFSTATE_RSSI            0b001
#define SPI_CMD_RFSTATE_THROUGHPUT      0b010
#define SPI_CMD_RFSTATE_PER             0b011
#define SPI_CMD_RFSTATE_CUR_BPS         0b100
#define SPI_CMD_RFSTATE_CUR_POWER       0b101
#define SPI_CMD_RFSTATE_CUR_FREQ        0b110
#define SPI_CMD_RFSTATE_ALL_DATA        0b111

// Interrupt
#define SPI_INTR_NORMAL_DATA            0b001
#define SPI_INTR_PARAMS_TXPOWER         0b001
#define SPI_INTR_PARAMS_CENTERFREQ      0b010
#define SPI_INTR_PARAMS_DATARATE        0b011
#define SPI_INTR_PARAMS_ALLSET          0b100
#define SPI_INTR_STATUS_RSSI			0b001

#endif
