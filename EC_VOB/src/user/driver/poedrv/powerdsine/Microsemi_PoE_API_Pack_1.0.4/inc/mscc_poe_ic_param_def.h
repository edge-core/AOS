/*************************************************************************
 *
 *            Copyright (c) 2008 by Microsemi Corp. Inc.
 *
 *  This software is copyrighted by, and is the sole property of Microsemi
 *  Corp. All rights, title, ownership, or other interests in the
 *  software  remain the property of Microsemi Corp. This software
 *  may only be used in accordance with the corresponding license
 *  agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice may not be removed or modified without prior
 *  written consent of Microsemi Corp.
 *
 *  Microsemi Corp. reserves the right to modify this software without
 *  notice. 
 *
 *************************************************************************
 *
 *  File Revision: 1.0   
 *  File        : Rotem_sw_defines 
 *  Author      : YakiD
 *  Created     : Mon Jan  1 09:15:41     2007
 *  Last update : The current date is: Thu 07/17/2008
 *  Enter the new date: (mm-dd-yy)  
 * 
 *************************************************************************  
 *
 *  Description: Title      :  PoE_IC_sw_defines.cxgate
 *			     Project    :  690xx
 *
 *************************************************************************/

/*#############################################################################
/                   !!!!!!!!! DO NOT CHANGE !!!!!!!!!
/#############################################################################*/

#ifndef _MSCC_POE_IC_PARAMETERS_DEFINITION_H
	#define _MSCC_POE_IC_PARAMETERS_DEFINITION_H
	
	#ifdef __cplusplus
		extern "C" {
	#endif	


	/*=========================================================================
	/ INCLUDES
	/========================================================================*/
	#include "mscc_poe_global_types.h"

/* #############################################################
/ Register CFGC_PWRGD
/ power good 2 pin. The pin is muxed ,  see - CFGC_IOSEL
/ reset value : CFGC_PWRGD = 000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pwrgd0_n : 1;
      U16 pwrgd1_n : 1;
      U16 pwrgd2_n : 1;
      U16 reserved : 13;
  }Bits;
}tCFGC_PWRGD;


#define CFGC_PWRGD_ADDR 0x300
#define CFGC_PWRGD _CFGC_PWRGD.Word
#define CFGC_PWRGD_PWRGD0_N _CFGC_PWRGD.Bits.pwrgd0_n
#define CFGC_PWRGD_PWRGD1_N _CFGC_PWRGD.Bits.pwrgd1_n
#define CFGC_PWRGD_PWRGD2_N _CFGC_PWRGD.Bits.pwrgd2_n


/* #############################################################
/ Register CFGC_PWRGDDIF
/ Indicates change of power good0 pin (from 0->1 or 1->0). Any change will cause an irq assertion (if not masked)
/ reset value : CFGC_PWRGDDIF = 000
/ ############################################################# */

typedef union
{
  U16 Word;
  struct
  {
      U16 pwrgd0_n_diff : 1;
      U16 pwrgd1_n_diff : 1;
      U16 pwrgd2_n_diff : 1;
      U16 reserved : 13;
  }Bits;
}tCFGC_PWRGDDIF;


#define CFGC_PWRGDDIF_ADDR 0x302
#define CFGC_PWRGDDIF _CFGC_PWRGDDIF.Word
#define CFGC_PWRGDDIF_PWRGD0_N_DIFF _CFGC_PWRGDDIF.Bits.pwrgd0_n_diff
#define CFGC_PWRGDDIF_PWRGD1_N_DIFF _CFGC_PWRGDDIF.Bits.pwrgd1_n_diff
#define CFGC_PWRGDDIF_PWRGD2_N_DIFF _CFGC_PWRGDDIF.Bits.pwrgd2_n_diff


/* #############################################################
/ Register CFGC_MPWRGD
/ IRQ indication of Power supply2 status
/ reset value : CFGC_MPWRGD = 111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 mask_pwrgd0 : 1;
      U16 mask_pwrgd1 : 1;
      U16 mask_pwrgd2 : 1;
      U16 reserved : 13;
  }Bits;
}tCFGC_MPWRGD;


#define CFGC_MPWRGD_ADDR 0x304
#define CFGC_MPWRGD _CFGC_MPWRGD.Word
#define CFGC_MPWRGD_MASK_PWRGD0 _CFGC_MPWRGD.Bits.mask_pwrgd0
#define CFGC_MPWRGD_MASK_PWRGD1 _CFGC_MPWRGD.Bits.mask_pwrgd1
#define CFGC_MPWRGD_MASK_PWRGD2 _CFGC_MPWRGD.Bits.mask_pwrgd2


/* #############################################################
/ Register CFGC_DISPRTS
/ Disable ports indication. Input to the chip
/ #############################################################*/
#define CFGC_DISPRTS_ADDR 0x306
/* #############################################################
/ Register CFGC_GPIO0
/ General purpose IO 0 out enable. The pin is muxed ,  see - CFGC_IOSEL
/ reset value : CFGC_GPIO0 = 000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 gpio0_in : 1;
      U16 gpio0_out : 1;
      U16 gpio0_oe : 1;
      U16 reserved : 13;
  }Bits;
}tCFGC_GPIO0;


#define CFGC_GPIO0_ADDR 0x308
#define CFGC_GPIO0 _CFGC_GPIO0.Word
#define CFGC_GPIO0_GPIO0_IN _CFGC_GPIO0.Bits.gpio0_in
#define CFGC_GPIO0_GPIO0_OUT _CFGC_GPIO0.Bits.gpio0_out
#define CFGC_GPIO0_GPIO0_OE _CFGC_GPIO0.Bits.gpio0_oe


/* #############################################################
/ Register CFGC_GPIO1
/ General purpose IO 1 out enable. The pin is muxed ,  see - CFGC_IOSEL.
/ reset value : CFGC_GPIO1 = 000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 gpio1_in : 1;
      U16 gpio1_out : 1;
      U16 gpio1_oe : 1;
      U16 reserved : 13;
  }Bits;
}tCFGC_GPIO1;


#define CFGC_GPIO1_ADDR 0x30A
#define CFGC_GPIO1 _CFGC_GPIO1.Word
#define CFGC_GPIO1_GPIO1_IN _CFGC_GPIO1.Bits.gpio1_in
#define CFGC_GPIO1_GPIO1_OUT _CFGC_GPIO1.Bits.gpio1_out
#define CFGC_GPIO1_GPIO1_OE _CFGC_GPIO1.Bits.gpio1_oe


/* #############################################################
/ Register CFGC_GPIO2
/ General purpose IO 2 out enable. The pin is muxed ,  see - CFGC_IOSEL.
/ reset value : CFGC_GPIO2 = 000
/ ############################################################# */

typedef union
{
  U16 Word;
  struct
  {
      U16 gpio2_in : 1;
      U16 gpio2_out : 1;
      U16 gpio2_oe : 1;
      U16 reserved : 13;
  }Bits;
}tCFGC_GPIO2;


#define CFGC_GPIO2_ADDR 0x30C
#define CFGC_GPIO2 _CFGC_GPIO2.Word
#define CFGC_GPIO2_GPIO2_IN _CFGC_GPIO2.Bits.gpio2_in
#define CFGC_GPIO2_GPIO2_OUT _CFGC_GPIO2.Bits.gpio2_out
#define CFGC_GPIO2_GPIO2_OE _CFGC_GPIO2.Bits.gpio2_oe


/* #############################################################
/ Register CFGC_GPIO3
/ General purpose IO 3 out enable. The pin is muxed ,  see - CFGC_IOSEL.
/ reset value : CFGC_GPIO3 = 000
/ ############################################################# */

typedef union
{
  U16 Word;
  struct
  {
      U16 gpio3_in : 1;
      U16 gpio3_out : 1;
      U16 gpio3_oe : 1;
      U16 reserved : 13;
  }Bits;
}tCFGC_GPIO3;


#define CFGC_GPIO3_ADDR 0x30E
#define CFGC_GPIO3 _CFGC_GPIO3.Word
#define CFGC_GPIO3_GPIO3_IN _CFGC_GPIO3.Bits.gpio3_in
#define CFGC_GPIO3_GPIO3_OUT _CFGC_GPIO3.Bits.gpio3_out
#define CFGC_GPIO3_GPIO3_OE _CFGC_GPIO3.Bits.gpio3_oe


/* #############################################################
/ Register CFGC_GPIO4
/ General purpose IO 4 out enable. The pin is muxed ,  see - CFGC_IOSEL.
/ reset value : CFGC_GPIO4 = 000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 gpio4_in : 1;
      U16 gpio4_out : 1;
      U16 gpio4_oe : 1;
      U16 reserved : 13;
  }Bits;
}tCFGC_GPIO4;


#define CFGC_GPIO4_ADDR 0x310
#define CFGC_GPIO4 _CFGC_GPIO4.Word
#define CFGC_GPIO4_GPIO4_IN _CFGC_GPIO4.Bits.gpio4_in
#define CFGC_GPIO4_GPIO4_OUT _CFGC_GPIO4.Bits.gpio4_out
#define CFGC_GPIO4_GPIO4_OE _CFGC_GPIO4.Bits.gpio4_oe


/* #############################################################
/ Register CFGC_IOSEL
/ IO Mux
/ reset value : CFGC_IOSEL = 111000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 iocfg0 : 1;
      U16 iocfg1 : 1;
      U16 iocfg2 : 1;
      U16 iocfg3 : 1;
      U16 iocfg4 : 1;
      U16 iocfg5 : 1;
      U16 reserved : 10;
  }Bits;
}tCFGC_IOSEL;


#define CFGC_IOSEL_ADDR 0x312
#define CFGC_IOSEL _CFGC_IOSEL.Word
#define CFGC_IOSEL_IOCFG0 _CFGC_IOSEL.Bits.iocfg0
#define CFGC_IOSEL_IOCFG1 _CFGC_IOSEL.Bits.iocfg1
#define CFGC_IOSEL_IOCFG2 _CFGC_IOSEL.Bits.iocfg2
#define CFGC_IOSEL_IOCFG3 _CFGC_IOSEL.Bits.iocfg3
#define CFGC_IOSEL_IOCFG4 _CFGC_IOSEL.Bits.iocfg4
#define CFGC_IOSEL_IOCFG5 _CFGC_IOSEL.Bits.iocfg5


/* #############################################################
/ Register CFGC_CHCLK
/ Select chopper amplifier clock 
/ #############################################################*/
#define CFGC_CHCLK_ADDR 0x314
/* #############################################################
/ Register CFGC_PDPSE
/ The bit is connected directly to the pd-pse communication signal
/ #############################################################*/
#define CFGC_PDPSE_ADDR 0x316
/* #############################################################
/ Register CFGC_USRREG
/ User Data
/ #############################################################*/
#define CFGC_USRREG_ADDR 0x318
/* #############################################################
/ Register CFGC_ICVER
/ HW Version
/ #############################################################*/
#define CFGC_ICVER_ADDR 0x31A
/* #############################################################
/ Register CFGC_RAMPROT
/ Ram area protection - RAM high address (the protection starts at 0x1000) - addr[12:1]
/ #############################################################*/
#define CFGC_RAMPROT_ADDR 0x31C
/* #############################################################
/ Register CFGC_PROTKEY
/ write enable key
/ #############################################################*/
#define CFGC_PROTKEY_ADDR 0x31E
/* #############################################################
/ Register CFGC_MAN
/ Manual mode
/ #############################################################*/
#define CFGC_MAN_ADDR 0x320
/* #############################################################
/ Register CFGC_EXTIE
/ Event 15 indication
/ reset value : CFGC_EXTIE = 0000000000000000
/ #############################################################*/

#define STARTUP_CFG 0x11A8

typedef union
{
  U16 Word;
  struct
  {
      U16 event_0 : 1;
      U16 event_1 : 1;
      U16 event_2 : 1;
      U16 event_3 : 1;
      U16 event_4 : 1;
      U16 event_5 : 1;
      U16 event_6 : 1;
      U16 event_7 : 1;
      U16 event_8 : 1;
      U16 event_9 : 1;
      U16 event_10 : 1;
      U16 event_11 : 1;
      U16 event_12 : 1;
      U16 event_13 : 1;
      U16 event_14 : 1;
      U16 event_15 : 1;
  }Bits;
}tCFGC_EXTIE;


#define CFGC_EXTIE_ADDR 0x324
#define CFGC_EXTIE _CFGC_EXTIE.Word
#define CFGC_EXTIE_EVENT_0 _CFGC_EXTIE.Bits.event_0
#define CFGC_EXTIE_EVENT_1 _CFGC_EXTIE.Bits.event_1
#define CFGC_EXTIE_EVENT_2 _CFGC_EXTIE.Bits.event_2
#define CFGC_EXTIE_EVENT_3 _CFGC_EXTIE.Bits.event_3
#define CFGC_EXTIE_EVENT_4 _CFGC_EXTIE.Bits.event_4
#define CFGC_EXTIE_EVENT_5 _CFGC_EXTIE.Bits.event_5
#define CFGC_EXTIE_EVENT_6 _CFGC_EXTIE.Bits.event_6
#define CFGC_EXTIE_EVENT_7 _CFGC_EXTIE.Bits.event_7
#define CFGC_EXTIE_EVENT_8 _CFGC_EXTIE.Bits.event_8
#define CFGC_EXTIE_EVENT_9 _CFGC_EXTIE.Bits.event_9
#define CFGC_EXTIE_EVENT_10 _CFGC_EXTIE.Bits.event_10
#define CFGC_EXTIE_EVENT_11 _CFGC_EXTIE.Bits.event_11
#define CFGC_EXTIE_EVENT_12 _CFGC_EXTIE.Bits.event_12
#define CFGC_EXTIE_EVENT_13 _CFGC_EXTIE.Bits.event_13
#define CFGC_EXTIE_EVENT_14 _CFGC_EXTIE.Bits.event_14
#define CFGC_EXTIE_EVENT_15 _CFGC_EXTIE.Bits.event_15


/* #############################################################
/ Register CFGC_RSRVDOUT
/ reserved for future use
/ #############################################################*/
#define CFGC_RSRVDOUT_ADDR 0x326
/* #############################################################
/ Register CFGC_RSRVDIN
/ reserved for future use
/ #############################################################*/
#define CFGC_RSRVDIN_ADDR 0x328
/* #############################################################
/ Register SPIC_CR
/ Re transmit the received data in slave mode
/ reset value : SPIC_CR = 00001
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 espi : 1;
      U16 ms_cfg : 1;
      U16 com_en : 1;
      U16 cont_cs : 1;
      U16 circ_data : 1;
      U16 reserved : 11;
  }Bits;
}tSPIC_CR;


#define SPIC_CR_ADDR 0x100
#define SPIC_CR _SPIC_CR.Word
#define SPIC_CR_ESPI _SPIC_CR.Bits.espi
#define SPIC_CR_MS_CFG _SPIC_CR.Bits.ms_cfg
#define SPIC_CR_COM_EN _SPIC_CR.Bits.com_en
#define SPIC_CR_CONT_CS _SPIC_CR.Bits.cont_cs
#define SPIC_CR_CIRC_DATA _SPIC_CR.Bits.circ_data


/* #############################################################
/ Register SPIC_SR
/ Transaction enable - start transaction
/ #############################################################*/
#define SPIC_SR_ADDR 0x102
/* #############################################################
/ Register SPIC_INVADDR
/ Indication of invalid address access. For example - access to a byte (odd address)
/ #############################################################*/
#define SPIC_INVADDR_ADDR 0x104
/* #############################################################
/ Register SPIC_NOWORD
/ number of words to transmit/receive
/ #############################################################*/
#define SPIC_NOWORD_ADDR 0x106
/* #############################################################
/ Register SPIC_CTRL
/ Type of access
/ reset value : SPIC_CTRL = 0000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 ic_addr : 6;
      U16 rw : 1;
      U16 reserved : 9;
  }Bits;
}tSPIC_CTRL;


#define SPIC_CTRL_ADDR 0x108
#define SPIC_CTRL _SPIC_CTRL.Word
#define SPIC_CTRL_IC_ADDR _SPIC_CTRL.Bits.ic_addr
#define SPIC_CTRL_RW _SPIC_CTRL.Bits.rw


/* #############################################################
/ Register SPIC_INTADD
/ Internal address.
/ #############################################################*/
#define SPIC_INTADD_ADDR 0x10A
/* #############################################################
/ Register SPIC_IRQ
/ SPIC IRQ Indication
/ #############################################################*/
#define SPIC_IRQ_ADDR 0x10C
/* #############################################################
/ Register I2CC_CR
/ Time Out Error Enable - enable the Time Out Error feature. If no communication is received for 10ms from the master ,  in between the access xxx   - the slave ignores the access.
/ reset value : I2CC_CR = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 slv_addr : 7;
      U16 com_en : 1;
      U16 toe_en : 1;
      U16 reserved : 7;
  }Bits;
}tI2CC_CR;


#define I2CC_CR_ADDR 0x140
#define I2CC_CR _I2CC_CR.Word
#define I2CC_CR_SLV_ADDR _I2CC_CR.Bits.slv_addr
#define I2CC_CR_COM_EN _I2CC_CR.Bits.com_en
#define I2CC_CR_TOE_EN _I2CC_CR.Bits.toe_en


/* #############################################################
/ Register I2CC_SR
/ Indication of invalid address access. For example - access to a byte (odd address)
/ reset value : I2CC_SR = 00
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 toe_ind : 1;
      U16 invalid_addr : 1;
      U16 reserved : 14;
  }Bits;
}tI2CC_SR;


#define I2CC_SR_ADDR 0x142
#define I2CC_SR _I2CC_SR.Word
#define I2CC_SR_TOE_IND _I2CC_SR.Bits.toe_ind
#define I2CC_SR_INVALID_ADDR _I2CC_SR.Bits.invalid_addr


/* #############################################################
/ Register I2CC_IRQ
/ I2CC IRQ - Due to access to a predefined fixed address TBD (SPIC irq and I2CC irq - caused by different addresses)
/ #############################################################*/
#define I2CC_IRQ_ADDR 0x144
/* #############################################################
/ Register LSD_CR
/ data ready to transmit and transmission end indication
/ #############################################################*/
#define LSD_CR_ADDR 0x200
/* #############################################################
/ Register RTP_PDP
/ Port Devilering Power indication (port is started up)
/ #############################################################*/
#define RTP_PDP_ADDR 0x180
/* #############################################################
/ Register RTP_POFF
/ Request from the analog IF - turn off port N
/ #############################################################*/
#define RTP_POFF_ADDR 0x182
/* #############################################################
/ Register RTP_PUSHOFF
/ Request from the analog IF - turn off port N
/ #############################################################*/
#define RTP_PUSHOFF_ADDR 0x184
/* #############################################################
/ Register RTP_PUSHON
/ Request from the analog IF - turn on port N
/ #############################################################*/
#define RTP_PUSHON_ADDR 0x186
/* #############################################################
/ Register RTP_CR
/ Reset vars while entering SC level1
/ reset value : RTP_CR = 00000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 rtp_en : 1;
      U16 ovl_en : 1;
      U16 sc_en : 1;
      U16 acd_dcd_sel : 1;
      U16 sc_reset_l1_n : 1;
      U16 reserved : 11;
  }Bits;
}tRTP_CR;


#define RTP_CR_ADDR 0x188
#define RTP_CR _RTP_CR.Word
#define RTP_CR_RTP_EN _RTP_CR.Bits.rtp_en
#define RTP_CR_OVL_EN _RTP_CR.Bits.ovl_en
#define RTP_CR_SC_EN _RTP_CR.Bits.sc_en
#define RTP_CR_ACD_DCD_SEL _RTP_CR.Bits.acd_dcd_sel
#define RTP_CR_SC_RESET_L1_N _RTP_CR.Bits.sc_reset_l1_n


/* #############################################################
/ Register RTP_SR
/ RTP slot end indication (sticky bit - asserted at the end/start of the slot)
/ #############################################################*/
#define RTP_SR_ADDR 0x18A
/* #############################################################
/ Register RTP_PTYPE
/ Port type (AF/AT)
/ #############################################################*/
#define RTP_PTYPE_ADDR 0x18C
/* #############################################################
/ Register RTP_VPMESEN
/ Vport measurement enable (Isense>Iref)
/For debug only
/ #############################################################*/
#define RTP_VPMESEN_ADDR 0x18E
/* #############################################################
/ Register RTP_OVLIND
/ Over load indication
/ reset value : RTP_OVLIND = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 ovl_ind_0 : 1;
      U16 ovl_ind_1 : 1;
      U16 ovl_ind_2 : 1;
      U16 ovl_ind_3 : 1;
      U16 ovl_ind_4 : 1;
      U16 ovl_ind_5 : 1;
      U16 ovl_ind_6 : 1;
      U16 ovl_ind_7 : 1;
      U16 ovl_ind_8 : 1;
      U16 ovl_ind_9 : 1;
      U16 ovl_ind_10 : 1;
      U16 ovl_ind_11 : 1;
      U16 reserved : 4;
  }Bits;
}tRTP_OVLIND;


#define RTP_OVLIND_ADDR 0x190
#define RTP_OVLIND _RTP_OVLIND.Word
#define RTP_OVLIND_OVL_IND_0 _RTP_OVLIND.Bits.ovl_ind_0
#define RTP_OVLIND_OVL_IND_1 _RTP_OVLIND.Bits.ovl_ind_1
#define RTP_OVLIND_OVL_IND_2 _RTP_OVLIND.Bits.ovl_ind_2
#define RTP_OVLIND_OVL_IND_3 _RTP_OVLIND.Bits.ovl_ind_3
#define RTP_OVLIND_OVL_IND_4 _RTP_OVLIND.Bits.ovl_ind_4
#define RTP_OVLIND_OVL_IND_5 _RTP_OVLIND.Bits.ovl_ind_5
#define RTP_OVLIND_OVL_IND_6 _RTP_OVLIND.Bits.ovl_ind_6
#define RTP_OVLIND_OVL_IND_7 _RTP_OVLIND.Bits.ovl_ind_7
#define RTP_OVLIND_OVL_IND_8 _RTP_OVLIND.Bits.ovl_ind_8
#define RTP_OVLIND_OVL_IND_9 _RTP_OVLIND.Bits.ovl_ind_9
#define RTP_OVLIND_OVL_IND_10 _RTP_OVLIND.Bits.ovl_ind_10
#define RTP_OVLIND_OVL_IND_11 _RTP_OVLIND.Bits.ovl_ind_11


/* #############################################################
/ Register RTP_SCIND
/ Short Circuit indication
/ reset value : RTP_SCIND = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 sc_ind_0 : 1;
      U16 sc_ind_1 : 1;
      U16 sc_ind_2 : 1;
      U16 sc_ind_3 : 1;
      U16 sc_ind_4 : 1;
      U16 sc_ind_5 : 1;
      U16 sc_ind_6 : 1;
      U16 sc_ind_7 : 1;
      U16 sc_ind_8 : 1;
      U16 sc_ind_9 : 1;
      U16 sc_ind_10 : 1;
      U16 sc_ind_11 : 1;
      U16 reserved : 4;
  }Bits;
}tRTP_SCIND;


#define RTP_SCIND_ADDR 0x192
#define RTP_SCIND _RTP_SCIND.Word
#define RTP_SCIND_SC_IND_0 _RTP_SCIND.Bits.sc_ind_0
#define RTP_SCIND_SC_IND_1 _RTP_SCIND.Bits.sc_ind_1
#define RTP_SCIND_SC_IND_2 _RTP_SCIND.Bits.sc_ind_2
#define RTP_SCIND_SC_IND_3 _RTP_SCIND.Bits.sc_ind_3
#define RTP_SCIND_SC_IND_4 _RTP_SCIND.Bits.sc_ind_4
#define RTP_SCIND_SC_IND_5 _RTP_SCIND.Bits.sc_ind_5
#define RTP_SCIND_SC_IND_6 _RTP_SCIND.Bits.sc_ind_6
#define RTP_SCIND_SC_IND_7 _RTP_SCIND.Bits.sc_ind_7
#define RTP_SCIND_SC_IND_8 _RTP_SCIND.Bits.sc_ind_8
#define RTP_SCIND_SC_IND_9 _RTP_SCIND.Bits.sc_ind_9
#define RTP_SCIND_SC_IND_10 _RTP_SCIND.Bits.sc_ind_10
#define RTP_SCIND_SC_IND_11 _RTP_SCIND.Bits.sc_ind_11


/* #############################################################
/ Register RTP_UDLIND
/ AC/DC Disconnect indication
/ reset value : RTP_UDLIND = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind_0 : 1;
      U16 udl_ind_1 : 1;
      U16 udl_ind_2 : 1;
      U16 udl_ind_3 : 1;
      U16 udl_ind_4 : 1;
      U16 udl_ind_5 : 1;
      U16 udl_ind_6 : 1;
      U16 udl_ind_7 : 1;
      U16 udl_ind_8 : 1;
      U16 udl_ind_9 : 1;
      U16 udl_ind_10 : 1;
      U16 udl_ind_11 : 1;
      U16 reserved : 4;
  }Bits;
}tRTP_UDLIND;


#define RTP_UDLIND_ADDR 0x194
#define RTP_UDLIND _RTP_UDLIND.Word
#define RTP_UDLIND_UDL_IND_0 _RTP_UDLIND.Bits.udl_ind_0
#define RTP_UDLIND_UDL_IND_1 _RTP_UDLIND.Bits.udl_ind_1
#define RTP_UDLIND_UDL_IND_2 _RTP_UDLIND.Bits.udl_ind_2
#define RTP_UDLIND_UDL_IND_3 _RTP_UDLIND.Bits.udl_ind_3
#define RTP_UDLIND_UDL_IND_4 _RTP_UDLIND.Bits.udl_ind_4
#define RTP_UDLIND_UDL_IND_5 _RTP_UDLIND.Bits.udl_ind_5
#define RTP_UDLIND_UDL_IND_6 _RTP_UDLIND.Bits.udl_ind_6
#define RTP_UDLIND_UDL_IND_7 _RTP_UDLIND.Bits.udl_ind_7
#define RTP_UDLIND_UDL_IND_8 _RTP_UDLIND.Bits.udl_ind_8
#define RTP_UDLIND_UDL_IND_9 _RTP_UDLIND.Bits.udl_ind_9
#define RTP_UDLIND_UDL_IND_10 _RTP_UDLIND.Bits.udl_ind_10
#define RTP_UDLIND_UDL_IND_11 _RTP_UDLIND.Bits.udl_ind_11


/* #############################################################
/ Register RTP_UDLEN
/ UDL enable per port
/ #############################################################*/
#define RTP_UDLEN_ADDR 0x196
/* #############################################################
/ Register RTP_ACDEN
/ Mask the ac_source_en_d signal (the signal behaves the same as port_off indication).
/For debug mode.
/ #############################################################*/
#define RTP_ACDEN_ADDR 0x198
/* #############################################################
/ Register RTP_ACDD
/ AC Disconnect data
/ #############################################################*/
#define RTP_ACDD_ADDR 0x19A
/* #############################################################
/ Register RTP_VACCLBPOS
/ Weight of the Vac measurement to be added to Vac.
/ #############################################################*/
#define RTP_VACCLBPOS_ADDR 0x19C
/* #############################################################
/ Register RTP_VACCLBNEG
/ Weight of the Vac measurement to be subtracted to Vac.
/ #############################################################*/
#define RTP_VACCLBNEG_ADDR 0x19E
/* #############################################################
/ Register RTP_ACD2KEN
/ Enable the 2K resistors.
/ #############################################################*/
#define RTP_ACD2KEN_ADDR 0x1A2
/* #############################################################
/ Register RTP_ADCD
/ Data received from the A/D output
/ #############################################################*/
#define RTP_ADCD_ADDR 0x1A4
/* #############################################################
/ Register RTP_ADCCAL
/ The offset value of the RTP A/D in two's complement.
/ #############################################################*/
#define RTP_ADCCAL_ADDR 0x1A6
/* #############################################################
/ Register RTP_ADCCR
/ Configure analog/digital gain select
/ reset value : RTP_ADCCR = 100000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 conv : 1;
      U16 clb : 1;
      U16 avg_dis : 1;
      U16 clb_dis : 1;
      U16 gain_out : 1;
      U16 gain_mode : 1;
      U16 reserved : 10;
  }Bits;
}tRTP_ADCCR;


#define RTP_ADCCR_ADDR 0x1A8
#define RTP_ADCCR _RTP_ADCCR.Word
#define RTP_ADCCR_CONV _RTP_ADCCR.Bits.conv
#define RTP_ADCCR_CLB _RTP_ADCCR.Bits.clb
#define RTP_ADCCR_AVG_DIS _RTP_ADCCR.Bits.avg_dis
#define RTP_ADCCR_CLB_DIS _RTP_ADCCR.Bits.clb_dis
#define RTP_ADCCR_GAIN_OUT _RTP_ADCCR.Bits.gain_out
#define RTP_ADCCR_GAIN_MODE _RTP_ADCCR.Bits.gain_mode


/* #############################################################
/ Register RTP_ADCGAININ
/ Chosen amplifying value � 4/16
/ #############################################################*/
#define RTP_ADCGAININ_ADDR 0x1AA
/* #############################################################
/ Register RTP_ADCSTP
/ Setup delay before issuing real start convert- in 2us units.
/ #############################################################*/
#define RTP_ADCSTP_ADDR 0x1AC
/* #############################################################
/ Register RTP_ADCACPS
/ AC Disconnect Calibration select
/ reset value : RTP_ADCACPS = 0000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 port : 12;
      U16 acd_cal : 1;
      U16 reserved : 3;
  }Bits;
}tRTP_ADCACPS;


#define RTP_ADCACPS_ADDR 0x1AE
#define RTP_ADCACPS _RTP_ADCACPS.Word
#define RTP_ADCACPS_PORT _RTP_ADCACPS.Bits.port
#define RTP_ADCACPS_ACD_CAL _RTP_ADCACPS.Bits.acd_cal


/* #############################################################
/ Register RTP_ADCPS
/ Select vtemp1 to be measured
/ reset value : RTP_ADCPS = 000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 port : 12;
      U16 vmain : 1;
      U16 vtemp0 : 1;
      U16 vtemp1 : 1;
      U16 reserved : 1;
  }Bits;
}tRTP_ADCPS;


#define RTP_ADCPS_ADDR 0x1B0
#define RTP_ADCPS _RTP_ADCPS.Word
#define RTP_ADCPS_PORT _RTP_ADCPS.Bits.port
#define RTP_ADCPS_VMAIN _RTP_ADCPS.Bits.vmain
#define RTP_ADCPS_VTEMP0 _RTP_ADCPS.Bits.vtemp0
#define RTP_ADCPS_VTEMP1 _RTP_ADCPS.Bits.vtemp1


/* #############################################################
/ Register RTP_ADCFS
/ ADC input mux sel
/ #############################################################*/
#define RTP_ADCFS_ADDR 0x1B2
/* #############################################################
/ Register RTP_ACDBG
/ max/min of which port will be written to ram
/ #############################################################*/
#define RTP_ACDBG_ADDR 0x1B4
/* #############################################################
/ Register RTP_SLOTCFG
/ First slot length
/ reset value : RTP_SLOTCFG = 0011000101111111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 dbl : 8;
      U16 first : 8;
  }Bits;
}tRTP_SLOTCFG;


#define RTP_SLOTCFG_ADDR 0x1B6
#define RTP_SLOTCFG _RTP_SLOTCFG.Word
#define RTP_SLOTCFG_DBL _RTP_SLOTCFG.Bits.dbl
#define RTP_SLOTCFG_FIRST _RTP_SLOTCFG.Bits.first


/* #############################################################
/ Register RTP_BBMCFG
/ Length in us of BBM
/ #############################################################*/
#define RTP_BBMCFG_ADDR 0x1B8
/* #############################################################
/ Register RTP_2KMSK
/ Mask the 2K resistor to 0
/ #############################################################*/
#define RTP_2KMSK_ADDR 0x1BA
/* #############################################################
/ Register VPM_CR
/ VPM_EN Indication
/ #############################################################*/
#define VPM_CR_ADDR 0x1C0
/* #############################################################
/ Register VPM_SR
/ VPM Slot error indication - indicated of a slot overflow
/ #############################################################*/
#define VPM_SR_ADDR 0x1C2
/* #############################################################
/ Register VPM_VPMR
/ Request measurement for line detection reference port
/ reset value : VPM_VPMR = 0000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 port : 12;
      U16 ld_ref_port : 1;
      U16 reserved : 3;
  }Bits;
}tVPM_VPMR;


#define VPM_VPMR_ADDR 0x1C4
#define VPM_VPMR _VPM_VPMR.Word
#define VPM_VPMR_PORT _VPM_VPMR.Bits.port
#define VPM_VPMR_LD_REF_PORT _VPM_VPMR.Bits.ld_ref_port


/* #############################################################
/ Register VPM_CDMR
/ Select port to perform CDR on.
/ reset value : VPM_CDMR = 00000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 cdr_en : 1;
      U16 port : 4;
      U16 reserved : 11;
  }Bits;
}tVPM_CDMR;


#define VPM_CDMR_ADDR 0x1C6
#define VPM_CDMR _VPM_CDMR.Word
#define VPM_CDMR_CDR_EN _VPM_CDMR.Bits.cdr_en
#define VPM_CDMR_PORT _VPM_CDMR.Bits.port


/* #############################################################
/ Register VPM_CDREND
/ Indication of cdr end ,  asserted on the last CDR measurement slot
/ #############################################################*/
#define VPM_CDREND_ADDR 0x1C8
/* #############################################################
/ Register VPM_IRQ
/ VPM_IRQ
/ #############################################################*/
#define VPM_IRQ_ADDR 0x1CA
/* #############################################################
/ Register VPM_ADCD
/ Data received from the A/D output
/ #############################################################*/
#define VPM_ADCD_ADDR 0x1CC
/* #############################################################
/ Register VPM_ADCCAL
/ The offset value of the RTP A/D
/ #############################################################*/
#define VPM_ADCCAL_ADDR 0x1CE
/* #############################################################
/ Register VPM_ADCCR
/ Disable calibration � do not calibrate the A/D output according to the offset register
/ reset value : VPM_ADCCR = 0000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 conv : 1;
      U16 clb : 1;
      U16 avg_dis : 1;
      U16 clb_dis : 1;
      U16 reserved : 12;
  }Bits;
}tVPM_ADCCR;


#define VPM_ADCCR_ADDR 0x1D0
#define VPM_ADCCR _VPM_ADCCR.Word
#define VPM_ADCCR_CONV _VPM_ADCCR.Bits.conv
#define VPM_ADCCR_CLB _VPM_ADCCR.Bits.clb
#define VPM_ADCCR_AVG_DIS _VPM_ADCCR.Bits.avg_dis
#define VPM_ADCCR_CLB_DIS _VPM_ADCCR.Bits.clb_dis


/* #############################################################
/ Register VPM_ADCSTP
/ Setup delay before issuing real start convert - in 2us units.
/ #############################################################*/
#define VPM_ADCSTP_ADDR 0x1D2
/* #############################################################
/ Register VPM_ADCFS
/ ADC input mux sel
/ #############################################################*/
#define VPM_ADCFS_ADDR 0x1D4
/* #############################################################
/ Register VPM_ADCPS
/ enable scaling the port voltage measurement (switch from �Line Detection mode� to �dv/dt� mode)
/ reset value : VPM_ADCPS = 000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 vport_sel : 12;
      U16 calib_sel : 1;
      U16 vmain_sel : 1;
      U16 ld_scl : 1;
      U16 reserved : 1;
  }Bits;
}tVPM_ADCPS;


#define VPM_ADCPS_ADDR 0x1D6
#define VPM_ADCPS _VPM_ADCPS.Word
#define VPM_ADCPS_VPORT_SEL _VPM_ADCPS.Bits.vport_sel
#define VPM_ADCPS_CALIB_SEL _VPM_ADCPS.Bits.calib_sel
#define VPM_ADCPS_VMAIN_SEL _VPM_ADCPS.Bits.vmain_sel
#define VPM_ADCPS_LD_SCL _VPM_ADCPS.Bits.ld_scl


/* #############################################################
/ Register VPM_CLST
/ Indication of which channels are under current limitation
/ #############################################################*/
#define VPM_CLST_ADDR 0x1D8
/* #############################################################
/ Register VPM_CSLTDG
/ Current limit state deglitcher length
/ #############################################################*/
#define VPM_CSLTDG_ADDR 0x1DA
/* #############################################################
/ Register VPM_SLOTSIZE
/ VPM Total slot size
/ #############################################################*/
#define VPM_SLOTSIZE_ADDR 0x1DC
/* #############################################################
/ Register VPM_ANLGSTP0
/ Analog set up delay - offset setup
/ #############################################################*/
#define VPM_ANLGSTP0_ADDR 0x1DE
/* #############################################################
/ Register VPM_ANLGSTP1
/ Analog set up delay - port measure setup
/ #############################################################*/
#define VPM_ANLGSTP1_ADDR 0x1E0
/* #############################################################
/ Register AIR_LDEN
/ Enable the reference channel (channel 13) to be line detected.
/ reset value : AIR_LDEN = 0000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 ld_sel : 12;
      U16 ld_clb : 1;
      U16 reserved : 3;
  }Bits;
}tAIR_LDEN;


#define AIR_LDEN_ADDR 0x240
#define AIR_LDEN _AIR_LDEN.Word
#define AIR_LDEN_LD_SEL _AIR_LDEN.Bits.ld_sel
#define AIR_LDEN_LD_CLB _AIR_LDEN.Bits.ld_clb


/* #############################################################
/ Register AIR_LDD
/ Line detection data value. 
/ #############################################################*/
#define AIR_LDD_ADDR 0x242
/* #############################################################
/ Register AIR_AFCRSEL
/ Indication of which channels are AF classified
/ reset value : AIR_AFCRSEL = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 port_0 : 1;
      U16 port_1 : 1;
      U16 port_2 : 1;
      U16 port_3 : 1;
      U16 port_4 : 1;
      U16 port_5 : 1;
      U16 port_6 : 1;
      U16 port_7 : 1;
      U16 port_8 : 1;
      U16 port_9 : 1;
      U16 port_10 : 1;
      U16 port_11 : 1;
      U16 reserved : 4;
  }Bits;
}tAIR_AFCRSEL;


#define AIR_AFCRSEL_ADDR 0x244
#define AIR_AFCRSEL _AIR_AFCRSEL.Word
#define AIR_AFCRSEL_PORT_0 _AIR_AFCRSEL.Bits.port_0
#define AIR_AFCRSEL_PORT_1 _AIR_AFCRSEL.Bits.port_1
#define AIR_AFCRSEL_PORT_2 _AIR_AFCRSEL.Bits.port_2
#define AIR_AFCRSEL_PORT_3 _AIR_AFCRSEL.Bits.port_3
#define AIR_AFCRSEL_PORT_4 _AIR_AFCRSEL.Bits.port_4
#define AIR_AFCRSEL_PORT_5 _AIR_AFCRSEL.Bits.port_5
#define AIR_AFCRSEL_PORT_6 _AIR_AFCRSEL.Bits.port_6
#define AIR_AFCRSEL_PORT_7 _AIR_AFCRSEL.Bits.port_7
#define AIR_AFCRSEL_PORT_8 _AIR_AFCRSEL.Bits.port_8
#define AIR_AFCRSEL_PORT_9 _AIR_AFCRSEL.Bits.port_9
#define AIR_AFCRSEL_PORT_10 _AIR_AFCRSEL.Bits.port_10
#define AIR_AFCRSEL_PORT_11 _AIR_AFCRSEL.Bits.port_11


/* #############################################################
/ Register AIR_ATCRSEL
/ Indication of which channels are AT classified
/ reset value : AIR_ATCRSEL = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 port_0 : 1;
      U16 port_1 : 1;
      U16 port_2 : 1;
      U16 port_3 : 1;
      U16 port_4 : 1;
      U16 port_5 : 1;
      U16 port_6 : 1;
      U16 port_7 : 1;
      U16 port_8 : 1;
      U16 port_9 : 1;
      U16 port_10 : 1;
      U16 port_11 : 1;
      U16 reserved : 4;
  }Bits;
}tAIR_ATCRSEL;


#define AIR_ATCRSEL_ADDR 0x246
#define AIR_ATCRSEL _AIR_ATCRSEL.Word
#define AIR_ATCRSEL_PORT_0 _AIR_ATCRSEL.Bits.port_0
#define AIR_ATCRSEL_PORT_1 _AIR_ATCRSEL.Bits.port_1
#define AIR_ATCRSEL_PORT_2 _AIR_ATCRSEL.Bits.port_2
#define AIR_ATCRSEL_PORT_3 _AIR_ATCRSEL.Bits.port_3
#define AIR_ATCRSEL_PORT_4 _AIR_ATCRSEL.Bits.port_4
#define AIR_ATCRSEL_PORT_5 _AIR_ATCRSEL.Bits.port_5
#define AIR_ATCRSEL_PORT_6 _AIR_ATCRSEL.Bits.port_6
#define AIR_ATCRSEL_PORT_7 _AIR_ATCRSEL.Bits.port_7
#define AIR_ATCRSEL_PORT_8 _AIR_ATCRSEL.Bits.port_8
#define AIR_ATCRSEL_PORT_9 _AIR_ATCRSEL.Bits.port_9
#define AIR_ATCRSEL_PORT_10 _AIR_ATCRSEL.Bits.port_10
#define AIR_ATCRSEL_PORT_11 _AIR_ATCRSEL.Bits.port_11


/* #############################################################
/ Register AIR_ATCLD
/ AT Class data
/ #############################################################*/
#define AIR_ATCLD_ADDR 0x248
/* #############################################################
/ Register AIR_ILIMP0SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP0SEL_ADDR 0x24A
/* #############################################################
/ Register AIR_ILIMP1SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP1SEL_ADDR 0x24C
/* #############################################################
/ Register AIR_ILIMP2SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP2SEL_ADDR 0x24E
/* #############################################################
/ Register AIR_ILIMP3SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP3SEL_ADDR 0x250
/* #############################################################
/ Register AIR_ILIMP4SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP4SEL_ADDR 0x252
/* #############################################################
/ Register AIR_ILIMP5SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP5SEL_ADDR 0x254
/* #############################################################
/ Register AIR_ILIMP6SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP6SEL_ADDR 0x256
/* #############################################################
/ Register AIR_ILIMP7SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP7SEL_ADDR 0x258
/* #############################################################
/ Register AIR_ILIMP8SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP8SEL_ADDR 0x25A
/* #############################################################
/ Register AIR_ILIMP9SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP9SEL_ADDR 0x25C
/* #############################################################
/ Register AIR_ILIMP10SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP10SEL_ADDR 0x25E
/* #############################################################
/ Register AIR_ILIMP11SEL
/ Port current limit 
/ #############################################################*/
#define AIR_ILIMP11SEL_ADDR 0x260
/* #############################################################
/ Register AIR_ILAFD
/ AF current limitation 
/reset value - 425
/ #############################################################*/
#define AIR_ILAFD_ADDR 0x262
/* #############################################################
/ Register AIR_ILATD
/ AT current limitation 
/reset value - 870
/ #############################################################*/
#define AIR_ILATD_ADDR 0x264
/* #############################################################
/ Register AIR_PDC
/ Indication of which port is communicating with the PD. One port can be asserted at a time
/ #############################################################*/
#define AIR_PDC_ADDR 0x266
/* #############################################################
/ Register AIR_RPR
/ RPR iniducation
/ #############################################################*/
#define AIR_RPR_ADDR 0x268
/* #############################################################
/ Register AIR_RPRCMP
/ mask the RPR function
/ #############################################################*/
#define AIR_RPRCMP_ADDR 0x26A
/* #############################################################
/ Register FUSE_ARR0
/ 
/ reset value : FUSE_ARR0 = 0000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 trim_comp_pse1 : 3;
      U16 trim_ilow5_14 : 3;
      U16 trim_ilow_70 : 2;
      U16 trim_vmain : 2;
      U16 trim_temp : 3;
      U16 trim_bg : 3;
  }Bits;
}tFUSE_ARR0;


#define FUSE_ARR0_ADDR 0x340
#define FUSE_ARR0 _FUSE_ARR0.Word
#define FUSE_ARR0_TRIM_COMP_PSE1 _FUSE_ARR0.Bits.trim_comp_pse1
#define FUSE_ARR0_TRIM_ILOW5_14 _FUSE_ARR0.Bits.trim_ilow5_14
#define FUSE_ARR0_TRIM_ILOW_70 _FUSE_ARR0.Bits.trim_ilow_70
#define FUSE_ARR0_TRIM_VMAIN _FUSE_ARR0.Bits.trim_vmain
#define FUSE_ARR0_TRIM_TEMP _FUSE_ARR0.Bits.trim_temp
#define FUSE_ARR0_TRIM_BG _FUSE_ARR0.Bits.trim_bg


/* #############################################################
/ Register FUSE_ARR1
/ 
/ reset value : FUSE_ARR1 = 000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 trim_class_ref : 3;
      U16 trim_adc2p5 : 2;
      U16 trim_calibvport : 2;
      U16 trim_ilimafdac : 3;
      U16 trim_ilimhpdac : 3;
      U16 trim_calibrtp : 2;
      U16 reserved : 1;
  }Bits;
}tFUSE_ARR1;


#define FUSE_ARR1_ADDR 0x342
#define FUSE_ARR1 _FUSE_ARR1.Word
#define FUSE_ARR1_TRIM_CLASS_REF _FUSE_ARR1.Bits.trim_class_ref
#define FUSE_ARR1_TRIM_ADC2P5 _FUSE_ARR1.Bits.trim_adc2p5
#define FUSE_ARR1_TRIM_CALIBVPORT _FUSE_ARR1.Bits.trim_calibvport
#define FUSE_ARR1_TRIM_ILIMAFDAC _FUSE_ARR1.Bits.trim_ilimafdac
#define FUSE_ARR1_TRIM_ILIMHPDAC _FUSE_ARR1.Bits.trim_ilimhpdac
#define FUSE_ARR1_TRIM_CALIBRTP _FUSE_ARR1.Bits.trim_calibrtp


/* #############################################################
/ Register FUSE_ARR2
/ 
/ reset value : FUSE_ARR2 = 0000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 rsrvd : 2;
      U16 trim_config : 3;
      U16 trim_port_gain : 3;
      U16 trim_class_threshold : 4;
      U16 trim_clk : 4;
  }Bits;
}tFUSE_ARR2;


#define FUSE_ARR2_ADDR 0x344
#define FUSE_ARR2 _FUSE_ARR2.Word
#define FUSE_ARR2_RSRVD _FUSE_ARR2.Bits.rsrvd
#define FUSE_ARR2_TRIM_CONFIG _FUSE_ARR2.Bits.trim_config
#define FUSE_ARR2_TRIM_PORT_GAIN _FUSE_ARR2.Bits.trim_port_gain
#define FUSE_ARR2_TRIM_CLASS_THRESHOLD _FUSE_ARR2.Bits.trim_class_threshold
#define FUSE_ARR2_TRIM_CLK _FUSE_ARR2.Bits.trim_clk


/* #############################################################
/ Register FUSE_ARR3
/ 
/ reset value : FUSE_ARR3 = 0000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 trim_part_id1 : 8;
      U16 trim_part_id2 : 8;
  }Bits;
}tFUSE_ARR3;


#define FUSE_ARR3_ADDR 0x346
#define FUSE_ARR3 _FUSE_ARR3.Word
#define FUSE_ARR3_TRIM_PART_ID1 _FUSE_ARR3.Bits.trim_part_id1
#define FUSE_ARR3_TRIM_PART_ID2 _FUSE_ARR3.Bits.trim_part_id2


/* #############################################################
/ Register FUSE_ARR4
/ 
/ reset value : FUSE_ARR4 = 0000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 reserved : 8;
      U16 test_fuse : 1;
      U16 rsrvd : 1;
      U16 trim_part_id0zzz : 6;
  }Bits;
}tFUSE_ARR4;


#define FUSE_ARR4_ADDR 0x348
#define FUSE_ARR4 _FUSE_ARR4.Word
#define FUSE_ARR4_RESERVED _FUSE_ARR4.Bits.reserved
#define FUSE_ARR4_TEST_FUSE _FUSE_ARR4.Bits.test_fuse
#define FUSE_ARR4_RSRVD _FUSE_ARR4.Bits.rsrvd
#define FUSE_ARR4_TRIM_PART_ID0ZZZ _FUSE_ARR4.Bits.trim_part_id0zzz


/* #############################################################
/ Register OVL_P0_ICUT
/ ICUT for port 0
/ #############################################################*/
#define OVL_P0_ICUT_ADDR 0x1000
/* #############################################################
/ Register OVL_P1_ICUT
/ ICUT for port 1
/ #############################################################*/
#define OVL_P1_ICUT_ADDR 0x1001
/* #############################################################
/ Register OVL_P2_ICUT
/ ICUT for port 2
/ #############################################################*/
#define OVL_P2_ICUT_ADDR 0x1002
/* #############################################################
/ Register OVL_P3_ICUT
/ ICUT for port 3
/ #############################################################*/
#define OVL_P3_ICUT_ADDR 0x1003
/* #############################################################
/ Register OVL_P4_ICUT
/ ICUT for port 4
/ #############################################################*/
#define OVL_P4_ICUT_ADDR 0x1004
/* #############################################################
/ Register OVL_P5_ICUT
/ ICUT for port 5
/ #############################################################*/
#define OVL_P5_ICUT_ADDR 0x1005
/* #############################################################
/ Register OVL_P6_ICUT
/ ICUT for port 6
/ #############################################################*/
#define OVL_P6_ICUT_ADDR 0x1006
/* #############################################################
/ Register OVL_P7_ICUT
/ ICUT for port 7
/ #############################################################*/
#define OVL_P7_ICUT_ADDR 0x1007
/* #############################################################
/ Register OVL_P8_ICUT
/ ICUT for port 8
/ #############################################################*/
#define OVL_P8_ICUT_ADDR 0x1008
/* #############################################################
/ Register OVL_P9_ICUT
/ ICUT for port 9
/ #############################################################*/
#define OVL_P9_ICUT_ADDR 0x1009
/* #############################################################
/ Register OVL_P10_ICUT
/ ICUT for port 10
/ #############################################################*/
#define OVL_P10_ICUT_ADDR 0x100A
/* #############################################################
/ Register OVL_P11_ICUT
/ ICUT for port 11
/ #############################################################*/
#define OVL_P11_ICUT_ADDR 0x100B
/* #############################################################
/ Register ISENSE_VPM_TH
/ Isense measurement threshold for VPM port measrement. Above this threshold - the VPM may measure the port ,  else the mesurement will affect the AC dis function 
/ #############################################################*/
#define ISENSE_VPM_TH_ADDR 0x100C
/* #############################################################
/ Register TOVLD_AF
/ Tovl for all AF ports
/ #############################################################*/
#define TOVLD_AF_ADDR 0x100E
/* #############################################################
/ Register TOVLD_AT
/ Tovl for all AT ports
/ #############################################################*/
#define TOVLD_AT_ADDR 0x1010
/* #############################################################
/ Register OVL_P0_TOVL_CNT
/ Tovl counter for port 0
/ #############################################################*/
#define OVL_P0_TOVL_CNT_ADDR 0x1012
/* #############################################################
/ Register OVL_P1_TOVL_CNT
/ Tovl counter for port 1
/ #############################################################*/
#define OVL_P1_TOVL_CNT_ADDR 0x1014
/* #############################################################
/ Register OVL_P2_TOVL_CNT
/ Tovl counter for port 2
/ #############################################################*/
#define OVL_P2_TOVL_CNT_ADDR 0x1016
/* #############################################################
/ Register OVL_P3_TOVL_CNT
/ Tovl counter for port 3
/ #############################################################*/
#define OVL_P3_TOVL_CNT_ADDR 0x1018
/* #############################################################
/ Register OVL_P4_TOVL_CNT
/ Tovl counter for port 4
/ #############################################################*/
#define OVL_P4_TOVL_CNT_ADDR 0x101A
/* #############################################################
/ Register OVL_P5_TOVL_CNT
/ Tovl counter for port 5
/ #############################################################*/
#define OVL_P5_TOVL_CNT_ADDR 0x101C
/* #############################################################
/ Register OVL_P6_TOVL_CNT
/ Tovl counter for port 6
/ #############################################################*/
#define OVL_P6_TOVL_CNT_ADDR 0x101E
/* #############################################################
/ Register OVL_P7_TOVL_CNT
/ Tovl counter for port 7
/ #############################################################*/
#define OVL_P7_TOVL_CNT_ADDR 0x1020
/* #############################################################
/ Register OVL_P8_TOVL_CNT
/ Tovl counter for port 8
/ #############################################################*/
#define OVL_P8_TOVL_CNT_ADDR 0x1022
/* #############################################################
/ Register OVL_P9_TOVL_CNT
/ Tovl counter for port 9
/ #############################################################*/
#define OVL_P9_TOVL_CNT_ADDR 0x1024
/* #############################################################
/ Register OVL_P10_TOVL_CNT
/ Tovl counter for port 10
/ #############################################################*/
#define OVL_P10_TOVL_CNT_ADDR 0x1026
/* #############################################################
/ Register OVL_P11_TOVL_CNT
/ Tovl counter for port 11
/ #############################################################*/
#define OVL_P11_TOVL_CNT_ADDR 0x1028
/* #############################################################
/ Register UDL_DC_TMPS
/ DC Disconnect TMPS
/ #############################################################*/
#define UDL_DC_TMPS_ADDR 0x102A
/* #############################################################
/ Register UDL_DC_TMPDO
/ DC Disconnect TMPDO
/ #############################################################*/
#define UDL_DC_TMPDO_ADDR 0x102B
/* #############################################################
/ Register UDL_DC_P0_TMPS_CNT
/ DC Disconnect Port 0 TMPS
/ #############################################################*/
#define UDL_DC_P0_TMPS_CNT_ADDR 0x102C
/* #############################################################
/ Register UDL_DC_P0_TMPDO_CNT
/ DC Disconnect Port 0 TMPDO
/ #############################################################*/
#define UDL_DC_P0_TMPDO_CNT_ADDR 0x102D
/* #############################################################
/ Register UDL_DC_P1_TMPS_CNT
/ DC Disconnect Port 1 TMPS
/ #############################################################*/
#define UDL_DC_P1_TMPS_CNT_ADDR 0x102E
/* #############################################################
/ Register UDL_DC_P1_TMPDO_CNT
/ DC Disconnect Port 1 TMPDO
/ #############################################################*/
#define UDL_DC_P1_TMPDO_CNT_ADDR 0x102F
/* #############################################################
/ Register UDL_DC_P2_TMPS_CNT
/ DC Disconnect Port 2 TMPS
/ #############################################################*/
#define UDL_DC_P2_TMPS_CNT_ADDR 0x1030
/* #############################################################
/ Register UDL_DC_P2_TMPDO_CNT
/ DC Disconnect Port 2 TMPDO
/ #############################################################*/
#define UDL_DC_P2_TMPDO_CNT_ADDR 0x1031
/* #############################################################
/ Register UDL_DC_P3_TMPS_CNT
/ DC Disconnect Port 3 TMPS
/ #############################################################*/
#define UDL_DC_P3_TMPS_CNT_ADDR 0x1032
/* #############################################################
/ Register UDL_DC_P3_TMPDO_CNT
/ DC Disconnect Port 3 TMPDO
/ #############################################################*/
#define UDL_DC_P3_TMPDO_CNT_ADDR 0x1033
/* #############################################################
/ Register UDL_DC_P4_TMPS_CNT
/ DC Disconnect Port 4 TMPS
/ #############################################################*/
#define UDL_DC_P4_TMPS_CNT_ADDR 0x1034
/* #############################################################
/ Register UDL_DC_P4_TMPDO_CNT
/ DC Disconnect Port 4 TMPDO
/ #############################################################*/
#define UDL_DC_P4_TMPDO_CNT_ADDR 0x1035
/* #############################################################
/ Register UDL_DC_P5_TMPS_CNT
/ DC Disconnect Port 5 TMPS
/ #############################################################*/
#define UDL_DC_P5_TMPS_CNT_ADDR 0x1036
/* #############################################################
/ Register UDL_DC_P5_TMPDO_CNT
/ DC Disconnect Port 5 TMPDO
/ #############################################################*/
#define UDL_DC_P5_TMPDO_CNT_ADDR 0x1037
/* #############################################################
/ Register UDL_DC_P6_TMPS_CNT
/ DC Disconnect Port 6 TMPS
/ #############################################################*/
#define UDL_DC_P6_TMPS_CNT_ADDR 0x1038
/* #############################################################
/ Register UDL_DC_P6_TMPDO_CNT
/ DC Disconnect Port 6 TMPDO
/ #############################################################*/
#define UDL_DC_P6_TMPDO_CNT_ADDR 0x1039
/* #############################################################
/ Register UDL_DC_P7_TMPS_CNT
/ DC Disconnect Port 7 TMPS
/ #############################################################*/
#define UDL_DC_P7_TMPS_CNT_ADDR 0x103A
/* #############################################################
/ Register UDL_DC_P7_TMPDO_CNT
/ DC Disconnect Port 7 TMPDO
/ #############################################################*/
#define UDL_DC_P7_TMPDO_CNT_ADDR 0x103B
/* #############################################################
/ Register UDL_DC_P8_TMPS_CNT
/ DC Disconnect Port 8 TMPS
/ #############################################################*/
#define UDL_DC_P8_TMPS_CNT_ADDR 0x103C
/* #############################################################
/ Register UDL_DC_P8_TMPDO_CNT
/ DC Disconnect Port 8 TMPDO
/ #############################################################*/
#define UDL_DC_P8_TMPDO_CNT_ADDR 0x103D
/* #############################################################
/ Register UDL_DC_P9_TMPS_CNT
/ DC Disconnect Port 9 TMPS
/ #############################################################*/
#define UDL_DC_P9_TMPS_CNT_ADDR 0x103E
/* #############################################################
/ Register UDL_DC_P9_TMPDO_CNT
/ DC Disconnect Port 9 TMPDO
/ #############################################################*/
#define UDL_DC_P9_TMPDO_CNT_ADDR 0x103F
/* #############################################################
/ Register UDL_DC_P10_TMPS_CNT
/ DC Disconnect Port 10 TMPS
/ #############################################################*/
#define UDL_DC_P10_TMPS_CNT_ADDR 0x1040
/* #############################################################
/ Register UDL_DC_P10_TMPDO_CNT
/ DC Disconnect Port 10 TMPDO
/ #############################################################*/
#define UDL_DC_P10_TMPDO_CNT_ADDR 0x1041
/* #############################################################
/ Register UDL_DC_P11_TMPS_CNT
/ DC Disconnect Port 11 TMPS
/ #############################################################*/
#define UDL_DC_P11_TMPS_CNT_ADDR 0x1042
/* #############################################################
/ Register UDL_DC_P11_TMPDO_CNT
/ DC Disconnect Port 11 TMPDO
/ #############################################################*/
#define UDL_DC_P11_TMPDO_CNT_ADDR 0x1043
/* #############################################################
/ Register P0_ISENSE
/ Port 0 Isense Measurement
/ #############################################################*/
#define P0_ISENSE_ADDR 0x1044
/* #############################################################
/ Register P1_ISENSE
/ Port 1 Isense Measurement
/ #############################################################*/
#define P1_ISENSE_ADDR 0x1046
/* #############################################################
/ Register P2_ISENSE
/ Port 2 Isense Measurement
/ #############################################################*/
#define P2_ISENSE_ADDR 0x1048
/* #############################################################
/ Register P3_ISENSE
/ Port 3 Isense Measurement
/ #############################################################*/
#define P3_ISENSE_ADDR 0x104A
/* #############################################################
/ Register P4_ISENSE
/ Port 4 Isense Measurement
/ #############################################################*/
#define P4_ISENSE_ADDR 0x104C
/* #############################################################
/ Register P5_ISENSE
/ Port 5 Isense Measurement
/ #############################################################*/
#define P5_ISENSE_ADDR 0x104E
/* #############################################################
/ Register P6_ISENSE
/ Port 6 Isense Measurement
/ #############################################################*/
#define P6_ISENSE_ADDR 0x1050
/* #############################################################
/ Register P7_ISENSE
/ Port 7 Isense Measurement
/ #############################################################*/
#define P7_ISENSE_ADDR 0x1052
/* #############################################################
/ Register P8_ISENSE
/ Port 8 Isense Measurement
/ #############################################################*/
#define P8_ISENSE_ADDR 0x1054
/* #############################################################
/ Register P9_ISENSE
/ Port 9 Isense Measurement
/ #############################################################*/
#define P9_ISENSE_ADDR 0x1056
/* #############################################################
/ Register P10_ISENSE
/ Port 10 Isense Measurement
/ #############################################################*/
#define P10_ISENSE_ADDR 0x1058
/* #############################################################
/ Register P11_ISENSE
/ Port 11 Isense Measurement
/ #############################################################*/
#define P11_ISENSE_ADDR 0x105A
/* #############################################################
/ Register VMAIN
/ Vmain measurement
/ #############################################################*/
#define VMAIN_ADDR 0x105C
/* #############################################################
/ Register VTEMP0
/ Temperature0 measurement
/ #############################################################*/
#define VTEMP0_ADDR 0x105E
/* #############################################################
/ Register VTEMP1
/ Temperature1 measurement
/ #############################################################*/
#define VTEMP1_ADDR 0x1060
/* #############################################################
/ Register UDL_DC_THRESHOLD
/ DC Disconnect threashhold
/ #############################################################*/
#define UDL_DC_THRESHOLD_ADDR 0x1062
/* #############################################################
/ Register UDL_AC_CYC_TH
/ number of cycles (time) before disconnecting
/ #############################################################*/
#define UDL_AC_CYC_TH_ADDR 0x1063
/* #############################################################
/ Register UDL_AC_REF_AMP
/ Amplitude of ac reference port
/ #############################################################*/
#define UDL_AC_REF_AMP_ADDR 0x1064
/* #############################################################
/ Register UDL_AC_REF_MAX
/ Reference port max value
/ #############################################################*/
#define UDL_AC_REF_MAX_ADDR 0x1066
/* #############################################################
/ Register UDL_AC_REF_MIN
/ Reference port min value
/ #############################################################*/
#define UDL_AC_REF_MIN_ADDR 0x1068
/* #############################################################
/ Register UDL_AC_P0_MAX
/ Port 0 max VAC
/ #############################################################*/
#define UDL_AC_P0_MAX_ADDR 0x106A
/* #############################################################
/ Register UDL_AC_P0_MIN
/ Port 0 Min VAC
/ #############################################################*/
#define UDL_AC_P0_MIN_ADDR 0x106C
/* #############################################################
/ Register UDL_AC_P1_MAX
/ Port 1 max VAC
/ #############################################################*/
#define UDL_AC_P1_MAX_ADDR 0x106E
/* #############################################################
/ Register UDL_AC_P1_MIN
/ Port 1 Min VAC
/ #############################################################*/
#define UDL_AC_P1_MIN_ADDR 0x1070
/* #############################################################
/ Register UDL_AC_P2_MAX
/ Port 2 max VAC
/ #############################################################*/
#define UDL_AC_P2_MAX_ADDR 0x1072
/* #############################################################
/ Register UDL_AC_P2_MIN
/ Port 2 Min VAC
/ #############################################################*/
#define UDL_AC_P2_MIN_ADDR 0x1074
/* #############################################################
/ Register UDL_AC_P3_MAX
/ Port 3 max VAC
/ #############################################################*/
#define UDL_AC_P3_MAX_ADDR 0x1076
/* #############################################################
/ Register UDL_AC_P3_MIN
/ Port 3 Min VAC
/ #############################################################*/
#define UDL_AC_P3_MIN_ADDR 0x1078
/* #############################################################
/ Register UDL_AC_P4_MAX
/ Port 4 max VAC
/ #############################################################*/
#define UDL_AC_P4_MAX_ADDR 0x107A
/* #############################################################
/ Register UDL_AC_P4_MIN
/ Port 4 Min VAC
/ #############################################################*/
#define UDL_AC_P4_MIN_ADDR 0x107C
/* #############################################################
/ Register UDL_AC_P5_MAX
/ Port 5 max VAC
/ #############################################################*/
#define UDL_AC_P5_MAX_ADDR 0x107E
/* #############################################################
/ Register UDL_AC_P5_MIN
/ Port 5 Min VAC
/ #############################################################*/
#define UDL_AC_P5_MIN_ADDR 0x1080
/* #############################################################
/ Register UDL_AC_P6_MAX
/ Port 6 max VAC
/ #############################################################*/
#define UDL_AC_P6_MAX_ADDR 0x1082
/* #############################################################
/ Register UDL_AC_P6_MIN
/ Port 6 Min VAC
/ #############################################################*/
#define UDL_AC_P6_MIN_ADDR 0x1084
/* #############################################################
/ Register UDL_AC_P7_MAX
/ Port 7 max VAC
/ #############################################################*/
#define UDL_AC_P7_MAX_ADDR 0x1086
/* #############################################################
/ Register UDL_AC_P7_MIN
/ Port 7 Min VAC
/ #############################################################*/
#define UDL_AC_P7_MIN_ADDR 0x1088
/* #############################################################
/ Register UDL_AC_P8_MAX
/ Port 8 max VAC
/ #############################################################*/
#define UDL_AC_P8_MAX_ADDR 0x108A
/* #############################################################
/ Register UDL_AC_P8_MIN
/ Port 8 Min VAC
/ #############################################################*/
#define UDL_AC_P8_MIN_ADDR 0x108C
/* #############################################################
/ Register UDL_AC_P9_MAX
/ Port 9 max VAC
/ #############################################################*/
#define UDL_AC_P9_MAX_ADDR 0x108E
/* #############################################################
/ Register UDL_AC_P9_MIN
/ Port 9 Min VAC
/ #############################################################*/
#define UDL_AC_P9_MIN_ADDR 0x1090
/* #############################################################
/ Register UDL_AC_P10_MAX
/ Port 10 max VAC
/ #############################################################*/
#define UDL_AC_P10_MAX_ADDR 0x1092
/* #############################################################
/ Register UDL_AC_P10_MIN
/ Port 10 Min VAC
/ #############################################################*/
#define UDL_AC_P10_MIN_ADDR 0x1094
/* #############################################################
/ Register UDL_AC_P11_MAX
/ Port 11 max VAC
/ #############################################################*/
#define UDL_AC_P11_MAX_ADDR 0x1096
/* #############################################################
/ Register UDL_AC_P11_MIN
/ Port 11 Min VAC
/ #############################################################*/
#define UDL_AC_P11_MIN_ADDR 0x1098
/* #############################################################
/ Register AC_PORT_MAX_DBG
/ AC debug port max value
/ #############################################################*/
#define AC_PORT_MAX_DBG_ADDR 0x109A
/* #############################################################
/ Register AC_PORT_MIN_DBG
/ AC debug port min value
/ #############################################################*/
#define AC_PORT_MIN_DBG_ADDR 0x109C
/* #############################################################
/ Register UDL_AC_P0_AC_CYC
/ Port 0 ac cycle counter
/ #############################################################*/
#define UDL_AC_P0_AC_CYC_ADDR 0x109E
/* #############################################################
/ Register UDL_AC_P1_AC_CYC
/ Port 1 ac cycle counter
/ #############################################################*/
#define UDL_AC_P1_AC_CYC_ADDR 0x109F
/* #############################################################
/ Register UDL_AC_P2_AC_CYC
/ Port 2 ac cycle counter
/ #############################################################*/
#define UDL_AC_P2_AC_CYC_ADDR 0x10A0
/* #############################################################
/ Register UDL_AC_P3_AC_CYC
/ Port 3 ac cycle counter
/ #############################################################*/
#define UDL_AC_P3_AC_CYC_ADDR 0x10A1
/* #############################################################
/ Register UDL_AC_P4_AC_CYC
/ Port 4 ac cycle counter
/ #############################################################*/
#define UDL_AC_P4_AC_CYC_ADDR 0x10A2
/* #############################################################
/ Register UDL_AC_P5_AC_CYC
/ Port 5 ac cycle counter
/ #############################################################*/
#define UDL_AC_P5_AC_CYC_ADDR 0x10A3
/* #############################################################
/ Register UDL_AC_P6_AC_CYC
/ Port 6 ac cycle counter
/ #############################################################*/
#define UDL_AC_P6_AC_CYC_ADDR 0x10A4
/* #############################################################
/ Register UDL_AC_P7_AC_CYC
/ Port 7 ac cycle counter
/ #############################################################*/
#define UDL_AC_P7_AC_CYC_ADDR 0x10A5
/* #############################################################
/ Register UDL_AC_P8_AC_CYC
/ Port 8 ac cycle counter
/ #############################################################*/
#define UDL_AC_P8_AC_CYC_ADDR 0x10A6
/* #############################################################
/ Register UDL_AC_P9_AC_CYC
/ Port 9 ac cycle counter
/ #############################################################*/
#define UDL_AC_P9_AC_CYC_ADDR 0x10A7
/* #############################################################
/ Register UDL_AC_P10_AC_CYC
/ Port 10 ac cycle counter
/ #############################################################*/
#define UDL_AC_P10_AC_CYC_ADDR 0x10A8
/* #############################################################
/ Register UDL_AC_P11_AC_CYC
/ Port 11 ac cycle counter
/ #############################################################*/
#define UDL_AC_P11_AC_CYC_ADDR 0x10A9
/* #############################################################
/ Register VPM_VMAIN_MES
/ VPORT ADC Vmain 
/ #############################################################*/
#define VPM_VMAIN_MES_ADDR 0x10AA
/* #############################################################
/ Register VPM_VPORT_0_MES
/ Port 0 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_0_MES_ADDR 0x10AC
/* #############################################################
/ Register VPM_VPORT_1_MES
/ Port 1 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_1_MES_ADDR 0x10AE
/* #############################################################
/ Register VPM_VPORT_2_MES
/ Port 2 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_2_MES_ADDR 0x10B0
/* #############################################################
/ Register VPM_VPORT_3_MES
/ Port 3 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_3_MES_ADDR 0x10B2
/* #############################################################
/ Register VPM_VPORT_4_MES
/ Port 4 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_4_MES_ADDR 0x10B4
/* #############################################################
/ Register VPM_VPORT_5_MES
/ Port 5 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_5_MES_ADDR 0x10B6
/* #############################################################
/ Register VPM_VPORT_6_MES
/ Port 6 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_6_MES_ADDR 0x10B8
/* #############################################################
/ Register VPM_VPORT_7_MES
/ Port 7 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_7_MES_ADDR 0x10BA
/* #############################################################
/ Register VPM_VPORT_8_MES
/ Port 8 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_8_MES_ADDR 0x10BC
/* #############################################################
/ Register VPM_VPORT_9_MES
/ Port 9 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_9_MES_ADDR 0x10BE
/* #############################################################
/ Register VPM_VPORT_10_MES
/ Port 10 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_10_MES_ADDR 0x10C0
/* #############################################################
/ Register VPM_VPORT_11_MES
/ Port 11 Voltage measurement
/ #############################################################*/
#define VPM_VPORT_11_MES_ADDR 0x10C2
/* #############################################################
/ Register VPM_LD_REF_PORT
/ LD REFERENCE PORT
/ #############################################################*/
#define VPM_LD_REF_PORT_ADDR 0x10C4
/* #############################################################
/ Register SC_V_LOW_TH_AF
/ SC AF Low Volatege Threshold
/ #############################################################*/
#define SC_V_LOW_TH_AF_ADDR 0x10C6
/* #############################################################
/ Register SC_V_LOW_TH_AT
/ SC AT Low Volatege Threshold
/ #############################################################*/
#define SC_V_LOW_TH_AT_ADDR 0x10C8
/* #############################################################
/ Register SC_V_HIGH_TH_AF
/ SC AF HighVolatege Threshold
/ #############################################################*/
#define SC_V_HIGH_TH_AF_ADDR 0x10CA
/* #############################################################
/ Register SC_V_HIGH_TH_AT
/ SC AT Volatege Threshold
/ #############################################################*/
#define SC_V_HIGH_TH_AT_ADDR 0x10CC
/* #############################################################
/ Register TLIM
/ Tlim counter
/ #############################################################*/
#define TLIM_ADDR 0x10CE
/* #############################################################
/ Register SC_TFB
/ Fold Back counter
/ #############################################################*/
#define SC_TFB_ADDR 0x10CF
/* #############################################################
/ Register TLIM_PORT_0_CNT
/ Tlim counter - port 0
/ #############################################################*/
#define TLIM_PORT_0_CNT_ADDR 0x10D0
/* #############################################################
/ Register SC_TFB_PORT_0_CNT
/ foldback counter - port 0
/ #############################################################*/
#define SC_TFB_PORT_0_CNT_ADDR 0x10D1
/* #############################################################
/ Register TLIM_PORT_1_CNT
/ Tlim counter - port 1
/ #############################################################*/
#define TLIM_PORT_1_CNT_ADDR 0x10D2
/* #############################################################
/ Register SC_TFB_PORT_1_CNT
/ foldback counter - port 1
/ #############################################################*/
#define SC_TFB_PORT_1_CNT_ADDR 0x10D3
/* #############################################################
/ Register TLIM_PORT_2_CNT
/ Tlim counter - port 2
/ #############################################################*/
#define TLIM_PORT_2_CNT_ADDR 0x10D4
/* #############################################################
/ Register SC_TFB_PORT_2_CNT
/ foldback counter - port 2
/ #############################################################*/
#define SC_TFB_PORT_2_CNT_ADDR 0x10D5
/* #############################################################
/ Register TLIM_PORT_3_CNT
/ Tlim counter - port 3
/ #############################################################*/
#define TLIM_PORT_3_CNT_ADDR 0x10D6
/* #############################################################
/ Register SC_TFB_PORT_3_CNT
/ foldback counter - port 3
/ #############################################################*/
#define SC_TFB_PORT_3_CNT_ADDR 0x10D7
/* #############################################################
/ Register TLIM_PORT_4_CNT
/ Tlim counter - port 4
/ #############################################################*/
#define TLIM_PORT_4_CNT_ADDR 0x10D8
/* #############################################################
/ Register SC_TFB_PORT_4_CNT
/ foldback counter - port 4
/ #############################################################*/
#define SC_TFB_PORT_4_CNT_ADDR 0x10D9
/* #############################################################
/ Register TLIM_PORT_5_CNT
/ Tlim counter - port 5
/ #############################################################*/
#define TLIM_PORT_5_CNT_ADDR 0x10DA
/* #############################################################
/ Register SC_TFB_PORT_5_CNT
/ foldback counter - port 5
/ #############################################################*/
#define SC_TFB_PORT_5_CNT_ADDR 0x10DB
/* #############################################################
/ Register TLIM_PORT_6_CNT
/ Tlim counter - port 6
/ #############################################################*/
#define TLIM_PORT_6_CNT_ADDR 0x10DC
/* #############################################################
/ Register SC_TFB_PORT_6_CNT
/ foldback counter - port 6
/ #############################################################*/
#define SC_TFB_PORT_6_CNT_ADDR 0x10DD
/* #############################################################
/ Register TLIM_PORT_7_CNT
/ Tlim counter - port 7
/ #############################################################*/
#define TLIM_PORT_7_CNT_ADDR 0x10DE
/* #############################################################
/ Register SC_TFB_PORT_7_CNT
/ foldback counter - port 7
/ #############################################################*/
#define SC_TFB_PORT_7_CNT_ADDR 0x10DF
/* #############################################################
/ Register TLIM_PORT_8_CNT
/ Tlim counter - port 8
/ #############################################################*/
#define TLIM_PORT_8_CNT_ADDR 0x10E0
/* #############################################################
/ Register SC_TFB_PORT_8_CNT
/ foldback counter - port 8
/ #############################################################*/
#define SC_TFB_PORT_8_CNT_ADDR 0x10E1
/* #############################################################
/ Register TLIM_PORT_9_CNT
/ Tlim counter - port 9
/ #############################################################*/
#define TLIM_PORT_9_CNT_ADDR 0x10E2
/* #############################################################
/ Register SC_TFB_PORT_9_CNT
/ foldback counter - port 9
/ #############################################################*/
#define SC_TFB_PORT_9_CNT_ADDR 0x10E3
/* #############################################################
/ Register TLIM_PORT_10_CNT
/ Tlim counter - port 10
/ #############################################################*/
#define TLIM_PORT_10_CNT_ADDR 0x10E4
/* #############################################################
/ Register SC_TFB_PORT_10_CNT
/ foldback counter - port 10
/ #############################################################*/
#define SC_TFB_PORT_10_CNT_ADDR 0x10E5
/* #############################################################
/ Register TLIM_PORT_11_CNT
/ Tlim counter - port 11
/ #############################################################*/
#define TLIM_PORT_11_CNT_ADDR 0x10E6
/* #############################################################
/ Register SC_TFB_PORT_11_CNT
/ foldback counter - port 11
/ #############################################################*/
#define SC_TFB_PORT_11_CNT_ADDR 0x10E7
/* #############################################################
/ Register VPM_CDR_MES_0
/ Cap Detection Rotine sample number 0
/ #############################################################*/
#define VPM_CDR_MES_0_ADDR 0x10E8
/* #############################################################
/ Register VPM_CDR_MES_1
/ Cap Detection Rotine sample number 1
/ #############################################################*/
#define VPM_CDR_MES_1_ADDR 0x10EA
/* #############################################################
/ Register VPM_CDR_MES_2
/ Cap Detection Rotine sample number 2
/ #############################################################*/
#define VPM_CDR_MES_2_ADDR 0x10EC
/* #############################################################
/ Register VPM_CDR_MES_3
/ Cap Detection Rotine sample number 3
/ #############################################################*/
#define VPM_CDR_MES_3_ADDR 0x10EE
/* #############################################################
/ Register VPM_CDR_MES_4
/ Cap Detection Rotine sample number 4
/ #############################################################*/
#define VPM_CDR_MES_4_ADDR 0x10F0
/* #############################################################
/ Register VPM_CDR_MES_5
/ Cap Detection Rotine sample number 5
/ #############################################################*/
#define VPM_CDR_MES_5_ADDR 0x10F2
/* #############################################################
/ Register VPM_CDR_MES_6
/ Cap Detection Rotine sample number 6
/ #############################################################*/
#define VPM_CDR_MES_6_ADDR 0x10F4
/* #############################################################
/ Register VPM_CDR_MES_7
/ Cap Detection Rotine sample number 7
/ #############################################################*/
#define VPM_CDR_MES_7_ADDR 0x10F6
/* #############################################################
/ Register VPM_CDR_MES_8
/ Cap Detection Rotine sample number 8
/ #############################################################*/
#define VPM_CDR_MES_8_ADDR 0x10F8
/* #############################################################
/ Register VPM_CDR_MES_9
/ Cap Detection Rotine sample number 9
/ #############################################################*/
#define VPM_CDR_MES_9_ADDR 0x10FA
/* #############################################################
/ Register VPM_CDR_MES_10
/ Cap Detection Rotine sample number 10
/ #############################################################*/
#define VPM_CDR_MES_10_ADDR 0x10FC
/* #############################################################
/ Register VPM_CDR_MES_11
/ Cap Detection Rotine sample number 11
/ #############################################################*/
#define VPM_CDR_MES_11_ADDR 0x10FE
/* #############################################################
/ Register VPM_CDR_MES_12
/ Cap Detection Rotine sample number 12
/ #############################################################*/
#define VPM_CDR_MES_12_ADDR 0x1100
/* #############################################################
/ Register VPM_CDR_MES_13
/ Cap Detection Rotine sample number 13
/ #############################################################*/
#define VPM_CDR_MES_13_ADDR 0x1102
/* #############################################################
/ Register VPM_CDR_MES_14
/ Cap Detection Rotine sample number 14
/ #############################################################*/
#define VPM_CDR_MES_14_ADDR 0x1104
/* #############################################################
/ Register VPM_CDR_MES_15
/ Cap Detection Rotine sample number 15
/ #############################################################*/
#define VPM_CDR_MES_15_ADDR 0x1106
/* #############################################################
/ Register VPM_CDR_MES_16
/ Cap Detection Rotine sample number 16
/ #############################################################*/
#define VPM_CDR_MES_16_ADDR 0x1108
/* #############################################################
/ Register VPM_CDR_MES_17
/ Cap Detection Rotine sample number 17
/ #############################################################*/
#define VPM_CDR_MES_17_ADDR 0x110A
/* #############################################################
/ Register VPM_CDR_MES_18
/ Cap Detection Rotine sample number 18
/ #############################################################*/
#define VPM_CDR_MES_18_ADDR 0x110C
/* #############################################################
/ Register VPM_CDR_MES_19
/ Cap Detection Rotine sample number 19
/ #############################################################*/
#define VPM_CDR_MES_19_ADDR 0x110E
/* #############################################################
/ Register VPM_CDR_MES_20
/ Cap Detection Rotine sample number 20
/ #############################################################*/
#define VPM_CDR_MES_20_ADDR 0x1110
/* #############################################################
/ Register VPM_CDR_MES_21
/ Cap Detection Rotine sample number 21
/ #############################################################*/
#define VPM_CDR_MES_21_ADDR 0x1112
/* #############################################################
/ Register VPM_CDR_MES_22
/ Cap Detection Rotine sample number 22
/ #############################################################*/
#define VPM_CDR_MES_22_ADDR 0x1114
/* #############################################################
/ Register VPM_CDR_MES_23
/ Cap Detection Rotine sample number 23
/ #############################################################*/
#define VPM_CDR_MES_23_ADDR 0x1116
/* #############################################################
/ Register VPM_CDR_MES_24
/ Cap Detection Rotine sample number 24
/ #############################################################*/
#define VPM_CDR_MES_24_ADDR 0x1118
/* #############################################################
/ Register VPM_CDR_MES_25
/ Cap Detection Rotine sample number 25
/ #############################################################*/
#define VPM_CDR_MES_25_ADDR 0x111A
/* #############################################################
/ Register VPM_CDR_MES_26
/ Cap Detection Rotine sample number 26
/ #############################################################*/
#define VPM_CDR_MES_26_ADDR 0x111C
/* #############################################################
/ Register VPM_CDR_MES_27
/ Cap Detection Rotine sample number 27
/ #############################################################*/
#define VPM_CDR_MES_27_ADDR 0x111E
/* #############################################################
/ Register VPM_CDR_MES_28
/ Cap Detection Rotine sample number 28
/ #############################################################*/
#define VPM_CDR_MES_28_ADDR 0x1120
/* #############################################################
/ Register VPM_CDR_MES_29
/ Cap Detection Rotine sample number 29
/ #############################################################*/
#define VPM_CDR_MES_29_ADDR 0x1122
/* #############################################################
/ Register VPM_CDR_MES_30
/ Cap Detection Rotine sample number 30
/ #############################################################*/
#define VPM_CDR_MES_30_ADDR 0x1124
/* #############################################################
/ Register VPM_CDR_MES_31
/ Cap Detection Rotine sample number 31
/ #############################################################*/
#define VPM_CDR_MES_31_ADDR 0x1126
/* #############################################################
/ Register VPM_CDR_MES_32
/ Cap Detection Rotine sample number 32
/ #############################################################*/
#define VPM_CDR_MES_32_ADDR 0x1128
/* #############################################################
/ Register SPIC_EXTADD
/ External address.
/ #############################################################*/
#define SPIC_EXTADD_ADDR 0x112A
/* #############################################################
/ Register LSD_DATA0
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA0_ADDR 0x112C
/* #############################################################
/ Register LSD_DATA1
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA1_ADDR 0x112E
/* #############################################################
/ Register LSD_DATA2
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA2_ADDR 0x1130
/* #############################################################
/ Register LSD_DATA3
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA3_ADDR 0x1132
/* #############################################################
/ Register LSD_DATA4
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA4_ADDR 0x1134
/* #############################################################
/ Register LSD_DATA5
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA5_ADDR 0x1136
/* #############################################################
/ Register LSD_DATA6
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA6_ADDR 0x1138
/* #############################################################
/ Register LSD_DATA7
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA7_ADDR 0x113A
/* #############################################################
/ Register LSD_DATA8
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA8_ADDR 0x113C
/* #############################################################
/ Register LSD_DATA9
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA9_ADDR 0x113E
/* #############################################################
/ Register LSD_DATA10
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA10_ADDR 0x1140
/* #############################################################
/ Register LSD_DATA11
/ LSD indication bits
/ #############################################################*/
#define LSD_DATA11_ADDR 0x1142
/* #############################################################
/ Register EXT_EV_IRQ
/ External event IRQ (sync.)
/ #############################################################*/
#define EXT_EV_IRQ_ADDR 0x1144
/* #############################################################
/ Register SysFlags
/ decide class out of class in error
/ reset value : SysFlags = 000000001110101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 dc_discoen : 1;
      U16 externalsyncdis : 1;
      U16 capdis : 1;
      U16 disportsoverride : 1;
      U16 icutmaxflag : 1;
      U16 lsden_or_intout : 1;
      U16 pm_mode : 1;
      U16 rpr_disable : 1;
      U16 disautointout : 1;
      U16 vmainatpolicyen : 1;
      U16 class0eqaf : 1;
      U16 class123eqaf : 1;
      U16 classbypass2nderror : 1;
      U16 classerroreq0 : 1;
      U16 classerroreq4 : 1;
      U16 reserved : 1;
  }Bits;
}tSysFlags;


#define SysFlags_ADDR 0x1160
#define SysFlags _SysFlags.Word
#define SYSFLAGS_DC_DISCOEN _SysFlags.Bits.dc_discoen
#define SYSFLAGS_EXTERNALSYNCDIS _SysFlags.Bits.externalsyncdis
#define SYSFLAGS_CAPDIS _SysFlags.Bits.capdis
#define SYSFLAGS_DISPORTSOVERRIDE _SysFlags.Bits.disportsoverride
#define SYSFLAGS_ICUTMAXFLAG _SysFlags.Bits.icutmaxflag
#define SYSFLAGS_LSDEN_OR_INTOUT _SysFlags.Bits.lsden_or_intout
#define SYSFLAGS_PM_MODE _SysFlags.Bits.pm_mode
#define SYSFLAGS_RPR_DISABLE _SysFlags.Bits.rpr_disable
#define SYSFLAGS_DISAUTOINTOUT _SysFlags.Bits.disautointout
#define SYSFLAGS_VMAINATPOLICYEN _SysFlags.Bits.vmainatpolicyen
#define SYSFLAGS_CLASS0EQAF _SysFlags.Bits.class0eqaf
#define SYSFLAGS_CLASS123EQAF _SysFlags.Bits.class123eqaf
#define SYSFLAGS_CLASSBYPASS2NDERROR _SysFlags.Bits.classbypass2nderror
#define SYSFLAGS_CLASSERROREQ0 _SysFlags.Bits.classerroreq0
#define SYSFLAGS_CLASSERROREQ4 _SysFlags.Bits.classerroreq4


/* #############################################################
/ Register ChipVersion
/ POE unit familiy prefix
/ reset value : ChipVersion = 0001000100100000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 swversion : 5;
      U16 rtlversion : 3;
      U16 analogversion : 3;
      U16 spare : 1;
      U16 familiprefix : 4;
  }Bits;
}tChipVersion;


#define ChipVersion_ADDR 0x1162
#define ChipVersion _ChipVersion.Word
#define CHIPVERSION_SWVERSION _ChipVersion.Bits.swversion
#define CHIPVERSION_RTLVERSION _ChipVersion.Bits.rtlversion
#define CHIPVERSION_ANALOGVERSION _ChipVersion.Bits.analogversion
#define CHIPVERSION_SPARE _ChipVersion.Bits.spare
#define CHIPVERSION_FAMILIPREFIX _ChipVersion.Bits.familiprefix


/* #############################################################
/ Register InitReg
/ register map version
/ reset value : InitReg = 0000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 asic_inival : 4;
      U16 i2c_inival : 4;
      U16 regmapversion : 8;
  }Bits;
}tInitReg;


#define InitReg_ADDR 0x1164
#define InitReg _InitReg.Word
#define INITREG_ASIC_INIVAL _InitReg.Bits.asic_inival
#define INITREG_I2C_INIVAL _InitReg.Bits.i2c_inival
#define INITREG_REGMAPVERSION _InitReg.Bits.regmapversion


/* #############################################################
/ Register EnhModeVerifKey
/ a key code  (0x5245) must be writen to this register to initialize the enhanced slave SW
/ #############################################################*/
#define EnhModeVerifKey_ADDR 0x1166
/* #############################################################
/ Register SW_BootState
/ indication for a valid Eeprom existance and update
/ reset value : SW_BootState = 000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 currbootcondition : 8;
      U16 sw_errorind : 6;
      U16 eepromvalid : 1;
      U16 reserved : 1;
  }Bits;
}tSW_BootState;


#define SW_BootState_ADDR 0x1168
#define SW_BootState _SW_BootState.Word
#define SW_BOOTSTATE_CURRBOOTCONDITION _SW_BootState.Bits.currbootcondition
#define SW_BOOTSTATE_SW_ERRORIND _SW_BootState.Bits.sw_errorind
#define SW_BOOTSTATE_EEPROMVALID _SW_BootState.Bits.eepromvalid


/* #############################################################
/ Register Pre1_DacData
/ DAC value for Pre1 level
/ #############################################################*/
#define Pre1_DacData_ADDR 0x116A
/* #############################################################
/ Register Pre2_DacData
/ DAC value for Pre2 level
/ #############################################################*/
#define Pre2_DacData_ADDR 0x116C
/* #############################################################
/ Register LD1_DacData
/ DAC value for LD1 level
/ #############################################################*/
#define LD1_DacData_ADDR 0x116E
/* #############################################################
/ Register LD2_DacData
/ DAC value for LD2 level
/ #############################################################*/
#define LD2_DacData_ADDR 0x1170
/* #############################################################
/ Register Pre1Length
/ length of Pre1 level
/ #############################################################*/
#define Pre1Length_ADDR 0x1172
/* #############################################################
/ Register Pre2Length
/ length of Pre2 level
/ #############################################################*/
#define Pre2Length_ADDR 0x1174
/* #############################################################
/ Register LD1Length
/ length of LD1 level
/ #############################################################*/
#define LD1Length_ADDR 0x1176
/* #############################################################
/ Register LD2Length
/ length of LD2 level
/ #############################################################*/
#define LD2Length_ADDR 0x1178
/* #############################################################
/ Register ResDetPreTh
/ threshold level for pre detection
/ #############################################################*/
#define ResDetPreTh_ADDR 0x117A
/* #############################################################
/ Register ResDetDeltaMarginMultiplyer
/ the mergin between the calculated delta on the reference port and the high and low thresholds
/ #############################################################*/
#define ResDetDeltaMarginMultiplyer_ADDR 0x117C
/* #############################################################
/ Register ResDetPreRslt
/ Pre Detection Results. 
/ #############################################################*/
#define ResDetPreRslt_ADDR 0x117E
/* #############################################################
/ Register ResDetDelta1Rslt
/ Level 1 Detection Results. 
/ #############################################################*/
#define ResDetDelta1Rslt_ADDR 0x1180
/* #############################################################
/ Register ResDetDelta2Rslt
/ Level 2 Detection Results
/ #############################################################*/
#define ResDetDelta2Rslt_ADDR 0x1182
/* #############################################################
/ Register ResDetResult
/ bit per port overall result for the resistor detection
/ #############################################################*/
#define ResDetResult_ADDR 0x1184
/* #############################################################
/ Register ResDetDelta1Reject
/ rejection area for the first delta
/ #############################################################*/
#define ResDetDelta1Reject_ADDR 0x1186
/* #############################################################
/ Register ResDetDelta2Reject
/ rejection area for the second delta
/ #############################################################*/
#define ResDetDelta2Reject_ADDR 0x1188
/* #############################################################
/ Register ClassLength
/ length of a mark event
/ reset value : ClassLength = 0001111101100010
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classeventlength : 8;
      U16 markeventlength : 8;
  }Bits;
}tClassLength;


#define ClassLength_ADDR 0x118A
#define ClassLength _ClassLength.Word
#define CLASSLENGTH_CLASSEVENTLENGTH _ClassLength.Bits.classeventlength
#define CLASSLENGTH_MARKEVENTLENGTH _ClassLength.Bits.markeventlength


/* #############################################################
/ Register ClassMarkDacValue
/ DAC value in mark event
/ #############################################################*/
#define ClassMarkDacValue_ADDR 0x118C
/* #############################################################
/ Register CapDetStatus
/ Last Cap detection result
/ reset value : CapDetStatus = 00000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 lastcapport : 4;
      U16 lastcapresult : 1;
      U16 reserved : 11;
  }Bits;
}tCapDetStatus;


#define CapDetStatus_ADDR 0x118E
#define CapDetStatus _CapDetStatus.Word
#define CAPDETSTATUS_LASTCAPPORT _CapDetStatus.Bits.lastcapport
#define CAPDETSTATUS_LASTCAPRESULT _CapDetStatus.Bits.lastcapresult


/* #############################################################
/ Register StartupCR
/ when startup power is not enough for the current port ,  check if the following ports can be started up or mark them all as PM
/ reset value : StartupCR = 0110010
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 numportsincycle : 4;
      U16 multiportstartup : 1;
      U16 dvdtenable : 1;
      U16 startup_hipr_override : 1;
      U16 reserved : 9;
  }Bits;
}tStartupCR;


#define StartupCR_ADDR 0x1190
#define StartupCR _StartupCR.Word
#define STARTUPCR_NUMPORTSINCYCLE _StartupCR.Bits.numportsincycle
#define STARTUPCR_MULTIPORTSTARTUP _StartupCR.Bits.multiportstartup
#define STARTUPCR_DVDTENABLE _StartupCR.Bits.dvdtenable
#define STARTUPCR_STARTUP_HIPR_OVERRIDE _StartupCR.Bits.startup_hipr_override


/* #############################################################
/ Register StartupLength
/ length of startup
/ #############################################################*/
#define StartupLength_ADDR 0x1192
/* #############################################################
/ Register StartupDelay
/ if Multi port startup is disabled then delay between startups for slaves according to slave # times this value
/ #############################################################*/
#define StartupDelay_ADDR 0x1194
/* #############################################################
/ Register DvDtSec1Delta
/ the delta added to MaxEnergy in section 1
/ #############################################################*/
#define DvDtSec1Delta_ADDR 0x1196
/* #############################################################
/ Register DvDtSec2Delta
/ the delta added to MaxEnergy in section 2
/ #############################################################*/
#define DvDtSec2Delta_ADDR 0x1198
/* #############################################################
/ Register DvDtSec3Delta
/ the delta added to MaxEnergy in section 3
/ #############################################################*/
#define DvDtSec3Delta_ADDR 0x119A
/* #############################################################
/ Register DvDtSec4Delta
/ the delta added to MaxEnergy in section 4
/ #############################################################*/
#define DvDtSec4Delta_ADDR 0x119C
/* #############################################################
/ Register DvDtSec5Delta
/ the delta added to MaxEnergy in section 5
/ #############################################################*/
#define DvDtSec5Delta_ADDR 0x119E
/* #############################################################
/ Register DvDtSec1EndSmpNo
/ delta1 is added to MaxEnergy until this sample number
/ #############################################################*/
#define DvDtSec1EndSmpNo_ADDR 0x11A0
/* #############################################################
/ Register DvDtSec2EndSmpNo
/ delta2 is added to MaxEnergy until this sample number
/ #############################################################*/
#define DvDtSec2EndSmpNo_ADDR 0x11A2
/* #############################################################
/ Register DvDtSec3EndSmpNo
/ delta3 is added to MaxEnergy until this sample number
/ #############################################################*/
#define DvDtSec3EndSmpNo_ADDR 0x11A4
/* #############################################################
/ Register DvDtSec4EndSmpNo
/ delta4 is added to MaxEnergy until this sample number
/ #############################################################*/
#define DvDtSec4EndSmpNo_ADDR 0x11A6
/* #############################################################
/ Register DvDtEndVth
/ if Vport reaches Vmain - DvDtEndVth then startup is finished
/ #############################################################*/
#define DvDtEndVth_ADDR 0x11A8
/* #############################################################
/ Register Port0_SR
/ after classification this bit indicates if the port is decided to be AF or AT
/ reset value : Port0_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort0_SR;


#define Port0_SR_ADDR 0x11AA
#define Port0_SR _Port0_SR.Word
#define PORT0_SR_INTERNALSTATUS _Port0_SR.Bits.internalstatus
#define PORT0_SR_EXTERNALSTATUS _Port0_SR.Bits.externalstatus
#define PORT0_SR_PORT_AT_BEHAVIOR _Port0_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port1_SR
/ 
/ reset value : Port1_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort1_SR;


#define Port1_SR_ADDR 0x11AC
#define Port1_SR _Port1_SR.Word
#define PORT1_SR_INTERNALSTATUS _Port1_SR.Bits.internalstatus
#define PORT1_SR_EXTERNALSTATUS _Port1_SR.Bits.externalstatus
#define PORT1_SR_PORT_AT_BEHAVIOR _Port1_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port2_SR
/ 
/ reset value : Port2_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort2_SR;


#define Port2_SR_ADDR 0x11AE
#define Port2_SR _Port2_SR.Word
#define PORT2_SR_INTERNALSTATUS _Port2_SR.Bits.internalstatus
#define PORT2_SR_EXTERNALSTATUS _Port2_SR.Bits.externalstatus
#define PORT2_SR_PORT_AT_BEHAVIOR _Port2_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port3_SR
/ 
/ reset value : Port3_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort3_SR;


#define Port3_SR_ADDR 0x11B0
#define Port3_SR _Port3_SR.Word
#define PORT3_SR_INTERNALSTATUS _Port3_SR.Bits.internalstatus
#define PORT3_SR_EXTERNALSTATUS _Port3_SR.Bits.externalstatus
#define PORT3_SR_PORT_AT_BEHAVIOR _Port3_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port4_SR
/ 
/ reset value : Port4_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort4_SR;


#define Port4_SR_ADDR 0x11B2
#define Port4_SR _Port4_SR.Word
#define PORT4_SR_INTERNALSTATUS _Port4_SR.Bits.internalstatus
#define PORT4_SR_EXTERNALSTATUS _Port4_SR.Bits.externalstatus
#define PORT4_SR_PORT_AT_BEHAVIOR _Port4_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port5_SR
/ 
/ reset value : Port5_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort5_SR;


#define Port5_SR_ADDR 0x11B4
#define Port5_SR _Port5_SR.Word
#define PORT5_SR_INTERNALSTATUS _Port5_SR.Bits.internalstatus
#define PORT5_SR_EXTERNALSTATUS _Port5_SR.Bits.externalstatus
#define PORT5_SR_PORT_AT_BEHAVIOR _Port5_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port6_SR
/ 
/ reset value : Port6_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort6_SR;


#define Port6_SR_ADDR 0x11B6
#define Port6_SR _Port6_SR.Word
#define PORT6_SR_INTERNALSTATUS _Port6_SR.Bits.internalstatus
#define PORT6_SR_EXTERNALSTATUS _Port6_SR.Bits.externalstatus
#define PORT6_SR_PORT_AT_BEHAVIOR _Port6_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port7_SR
/ 
/ reset value : Port7_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort7_SR;


#define Port7_SR_ADDR 0x11B8
#define Port7_SR _Port7_SR.Word
#define PORT7_SR_INTERNALSTATUS _Port7_SR.Bits.internalstatus
#define PORT7_SR_EXTERNALSTATUS _Port7_SR.Bits.externalstatus
#define PORT7_SR_PORT_AT_BEHAVIOR _Port7_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port8_SR
/ 
/ reset value : Port8_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort8_SR;


#define Port8_SR_ADDR 0x11BA
#define Port8_SR _Port8_SR.Word
#define PORT8_SR_INTERNALSTATUS _Port8_SR.Bits.internalstatus
#define PORT8_SR_EXTERNALSTATUS _Port8_SR.Bits.externalstatus
#define PORT8_SR_PORT_AT_BEHAVIOR _Port8_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port9_SR
/ 
/ reset value : Port9_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort9_SR;


#define Port9_SR_ADDR 0x11BC
#define Port9_SR _Port9_SR.Word
#define PORT9_SR_INTERNALSTATUS _Port9_SR.Bits.internalstatus
#define PORT9_SR_EXTERNALSTATUS _Port9_SR.Bits.externalstatus
#define PORT9_SR_PORT_AT_BEHAVIOR _Port9_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port10_SR
/ 
/ reset value : Port10_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort10_SR;


#define Port10_SR_ADDR 0x11BE
#define Port10_SR _Port10_SR.Word
#define PORT10_SR_INTERNALSTATUS _Port10_SR.Bits.internalstatus
#define PORT10_SR_EXTERNALSTATUS _Port10_SR.Bits.externalstatus
#define PORT10_SR_PORT_AT_BEHAVIOR _Port10_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port11_SR
/ 
/ reset value : Port11_SR = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 internalstatus : 8;
      U16 externalstatus : 3;
      U16 port_at_behavior : 1;
      U16 reserved : 4;
  }Bits;
}tPort11_SR;


#define Port11_SR_ADDR 0x11C0
#define Port11_SR _Port11_SR.Word
#define PORT11_SR_INTERNALSTATUS _Port11_SR.Bits.internalstatus
#define PORT11_SR_EXTERNALSTATUS _Port11_SR.Bits.externalstatus
#define PORT11_SR_PORT_AT_BEHAVIOR _Port11_SR.Bits.port_at_behavior


/* #############################################################
/ Register Port0_Class
/ decided class
/ reset value : Port0_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort0_Class;


#define Port0_Class_ADDR 0x11C2
#define Port0_Class _Port0_Class.Word
#define PORT0_CLASS_CLASSIN1 _Port0_Class.Bits.classin1
#define PORT0_CLASS_CLASSIN2 _Port0_Class.Bits.classin2
#define PORT0_CLASS_CLASSOUT _Port0_Class.Bits.classout


/* #############################################################
/ Register Port1_Class
/ decided class
/ reset value : Port1_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort1_Class;


#define Port1_Class_ADDR 0x11C4
#define Port1_Class _Port1_Class.Word
#define PORT1_CLASS_CLASSIN1 _Port1_Class.Bits.classin1
#define PORT1_CLASS_CLASSIN2 _Port1_Class.Bits.classin2
#define PORT1_CLASS_CLASSOUT _Port1_Class.Bits.classout


/* #############################################################
/ Register Port2_Class
/ decided class
/ reset value : Port2_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort2_Class;


#define Port2_Class_ADDR 0x11C6
#define Port2_Class _Port2_Class.Word
#define PORT2_CLASS_CLASSIN1 _Port2_Class.Bits.classin1
#define PORT2_CLASS_CLASSIN2 _Port2_Class.Bits.classin2
#define PORT2_CLASS_CLASSOUT _Port2_Class.Bits.classout


/* #############################################################
/ Register Port3_Class
/ decided class
/ reset value : Port3_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort3_Class;


#define Port3_Class_ADDR 0x11C8
#define Port3_Class _Port3_Class.Word
#define PORT3_CLASS_CLASSIN1 _Port3_Class.Bits.classin1
#define PORT3_CLASS_CLASSIN2 _Port3_Class.Bits.classin2
#define PORT3_CLASS_CLASSOUT _Port3_Class.Bits.classout


/* #############################################################
/ Register Port4_Class
/ decided class
/ reset value : Port4_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort4_Class;


#define Port4_Class_ADDR 0x11CA
#define Port4_Class _Port4_Class.Word
#define PORT4_CLASS_CLASSIN1 _Port4_Class.Bits.classin1
#define PORT4_CLASS_CLASSIN2 _Port4_Class.Bits.classin2
#define PORT4_CLASS_CLASSOUT _Port4_Class.Bits.classout


/* #############################################################
/ Register Port5_Class
/ decided class
/ reset value : Port5_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort5_Class;


#define Port5_Class_ADDR 0x11CC
#define Port5_Class _Port5_Class.Word
#define PORT5_CLASS_CLASSIN1 _Port5_Class.Bits.classin1
#define PORT5_CLASS_CLASSIN2 _Port5_Class.Bits.classin2
#define PORT5_CLASS_CLASSOUT _Port5_Class.Bits.classout


/* #############################################################
/ Register Port6_Class
/ decided class
/ reset value : Port6_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort6_Class;


#define Port6_Class_ADDR 0x11CE
#define Port6_Class _Port6_Class.Word
#define PORT6_CLASS_CLASSIN1 _Port6_Class.Bits.classin1
#define PORT6_CLASS_CLASSIN2 _Port6_Class.Bits.classin2
#define PORT6_CLASS_CLASSOUT _Port6_Class.Bits.classout


/* #############################################################
/ Register Port7_Class
/ decided class
/ reset value : Port7_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort7_Class;


#define Port7_Class_ADDR 0x11D0
#define Port7_Class _Port7_Class.Word
#define PORT7_CLASS_CLASSIN1 _Port7_Class.Bits.classin1
#define PORT7_CLASS_CLASSIN2 _Port7_Class.Bits.classin2
#define PORT7_CLASS_CLASSOUT _Port7_Class.Bits.classout


/* #############################################################
/ Register Port8_Class
/ decided class
/ reset value : Port8_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort8_Class;


#define Port8_Class_ADDR 0x11D2
#define Port8_Class _Port8_Class.Word
#define PORT8_CLASS_CLASSIN1 _Port8_Class.Bits.classin1
#define PORT8_CLASS_CLASSIN2 _Port8_Class.Bits.classin2
#define PORT8_CLASS_CLASSOUT _Port8_Class.Bits.classout


/* #############################################################
/ Register Port9_Class
/ decided class
/ reset value : Port9_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort9_Class;


#define Port9_Class_ADDR 0x11D4
#define Port9_Class _Port9_Class.Word
#define PORT9_CLASS_CLASSIN1 _Port9_Class.Bits.classin1
#define PORT9_CLASS_CLASSIN2 _Port9_Class.Bits.classin2
#define PORT9_CLASS_CLASSOUT _Port9_Class.Bits.classout


/* #############################################################
/ Register Port10_Class
/ decided class
/ reset value : Port10_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort10_Class;


#define Port10_Class_ADDR 0x11D6
#define Port10_Class _Port10_Class.Word
#define PORT10_CLASS_CLASSIN1 _Port10_Class.Bits.classin1
#define PORT10_CLASS_CLASSIN2 _Port10_Class.Bits.classin2
#define PORT10_CLASS_CLASSOUT _Port10_Class.Bits.classout


/* #############################################################
/ Register Port11_Class
/ decided class
/ reset value : Port11_Class = 0000011101110111
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 classin1 : 4;
      U16 classin2 : 4;
      U16 classout : 8;
  }Bits;
}tPort11_Class;


#define Port11_Class_ADDR 0x11D8
#define Port11_Class _Port11_Class.Word
#define PORT11_CLASS_CLASSIN1 _Port11_Class.Bits.classin1
#define PORT11_CLASS_CLASSIN2 _Port11_Class.Bits.classin2
#define PORT11_CLASS_CLASSOUT _Port11_Class.Bits.classout


/* #############################################################
/ Register ClassResults
/ bit per port overall result for the classification
/ #############################################################*/
#define ClassResults_ADDR 0x11DA
/* #############################################################
/ Register Port0LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port0LastDisconnect_ADDR 0x11DC
/* #############################################################
/ Register Port1LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port1LastDisconnect_ADDR 0x11DE
/* #############################################################
/ Register Port2LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port2LastDisconnect_ADDR 0x11E0
/* #############################################################
/ Register Port3LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port3LastDisconnect_ADDR 0x11E2
/* #############################################################
/ Register Port4LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port4LastDisconnect_ADDR 0x11E4
/* #############################################################
/ Register Port5LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port5LastDisconnect_ADDR 0x11E6
/* #############################################################
/ Register Port6LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port6LastDisconnect_ADDR 0x11E8
/* #############################################################
/ Register Port7LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port7LastDisconnect_ADDR 0x11EA
/* #############################################################
/ Register Port8LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port8LastDisconnect_ADDR 0x11EC
/* #############################################################
/ Register Port9LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port9LastDisconnect_ADDR 0x11EE
/* #############################################################
/ Register Port10LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port10LastDisconnect_ADDR 0x11F0
/* #############################################################
/ Register Port11LastDisconnect
/ Reason for ports last disconnection
/ #############################################################*/
#define Port11LastDisconnect_ADDR 0x11F2
/* #############################################################
/ Register Port0InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port0InvalidSigCnt_ADDR 0x11F4
/* #############################################################
/ Register Port1InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port1InvalidSigCnt_ADDR 0x11F6
/* #############################################################
/ Register Port2InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port2InvalidSigCnt_ADDR 0x11F8
/* #############################################################
/ Register Port3InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port3InvalidSigCnt_ADDR 0x11FA
/* #############################################################
/ Register Port4InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port4InvalidSigCnt_ADDR 0x11FC
/* #############################################################
/ Register Port5InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port5InvalidSigCnt_ADDR 0x11FE
/* #############################################################
/ Register Port6InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port6InvalidSigCnt_ADDR 0x1200
/* #############################################################
/ Register Port7InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port7InvalidSigCnt_ADDR 0x1202
/* #############################################################
/ Register Port8InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port8InvalidSigCnt_ADDR 0x1204
/* #############################################################
/ Register Port9InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port9InvalidSigCnt_ADDR 0x1206
/* #############################################################
/ Register Port10InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port10InvalidSigCnt_ADDR 0x1208
/* #############################################################
/ Register Port11InvalidSigCnt
/ Cyclic counter for invalid signature events
/ #############################################################*/
#define Port11InvalidSigCnt_ADDR 0x120A
/* #############################################################
/ Register Port0PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port0PowerDeniedCnt_ADDR 0x120C
/* #############################################################
/ Register Port1PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port1PowerDeniedCnt_ADDR 0x120E
/* #############################################################
/ Register Port2PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port2PowerDeniedCnt_ADDR 0x1210
/* #############################################################
/ Register Port3PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port3PowerDeniedCnt_ADDR 0x1212
/* #############################################################
/ Register Port4PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port4PowerDeniedCnt_ADDR 0x1214
/* #############################################################
/ Register Port5PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port5PowerDeniedCnt_ADDR 0x1216
/* #############################################################
/ Register Port6PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port6PowerDeniedCnt_ADDR 0x1218
/* #############################################################
/ Register Port7PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port7PowerDeniedCnt_ADDR 0x121A
/* #############################################################
/ Register Port8PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port8PowerDeniedCnt_ADDR 0x121C
/* #############################################################
/ Register Port9PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port9PowerDeniedCnt_ADDR 0x121E
/* #############################################################
/ Register Port10PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port10PowerDeniedCnt_ADDR 0x1220
/* #############################################################
/ Register Port11PowerDeniedCnt
/ Cyclic counter for Power Denied events
/ #############################################################*/
#define Port11PowerDeniedCnt_ADDR 0x1222
/* #############################################################
/ Register Port0OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port0OverloadCnt_ADDR 0x1224
/* #############################################################
/ Register Port1OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port1OverloadCnt_ADDR 0x1226
/* #############################################################
/ Register Port2OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port2OverloadCnt_ADDR 0x1228
/* #############################################################
/ Register Port3OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port3OverloadCnt_ADDR 0x122A
/* #############################################################
/ Register Port4OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port4OverloadCnt_ADDR 0x122C
/* #############################################################
/ Register Port5OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port5OverloadCnt_ADDR 0x122E
/* #############################################################
/ Register Port6OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port6OverloadCnt_ADDR 0x1230
/* #############################################################
/ Register Port7OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port7OverloadCnt_ADDR 0x1232
/* #############################################################
/ Register Port8OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port8OverloadCnt_ADDR 0x1234
/* #############################################################
/ Register Port9OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port9OverloadCnt_ADDR 0x1236
/* #############################################################
/ Register Port10OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port10OverloadCnt_ADDR 0x1238
/* #############################################################
/ Register Port11OverloadCnt
/ Cyclic counter for Overload events
/ #############################################################*/
#define Port11OverloadCnt_ADDR 0x123A
/* #############################################################
/ Register Port0UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port0UnderloadCnt_ADDR 0x123C
/* #############################################################
/ Register Port1UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port1UnderloadCnt_ADDR 0x123E
/* #############################################################
/ Register Port2UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port2UnderloadCnt_ADDR 0x1240
/* #############################################################
/ Register Port3UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port3UnderloadCnt_ADDR 0x1242
/* #############################################################
/ Register Port4UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port4UnderloadCnt_ADDR 0x1244
/* #############################################################
/ Register Port5UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port5UnderloadCnt_ADDR 0x1246
/* #############################################################
/ Register Port6UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port6UnderloadCnt_ADDR 0x1248
/* #############################################################
/ Register Port7UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port7UnderloadCnt_ADDR 0x124A
/* #############################################################
/ Register Port8UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port8UnderloadCnt_ADDR 0x124C
/* #############################################################
/ Register Port9UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port9UnderloadCnt_ADDR 0x124E
/* #############################################################
/ Register Port10UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port10UnderloadCnt_ADDR 0x1250
/* #############################################################
/ Register Port11UnderloadCnt
/ Cyclic counter for Underload events
/ #############################################################*/
#define Port11UnderloadCnt_ADDR 0x1252
/* #############################################################
/ Register Port0ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port0ShortCnt_ADDR 0x1254
/* #############################################################
/ Register Port1ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port1ShortCnt_ADDR 0x1256
/* #############################################################
/ Register Port2ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port2ShortCnt_ADDR 0x1258
/* #############################################################
/ Register Port3ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port3ShortCnt_ADDR 0x125A
/* #############################################################
/ Register Port4ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port4ShortCnt_ADDR 0x125C
/* #############################################################
/ Register Port5ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port5ShortCnt_ADDR 0x125E
/* #############################################################
/ Register Port6ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port6ShortCnt_ADDR 0x1260
/* #############################################################
/ Register Port7ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port7ShortCnt_ADDR 0x1262
/* #############################################################
/ Register Port8ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port8ShortCnt_ADDR 0x1264
/* #############################################################
/ Register Port9ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port9ShortCnt_ADDR 0x1266
/* #############################################################
/ Register Port10ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port10ShortCnt_ADDR 0x1268
/* #############################################################
/ Register Port11ShortCnt
/ Cyclic counter for Short Circuit events
/ #############################################################*/
#define Port11ShortCnt_ADDR 0x126A
/* #############################################################
/ Register Port0ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port0ClassErrorCnt_ADDR 0x126C
/* #############################################################
/ Register Port1ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port1ClassErrorCnt_ADDR 0x126E
/* #############################################################
/ Register Port2ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port2ClassErrorCnt_ADDR 0x1270
/* #############################################################
/ Register Port3ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port3ClassErrorCnt_ADDR 0x1272
/* #############################################################
/ Register Port4ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port4ClassErrorCnt_ADDR 0x1274
/* #############################################################
/ Register Port5ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port5ClassErrorCnt_ADDR 0x1276
/* #############################################################
/ Register Port6ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port6ClassErrorCnt_ADDR 0x1278
/* #############################################################
/ Register Port7ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port7ClassErrorCnt_ADDR 0x127A
/* #############################################################
/ Register Port8ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port8ClassErrorCnt_ADDR 0x127C
/* #############################################################
/ Register Port9ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port9ClassErrorCnt_ADDR 0x127E
/* #############################################################
/ Register Port10ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port10ClassErrorCnt_ADDR 0x1280
/* #############################################################
/ Register Port11ClassErrorCnt
/ Cyclic counter for Class Error events
/ #############################################################*/
#define Port11ClassErrorCnt_ADDR 0x1282
/* #############################################################
/ Register Port0Indications
/ Class Error has occurred
/ reset value : Port0Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort0Indications;


#define Port0Indications_ADDR 0x1284
#define Port0Indications _Port0Indications.Word
#define PORT0INDICATIONS_UDL_IND _Port0Indications.Bits.udl_ind
#define PORT0INDICATIONS_OVL_IND _Port0Indications.Bits.ovl_ind
#define PORT0INDICATIONS_SHORT_IND _Port0Indications.Bits.short_ind
#define PORT0INDICATIONS_INVALIDSIG_IND _Port0Indications.Bits.invalidsig_ind
#define PORT0INDICATIONS_VALIDSIG_IND _Port0Indications.Bits.validsig_ind
#define PORT0INDICATIONS_POWERDENIED_IND _Port0Indications.Bits.powerdenied_ind
#define PORT0INDICATIONS_VALIDCAP_IND _Port0Indications.Bits.validcap_ind
#define PORT0INDICATIONS_BACKOFF_IND _Port0Indications.Bits.backoff_ind
#define PORT0INDICATIONS_CLASSERROR_IND _Port0Indications.Bits.classerror_ind


/* #############################################################
/ Register Port1Indications
/ 
/ reset value : Port1Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort1Indications;


#define Port1Indications_ADDR 0x1286
#define Port1Indications _Port1Indications.Word
#define PORT1INDICATIONS_UDL_IND _Port1Indications.Bits.udl_ind
#define PORT1INDICATIONS_OVL_IND _Port1Indications.Bits.ovl_ind
#define PORT1INDICATIONS_SHORT_IND _Port1Indications.Bits.short_ind
#define PORT1INDICATIONS_INVALIDSIG_IND _Port1Indications.Bits.invalidsig_ind
#define PORT1INDICATIONS_VALIDSIG_IND _Port1Indications.Bits.validsig_ind
#define PORT1INDICATIONS_POWERDENIED_IND _Port1Indications.Bits.powerdenied_ind
#define PORT1INDICATIONS_VALIDCAP_IND _Port1Indications.Bits.validcap_ind
#define PORT1INDICATIONS_BACKOFF_IND _Port1Indications.Bits.backoff_ind
#define PORT1INDICATIONS_CLASSERROR_IND _Port1Indications.Bits.classerror_ind


/* #############################################################
/ Register Port2Indications
/ 
/ reset value : Port2Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort2Indications;


#define Port2Indications_ADDR 0x1288
#define Port2Indications _Port2Indications.Word
#define PORT2INDICATIONS_UDL_IND _Port2Indications.Bits.udl_ind
#define PORT2INDICATIONS_OVL_IND _Port2Indications.Bits.ovl_ind
#define PORT2INDICATIONS_SHORT_IND _Port2Indications.Bits.short_ind
#define PORT2INDICATIONS_INVALIDSIG_IND _Port2Indications.Bits.invalidsig_ind
#define PORT2INDICATIONS_VALIDSIG_IND _Port2Indications.Bits.validsig_ind
#define PORT2INDICATIONS_POWERDENIED_IND _Port2Indications.Bits.powerdenied_ind
#define PORT2INDICATIONS_VALIDCAP_IND _Port2Indications.Bits.validcap_ind
#define PORT2INDICATIONS_BACKOFF_IND _Port2Indications.Bits.backoff_ind
#define PORT2INDICATIONS_CLASSERROR_IND _Port2Indications.Bits.classerror_ind


/* #############################################################
/ Register Port3Indications
/ 
/ reset value : Port3Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort3Indications;


#define Port3Indications_ADDR 0x128A
#define Port3Indications _Port3Indications.Word
#define PORT3INDICATIONS_UDL_IND _Port3Indications.Bits.udl_ind
#define PORT3INDICATIONS_OVL_IND _Port3Indications.Bits.ovl_ind
#define PORT3INDICATIONS_SHORT_IND _Port3Indications.Bits.short_ind
#define PORT3INDICATIONS_INVALIDSIG_IND _Port3Indications.Bits.invalidsig_ind
#define PORT3INDICATIONS_VALIDSIG_IND _Port3Indications.Bits.validsig_ind
#define PORT3INDICATIONS_POWERDENIED_IND _Port3Indications.Bits.powerdenied_ind
#define PORT3INDICATIONS_VALIDCAP_IND _Port3Indications.Bits.validcap_ind
#define PORT3INDICATIONS_BACKOFF_IND _Port3Indications.Bits.backoff_ind
#define PORT3INDICATIONS_CLASSERROR_IND _Port3Indications.Bits.classerror_ind


/* #############################################################
/ Register Port4Indications
/ 
/ reset value : Port4Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort4Indications;


#define Port4Indications_ADDR 0x128C
#define Port4Indications _Port4Indications.Word
#define PORT4INDICATIONS_UDL_IND _Port4Indications.Bits.udl_ind
#define PORT4INDICATIONS_OVL_IND _Port4Indications.Bits.ovl_ind
#define PORT4INDICATIONS_SHORT_IND _Port4Indications.Bits.short_ind
#define PORT4INDICATIONS_INVALIDSIG_IND _Port4Indications.Bits.invalidsig_ind
#define PORT4INDICATIONS_VALIDSIG_IND _Port4Indications.Bits.validsig_ind
#define PORT4INDICATIONS_POWERDENIED_IND _Port4Indications.Bits.powerdenied_ind
#define PORT4INDICATIONS_VALIDCAP_IND _Port4Indications.Bits.validcap_ind
#define PORT4INDICATIONS_BACKOFF_IND _Port4Indications.Bits.backoff_ind
#define PORT4INDICATIONS_CLASSERROR_IND _Port4Indications.Bits.classerror_ind


/* #############################################################
/ Register Port5Indications
/ 
/ reset value : Port5Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort5Indications;


#define Port5Indications_ADDR 0x128E
#define Port5Indications _Port5Indications.Word
#define PORT5INDICATIONS_UDL_IND _Port5Indications.Bits.udl_ind
#define PORT5INDICATIONS_OVL_IND _Port5Indications.Bits.ovl_ind
#define PORT5INDICATIONS_SHORT_IND _Port5Indications.Bits.short_ind
#define PORT5INDICATIONS_INVALIDSIG_IND _Port5Indications.Bits.invalidsig_ind
#define PORT5INDICATIONS_VALIDSIG_IND _Port5Indications.Bits.validsig_ind
#define PORT5INDICATIONS_POWERDENIED_IND _Port5Indications.Bits.powerdenied_ind
#define PORT5INDICATIONS_VALIDCAP_IND _Port5Indications.Bits.validcap_ind
#define PORT5INDICATIONS_BACKOFF_IND _Port5Indications.Bits.backoff_ind
#define PORT5INDICATIONS_CLASSERROR_IND _Port5Indications.Bits.classerror_ind


/* #############################################################
/ Register Port6Indications
/ 
/ reset value : Port6Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort6Indications;


#define Port6Indications_ADDR 0x1290
#define Port6Indications _Port6Indications.Word
#define PORT6INDICATIONS_UDL_IND _Port6Indications.Bits.udl_ind
#define PORT6INDICATIONS_OVL_IND _Port6Indications.Bits.ovl_ind
#define PORT6INDICATIONS_SHORT_IND _Port6Indications.Bits.short_ind
#define PORT6INDICATIONS_INVALIDSIG_IND _Port6Indications.Bits.invalidsig_ind
#define PORT6INDICATIONS_VALIDSIG_IND _Port6Indications.Bits.validsig_ind
#define PORT6INDICATIONS_POWERDENIED_IND _Port6Indications.Bits.powerdenied_ind
#define PORT6INDICATIONS_VALIDCAP_IND _Port6Indications.Bits.validcap_ind
#define PORT6INDICATIONS_BACKOFF_IND _Port6Indications.Bits.backoff_ind
#define PORT6INDICATIONS_CLASSERROR_IND _Port6Indications.Bits.classerror_ind


/* #############################################################
/ Register Port7Indications
/ 
/ reset value : Port7Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort7Indications;


#define Port7Indications_ADDR 0x1292
#define Port7Indications _Port7Indications.Word
#define PORT7INDICATIONS_UDL_IND _Port7Indications.Bits.udl_ind
#define PORT7INDICATIONS_OVL_IND _Port7Indications.Bits.ovl_ind
#define PORT7INDICATIONS_SHORT_IND _Port7Indications.Bits.short_ind
#define PORT7INDICATIONS_INVALIDSIG_IND _Port7Indications.Bits.invalidsig_ind
#define PORT7INDICATIONS_VALIDSIG_IND _Port7Indications.Bits.validsig_ind
#define PORT7INDICATIONS_POWERDENIED_IND _Port7Indications.Bits.powerdenied_ind
#define PORT7INDICATIONS_VALIDCAP_IND _Port7Indications.Bits.validcap_ind
#define PORT7INDICATIONS_BACKOFF_IND _Port7Indications.Bits.backoff_ind
#define PORT7INDICATIONS_CLASSERROR_IND _Port7Indications.Bits.classerror_ind


/* #############################################################
/ Register Port8Indications
/ 
/ reset value : Port8Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort8Indications;


#define Port8Indications_ADDR 0x1294
#define Port8Indications _Port8Indications.Word
#define PORT8INDICATIONS_UDL_IND _Port8Indications.Bits.udl_ind
#define PORT8INDICATIONS_OVL_IND _Port8Indications.Bits.ovl_ind
#define PORT8INDICATIONS_SHORT_IND _Port8Indications.Bits.short_ind
#define PORT8INDICATIONS_INVALIDSIG_IND _Port8Indications.Bits.invalidsig_ind
#define PORT8INDICATIONS_VALIDSIG_IND _Port8Indications.Bits.validsig_ind
#define PORT8INDICATIONS_POWERDENIED_IND _Port8Indications.Bits.powerdenied_ind
#define PORT8INDICATIONS_VALIDCAP_IND _Port8Indications.Bits.validcap_ind
#define PORT8INDICATIONS_BACKOFF_IND _Port8Indications.Bits.backoff_ind
#define PORT8INDICATIONS_CLASSERROR_IND _Port8Indications.Bits.classerror_ind


/* #############################################################
/ Register Port9Indications
/ 
/ reset value : Port9Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort9Indications;


#define Port9Indications_ADDR 0x1296
#define Port9Indications _Port9Indications.Word
#define PORT9INDICATIONS_UDL_IND _Port9Indications.Bits.udl_ind
#define PORT9INDICATIONS_OVL_IND _Port9Indications.Bits.ovl_ind
#define PORT9INDICATIONS_SHORT_IND _Port9Indications.Bits.short_ind
#define PORT9INDICATIONS_INVALIDSIG_IND _Port9Indications.Bits.invalidsig_ind
#define PORT9INDICATIONS_VALIDSIG_IND _Port9Indications.Bits.validsig_ind
#define PORT9INDICATIONS_POWERDENIED_IND _Port9Indications.Bits.powerdenied_ind
#define PORT9INDICATIONS_VALIDCAP_IND _Port9Indications.Bits.validcap_ind
#define PORT9INDICATIONS_BACKOFF_IND _Port9Indications.Bits.backoff_ind
#define PORT9INDICATIONS_CLASSERROR_IND _Port9Indications.Bits.classerror_ind


/* #############################################################
/ Register Port10Indications
/ 
/ reset value : Port10Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort10Indications;


#define Port10Indications_ADDR 0x1298
#define Port10Indications _Port10Indications.Word
#define PORT10INDICATIONS_UDL_IND _Port10Indications.Bits.udl_ind
#define PORT10INDICATIONS_OVL_IND _Port10Indications.Bits.ovl_ind
#define PORT10INDICATIONS_SHORT_IND _Port10Indications.Bits.short_ind
#define PORT10INDICATIONS_INVALIDSIG_IND _Port10Indications.Bits.invalidsig_ind
#define PORT10INDICATIONS_VALIDSIG_IND _Port10Indications.Bits.validsig_ind
#define PORT10INDICATIONS_POWERDENIED_IND _Port10Indications.Bits.powerdenied_ind
#define PORT10INDICATIONS_VALIDCAP_IND _Port10Indications.Bits.validcap_ind
#define PORT10INDICATIONS_BACKOFF_IND _Port10Indications.Bits.backoff_ind
#define PORT10INDICATIONS_CLASSERROR_IND _Port10Indications.Bits.classerror_ind


/* #############################################################
/ Register Port11Indications
/ 
/ reset value : Port11Indications = 000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 udl_ind : 1;
      U16 ovl_ind : 1;
      U16 short_ind : 1;
      U16 invalidsig_ind : 1;
      U16 validsig_ind : 1;
      U16 powerdenied_ind : 1;
      U16 validcap_ind : 1;
      U16 backoff_ind : 1;
      U16 classerror_ind : 1;
      U16 reserved : 7;
  }Bits;
}tPort11Indications;


#define Port11Indications_ADDR 0x129A
#define Port11Indications _Port11Indications.Word
#define PORT11INDICATIONS_UDL_IND _Port11Indications.Bits.udl_ind
#define PORT11INDICATIONS_OVL_IND _Port11Indications.Bits.ovl_ind
#define PORT11INDICATIONS_SHORT_IND _Port11Indications.Bits.short_ind
#define PORT11INDICATIONS_INVALIDSIG_IND _Port11Indications.Bits.invalidsig_ind
#define PORT11INDICATIONS_VALIDSIG_IND _Port11Indications.Bits.validsig_ind
#define PORT11INDICATIONS_POWERDENIED_IND _Port11Indications.Bits.powerdenied_ind
#define PORT11INDICATIONS_VALIDCAP_IND _Port11Indications.Bits.validcap_ind
#define PORT11INDICATIONS_BACKOFF_IND _Port11Indications.Bits.backoff_ind
#define PORT11INDICATIONS_CLASSERROR_IND _Port11Indications.Bits.classerror_ind


/* #############################################################
/ Register Port_PM_Indication
/ bit per port indication of power management
/ #############################################################*/
#define Port_PM_Indication_ADDR 0x129C
/* #############################################################
/ Register PowerResolutionFactor
/ factor to divide by to get the wanted power resolution
/ #############################################################*/
#define PowerResolutionFactor_ADDR 0x129E
/* #############################################################
/ Register PowerLimitMultiplier
/ Allocation multiplier for dPowerLimit calculation
/ #############################################################*/
#define PowerLimitMultiplier_ADDR 0x12A0
/* #############################################################
/ Register DiscoThMultiplier
/ Allocation multiplier for the DiscoThDelta calculation
/ #############################################################*/
#define DiscoThMultiplier_ADDR 0x12A2
/* #############################################################
/ Register PoverMultiplier
/ Multiplier for Pover calculation
/ #############################################################*/
#define PoverMultiplier_ADDR 0x12A4
/* #############################################################
/ Register PoverDivider
/ Divider for Pover calculation
/ #############################################################*/
#define PoverDivider_ADDR 0x12A6
/* #############################################################
/ Register IcutMaxAt
/ max Icut value for AT ports
/ #############################################################*/
#define IcutMaxAt_ADDR 0x12A8
/* #############################################################
/ Register LocalTotalRealPowerCons
/ Real total power consumption
/ #############################################################*/
#define LocalTotalRealPowerCons_ADDR 0x12AA
/* #############################################################
/ Register LocalTotalCalcPowerCons
/ Calculated total power consumption (class)
/ #############################################################*/
#define LocalTotalCalcPowerCons_ADDR 0x12AC
/* #############################################################
/ Register LocalCriticalPriPowerCons
/ Calculated power consumption of critical priority ports
/ #############################################################*/
#define LocalCriticalPriPowerCons_ADDR 0x12AE
/* #############################################################
/ Register LocalHighPriPowerCons
/ Calculated power consumption of high priority ports
/ #############################################################*/
#define LocalHighPriPowerCons_ADDR 0x12B0
/* #############################################################
/ Register LocalLowPriPowerCons
/ Calculated power consumption of low priority ports
/ #############################################################*/
#define LocalLowPriPowerCons_ADDR 0x12B2
/* #############################################################
/ Register Port0PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port0PowerCons_ADDR 0x12B4
/* #############################################################
/ Register Port1PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port1PowerCons_ADDR 0x12B6
/* #############################################################
/ Register Port2PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port2PowerCons_ADDR 0x12B8
/* #############################################################
/ Register Port3PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port3PowerCons_ADDR 0x12BA
/* #############################################################
/ Register Port4PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port4PowerCons_ADDR 0x12BC
/* #############################################################
/ Register Port5PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port5PowerCons_ADDR 0x12BE
/* #############################################################
/ Register Port6PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port6PowerCons_ADDR 0x12C0
/* #############################################################
/ Register Port7PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port7PowerCons_ADDR 0x12C2
/* #############################################################
/ Register Port8PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port8PowerCons_ADDR 0x12C4
/* #############################################################
/ Register Port9PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port9PowerCons_ADDR 0x12C6
/* #############################################################
/ Register Port10PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port10PowerCons_ADDR 0x12C8
/* #############################################################
/ Register Port11PowerCons
/ Real port power consumption
/ #############################################################*/
#define Port11PowerCons_ADDR 0x12CA
/* #############################################################
/ Register CriticalPriStartupPowerReq
/ Real port power consumption
/ #############################################################*/
#define CriticalPriStartupPowerReq_ADDR 0x12CC
/* #############################################################
/ Register HighPriStartupPowerReq
/ Real port power consumption
/ #############################################################*/
#define HighPriStartupPowerReq_ADDR 0x12CE
/* #############################################################
/ Register LowPriStartupPowerReq
/ Real port power consumption
/ #############################################################*/
#define LowPriStartupPowerReq_ADDR 0x12D0
/* #############################################################
/ Register ChipTotalCurrentCons
/ total current consumption for the whole chip
/ #############################################################*/
#define ChipTotalCurrentCons_ADDR 0x12D2
/* #############################################################
/ Register PowerRequestReadyFlag
/ indication that the startup power request values are ready
/ #############################################################*/
#define PowerRequestReadyFlag_ADDR 0x12D4
/* #############################################################
/ Register SysTotalCriticalCons
/ calculated system power consumption of critical priority ports
/ #############################################################*/
#define SysTotalCriticalCons_ADDR 0x12D6
/* #############################################################
/ Register SysTotalHighCons
/ calculated system power consumption of high priority ports
/ #############################################################*/
#define SysTotalHighCons_ADDR 0x12D8
/* #############################################################
/ Register SysTotalLowCons
/ calculated system power consumption of low priority ports
/ #############################################################*/
#define SysTotalLowCons_ADDR 0x12DA
/* #############################################################
/ Register SysTotalCriticalReq
/ total system power request for critical priority ports
/ #############################################################*/
#define SysTotalCriticalReq_ADDR 0x12DC
/* #############################################################
/ Register SysTotalHighReq
/ total system power request for high priority ports
/ #############################################################*/
#define SysTotalHighReq_ADDR 0x12DE
/* #############################################################
/ Register SysTotalLowReq
/ total system power request for low priority ports
/ #############################################################*/
#define SysTotalLowReq_ADDR 0x12E0
/* #############################################################
/ Register SysTotalCalcPowerCons
/ the sum of the calculated power consumption by all the system
/ #############################################################*/
#define SysTotalCalcPowerCons_ADDR 0x12E2
/* #############################################################
/ Register SysTotalPowerRequest
/ total power request for the whole system
/ #############################################################*/
#define SysTotalPowerRequest_ADDR 0x12E4
/* #############################################################
/ Register SysTotalDeltaPower
/ the power held by the system ,  for use of the rack level algorithm.
/Note - can be negative so the MSb is a sign bit.
/ #############################################################*/
#define SysTotalDeltaPower_ADDR 0x12E6
/* #############################################################
/ Register SysTotalRealPowerCons
/ the sum of the real power consumed by all the system
/ #############################################################*/
#define SysTotalRealPowerCons_ADDR 0x12E8
/* #############################################################
/ Register ActiveSlaveList
/ detected active slaves at system init
/ reset value : ActiveSlaveList = 0000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 currentactiveslaves : 8;
      U16 initialactiveslaves : 8;
  }Bits;
}tActiveSlaveList;


#define ActiveSlaveList_ADDR 0x12EA
#define ActiveSlaveList _ActiveSlaveList.Word
#define ACTIVESLAVELIST_CURRENTACTIVESLAVES _ActiveSlaveList.Bits.currentactiveslaves
#define ACTIVESLAVELIST_INITIALACTIVESLAVES _ActiveSlaveList.Bits.initialactiveslaves


/* #############################################################
/ Register TotalPowerConsSlave0
/ total real power consumption of slave0(master)
/ #############################################################*/
#define TotalPowerConsSlave0_ADDR 0x12EC
/* #############################################################
/ Register TotalPowerConsSlave1
/ total real power consumption of slave1
/ #############################################################*/
#define TotalPowerConsSlave1_ADDR 0x12EE
/* #############################################################
/ Register TotalPowerConsSlave2
/ total real power consumption of slave2
/ #############################################################*/
#define TotalPowerConsSlave2_ADDR 0x12F0
/* #############################################################
/ Register TotalPowerConsSlave3
/ total real power consumption of slave3
/ #############################################################*/
#define TotalPowerConsSlave3_ADDR 0x12F2
/* #############################################################
/ Register TotalPowerConsSlave4
/ total real power consumption of slave4
/ #############################################################*/
#define TotalPowerConsSlave4_ADDR 0x12F4
/* #############################################################
/ Register TotalPowerConsSlave5
/ total real power consumption of slave5
/ #############################################################*/
#define TotalPowerConsSlave5_ADDR 0x12F6
/* #############################################################
/ Register TotalPowerConsSlave6
/ total real power consumption of slave6
/ #############################################################*/
#define TotalPowerConsSlave6_ADDR 0x12F8
/* #############################################################
/ Register TotalPowerConsSlave7
/ total real power consumption of slave7
/ #############################################################*/
#define TotalPowerConsSlave7_ADDR 0x12FA
/* #############################################################
/ Register MasterSyncInd
/ when the master sends a detection sync this bit toggles its value
/ #############################################################*/
#define MasterSyncInd_ADDR 0x12FC
/* #############################################################
/ Register VmainHighTh
/ max Vmain level
/ #############################################################*/
#define VmainHighTh_ADDR 0x12FE
/* #############################################################
/ Register VmainAtLowTh
/ min Vmain level
/ #############################################################*/
#define VmainAtLowTh_ADDR 0x1300
/* #############################################################
/ Register VmainAfLowTh
/ min Vmain level
/ #############################################################*/
#define VmainAfLowTh_ADDR 0x1302
/* #############################################################
/ Register VmainHyster
/ delta from high or low level of Vmain to return to normal operation
/ #############################################################*/
#define VmainHyster_ADDR 0x1304
/* #############################################################
/ Register SC_V_HIGH_DELTA_AT
/ 
/ #############################################################*/
#define SC_V_HIGH_DELTA_AT_ADDR 0x1306
/* #############################################################
/ Register SC_V_HIGH_DELTA_AF
/ 
/ #############################################################*/
#define SC_V_HIGH_DELTA_AF_ADDR 0x1308
/* #############################################################
/ Register MeasuredTemp
/ the average of the two temperature measurements made by the RTP
/ #############################################################*/
#define MeasuredTemp_ADDR 0x130A
/* #############################################################
/ Register TempHighTh
/ max temperature for ports operation
/ #############################################################*/
#define TempHighTh_ADDR 0x130C
/* #############################################################
/ Register TempAlarmTh
/ temperature level for alarm
/ #############################################################*/
#define TempAlarmTh_ADDR 0x130E
/* #############################################################
/ Register TempHyster
/ temperature hysteresis to return to valid
/ #############################################################*/
#define TempHyster_ADDR 0x1310
/* #############################################################
/ Register MaxMeasuredTemp
/ highest measured temperature
/ #############################################################*/
#define MaxMeasuredTemp_ADDR 0x1312
/* #############################################################
/ Register SystemErrorFlags
/ The temperature is over the alarm threshold
/ reset value : SystemErrorFlags = 000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 vmainhigh : 1;
      U16 overtemp : 1;
      U16 disableportsactive : 1;
      U16 vmainlowaf : 1;
      U16 vmainlowat : 1;
      U16 tempalarm : 1;
      U16 reserved : 10;
  }Bits;
}tSystemErrorFlags;


#define SystemErrorFlags_ADDR 0x1314
#define SystemErrorFlags _SystemErrorFlags.Word
#define SYSTEMERRORFLAGS_VMAINHIGH _SystemErrorFlags.Bits.vmainhigh
#define SYSTEMERRORFLAGS_OVERTEMP _SystemErrorFlags.Bits.overtemp
#define SYSTEMERRORFLAGS_DISABLEPORTSACTIVE _SystemErrorFlags.Bits.disableportsactive
#define SYSTEMERRORFLAGS_VMAINLOWAF _SystemErrorFlags.Bits.vmainlowaf
#define SYSTEMERRORFLAGS_VMAINLOWAT _SystemErrorFlags.Bits.vmainlowat
#define SYSTEMERRORFLAGS_TEMPALARM _SystemErrorFlags.Bits.tempalarm


/* #############################################################
/ Register SW_LSD_CR
/ clock divider for the LMCU low clock which goes to the LSD
/ #############################################################*/
#define SW_LSD_CR_ADDR 0x1316
/* #############################################################
/ Register ExtSyncType
/ the type of the external sync wanted
/ #############################################################*/
#define SPI_ExtSyncType_ADDR 0x13A8
/* #############################################################
/ Register Port0_CR
/ Port Priority Level
/ reset value : Port0_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort0_CR;


#define Port0_CR_ADDR 0x131A
#define Port0_CR _Port0_CR.Word
#define PORT0_CR_PSE_ENABLE _Port0_CR.Bits.pse_enable
#define PORT0_CR_PAIRCONTROL _Port0_CR.Bits.paircontrol
#define PORT0_CR_PORTMODE _Port0_CR.Bits.portmode
#define PORT0_CR_PORTPRIORITY _Port0_CR.Bits.portpriority


/* #############################################################
/ Register Port1_CR
/ Port Priority Level
/ reset value : Port1_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort1_CR;


#define Port1_CR_ADDR 0x131C
#define Port1_CR _Port1_CR.Word
#define PORT1_CR_PSE_ENABLE _Port1_CR.Bits.pse_enable
#define PORT1_CR_PAIRCONTROL _Port1_CR.Bits.paircontrol
#define PORT1_CR_PORTMODE _Port1_CR.Bits.portmode
#define PORT1_CR_PORTPRIORITY _Port1_CR.Bits.portpriority


/* #############################################################
/ Register Port2_CR
/ Port Priority Level
/ reset value : Port2_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort2_CR;


#define Port2_CR_ADDR 0x131E
#define Port2_CR _Port2_CR.Word
#define PORT2_CR_PSE_ENABLE _Port2_CR.Bits.pse_enable
#define PORT2_CR_PAIRCONTROL _Port2_CR.Bits.paircontrol
#define PORT2_CR_PORTMODE _Port2_CR.Bits.portmode
#define PORT2_CR_PORTPRIORITY _Port2_CR.Bits.portpriority


/* #############################################################
/ Register Port3_CR
/ Port Priority Level
/ reset value : Port3_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort3_CR;


#define Port3_CR_ADDR 0x1320
#define Port3_CR _Port3_CR.Word
#define PORT3_CR_PSE_ENABLE _Port3_CR.Bits.pse_enable
#define PORT3_CR_PAIRCONTROL _Port3_CR.Bits.paircontrol
#define PORT3_CR_PORTMODE _Port3_CR.Bits.portmode
#define PORT3_CR_PORTPRIORITY _Port3_CR.Bits.portpriority


/* #############################################################
/ Register Port4_CR
/ Port Priority Level
/ reset value : Port4_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort4_CR;


#define Port4_CR_ADDR 0x1322
#define Port4_CR _Port4_CR.Word
#define PORT4_CR_PSE_ENABLE _Port4_CR.Bits.pse_enable
#define PORT4_CR_PAIRCONTROL _Port4_CR.Bits.paircontrol
#define PORT4_CR_PORTMODE _Port4_CR.Bits.portmode
#define PORT4_CR_PORTPRIORITY _Port4_CR.Bits.portpriority


/* #############################################################
/ Register Port5_CR
/ Port Priority Level
/ reset value : Port5_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort5_CR;


#define Port5_CR_ADDR 0x1324
#define Port5_CR _Port5_CR.Word
#define PORT5_CR_PSE_ENABLE _Port5_CR.Bits.pse_enable
#define PORT5_CR_PAIRCONTROL _Port5_CR.Bits.paircontrol
#define PORT5_CR_PORTMODE _Port5_CR.Bits.portmode
#define PORT5_CR_PORTPRIORITY _Port5_CR.Bits.portpriority


/* #############################################################
/ Register Port6_CR
/ Port Priority Level
/ reset value : Port6_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort6_CR;


#define Port6_CR_ADDR 0x1326
#define Port6_CR _Port6_CR.Word
#define PORT6_CR_PSE_ENABLE _Port6_CR.Bits.pse_enable
#define PORT6_CR_PAIRCONTROL _Port6_CR.Bits.paircontrol
#define PORT6_CR_PORTMODE _Port6_CR.Bits.portmode
#define PORT6_CR_PORTPRIORITY _Port6_CR.Bits.portpriority


/* #############################################################
/ Register Port7_CR
/ Port Priority Level
/ reset value : Port7_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort7_CR;


#define Port7_CR_ADDR 0x1328
#define Port7_CR _Port7_CR.Word
#define PORT7_CR_PSE_ENABLE _Port7_CR.Bits.pse_enable
#define PORT7_CR_PAIRCONTROL _Port7_CR.Bits.paircontrol
#define PORT7_CR_PORTMODE _Port7_CR.Bits.portmode
#define PORT7_CR_PORTPRIORITY _Port7_CR.Bits.portpriority


/* #############################################################
/ Register Port8_CR
/ Port Priority Level
/ reset value : Port8_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort8_CR;


#define Port8_CR_ADDR 0x132A
#define Port8_CR _Port8_CR.Word
#define PORT8_CR_PSE_ENABLE _Port8_CR.Bits.pse_enable
#define PORT8_CR_PAIRCONTROL _Port8_CR.Bits.paircontrol
#define PORT8_CR_PORTMODE _Port8_CR.Bits.portmode
#define PORT8_CR_PORTPRIORITY _Port8_CR.Bits.portpriority


/* #############################################################
/ Register Port9_CR
/ Port Priority Level
/ reset value : Port9_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort9_CR;


#define Port9_CR_ADDR 0x132C
#define Port9_CR _Port9_CR.Word
#define PORT9_CR_PSE_ENABLE _Port9_CR.Bits.pse_enable
#define PORT9_CR_PAIRCONTROL _Port9_CR.Bits.paircontrol
#define PORT9_CR_PORTMODE _Port9_CR.Bits.portmode
#define PORT9_CR_PORTPRIORITY _Port9_CR.Bits.portpriority


/* #############################################################
/ Register Port10_CR
/ Port Priority Level
/ reset value : Port10_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort10_CR;


#define Port10_CR_ADDR 0x132E
#define Port10_CR _Port10_CR.Word
#define PORT10_CR_PSE_ENABLE _Port10_CR.Bits.pse_enable
#define PORT10_CR_PAIRCONTROL _Port10_CR.Bits.paircontrol
#define PORT10_CR_PORTMODE _Port10_CR.Bits.portmode
#define PORT10_CR_PORTPRIORITY _Port10_CR.Bits.portpriority


/* #############################################################
/ Register Port11_CR
/ Port Priority Level
/ reset value : Port11_CR = 00010101
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 pse_enable : 2;
      U16 paircontrol : 2;
      U16 portmode : 2;
      U16 portpriority : 2;
      U16 reserved : 8;
  }Bits;
}tPort11_CR;


#define Port11_CR_ADDR 0x1330
#define Port11_CR _Port11_CR.Word
#define PORT11_CR_PSE_ENABLE _Port11_CR.Bits.pse_enable
#define PORT11_CR_PAIRCONTROL _Port11_CR.Bits.paircontrol
#define PORT11_CR_PORTMODE _Port11_CR.Bits.portmode
#define PORT11_CR_PORTPRIORITY _Port11_CR.Bits.portpriority


/* #############################################################
/ Register DisPortsCmd
/ bit per port external disable port command
/ #############################################################*/
#define DisPortsCmd_ADDR 0x1332
/* #############################################################
/ Register Port0_PPL
/ port power allocation limit
/ #############################################################*/
#define Port0_PPL_ADDR 0x1334
/* #############################################################
/ Register Port1_PPL
/ 
/ #############################################################*/
#define Port1_PPL_ADDR 0x1336
/* #############################################################
/ Register Port2_PPL
/ 
/ #############################################################*/
#define Port2_PPL_ADDR 0x1338
/* #############################################################
/ Register Port3_PPL
/ 
/ #############################################################*/
#define Port3_PPL_ADDR 0x133A
/* #############################################################
/ Register Port4_PPL
/ 
/ #############################################################*/
#define Port4_PPL_ADDR 0x133C
/* #############################################################
/ Register Port5_PPL
/ 
/ #############################################################*/
#define Port5_PPL_ADDR 0x133E
/* #############################################################
/ Register Port6_PPL
/ 
/ #############################################################*/
#define Port6_PPL_ADDR 0x1340
/* #############################################################
/ Register Port7_PPL
/ 
/ #############################################################*/
#define Port7_PPL_ADDR 0x1342
/* #############################################################
/ Register Port8_PPL
/ 
/ #############################################################*/
#define Port8_PPL_ADDR 0x1344
/* #############################################################
/ Register Port9_PPL
/ 
/ #############################################################*/
#define Port9_PPL_ADDR 0x1346
/* #############################################################
/ Register Port10_PPL
/ 
/ #############################################################*/
#define Port10_PPL_ADDR 0x1348
/* #############################################################
/ Register Port11_PPL
/ 
/ #############################################################*/
#define Port11_PPL_ADDR 0x134A
/* #############################################################
/ Register Port0_TPPL
/ temporary port power allocation limit (for layer 2 support)
/ #############################################################*/
#define Port0_TPPL_ADDR 0x134C
/* #############################################################
/ Register Port1_TPPL
/ 
/ #############################################################*/
#define Port1_TPPL_ADDR 0x134E
/* #############################################################
/ Register Port2_TPPL
/ 
/ #############################################################*/
#define Port2_TPPL_ADDR 0x1350
/* #############################################################
/ Register Port3_TPPL
/ 
/ #############################################################*/
#define Port3_TPPL_ADDR 0x1352
/* #############################################################
/ Register Port4_TPPL
/ 
/ #############################################################*/
#define Port4_TPPL_ADDR 0x1354
/* #############################################################
/ Register Port5_TPPL
/ 
/ #############################################################*/
#define Port5_TPPL_ADDR 0x1356
/* #############################################################
/ Register Port6_TPPL
/ 
/ #############################################################*/
#define Port6_TPPL_ADDR 0x1358
/* #############################################################
/ Register Port7_TPPL
/ 
/ #############################################################*/
#define Port7_TPPL_ADDR 0x135A
/* #############################################################
/ Register Port8_TPPL
/ 
/ #############################################################*/
#define Port8_TPPL_ADDR 0x135C
/* #############################################################
/ Register Port9_TPPL
/ 
/ #############################################################*/
#define Port9_TPPL_ADDR 0x135E
/* #############################################################
/ Register Port10_TPPL
/ 
/ #############################################################*/
#define Port10_TPPL_ADDR 0x1360
/* #############################################################
/ Register Port11_TPPL
/ 
/ #############################################################*/
#define Port11_TPPL_ADDR 0x1362
/* #############################################################
/ Register IndicationClearPortSelect
/ port number to be cleared using the indications clear sync event
/ #############################################################*/
#define IndicationClearPortSelect_ADDR 0x1364
/* #############################################################
/ Register ProblemPortsList
/ port is problematic indication
/ reset value : ProblemPortsList = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 portproblemind0 : 1;
      U16 portproblemind1 : 1;
      U16 portproblemind2 : 1;
      U16 portproblemind3 : 1;
      U16 portproblemind4 : 1;
      U16 portproblemind5 : 1;
      U16 portproblemind6 : 1;
      U16 portproblemind7 : 1;
      U16 portproblemind8 : 1;
      U16 portproblemind9 : 1;
      U16 portproblemind10 : 1;
      U16 portproblemind11 : 1;
      U16 reserved : 4;
  }Bits;
}tProblemPortsList;


#define ProblemPortsList_ADDR 0x1366
#define ProblemPortsList _ProblemPortsList.Word
#define PROBLEMPORTSLIST_PORTPROBLEMIND0 _ProblemPortsList.Bits.portproblemind0
#define PROBLEMPORTSLIST_PORTPROBLEMIND1 _ProblemPortsList.Bits.portproblemind1
#define PROBLEMPORTSLIST_PORTPROBLEMIND2 _ProblemPortsList.Bits.portproblemind2
#define PROBLEMPORTSLIST_PORTPROBLEMIND3 _ProblemPortsList.Bits.portproblemind3
#define PROBLEMPORTSLIST_PORTPROBLEMIND4 _ProblemPortsList.Bits.portproblemind4
#define PROBLEMPORTSLIST_PORTPROBLEMIND5 _ProblemPortsList.Bits.portproblemind5
#define PROBLEMPORTSLIST_PORTPROBLEMIND6 _ProblemPortsList.Bits.portproblemind6
#define PROBLEMPORTSLIST_PORTPROBLEMIND7 _ProblemPortsList.Bits.portproblemind7
#define PROBLEMPORTSLIST_PORTPROBLEMIND8 _ProblemPortsList.Bits.portproblemind8
#define PROBLEMPORTSLIST_PORTPROBLEMIND9 _ProblemPortsList.Bits.portproblemind9
#define PROBLEMPORTSLIST_PORTPROBLEMIND10 _ProblemPortsList.Bits.portproblemind10
#define PROBLEMPORTSLIST_PORTPROBLEMIND11 _ProblemPortsList.Bits.portproblemind11


/* #############################################################
/ Register MacroCR
/ Startup macro request
/ reset value : MacroCR = 0000000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 macroreqportlist : 12;
      U16 resreq : 1;
      U16 capreq : 1;
      U16 classreq : 1;
      U16 startupreq : 1;
  }Bits;
}tMacroCR;


#define MacroCR_ADDR 0x1368
#define MacroCR _MacroCR.Word
#define MACROCR_MACROREQPORTLIST _MacroCR.Bits.macroreqportlist
#define MACROCR_RESREQ _MacroCR.Bits.resreq
#define MACROCR_CAPREQ _MacroCR.Bits.capreq
#define MACROCR_CLASSREQ _MacroCR.Bits.classreq
#define MACROCR_STARTUPREQ _MacroCR.Bits.startupreq


/* #############################################################
/ Register PowerBudget0
/ power budget for state 000 of the power good lines
/ #############################################################*/
#define PowerBudget0_ADDR 0x136A
/* #############################################################
/ Register PowerBudget1
/ power budget for state 001 of the power good lines
/ #############################################################*/
#define PowerBudget1_ADDR 0x136C
/* #############################################################
/ Register PowerBudget2
/ power budget for state 010 of the power good lines
/ #############################################################*/
#define PowerBudget2_ADDR 0x136E
/* #############################################################
/ Register PowerBudget3
/ power budget for state 011 of the power good lines
/ #############################################################*/
#define PowerBudget3_ADDR 0x1370
/* #############################################################
/ Register PowerBudget4
/ power budget for state 100 of the power good lines
/ #############################################################*/
#define PowerBudget4_ADDR 0x1372
/* #############################################################
/ Register PowerBudget5
/ power budget for state 101 of the power good lines
/ #############################################################*/
#define PowerBudget5_ADDR 0x1374
/* #############################################################
/ Register PowerBudget6
/ power budget for state 110 of the power good lines
/ #############################################################*/
#define PowerBudget6_ADDR 0x1376
/* #############################################################
/ Register PowerBudget7
/ power budget for state 111 of the power good lines
/ #############################################################*/
#define PowerBudget7_ADDR 0x1378
/* #############################################################
/ Register DiscoTableBudget0
/ Ports to disconnect in budget 0
/ #############################################################*/
#define DiscoTableBudget0_ADDR 0x137A
/* #############################################################
/ Register DiscoTableBudget1
/ Ports to disconnect in budget 1
/ #############################################################*/
#define DiscoTableBudget1_ADDR 0x137C
/* #############################################################
/ Register DiscoTableBudget2
/ Ports to disconnect in budget 2
/ #############################################################*/
#define DiscoTableBudget2_ADDR 0x137E
/* #############################################################
/ Register DiscoTableBudget3
/ Ports to disconnect in budget 3
/ #############################################################*/
#define DiscoTableBudget3_ADDR 0x1380
/* #############################################################
/ Register DiscoTableBudget4
/ Ports to disconnect in budget 4
/ #############################################################*/
#define DiscoTableBudget4_ADDR 0x1382
/* #############################################################
/ Register DiscoTableBudget5
/ Ports to disconnect in budget 5
/ #############################################################*/
#define DiscoTableBudget5_ADDR 0x1384
/* #############################################################
/ Register DiscoTableBudget6
/ Ports to disconnect in budget 6
/ #############################################################*/
#define DiscoTableBudget6_ADDR 0x1386
/* #############################################################
/ Register DiscoTableBudget7
/ Ports to disconnect in budget 7
/ #############################################################*/
#define DiscoTableBudget7_ADDR 0x1388
/* #############################################################
/ Register ExtPMDiscoInd
/ 
/ reset value : ExtPMDiscoInd = 000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 extpmdiscoind_p0 : 1;
      U16 extpmdiscoind_p1 : 1;
      U16 extpmdiscoind_p2 : 1;
      U16 extpmdiscoind_p3 : 1;
      U16 extpmdiscoind_p4 : 1;
      U16 extpmdiscoind_p5 : 1;
      U16 extpmdiscoind_p6 : 1;
      U16 extpmdiscoind_p7 : 1;
      U16 extpmdiscoind_p8 : 1;
      U16 extpmdiscoind_p9 : 1;
      U16 extpmdiscoind_p10 : 1;
      U16 extpmdiscoind_p11 : 1;
      U16 reserved : 4;
  }Bits;
}tExtPMDiscoInd;


#define ExtPMDiscoInd_ADDR 0x138A
#define ExtPMDiscoInd _ExtPMDiscoInd.Word
#define EXTPMDISCOIND_EXTPMDISCOIND_P0 _ExtPMDiscoInd.Bits.extpmdiscoind_p0
#define EXTPMDISCOIND_EXTPMDISCOIND_P1 _ExtPMDiscoInd.Bits.extpmdiscoind_p1
#define EXTPMDISCOIND_EXTPMDISCOIND_P2 _ExtPMDiscoInd.Bits.extpmdiscoind_p2
#define EXTPMDISCOIND_EXTPMDISCOIND_P3 _ExtPMDiscoInd.Bits.extpmdiscoind_p3
#define EXTPMDISCOIND_EXTPMDISCOIND_P4 _ExtPMDiscoInd.Bits.extpmdiscoind_p4
#define EXTPMDISCOIND_EXTPMDISCOIND_P5 _ExtPMDiscoInd.Bits.extpmdiscoind_p5
#define EXTPMDISCOIND_EXTPMDISCOIND_P6 _ExtPMDiscoInd.Bits.extpmdiscoind_p6
#define EXTPMDISCOIND_EXTPMDISCOIND_P7 _ExtPMDiscoInd.Bits.extpmdiscoind_p7
#define EXTPMDISCOIND_EXTPMDISCOIND_P8 _ExtPMDiscoInd.Bits.extpmdiscoind_p8
#define EXTPMDISCOIND_EXTPMDISCOIND_P9 _ExtPMDiscoInd.Bits.extpmdiscoind_p9
#define EXTPMDISCOIND_EXTPMDISCOIND_P10 _ExtPMDiscoInd.Bits.extpmdiscoind_p10
#define EXTPMDISCOIND_EXTPMDISCOIND_P11 _ExtPMDiscoInd.Bits.extpmdiscoind_p11


/* #############################################################
/ Register SysPowerBudget0
/ System power budget for state 000 of the power good lines
/ #############################################################*/
#define SysPowerBudget0_ADDR 0x138C
/* #############################################################
/ Register SysPowerBudget1
/ System power budget for state 001 of the power good lines
/ #############################################################*/
#define SysPowerBudget1_ADDR 0x138E
/* #############################################################
/ Register SysPowerBudget2
/ System power budget for state 010 of the power good lines
/ #############################################################*/
#define SysPowerBudget2_ADDR 0x1390
/* #############################################################
/ Register SysPowerBudget3
/ System power budget for state 011 of the power good lines
/ #############################################################*/
#define SysPowerBudget3_ADDR 0x1392
/* #############################################################
/ Register SysPowerBudget4
/ System power budget for state 100 of the power good lines
/ #############################################################*/
#define SysPowerBudget4_ADDR 0x1394
/* #############################################################
/ Register SysPowerBudget5
/ System power budget for state 101 of the power good lines
/ #############################################################*/
#define SysPowerBudget5_ADDR 0x1396
/* #############################################################
/ Register SysPowerBudget6
/ System power budget for state 110 of the power good lines
/ #############################################################*/
#define SysPowerBudget6_ADDR 0x1398
/* #############################################################
/ Register SysPowerBudget7
/ System power budget for state 111 of the power good lines
/ #############################################################*/
#define SysPowerBudget7_ADDR 0x139A
/* #############################################################
/ Register UpdateRLPMParams
/ RLPM request for parameters update
/ #############################################################*/
#define UpdateRLPMParams_ADDR 0x139C
/* #############################################################
/ Register SW_ConfigReg
/ verification key for the mode change
/ reset value : SW_ConfigReg = 0000000000000011
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 swconfig : 3;
      U16 margin : 5;
      U16 modekey : 8;
  }Bits;
}tSW_ConfigReg;


#define SW_ConfigReg_ADDR 0x139E
#define SW_ConfigReg _SW_ConfigReg.Word
#define SW_CONFIGREG_SWCONFIG _SW_ConfigReg.Bits.swconfig
#define SW_CONFIGREG_MARGIN _SW_ConfigReg.Bits.margin
#define SW_CONFIGREG_MODEKEY _SW_ConfigReg.Bits.modekey


/* #############################################################
/ Register UserStatusReg
/ used by the host \ CPU to detect reset
/ #############################################################*/
#define UserStatusReg_ADDR 0x13A0
/* #############################################################
/ Register Ext_RPR_Ind
/ used by the master to indicate RPR to the slave
/ #############################################################*/
#define Ext_RPR_Ind_ADDR 0x13A2
/* #############################################################
/ Register IntOutEvEn
/ RPR
/ reset value : IntOutEvEn = 00000000000000
/ #############################################################*/

typedef union
{
  U16 Word;
  struct
  {
      U16 portonmask : 1;
      U16 portoffmask : 1;
      U16 detectionfailedmask : 1;
      U16 portfaultmask : 1;
      U16 portudlmask : 1;
      U16 portovlmask : 1;
      U16 portpmmask : 1;
      U16 powerdeniedmask : 1;
      U16 overtempmask : 1;
      U16 tempalarmmask : 1;
      U16 vmainhighmask : 1;
      U16 vmainlowafmask : 1;
      U16 vmainlowatmask : 1;
      U16 rpreventmask : 1;
      U16 reserved : 2;
  }Bits;
}tIntOutEvEn;


#define IntOutEvEn_ADDR 0x13A4
#define IntOutEvEn _IntOutEvEn.Word
#define INTOUTEVEN_PORTONMASK _IntOutEvEn.Bits.portonmask
#define INTOUTEVEN_PORTOFFMASK _IntOutEvEn.Bits.portoffmask
#define INTOUTEVEN_DETECTIONFAILEDMASK _IntOutEvEn.Bits.detectionfailedmask
#define INTOUTEVEN_PORTFAULTMASK _IntOutEvEn.Bits.portfaultmask
#define INTOUTEVEN_PORTUDLMASK _IntOutEvEn.Bits.portudlmask
#define INTOUTEVEN_PORTOVLMASK _IntOutEvEn.Bits.portovlmask
#define INTOUTEVEN_PORTPMMASK _IntOutEvEn.Bits.portpmmask
#define INTOUTEVEN_POWERDENIEDMASK _IntOutEvEn.Bits.powerdeniedmask
#define INTOUTEVEN_OVERTEMPMASK _IntOutEvEn.Bits.overtempmask
#define INTOUTEVEN_TEMPALARMMASK _IntOutEvEn.Bits.tempalarmmask
#define INTOUTEVEN_VMAINHIGHMASK _IntOutEvEn.Bits.vmainhighmask
#define INTOUTEVEN_VMAINLOWAFMASK _IntOutEvEn.Bits.vmainlowafmask
#define INTOUTEVEN_VMAINLOWATMASK _IntOutEvEn.Bits.vmainlowatmask
#define INTOUTEVEN_RPREVENTMASK _IntOutEvEn.Bits.rpreventmask


/* #############################################################
/ Register IntOutPort
/ bit per port indication of the port that had the interrupt out event
/ #############################################################*/
#define IntOutPort_ADDR 0x13A6
/* #############################################################
/ Register CableQualitySample0
/ DvDt Sample 0
/ #############################################################*/
#define CableQualitySample0_ADDR 0x13A8
/* #############################################################
/ Register CableQualitySample1
/ DvDt Sample 1
/ #############################################################*/
#define CableQualitySample1_ADDR 0x13AA
/* #############################################################
/ Register CableQualitySample2
/ DvDt Sample 2
/ #############################################################*/
#define CableQualitySample2_ADDR 0x13AC
/* #############################################################
/ Register CableQualitySample3
/ DvDt Sample 3
/ #############################################################*/
#define CableQualitySample3_ADDR 0x13AE
/* #############################################################
/ Register CableQualitySample4
/ DvDt Sample 4
/ #############################################################*/
#define CableQualitySample4_ADDR 0x13B0
/* #############################################################
/ Register CableQualitySample5
/ DvDt Sample 5
/ #############################################################*/
#define CableQualitySample5_ADDR 0x13B2
/* #############################################################
/ Register CableQualitySample6
/ DvDt Sample 6
/ #############################################################*/
#define CableQualitySample6_ADDR 0x13B4
/* #############################################################
/ Register CableQualitySample7
/ DvDt Sample 7
/ #############################################################*/
#define CableQualitySample7_ADDR 0x13B6
/* #############################################################
/ Register CableQualitySample8
/ DvDt Sample 8
/ #############################################################*/
#define CableQualitySample8_ADDR 0x13B8
/* #############################################################
/ Register CableQualitySample9
/ DvDt Sample 9
/ #############################################################*/
#define CableQualitySample9_ADDR 0x13BA
/* #############################################################
/ Register CableQualitySample10
/ DvDt Sample 10
/ #############################################################*/
#define CableQualitySample10_ADDR 0x13BC
/* #############################################################
/ Register CableQualitySample11
/ DvDt Sample 11
/ #############################################################*/
#define CableQualitySample11_ADDR 0x13BE
/* #############################################################
/ Register CableQualitySample12
/ DvDt Sample 12
/ #############################################################*/
#define CableQualitySample12_ADDR 0x13C0
/* #############################################################
/ Register CableQualitySample13
/ DvDt Sample 13
/ #############################################################*/
#define CableQualitySample13_ADDR 0x13C2
/* #############################################################
/ Register CableQualitySample14
/ DvDt Sample 14
/ #############################################################*/
#define CableQualitySample14_ADDR 0x13C4
/* #############################################################
/ Register CableQualitySample15
/ DvDt Sample 15
/ #############################################################*/
#define CableQualitySample15_ADDR 0x13C6
/* #############################################################
/ Register CableQualitySample16
/ DvDt Sample 16
/ #############################################################*/
#define CableQualitySample16_ADDR 0x13C8
/* #############################################################
/ Register CableQualitySample17
/ DvDt Sample 17
/ #############################################################*/
#define CableQualitySample17_ADDR 0x13CA
/* #############################################################
/ Register CableQualitySample18
/ DvDt Sample 18
/ #############################################################*/
#define CableQualitySample18_ADDR 0x13CC
/* #############################################################
/ Register CableQualitySample19
/ DvDt Sample 19
/ #############################################################*/
#define CableQualitySample19_ADDR 0x13CE
/* #############################################################
/ Register CableQualitySample20
/ DvDt Sample 20
/ #############################################################*/
#define CableQualitySample20_ADDR 0x13D0
/* #############################################################
/ Register CableQualitySample21
/ DvDt Sample 21
/ #############################################################*/
#define CableQualitySample21_ADDR 0x13D2
/* #############################################################
/ Register CableQualitySample22
/ DvDt Sample 22
/ #############################################################*/
#define CableQualitySample22_ADDR 0x13D4
/* #############################################################
/ Register CableQualitySample23
/ DvDt Sample 23
/ #############################################################*/
#define CableQualitySample23_ADDR 0x13D6
/* #############################################################
/ Register CableQualitySample24
/ DvDt Sample 24
/ #############################################################*/
#define CableQualitySample24_ADDR 0x13D8
/* #############################################################
/ Register CableQualitySample25
/ DvDt Sample 25
/ #############################################################*/
#define CableQualitySample25_ADDR 0x13DA
/* #############################################################
/ Register CableQualitySample26
/ DvDt Sample 26
/ #############################################################*/
#define CableQualitySample26_ADDR 0x13DC
/* #############################################################
/ Register CableQualitySample27
/ DvDt Sample 27
/ #############################################################*/
#define CableQualitySample27_ADDR 0x13DE
/* #############################################################
/ Register CableQualitySample28
/ DvDt Sample 28
/ #############################################################*/
#define CableQualitySample28_ADDR 0x13E0
/* #############################################################
/ Register CableQualitySample29
/ DvDt Sample 29
/ #############################################################*/
#define CableQualitySample29_ADDR 0x13E2
/* #############################################################
/ Register CableQualitySample30
/ DvDt Sample 30
/ #############################################################*/
#define CableQualitySample30_ADDR 0x13E4
/* #############################################################
/ Register CableQualitySample31
/ DvDt Sample 31
/ #############################################################*/
#define CableQualitySample31_ADDR 0x13E6
/* #############################################################
/ Register CableQualitySample32
/ DvDt Sample 32
/ #############################################################*/
#define CableQualitySample32_ADDR 0x13E8
/* #############################################################
/ Register CableQualitySample33
/ DvDt Sample 33
/ #############################################################*/
#define CableQualitySample33_ADDR 0x13EA
/* #############################################################
/ Register CableQualitySample34
/ DvDt Sample 34
/ #############################################################*/
#define CableQualitySample34_ADDR 0x13EC
/* #############################################################
/ Register CableQualitySample35
/ DvDt Sample 35
/ #############################################################*/
#define CableQualitySample35_ADDR 0x13EE
/* #############################################################
/ Register CableQualitySample36
/ DvDt Sample 36
/ #############################################################*/
#define CableQualitySample36_ADDR 0x13F0
/* #############################################################
/ Register CableQualitySample37
/ DvDt Sample 37
/ #############################################################*/
#define CableQualitySample37_ADDR 0x13F2
/* #############################################################
/ Register CableQualitySample38
/ DvDt Sample 38
/ #############################################################*/
#define CableQualitySample38_ADDR 0x13F4
/* #############################################################
/ Register CableQualitySample39
/ DvDt Sample 39
/ #############################################################*/
#define CableQualitySample39_ADDR 0x13F6
/* #############################################################
/ Register CableQualitySample40
/ DvDt Sample 40
/ #############################################################*/
#define CableQualitySample40_ADDR 0x13F8
/* #############################################################
/ Register CableQualitySample41
/ DvDt Sample 41
/ #############################################################*/
#define CableQualitySample41_ADDR 0x13FA
/* #############################################################
/ Register CableQualitySample42
/ DvDt Sample 42
/ #############################################################*/
#define CableQualitySample42_ADDR 0x13FC
/* #############################################################
/ Register CableQualitySample43
/ DvDt Sample 43
/ #############################################################*/
#define CableQualitySample43_ADDR 0x13FE
/* #############################################################
/ Register CableQualitySample44
/ DvDt Sample 44
/ #############################################################*/
#define CableQualitySample44_ADDR 0x1400
/* #############################################################
/ Register CableQualitySample45
/ DvDt Sample 45
/ #############################################################*/
#define CableQualitySample45_ADDR 0x1402
/* #############################################################
/ Register CableQualitySample46
/ DvDt Sample 46
/ #############################################################*/
#define CableQualitySample46_ADDR 0x1404
/* #############################################################
/ Register CableQualitySample47
/ DvDt Sample 47
/ #############################################################*/
#define CableQualitySample47_ADDR 0x1406
/* #############################################################
/ Register CableQualitySample48
/ DvDt Sample 48
/ #############################################################*/
#define CableQualitySample48_ADDR 0x1408
/* #############################################################
/ Register CableQualitySample49
/ DvDt Sample 49
/ #############################################################*/
#define CableQualitySample49_ADDR 0x140A
/* #############################################################
/ Register CableQualitySample50
/ DvDt Sample 50
/ #############################################################*/
#define CableQualitySample50_ADDR 0x140C
/* #############################################################
/ Register CableQualitySample51
/ DvDt Sample 51
/ #############################################################*/
#define CableQualitySample51_ADDR 0x140E
/* #############################################################
/ Register CableQualitySample52
/ DvDt Sample 52
/ #############################################################*/
#define CableQualitySample52_ADDR 0x1410
/* #############################################################
/ Register CableQualitySample53
/ DvDt Sample 53
/ #############################################################*/
#define CableQualitySample53_ADDR 0x1412
/* #############################################################
/ Register CableQualitySample54
/ DvDt Sample 54
/ #############################################################*/
#define CableQualitySample54_ADDR 0x1414
/* #############################################################
/ Register CableQualitySample55
/ DvDt Sample 55
/ #############################################################*/
#define CableQualitySample55_ADDR 0x1416
/* #############################################################
/ Register CableQualitySample56
/ DvDt Sample 56
/ #############################################################*/
#define CableQualitySample56_ADDR 0x1418
/* #############################################################
/ Register CableQualitySample57
/ DvDt Sample 57
/ #############################################################*/
#define CableQualitySample57_ADDR 0x141A
/* #############################################################
/ Register CableQualitySample58
/ DvDt Sample 58
/ #############################################################*/
#define CableQualitySample58_ADDR 0x141C
/* #############################################################
/ Register CableQualitySample59
/ DvDt Sample 59
/ #############################################################*/
#define CableQualitySample59_ADDR 0x141E
/* #############################################################
/ Register CableQualitySample60
/ DvDt Sample 60
/ #############################################################*/
#define CableQualitySample60_ADDR 0x1420
/* #############################################################
/ Register CableQualitySample61
/ DvDt Sample 61
/ #############################################################*/
#define CableQualitySample61_ADDR 0x1422
/* #############################################################
/ Register CableQualitySample62
/ DvDt Sample 62
/ #############################################################*/
#define CableQualitySample62_ADDR 0x1424
/* #############################################################
/ Register CableQualitySample63
/ DvDt Sample 63
/ #############################################################*/
#define CableQualitySample63_ADDR 0x1426
/* #############################################################
/ Register CableQualitySample64
/ DvDt Sample 64
/ #############################################################*/
#define CableQualitySample64_ADDR 0x1428
/* #############################################################
/ Register CableQualitySample65
/ DvDt Sample 65
/ #############################################################*/
#define CableQualitySample65_ADDR 0x142A
/* #############################################################
/ Register CableQualitySample66
/ DvDt Sample 66
/ #############################################################*/
#define CableQualitySample66_ADDR 0x142C
/* #############################################################
/ Register CableQualitySample67
/ DvDt Sample 67
/ #############################################################*/
#define CableQualitySample67_ADDR 0x142E
/* #############################################################
/ Register CableQualitySample68
/ DvDt Sample 68
/ #############################################################*/
#define CableQualitySample68_ADDR 0x1430
/* #############################################################
/ Register CableQualitySample69
/ DvDt Sample 69
/ #############################################################*/
#define CableQualitySample69_ADDR 0x1432

/* Extra registers */
#define I2C_ExtSyncType_ADDR 	0x1318
#define SWVersion_ADDR 			0x1162




#endif /* _MSCC_POE_IC_PARAMETERS_DEFINITION_H */
