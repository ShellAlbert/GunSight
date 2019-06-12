/*
 * wiringPi.h:
 *	Arduino like Wiring library for the Raspberry Pi.
 *	Copyright (c) 2012-2017 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */
/*----------------------------------------------------------------------------*/
#ifndef __WIRING_FRIENDLYELEC_H__
#define __WIRING_FRIENDLYELEC_H__

/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/
#define	ENV_DEBUG		"WIRINGPI_DEBUG"
#define	ENV_CODES		"WIRINGPI_CODES"
#define	ENV_GPIOMEM		"WIRINGPI_GPIOMEM"

#define	MODEL_UNKNOWN		0
#define	MODEL_NANOPC_T4		1
#define MODEL_NANOPI_M4         2
#define MODEL_NANOPI_NEO4       3

#define	MAKER_UNKNOWN		0
#define	MAKER_AMLOGIC		1
#define	MAKER_SAMSUNG		2
#define	MAKER_ROCKCHIP		3

#define	MODE_PINS		0
#define	MODE_GPIO		1
#define	MODE_GPIO_SYS		2
#define	MODE_PHYS		3
#define	MODE_PIFACE		4
#define	MODE_UNINITIALISED	-1

// Pin modes
#define	INPUT			0
#define	OUTPUT			1
#define	PWM_OUTPUT		2
#define	GPIO_CLOCK		3
#define	SOFT_PWM_OUTPUT		4
#define	SOFT_TONE_OUTPUT	5
#define	PWM_TONE_OUTPUT		6

#define	LOW			0
#define	HIGH			1

// Pull up/down/none
#define	PUD_OFF			0
#define	PUD_DOWN		1
#define	PUD_UP			2

// Module names
#define AML_MODULE_I2C		"aml_i2c"

/*----------------------------------------------------------------------------*/
#define	PAGE_SIZE		(4*1024)
#define	BLOCK_SIZE		(4*1024)

/*----------------------------------------------------------------------------*/
/* Debuf message display function */
/*----------------------------------------------------------------------------*/
#define	MSG_ERR		-1
#define	MSG_WARN	-2

/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
// Export function define
/*----------------------------------------------------------------------------*/
extern	int msg (int type, const char *message, ...);
extern	int moduleLoaded(char *);

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
struct libfriendlyelec
{
	/* H/W model info */
	int	model, rev, mem, maker;

	/* wiringPi init Mode */
	int	mode;

	/* wiringPi core func */
	int	(*getModeToGpio)	(int mode, int pin);
	void	(*pinMode)		(int pin, int mode);
	int	(*getAlt)		(int pin);
	void	(*pullUpDnControl)	(int pin, int pud);
	int	(*digitalRead)		(int pin);
	void	(*digitalWrite)		(int pin, int value);
	int	(*analogRead)		(int pin);
	void	(*digitalWriteByte)	(const int value);
	unsigned int (*digitalReadByte)	(void);

	/* ISR Function pointer */
	void 	(*isrFunctions[256])(void);

	/* GPIO sysfs file discripter */
	int 	sysFds[256];

	/* GPIO pin base number */
	int	pinBase;

	// Time for easy calculations
	uint64_t epochMilli, epochMicro ;
};

union	reg_bitfield {
	unsigned int	wvalue;
	struct {
		unsigned int	bit0  : 1;
		unsigned int	bit1  : 1;
		unsigned int	bit2  : 1;
		unsigned int	bit3  : 1;
		unsigned int	bit4  : 1;
		unsigned int	bit5  : 1;
		unsigned int	bit6  : 1;
		unsigned int	bit7  : 1;
		unsigned int	bit8  : 1;
		unsigned int	bit9  : 1;
		unsigned int	bit10 : 1;
		unsigned int	bit11 : 1;
		unsigned int	bit12 : 1;
		unsigned int	bit13 : 1;
		unsigned int	bit14 : 1;
		unsigned int	bit15 : 1;
		unsigned int	bit16 : 1;
		unsigned int	bit17 : 1;
		unsigned int	bit18 : 1;
		unsigned int	bit19 : 1;
		unsigned int	bit20 : 1;
		unsigned int	bit21 : 1;
		unsigned int	bit22 : 1;
		unsigned int	bit23 : 1;
		unsigned int	bit24 : 1;
		unsigned int	bit25 : 1;
		unsigned int	bit26 : 1;
		unsigned int	bit27 : 1;
		unsigned int	bit28 : 1;
		unsigned int	bit29 : 1;
		unsigned int	bit30 : 1;
		unsigned int	bit31 : 1;
	} bits;
};

/*----------------------------------------------------------------------------*/
#endif	/* __WIRING_FRIENDLYELEC_H__ */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
