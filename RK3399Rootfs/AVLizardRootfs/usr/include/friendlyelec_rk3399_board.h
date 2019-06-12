/*----------------------------------------------------------------------------*/
/*

	WiringPi RK3399 Board Header file

 */
/*----------------------------------------------------------------------------*/
#ifndef	__FRIENDLYELEC_RK3399_BOARD_H__
#define __FRIENDLYELEC_RK3399_BOARD_H__

/*----------------------------------------------------------------------------*/
// Common mmap block size for ODROID-N1 GRF register
#define GRF_BLOCK_SIZE		0xF000

// Common offset for GPIO registers from each GPIO bank's base address
#define GPIO_CON_OFFSET		0x04		// GPIO_SWPORTA_DDR
#define GPIO_SET_OFFSET		0x00		// GPIO_SWPORTA_DR
#define GPIO_GET_OFFSET		0x50		// GPIO_EXT_PORTA

#define FUNC_GPIO		0b00		// Bit for IOMUX GPIO mode

// GPIO{0, 1}
#define PMUGRF_BASE		0xFF320000
#define PMUGRF_IOMUX_OFFSET	0x0000		// GRF_GPIO0A_IOMUX
#define PMUGRF_PUPD_OFFSET	0x0040		// PMUGRF_GPIO0A_P

// GPIO{2, 3, 4}
#define GRF_BASE 		0xFF770000
#define GRF_IOMUX_OFFSET	0xE000		// GRF_GPIO2A_IOMUX
#define GRF_PUPD_OFFSET		0xE040		// GRF_GPIO2A_P

// Offset to control GPIO clock
// Make 31:16 bit HIGH to enable the writing corresponding bit
#define PMUCRU_BASE		0xFF750000
#define PMUCRU_GPIO_CLK_OFFSET	0x0104		// PMUCRU_CLKGATE_CON1

#define CRU_BASE		0xFF760000
#define CRU_GPIO_CLK_OFFSET	0x037C		// CRU_CLKGATE_CON31

#define CLK_ENABLE		0b0
#define CLK_DISABLE		0b1

#define GPIO_PIN_BASE		0

// Refre:
// /opt/FriendlyARM/rk3399/rk3399-android8/kernel/arch/arm64/boot/dts/rockchip/rk3399.dtsi

// GPIO1_A.	0,1,2,3,4,7
// GPIO1_B. 	0,1,2,3,4,5
// GPIO1_C.	2,4,5,6
// GPIO1_D.	0
#define GPIO_1_BASE		0xFF730000

// GPIO2_C.	0_B,1_B
#define GPIO_2_BASE		0xFF780000

// GPIO4_C.	5,6
// GPIO4_D.	0,4,5,6
#define GPIO_4_BASE		0xFF790000

// Reserved
// GPIO{0, 3}
#define GPIO_0_BASE		0xFF720000
#define GPIO_3_BASE		0xFF788000

#ifdef __cplusplus
extern "C" {
#endif

extern void init_nanopct4 (struct libfriendlyelec *libwiring);
extern void init_nanopim4 (struct libfriendlyelec *libwiring);
extern void init_nanopineo4 (struct libfriendlyelec *libwiring);

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
#endif	/* __FRIENDLYELEC_RK3399_BOARD_H__ */
/*----------------------------------------------------------------------------*/
