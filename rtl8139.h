typedef struct {
    uint32_t txdw0;
    uint32_t txdw1;
    uint32_t txbufLO;
    uint32_t txbufHI;
} rtl8139_cplus_tx_ring_desc;

typedef struct {
    uint32_t rxdw0;
    uint32_t rxdw1;
    uint32_t rxbufLO;
    uint32_t rxbufHI;
} rtl8139_cplus_rx_ring_desc;

typedef struct {
    rtl8139_cplus_rx_ring_desc *cplus_rx_ring_desc;
    void *buf;
} rtl8139_cplus_rx_ring;

#define PAGE_SHIFT  12
#define PAGE_SIZE   (1 << PAGE_SHIFT)

/* w0 ownership flag */
#define CP_RX_OWN (1<<31)
/* w0 end of ring flag */
#define CP_RX_EOR (1<<30)
/* w0 bits 0...12 : buffer size */
#define CP_RX_BUFFER_SIZE_MASK ((1<<13) - 1)
/* w1 tag available flag */
#define CP_RX_TAVA (1<<16)
/* w1 bits 0...15 : VLAN tag */
#define CP_RX_VLAN_TAG_MASK ((1<<16) - 1)

#define CP_RX_BUFFER_SIZE 1514 + 4
#define CP_RX_BUFFER_N (65535 / CP_RX_BUFFER_SIZE)

/* w0 ownership flag */
#define CP_TX_OWN (1<<31)
/* w0 end of ring flag */
#define CP_TX_EOR (1<<30)
/* first segment of received packet flag */
#define CP_TX_FS (1<<29)
/* last segment of received packet flag */
#define CP_TX_LS (1<<28)
/* large send packet flag */
#define CP_TX_LGSEN (1<<27)
/* large send MSS mask, bits 16...25 */
#define CP_TC_LGSEN_MSS_MASK ((1 << 12) - 1)

/* IP checksum offload flag */
#define CP_TX_IPCS (1<<18)
/* UDP checksum offload flag */
#define CP_TX_UDPCS (1<<17)
/* TCP checksum offload flag */
#define CP_TX_TCPCS (1<<16)

/* w0 bits 0...15 : buffer size */
#define CP_TX_BUFFER_SIZE (1<<16)
#define CP_TX_BUFFER_SIZE_MASK (CP_TX_BUFFER_SIZE - 1)
/* w1 add tag flag */
#define CP_TX_TAGC (1<<17)
/* w1 bits 0...15 : VLAN tag (big endian) */
#define CP_TX_VLAN_TAG_MASK ((1<<16) - 1)
/* w2 low  32bit of Rx buffer ptr */
/* w3 high 32bit of Rx buffer ptr */

/* set after transmission */
/* FIFO underrun flag */
#define CP_TX_STATUS_UNF (1<<25)
/* transmit error summary flag, valid if set any of three below */
#define CP_TX_STATUS_TES (1<<23)
/* out-of-window collision flag */
#define CP_TX_STATUS_OWC (1<<22)
/* link failure flag */
#define CP_TX_STATUS_LNKF (1<<21)
/* excessive collisions flag */
#define CP_TX_STATUS_EXC (1<<20)

enum ClearBitMasks {
    MultiIntrClear = 0xF000,
    ChipCmdClear = 0xE2,
    Config1Clear = (1<<7)|(1<<6)|(1<<3)|(1<<2)|(1<<1),
};

enum ChipCmdBits {
    CmdReset = 0x10,
    CmdRxEnb = 0x08,
    CmdTxEnb = 0x04,
    RxBufEmpty = 0x01,
};

/* C+ mode */
enum CplusCmdBits {
    CPlusRxVLAN   = 0x0040, /* enable receive VLAN detagging */
    CPlusRxChkSum = 0x0020, /* enable receive checksum offloading */
    CPlusRxEnb    = 0x0002,
    CPlusTxEnb    = 0x0001,
};

/* Interrupt register bits, using my own meaningful names. */
enum IntrStatusBits {
    PCIErr = 0x8000,
    PCSTimeout = 0x4000,
    RxFIFOOver = 0x40,
    RxUnderrun = 0x20, /* Packet Underrun / Link Change */
    RxOverflow = 0x10,
    TxErr = 0x08,
    TxOK = 0x04,
    RxErr = 0x02,
    RxOK = 0x01,

    RxAckBits = RxFIFOOver | RxOverflow | RxOK,
};

enum TxStatusBits {
    TxHostOwns = 0x2000,
    TxUnderrun = 0x4000,
    TxStatOK = 0x8000,
    TxOutOfWindow = 0x20000000,
    TxAborted = 0x40000000,
    TxCarrierLost = 0x80000000,
};
enum RxStatusBits {
    RxMulticast = 0x8000,
    RxPhysical = 0x4000,
    RxBroadcast = 0x2000,
    RxBadSymbol = 0x0020,
    RxRunt = 0x0010,
    RxTooLong = 0x0008,
    RxCRCErr = 0x0004,
    RxBadAlign = 0x0002,
    RxStatusOK = 0x0001,
};

/* Bits in RxConfig. */
enum rx_mode_bits {
    AcceptErr = 0x20,
    AcceptRunt = 0x10,
    AcceptBroadcast = 0x08,
    AcceptMulticast = 0x04,
    AcceptMyPhys = 0x02,
    AcceptAllPhys = 0x01,
};

/* Bits in TxConfig. */
enum tx_config_bits {

        /* Interframe Gap Time. Only TxIFG96 doesn't violate IEEE 802.3 */
        TxIFGShift = 24,
        TxIFG84 = (0 << TxIFGShift),    /* 8.4us / 840ns (10 / 100Mbps) */
        TxIFG88 = (1 << TxIFGShift),    /* 8.8us / 880ns (10 / 100Mbps) */
        TxIFG92 = (2 << TxIFGShift),    /* 9.2us / 920ns (10 / 100Mbps) */
        TxIFG96 = (3 << TxIFGShift),    /* 9.6us / 960ns (10 / 100Mbps) */

    TxLoopBack = (1 << 18) | (1 << 17), /* enable loopback test mode */
    TxCRC = (1 << 16),    /* DISABLE appending CRC to end of Tx packets */
    TxClearAbt = (1 << 0),    /* Clear abort (WO) */
    TxDMAShift = 8,        /* DMA burst value (0-7) is shifted this many bits */
    TxRetryShift = 4,    /* TXRR value (0-15) is shifted this many bits */

    TxVersionMask = 0x7C800000, /* mask out version bits 30-26, 23 */
};


/* Symbolic offsets to registers. */
enum RTL8139_registers {
    MAC0 = 0,        /* Ethernet hardware address. */
    MAR0 = 8,        /* Multicast filter. */
    TxStatus0 = 0x10,/* Transmit status (Four 32bit registers). C mode only */
                     /* Dump Tally Conter control register(64bit). C+ mode only */
    TxAddr0 = 0x20,  /* Tx descriptors (also four 32bit). */
    RxBuf = 0x30,
    ChipCmd = 0x37,
    RxBufPtr = 0x38,
    RxBufAddr = 0x3A,
    IntrMask = 0x3C,
    IntrStatus = 0x3E,
    TxConfig = 0x40,
    RxConfig = 0x44,
    Timer = 0x48,        /* A general-purpose counter. */
    RxMissed = 0x4C,    /* 24 bits valid, write clears. */
    Cfg9346 = 0x50,
    Config0 = 0x51,
    Config1 = 0x52,
    FlashReg = 0x54,
    MediaStatus = 0x58,
    Config3 = 0x59,
    Config4 = 0x5A,        /* absent on RTL-8139A */
    HltClk = 0x5B,
    MultiIntr = 0x5C,
    PCIRevisionID = 0x5E,
    TxSummary = 0x60, /* TSAD register. Transmit Status of All Descriptors*/
    BasicModeCtrl = 0x62,
    BasicModeStatus = 0x64,
    NWayAdvert = 0x66,
    NWayLPAR = 0x68,
    NWayExpansion = 0x6A,
    /* Undocumented registers, but required for proper operation. */
    FIFOTMS = 0x70,        /* FIFO Control and test. */
    CSCR = 0x74,        /* Chip Status and Configuration Register. */
    PARA78 = 0x78,
    PARA7c = 0x7c,        /* Magic transceiver parameter register. */
    Config5 = 0xD8,        /* absent on RTL-8139A */
    /* C+ mode */
    TxPoll        = 0xD9,    /* Tell chip to check Tx descriptors for work */
    RxMaxSize    = 0xDA, /* Max size of an Rx packet (8169 only) */
    CpCmd        = 0xE0, /* C+ Command register (C+ mode only) */
    IntrMitigate    = 0xE2,    /* rx/tx interrupt mitigation control */
    RxRingAddrLO    = 0xE4, /* 64-bit start addr of Rx ring */
    RxRingAddrHI    = 0xE8, /* 64-bit start addr of Rx ring */
    TxThresh    = 0xEC, /* Early Tx threshold */
};
