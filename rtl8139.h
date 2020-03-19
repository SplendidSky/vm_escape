typedef struct RTL8139State {
    /*< private >*/
    PCIDevice parent_obj;
    /*< public >*/

    uint8_t phys[8]; /* mac address */
    uint8_t mult[8]; /* multicast mask array */

    uint32_t TxStatus[4]; /* TxStatus0 in C mode*/ /* also DTCCR[0] and DTCCR[1] in C+ mode */
    uint32_t TxAddr[4];   /* TxAddr0 */
    uint32_t RxBuf;       /* Receive buffer */
    uint32_t RxBufferSize;/* internal variable, receive ring buffer size in C mode */
    uint32_t RxBufPtr;
    uint32_t RxBufAddr;

    uint16_t IntrStatus;
    uint16_t IntrMask;

    uint32_t TxConfig;
    uint32_t RxConfig;
    uint32_t RxMissed;

    uint16_t CSCR;

    uint8_t  Cfg9346;
    uint8_t  Config0;
    uint8_t  Config1;
    uint8_t  Config3;
    uint8_t  Config4;
    uint8_t  Config5;

    uint8_t  clock_enabled;
    uint8_t  bChipCmdState;

    uint16_t MultiIntr;

    uint16_t BasicModeCtrl;
    uint16_t BasicModeStatus;
    uint16_t NWayAdvert;
    uint16_t NWayLPAR;
    uint16_t NWayExpansion;

    uint16_t CpCmd;
    uint8_t  TxThresh;

    NICState *nic;
    NICConf conf;

    /* C ring mode */
    uint32_t   currTxDesc;

    /* C+ mode */
    uint32_t   cplus_enabled;

    uint32_t   currCPlusRxDesc;
    uint32_t   currCPlusTxDesc;

    uint32_t   RxRingAddrLO;
    uint32_t   RxRingAddrHI;

    EEprom9346 eeprom;

    uint32_t   TCTR;
    uint32_t   TimerInt;
    int64_t    TCTR_base;

    /* Tally counters */
    RTL8139TallyCounters tally_counters;

    /* Non-persistent data */
    uint8_t   *cplus_txbuffer;
    int        cplus_txbuffer_len;
    int        cplus_txbuffer_offset;

    /* PCI interrupt timer */
    QEMUTimer *timer;

    MemoryRegion bar_io;
    MemoryRegion bar_mem;

    /* Support migration to/from old versions */
    int rtl8139_mmio_io_addr_dummy;
} RTL8139State;


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
