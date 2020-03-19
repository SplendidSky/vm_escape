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
