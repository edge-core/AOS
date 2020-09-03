/*********************************************************************************
* Lattice Semiconductor Corp. Copyright 2000-2008
* 
* This is the hardware.c of ispVME V12.1 for JTAG programmable devices.
* All the functions requiring customization are organized into this file for 
* the convinience of porting. 
*********************************************************************************/
/*********************************************************************************
* Revision History:
* 
* 09/11/07 NN Type cast mismatch variables
* 09/24/07 NN Added calibration function.
*             Calibration will help to determine the system clock frequency
*             and the count value for one micro-second delay of the target 
*             specific hardware.
*             Modified the ispVMDelay function
*             Removed Delay Percent support
*             Moved the sclock() function from ivm_core.c to hardware.c
*********************************************************************************/

#include "phyaddr_access.h"
#include "backdoor_mgr.h"
#include "sys_hwcfg.h"

/********************************************************************************
* Declaration of global variables 
*
*********************************************************************************/

unsigned char  g_siIspPins        = SYS_HWCFG_CPLD_ISP_PIN;   /*Keeper of JTAG pin state*/
SYS_TYPE_PAddr_T g_usInPort       = SYS_HWCFG_CPLD_INPORT_ADDR;  /*Address of the TDO pin*/
SYS_TYPE_PAddr_T g_usOutPort      = SYS_HWCFG_CPLD_OUTPORT_ADDR;  /*Address of TDI, TMS, TCK pin*/
unsigned short g_usCpu_Frequency  = SYS_HWCFG_CPLD_CPU_FREQUENCY;  /*Enter your CPU frequency here, unit in MHz.*/

/*********************************************************************************
* This is the definition of the bit locations of each respective
* signal in the global variable g_siIspPins.
*
* NOTE: Users must add their own implementation here to define
*       the bit location of the signal to target their hardware.
*       The example below is for the Lattice download cable on
*       on the parallel port.
*
*********************************************************************************/

const unsigned char g_ucPinTDI          = SYS_HWCFG_CPLD_PIN_TDI_MASK;    /* Bit address of TDI */
const unsigned char g_ucPinTCK          = SYS_HWCFG_CPLD_PIN_TCK_MASK;    /* Bit address of TCK */
const unsigned char g_ucPinTMS          = SYS_HWCFG_CPLD_PIN_TMS_MASK;    /* Bit address of TMS */
const unsigned char g_ucPinENABLE       = SYS_HWCFG_CPLD_PIN_ENABLE;    /* Bit address of ENABLE */
const unsigned char g_ucPinTRST         = SYS_HWCFG_CPLD_PIN_TRST_MASK;    /* Bit address of TRST */
const unsigned char g_ucPinTDO          = SYS_HWCFG_CPLD_PIN_TDO_MASK;    /* Bit address of TDO*/

//#define PIN_TCK 0x00000200
//#define PIN_TMS 0x00010000
//#define PIN_TDI 0x00020000
//#define PIN_TDO 0x00000400
//#define GPIO_PDIRD (*(volatile unsigned int *)(0xf0010D60))
//#define GPIO_PPARD (*(volatile unsigned int *)(0xf0010D64))
//#define GPIO_POSRD (*(volatile unsigned int *)(0xf0010D68))
//#define GPIO_PODRD (*(volatile unsigned int *)(0xf0010D6C))
//#define GPIO_PDATD (*(volatile unsigned int *)(0xf0010D70))
/***************************************************************
*
* Functions declared in hardware.c module.
*
***************************************************************/
void writePort( unsigned char a_ucPins, unsigned char a_ucValue );
unsigned char readPort();
void sclock();
void ispVMDelay( unsigned short a_usTimeDelay );
void calibration(void);

/********************************************************************************
* writePort
* To apply the specified value to the pins indicated. This routine will
* be modified for specific systems. 
* As an example, this code uses the IBM-PC standard Parallel port, along with the
* schematic shown in Lattice documentation, to apply the signals to the
* JTAG pins.
*
* PC Parallel port pin    Signal name           Port bit address
*        2                g_ucPinTDI             1
*        3                g_ucPinTCK             2
*        4                g_ucPinTMS             4
*        5                g_ucPinENABLE          8
*        6                g_ucPinTRST            16
*        10               g_ucPinTDO             64
*                     
*  Parameters:
*   - a_ucPins, which is actually a set of bit flags (defined above)
*     that correspond to the bits of the data port. Each of the I/O port
*     bits that drives an isp programming pin is assigned a flag 
*     (through a #define) corresponding to the signal it drives. To 
*     change the value of more than one pin at once, the flags are added 
*     together, much like file access flags are.
*
*     The bit flags are only set if the pin is to be changed. Bits that 
*     do not have their flags set do not have their levels changed. The 
*     state of the port is always manintained in the static global 
*     variable g_siIspPins, so that each pin can be addressed individually 
*     without disturbing the others.
*
*   - a_ucValue, which is either HIGH (0x01 ) or LOW (0x00 ). Only these two
*     values are valid. Any non-zero number sets the pin(s) high.
*
*********************************************************************************/

void writePort( unsigned char a_ucPins, unsigned char a_ucValue )
{
    unsigned char ucValue;
    SYS_TYPE_VAddr_T virt_addr;

    //ucValue = *((unsigned char *)(g_usOutPort));
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(g_usOutPort, &virt_addr))
    {
        BACKDOOR_MGR_Printf("get virtualaddr error, %x %x\n", g_usOutPort, virt_addr);
        return;
    }


    if(FALSE==PHYADDR_ACCESS_Read(virt_addr, 1, 1, &ucValue))
    {
        BACKDOOR_MGR_Printf("Read error, %x %x\n", g_usOutPort, virt_addr);
        return;
    }

    if ( a_ucValue ) {
        g_siIspPins = (unsigned char) (a_ucPins | ucValue );
    }
    else {
        g_siIspPins = (unsigned char) ((~a_ucPins) & ucValue );
    }

    /* This is a sample code for Windows/DOS without Windows Driver.
	_outp( g_usOutPort, g_siIspPins );
	*/
	//*((unsigned char *)g_usOutPort) = g_siIspPins;

    if(FALSE==PHYADDR_ACCESS_Write(virt_addr, 1 , 1 , &g_siIspPins))
    {
        BACKDOOR_MGR_Printf("Write error, %x %x %x\n", g_usOutPort, virt_addr, g_siIspPins);
        return;
    }
}

/*********************************************************************************
*
* readPort
*
* Returns the value of the TDO from the device.
*
**********************************************************************************/
unsigned char readPort()
{
    unsigned char ucValue;
    unsigned char ucRet;
    SYS_TYPE_VAddr_T virt_addr;
	/* This is a sample code for Windows/DOS

	if ( _inp( g_usInPort ) & g_ucPinTDO ) {
		ucRet = 0x01;
    }
	else {
       ucRet = 0x00;
    }
	*/
    //ucValue = *((unsigned char *)(g_usInPort));
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(g_usInPort, &virt_addr))
    {   
        BACKDOOR_MGR_Printf("get virtualaddr error, %x %x\n", g_usInPort, virt_addr);
        return;
    }

    if(FALSE==PHYADDR_ACCESS_Read(virt_addr, 1, 1, &ucValue))
    {
        BACKDOOR_MGR_Printf("Read error, %x %x %x\n", g_usInPort, virt_addr, g_siIspPins);
        return;
    }

    if ( ucValue & g_ucPinTDO )
      ucRet = 0x01;
    else
      ucRet = 0x00; 

    return ( ucRet );
} 

/*********************************************************************************
* sclock
*
* Apply a pulse to TCK.
*
* This function is located here so that users can modify to slow down TCK if
* it is too fast (> 25MHZ). Users can change the IdleTime assignment from 0 to 
* 1, 2... to effectively slowing down TCK by half, quarter...
*
*********************************************************************************/
void sclock()
{	
	unsigned short IdleTime    = 0; //change to > 0 if need to slow down TCK
	unsigned short usIdleIndex = 0;
	IdleTime++;
	for ( usIdleIndex = 0; usIdleIndex < IdleTime; usIdleIndex++ ) {
		writePort( g_ucPinTCK, 0x01 );
	}
	for ( usIdleIndex = 0; usIdleIndex < IdleTime; usIdleIndex++ ) { 
		writePort( g_ucPinTCK, 0x00 );
	}
}
/********************************************************************************
*
* ispVMDelay
*
*
* Users must implement a delay to observe a_usTimeDelay, where
* bit 15 of the a_usTimeDelay defines the unit.
*      1 = milliseconds
*      0 = microseconds
* Example:
*      a_usTimeDelay = 0x0001 = 1 microsecond delay.
*      a_usTimeDelay = 0x8001 = 1 millisecond delay.
*
* This subroutine is called upon to provide a delay from 1 millisecond to a few 
* hundreds milliseconds each time. 
* It is understood that due to a_usTimeDelay is defined as unsigned short, a 16 bits
* integer, this function is restricted to produce a delay to 64000 micro-seconds 
* or 32000 milli-second maximum. The VME file will never pass on to this function
* a delay time > those maximum number. If it needs more than those maximum, the VME
* file will launch the delay function several times to realize a larger delay time
* cummulatively.
* It is perfectly alright to provide a longer delay than required. It is not 
* acceptable if the delay is shorter.
*
* Delay function example--using the machine clock signal of the native CPU------
* When porting ispVME to a native CPU environment, the speed of CPU or 
* the system clock that drives the CPU is usually known. 
* The speed or the time it takes for the native CPU to execute one for loop 
* then can be calculated as follows:
*       The for loop usually is compiled into the ASSEMBLY code as shown below:
*       LOOP: DEC RA;
*             JNZ LOOP;
*       If each line of assembly code needs 4 machine cycles to execute, 
*       the total number of machine cycles to execute the loop is 2 x 4 = 8.
*       Usually system clock = machine clock (the internal CPU clock). 
*       Note: Some CPU has a clock multiplier to double the system clock for 
              the machine clock.
*
*       Let the machine clock frequency of the CPU be F, or 1 machine cycle = 1/F.
*       The time it takes to execute one for loop = (1/F ) x 8.
*       Or one micro-second = F(MHz)/8;
*
* Example: The CPU internal clock is set to 100Mhz, then one micro-second = 100/8 = 12
*
* The C code shown below can be used to create the milli-second accuracy. 
* Users only need to enter the speed of the cpu.
*
**********************************************************************************/
void ispVMDelay( unsigned short a_usTimeDelay )
{
	unsigned short loop_index     = 0;
	unsigned short ms_index       = 0;
	unsigned short us_index       = 0;
    unsigned short useless;

	if ( a_usTimeDelay & 0x8000 ) /*Test for unit*/
	{
		a_usTimeDelay &= ~0x8000; /*unit in milliseconds*/
	}
	else { /*unit in microseconds*/
		a_usTimeDelay = (unsigned short) (a_usTimeDelay/1000); /*convert to milliseconds*/
		if ( a_usTimeDelay <= 0 ) {
			 a_usTimeDelay = 1; /*delay is 1 millisecond minimum*/
		}
	}
	/*Users can replace the following section of code by their own*/
	for( ms_index = 0; ms_index < a_usTimeDelay; ms_index++)
	{
		/*Loop 1000 times to produce the milliseconds delay*/
		for (us_index = 0; us_index < 1000; us_index++)
		{ /*each loop should delay for 1 microsecond or more.*/
			loop_index = 0;
			do {
				/*The NOP fakes the optimizer out so that it doesn't toss out the loop code entirely*/
				//	asm NOP
				useless = loop_index; 
			}while (loop_index++ < ((g_usCpu_Frequency/8)+(+ ((g_usCpu_Frequency % 8) ? 1 : 0))));/*use do loop to force at least one loop*/
		}
	}
}

/*********************************************************************************
*
* calibration
*
* It is important to confirm if the delay function is indeed providing 
* the accuracy required. Also one other important parameter needed 
* checking is the clock frequency. 
* Calibration will help to determine the system clock frequency 
* and the loop_per_micro value for one micro-second delay of the target 
* specific hardware.
*              
**********************************************************************************/
void calibration(void)
{
	/*Apply 2 pulses to TCK.*/
	writePort( g_ucPinTCK, 0x00 );
	writePort( g_ucPinTCK, 0x01 );
	writePort( g_ucPinTCK, 0x00 );
	writePort( g_ucPinTCK, 0x01 );
	writePort( g_ucPinTCK, 0x00 );

	/*Delay for 1 millisecond. Pass on 1000 or 0x8001 both = 1ms delay.*/
	ispVMDelay(0x8001);

	/*Apply 2 pulses to TCK*/
	writePort( g_ucPinTCK, 0x01 );
	writePort( g_ucPinTCK, 0x00 );
	writePort( g_ucPinTCK, 0x01 );
	writePort( g_ucPinTCK, 0x00 );
}
