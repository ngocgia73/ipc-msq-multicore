/*
 * Register offset and bit definition for Core Communicator module
 * depend on specific SOC
 *
 */

#ifndef _CC_REG_H
#define _CC_REG_H

/*************************************************************************************
 *  CPU2 Exception Base Register
 *************************************************************************************/
#define CC_CPU2_EXB_REG_OFS			0x00

typedef union cc_cpu2_exb_reg_t {
    unsigned int reg;
    struct {
    	unsigned int keyctrl       :12;
    	unsigned int exception_base:18;
    	unsigned int reserved0     :2;
    } bit;
} CC_CPU2_EXB_REG;

/*************************************************************************************
 *  CPUx Action Register
 *************************************************************************************/
#define CC_CPU1_ACT_REG_OFS			0x10
#define CC_CPU2_ACT_REG_OFS			0x20
#define CC_DSP1_ACT_REG_OFS			0x30

typedef union cc_cpux_act_reg_t {
    unsigned int reg;
    struct {
    	unsigned int trigger_cpu1:1;
    	unsigned int trigger_cpu2:1;
    	unsigned int trigger_dsp1:1;
    	unsigned int reserved0   :13;
    	unsigned int ack_cpu1    :1;
    	unsigned int ack_cpu2    :1;
    	unsigned int ack_dsp1    :1;
    	unsigned int reserved1   :13;
    } bit;
} CC_CPUX_ACT_REG;

/*************************************************************************************
 *  CPUx Status Register
 *************************************************************************************/
#define CC_CPU1_STS_REG_OFS			0x14
#define CC_CPU2_STS_REG_OFS			0x24
#define CC_DSP1_STS_REG_OFS			0x34

typedef union cc_cpux_sts_reg_t {
    unsigned int reg;
    struct {
    	unsigned int event_from_cpu1:1;
    	unsigned int event_from_cpu2:1;
    	unsigned int event_from_dsp1:1;
    	unsigned int reserved0      :13;
    	unsigned int ack_from_cpu1  :1;
    	unsigned int ack_from_cpu2  :1;
    	unsigned int ack_from_dsp1  :1;
    	unsigned int reserved1      :13;
    } bit;
} CC_CPUX_STS_REG;

typedef union cc_cpu1_sts_reg_t {
    unsigned int reg;
    struct {
    	unsigned int event_from_cpu1:1;
    	unsigned int event_from_cpu2:1;
    	unsigned int event_from_dsp1:1;
    	unsigned int reserved0      :13;
    	unsigned int ack_from_cpu1  :1;
    	unsigned int ack_from_cpu2  :1;
    	unsigned int ack_from_dsp1  :1;
    	unsigned int reserved1      :11;
    	unsigned int cpu2_go_sleep  :1;
    	unsigned int reserved2      :1;
    } bit;
} CC_CPU1_STS_REG;

/*************************************************************************************
 *  CPUx Interrupt Enable Register
 *************************************************************************************/
#define CC_CPU1_INTEN_REG_OFS			0x18
#define CC_CPU2_INTEN_REG_OFS			0x28
#define CC_DSP1_INTEN_REG_OFS			0x38

typedef union cc_cpux_inten_reg_t {
    unsigned int reg;
    struct {
    	unsigned int event_from_cpu1_inten:1;
    	unsigned int event_from_cpu2_inten:1;
    	unsigned int event_from_dsp1_inten:1;
    	unsigned int reserved0            :13;
    	unsigned int ack_from_cpu1_inten  :1;
    	unsigned int ack_from_cpu2_inten  :1;
    	unsigned int ack_from_dsp1_inten  :1;
    	unsigned int reserved1            :13;
    } bit;
} CC_CPUX_INTEN_REG;

typedef union cc_cpu1_inten_reg_t {
    unsigned int reg;
    struct {
    	unsigned int event_from_cpu1_inten:1;
    	unsigned int event_from_cpu2_inten:1;
    	unsigned int event_from_dsp1_inten:1;
    	unsigned int reserved0            :13;
    	unsigned int ack_from_cpu1_inten  :1;
    	unsigned int ack_from_cpu2_inten  :1;
    	unsigned int ack_from_dsp1_inten  :1;
    	unsigned int reserved1            :11;
    	unsigned int cpu2_go_sleep_inten  :1;
    	unsigned int reserved2            :1;
    } bit;
} CC_CPU1_INTEN_REG;

/*************************************************************************************
 *  CPUx Request Hardware Resource Register
 *************************************************************************************/
#define CC_CPU1_REQ_REG_OFS				0x40
#define CC_CPU2_REQ_REG_OFS				0x50
#define CC_DSP1_REQ_REG_OFS				0x60

typedef union cc_cpux_req_reg_t {
    unsigned int reg;
    struct {
    	unsigned int request;
    } bit;
} CC_CPUX_REQ_REG;

/*************************************************************************************
 *  CPUx Release Hardware Resource Register
 *************************************************************************************/
#define CC_CPU1_REL_REG_OFS				0x44
#define CC_CPU2_REL_REG_OFS				0x54
#define CC_DSP1_REL_REG_OFS				0x64

typedef union cc_cpux_rel_reg_t {
    unsigned int reg;
    struct {
    	unsigned int release;
    } bit;
} CC_CPUX_REL_REG;

/*************************************************************************************
 *  CPUx Grant Hardware Resource Register
 *************************************************************************************/
#define CC_CPU1_GRANT_REG_OFS			0x48
#define CC_CPU2_GRANT_REG_OFS			0x58
#define CC_DSP1_GRANT_REG_OFS			0x68

typedef union cc_cpux_grant_reg_t {
    unsigned int reg;
    struct {
    	unsigned int grant;
    } bit;
} CC_CPUX_GRANT_REG;

/*************************************************************************************
 *  CPUx HW Resource Flag Register
 *************************************************************************************/
#define CC_CPU1_FLG_REG_OFS				0x4C
#define CC_CPU2_FLG_REG_OFS				0x5C
#define CC_DSP1_FLG_REG_OFS				0x6C

typedef union cc_cpux_flg_reg_t {
    unsigned int reg;
    struct {
    	unsigned int flag;
    } bit;
} CC_CPUX_FLG_REG;

/*************************************************************************************
 *  Software Reset Register
 *************************************************************************************/
#define CC_SW_RST_REG_OFS				0x80

typedef union cc_sw_rst_reg_t {
    unsigned int reg;
    struct {
    	unsigned int reset    :1;
    	unsigned int reserved0:31;
    } bit;
} CC_SW_RST_REG;

/*************************************************************************************
 *  OCP Bridge FIFO Control Register
 *************************************************************************************/
#define CC_OCP_FIFO_CTRL_REG_OFS		0x84

typedef union cc_ocp_fifo_ctrl_reg_t {
    unsigned int reg;
    struct {
    	unsigned int reset_ocp1_fifo_info :1;
    	unsigned int latch_ocp1_fifo_depth:1;
    	unsigned int reserved0            :14;
    	unsigned int reset_ocp2_fifo_info :1;
    	unsigned int latch_ocp2_fifo_depth:1;
    	unsigned int reserved1            :14;
    } bit;
} CC_OCP_FIFO_CTRL_REG;

/*************************************************************************************
 *  OCPx Bridge FIFO Information Register
 *************************************************************************************/
#define CC_OCP1_FIFO_INFO_REG_OFS		0x88
#define CC_OCP2_FIFO_INFO_REG_OFS		0x8C

typedef union cc_ocpx_fifo_info_reg_t {
    unsigned int reg;
    struct {
    	unsigned int max_cmd_fifo_depth  :8;
    	unsigned int max_read_fifo_depth :8;
    	unsigned int max_write_fifo_depth:8;
    	unsigned int reserved0           :8;
    } bit;
} CC_OCPX_FIFO_INFO_REG;

/*************************************************************************************
 *  MIPSx Control Register
 *************************************************************************************/
#define CC_MIPS1_CTRL_REG_OFS			0x90
#define CC_MIPS2_CTRL_REG_OFS			0x94

typedef union cc_mipsx_ctrl_reg_t {
    unsigned int reg;
    struct {
    	unsigned int ocp_reorder      :1;
    	unsigned int ocp_multi_apb_cmd:1;
    	unsigned int ocp_multi_dma_cmd:1;
    	unsigned int reserved0        :29;
    } bit;
} CC_MIPSX_CTRL_REG;

/*************************************************************************************
 *  DSP Control and Status Register
 *************************************************************************************/
#define CC_DSP_CTRL_REG_OFS				0x98

typedef union cc_dsp_ctrl_reg_t {
    unsigned int reg;
    struct {
    	unsigned int reset_mode   :1;
    	unsigned int multi_epp_cmd:1;
    	unsigned int multi_edp_cmd:1;
    	unsigned int reserved0    :15;
    	unsigned int cpu2_sleep   :1;
    	unsigned int reserved1    :13;
    } bit;
} CC_DSP_CTRL_REG;

/*************************************************************************************
 *  DSP Error Status Register
 *************************************************************************************/
#define CC_DSP_ERR_REG_OFS				0x9C

typedef union cc_dsp_err_reg_t {
    unsigned int reg;
    struct {
    	unsigned int epp_invalid_addr:1;
    	unsigned int edp_invalid_addr:1;
    	unsigned int reserved0       :30;
    } bit;
} CC_DSP_ERR_REG;

/*************************************************************************************
 *  Command Buffer Register
 *************************************************************************************/
#define CC_CPU1_CPU2_CMDBUF1_REG_OFS		0x100	///< CPU1 -> CPU2
#define CC_CPU1_CPU2_CMDBUF2_REG_OFS		0x104
#define CC_CPU1_CPU2_CMDBUF3_REG_OFS		0x108
#define CC_CPU1_DSP1_CMDBUF1_REG_OFS		0x10C	///< CPU1 -> DSP1
#define CC_CPU1_DSP1_CMDBUF2_REG_OFS		0x110
#define CC_CPU1_DSP1_CMDBUF3_REG_OFS		0x114

#define CC_CPU2_CPU1_CMDBUF1_REG_OFS		0x118	///< CPU2 -> CPU1
#define CC_CPU2_CPU1_CMDBUF2_REG_OFS		0x11C
#define CC_CPU2_CPU1_CMDBUF3_REG_OFS		0x120
#define CC_CPU2_DSP1_CMDBUF1_REG_OFS		0x124	///< CPU2 -> DSP1
#define CC_CPU2_DSP1_CMDBUF2_REG_OFS		0x128
#define CC_CPU2_DSP1_CMDBUF3_REG_OFS		0x12C

#define CC_DSP1_CPU1_CMDBUF1_REG_OFS		0x130	///< DSP1 -> CPU1
#define CC_DSP1_CPU1_CMDBUF2_REG_OFS		0x134
#define CC_DSP1_CPU1_CMDBUF3_REG_OFS		0x138
#define CC_DSP1_CPU2_CMDBUF1_REG_OFS		0x13C	///< DSP1 -> CPU2
#define CC_DSP1_CPU2_CMDBUF2_REG_OFS		0x140
#define CC_DSP1_CPU2_CMDBUF3_REG_OFS		0x144

typedef union cc_cmdbuf1_reg_t {
    unsigned int reg;
    struct {
    	unsigned int serial_id:16;
    	unsigned int opcode   :16;
    } bit;
} CC_CMDBUF1_REG;

typedef union cc_cmdbuf2_reg_t {
    unsigned int reg;
    struct {
    	unsigned int data_addr;
    } bit;
} CC_CMDBUF2_REG;

typedef union cc_cmdbuf3_reg_t {
    unsigned int reg;
    struct {
    	unsigned int data_size;
    } bit;
} CC_CMDBUF3_REG;

#endif	/* _CC_REG_H */
