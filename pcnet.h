
#include <inttypes.h>
#include <stdio.h>


#define page_aligned __attribute__((aligned(PAGE_SIZE)))

#define PCNET_BUFFER_SIZE 4096
#define PCNET_PORT        0xc140

#define DRX     0x0001
#define DTX     0x0002
#define LOOP    0x0004
#define DXMTFCS 0x0008
#define INTL    0x0040
#define DRCVPA  0x2000
#define DRCVBC  0x4000
#define PROM    0x8000

#define CRC(crc, ch) (crc = (crc >> 8) ^ crctab[(crc ^ (ch)) & 0xff])

enum PCNET_registers {
	RDP = 0x10,
	RAP = 0x12,
	RST = 0x14,
};
struct pcnet_config {
	uint16_t  mode;
	uint8_t   rlen;
	uint8_t   tlen;
	uint8_t   mac[6];
	uint16_t _reserved;
	uint8_t   ladr[8];
	uint32_t  rx_desc;
	uint32_t  tx_desc;
};

struct pcnet_desc {
	uint32_t  addr;
	int16_t   length;
	int8_t    status_1;
	int8_t    status_2;
	uint32_t  misc;
	uint32_t _reserved;
};
