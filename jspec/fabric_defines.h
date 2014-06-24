

/* This file is generated automatically by the vh2h.pl Perl script
   from the Verilog source.  Editing it by hand will eventually cause
   you grief.
*/


#define CFAB_REGISTER_INPUTS           0x1
#define DFAB_REGISTER_INPUTS           0x1
#define CFAB_DATA_WIDTH                0x20
#define CFAB_ADDR_WIDTH                0x16
#define CFAB_TYPE_WIDTH                0x3
#define CFAB_TAG_WIDTH                 0x2
#define CFAB_NID_WIDTH                 0x4
#define CFAB_PORT_WIDTH                0x2
#define CFAB_FAIR_WIDTH                0x4
#define CFAB_WIDTH                     0x43
#define DFAB_DATA_WIDTH                0x100
#define DFAB_ADDR_WIDTH                0x13
#define DFAB_TYPE_WIDTH                0x3
#define DFAB_TAG_WIDTH                 0x2
#define DFAB_NID_WIDTH                 0x4
#define DFAB_PORT_WIDTH                0x2
#define DFAB_FAIR_WIDTH                0x4
#define DFAB_WIDTH                     0x120
#define FAB_TYPE_NOP                   0x0
#define FAB_TYPE_READ_REQ              0x1
#define FAB_TYPE_READ_RESP             0x2
#define FAB_TYPE_WRITE                 0x3
#define FAB_TYPE_DMA_REQ               0x4
#define FAB_TYPE_DMA_RESP              0x5
#define FAB_NID_ROOT                   0x0
#define FAB_NID_PCI                    0x0
#define FAB_NID_PBUF                   0x1
#define FAB_NID_FLOW                   0x2
#define FAB_NID_CTX                    0x3
#define FAB_NID_IDMA                   0x4
#define FAB_NID_EDMA                   0x5
#define FAB_NID_ENETA                  0x6
#define FAB_NID_ENETB                  0x7
#define FAB_NID_QMGR                   0x8
#define FAB_NID_TEST                   0xc
#define FAB_NID_LOOPBACK               0xe
#define CFAB_FAIRNESS                  0x2
#define DFAB_FAIRNESS                  0x0
#define FAB_TAG_UNKNOWN                0x0

#define IDMA_CSR_START                 0x40000000
#define IDMA_CSR_END                   0x40ffffff
#define IDMA_CSR_SIZE                  16777216

#define EDMA_CSR_START                 0x50000000
#define EDMA_CSR_END                   0x50ffffff
#define EDMA_CSR_SIZE                  16777216
#define IDMA_CSR_ID                    0x00000000
#define IDMA_CSR_SRC_ADDR              0x00000004
#define IDMA_CSR_DEST_ADDR             0x00000008
#define IDMA_CSR_LENGTH                0x0000000c
#define IDMA_CSR_STATUS                0x00000010

#define PBUF_CSR_START                 0x10000000
#define PBUF_CSR_END                   0x10ffffff
#define PBUF_CSR_SIZE                  16777216
#define PBUF_CSR_ID                    0x00000000
#define PBUF_CSR_DDR_CTL               0x00000004
#define PBUF_CSR_DDR_STATUS            0x00000008
#define PBUF_CSR_WR_RAM_ADDR           0x00000010
#define PBUF_CSR_WR_LCL_ADDR           0x00000014
#define PBUF_CSR_WR_STATUS             0x00000018
#define PBUF_CSR_RD_RAM_ADDR           0x00000020
#define PBUF_CSR_RD_LCL_ADDR           0x00000024
#define PBUF_CSR_RD_STATUS             0x00000028

#define CTX_CSR_START                  0x30000000
#define CTX_CSR_END                    0x30ffffff
#define CTX_CSR_SIZE                   16777216
#define CTX_CSR_ID                     0x00000000
#define CTX_CSR_QDR_CTL                0x00000004
#define CTX_CSR_QDR_STATUS             0x00000008
#define CTX_CSR_WR_RAM_ADDR            0x00000010
#define CTX_CSR_WR_LCL_ADDR            0x00000014
#define CTX_CSR_WR_STATUS              0x00000018
#define CTX_CSR_RD_RAM_ADDR            0x00000020
#define CTX_CSR_RD_LCL_ADDR            0x00000024
#define CTX_CSR_RD_STATUS              0x00000028

#define PCI_CSR_START                  0x00000000
#define PCI_CSR_END                    0x00ffffff
#define PCI_CSR_SIZE                   16777216
#define PCI_CSR_ID                     0x00000000
#define PCI_CSR_CTL                    0x00000004
#define PCI_CSR_STATUS                 0x00000008
#define PCI_CSR_CMD_Q                  0x00000010

#define QMGR_CSR_START                 0x80000000
#define QMGR_CSR_END                   0x80ffffff
#define QMGR_CSR_SIZE                  16777216
#define QMGR_CSR_ID                    0x00000000
#define QMGR_CSR_CTL                   0x00000004
#define QMGR_CSR_STATUS                0x00000008
#define QMGR_CSR_STATS_CLR             0x0000000c
#define QMGR_CSR_I_CMD_Q               0x00001000
#define QMGR_CSR_E_CMD_Q               0x00002000
#define QMGR_CSR_INGRESS_STATS_START   0x00008000
#define QMGR_CSR_INGRESS_STATS_END     0x00008fff
#define QMGR_CSR_INGRESS_STATS_SIZE    4096
#define QMGR_CSR_EGRESS_STATS_START    0x00009000
#define QMGR_CSR_EGRESS_STATS_END      0x00009fff
#define QMGR_CSR_EGRESS_STATS_SIZE     4096

#define LOOP_CSR_START                 0xe0000000
#define LOOP_CSR_END                   0xe0ffffff
#define LOOP_CSR_SIZE                  16777216
#define LOOP_CSR_ID                    0x00000000

#define ENETA_CSR_START                0x60000000
#define ENETA_CSR_END                  0x60ffffff
#define ENETA_CSR_SIZE                 16777216

#define ENETB_CSR_START                0x70000000
#define ENETB_CSR_END                  0x70ffffff
#define ENETB_CSR_SIZE                 16777216
#define ENET_CSR_ID                    0x00000000
#define ENET_CSR_HIGIG_CTL_1           0x00000010
#define ENET_CSR_HIGIG_CTL_1_RXEN_M    0x1
#define ENET_CSR_HIGIG_CTL_1_RXEN_S    0
#define ENET_CSR_HIGIG_CTL_1_RXEN      0x00000001
#define ENET_CSR_HIGIG_CTL_1_TXEN_M    0x1
#define ENET_CSR_HIGIG_CTL_1_TXEN_S    1
#define ENET_CSR_HIGIG_CTL_1_TXEN      0x00000002
#define ENET_CSR_HIGIG_CTL_2           0x00000014
#define ENET_CSR_HIGIG_CTL_3           0x00000018
#define ENET_CSR_HIGIG_CTL_4           0x0000001c
#define ENET_CSR_HIGIG_CTL_5           0x00000020
#define ENET_CSR_HIGIG_STAT_1          0x00000024
#define ENET_CSR_HIGIG_STAT_2          0x00000028
#define ENET_CSR_HIGIG_STAT_3          0x0000002c
#define ENET_INGRESS_DEST_Q            0x00000030
#define ENET_CSR_HIGIG_HDR_1           0x00000040
#define ENET_CSR_HIGIG_HDR_2           0x00000044
#define ENET_CSR_HIGIG_HDR_3           0x00000048
#define ENET_CSR_HIGIG_HDR_4           0x0000004c

#define TEST_CSR_START                 0xc0000000
#define TEST_CSR_END                   0xc0ffffff
#define TEST_CSR_SIZE                  16777216
#define TEST_CSR_ID                    0x00000000
#define TEST_CSR_CTL                   0x00000004
#define TEST_CSR_STATUS                0x00000008
#define COMM_CSR_CMD_Q                 0x00000100
