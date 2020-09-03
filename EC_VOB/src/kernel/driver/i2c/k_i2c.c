/* Module Name: k_i2c.c (Kernel space)
 * Purpose: 
 *         the implementation for i2c in linux kernel mode
 *         
 *
 * Notes:  
 *        
 * History:                                                               
 *    09/03/2007 - Echo Chen, Created	
 *       
 * Copyright(C)      Accton Corporation, 2007   				
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/cpm2.h>
#include <asm/io.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "k_sysfun.h"
#include "sys_hwcfg.h"
#include "l_cvrt.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CFG_IMMR 0xF0000000

/* uSec to wait between polls of the i2c */
#define DELAY_US	100
/* uSec to wait for the CPM to start processing the buffer */
#define START_DELAY_US	1000
/*
 * tx/rx per-byte timeout: we delay DELAY_US uSec between polls so the
 * timeout will be (tx_length + rx_length) * DELAY_US * TOUT_LOOP
 */
#define TOUT_LOOP 255

/* flags for i2c_send() and i2c_receive() */
#define	I2CF_ENABLE_SECONDARY	0x01	/* secondary_address is valid	*/
#define	I2CF_START_COND		0x02	/* tx: generate start condition	*/
#define I2CF_STOP_COND		0x04	/* tx: generate stop  condition	*/

/* return codes */
#define I2CERR_NO_BUFFERS	1	/* no more BDs or buffer space	*/
#define I2CERR_MSG_TOO_LONG	2	/* tried to send/receive to much data   */
#define I2CERR_TIMEOUT		3	/* timeout in i2c_doio()	*/
#define I2CERR_QUEUE_EMPTY	4	/* i2c_doio called without send/receive */
#define I2CERR_IO_ERROR		5	/* had an error during comms	*/

/* error callback flags */
#define I2CECB_RX_ERR		0x10	/* this is a receive error	*/
#define     I2CECB_RX_OV	0x02	/* receive overrun error	*/
#define     I2CECB_RX_MASK	0x0f	/* mask for error bits		*/
#define I2CECB_TX_ERR		0x20	/* this is a transmit error	*/
#define     I2CECB_TX_CL	0x01	/* transmit collision error	*/
#define     I2CECB_TX_UN	0x02	/* transmit underflow error	*/
#define     I2CECB_TX_NAK	0x04	/* transmit no ack error	*/
#define     I2CECB_TX_MASK	0x0f	/* mask for error bits		*/
#define I2CECB_TIMEOUT		0x40	/* this is a timeout error	*/

#define ERROR_I2C_NONE		0
#define ERROR_I2C_LENGTH	1

#define I2C_WRITE_BIT		0x00
#define I2C_READ_BIT		0x01

#define I2C_RXTX_LEN	32	/* maximum tx/rx buffer length */

#define NUM_RX_BDS 4
#define NUM_TX_BDS 4
#define MAX_TX_SPACE 128

#define BD_I2C_TX_START 0x0400  /* special status for i2c: Start condition */
#define BD_I2C_TX_CL	0x0001	/* collision error */
#define BD_I2C_TX_UN	0x0002	/* underflow error */
#define BD_I2C_TX_NAK	0x0004	/* no acknowledge error */
#define BD_I2C_TX_ERR	(BD_I2C_TX_NAK|BD_I2C_TX_UN|BD_I2C_TX_CL)
#define BD_I2C_RX_ERR	BD_SC_OV

/* MACRO FUNCTION DECLARATIONS
 */
//#define PRINTD(x)
#define PRINTD(x) printk x 

#define udelay(x) {int i;volatile int j; for (i=0,j=0;i<(x);i++) j++; }

/* DATA TYPE DECLARATIONS
 */
 
typedef enum _i2c_transaction_mode
{
	I2C_MASTER_RCV =  0,
	I2C_MASTER_XMIT = 1,
} I2C_TRANSACTION_MODE;

typedef enum _i2c_status
{
 I2C_SUCCESS     = 0,
 I2C_ERROR,
} I2C_Status;

typedef void (*i2c_ecb_t)(int, int, void *);    /* error callback function */

typedef struct i2c_state {
	int		rx_idx;		/* index   to next free Rx BD */
	int		tx_idx;		/* index   to next free Tx BD */
	void		*rxbd;		/* pointer to next free Rx BD */
	void		*txbd;		/* pointer to next free Tx BD */
	int		tx_space;	/* number  of Tx bytes left   */
	unsigned char	*tx_buf;	/* pointer to free Tx area    */
	i2c_ecb_t	err_cb;		/* error callback function    */
	void		*cb_data;	/* private data to be passed  */
} i2c_state_t;

typedef struct I2C_BD_S
{
  unsigned short status;
  unsigned short length;
  unsigned char *addr;
} I2C_BD;


typedef struct I2C_ChannelInfo_S
{
    UI8_T logical_addr;
    UI8_T physical_addr;  
} I2C_ChannelInfo_T;

#define PCA9548 0xE0
//static UI8_T I2C_channel_address[]=
//{0xd0,0x52,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0};

#ifdef SYS_HWCFG_I2C_LOGICAL_ADDR_M41T11
static I2C_ChannelInfo_T channel_info[SYS_HWCFG_MAX_NBR_OF_I2C_CHANNEL] =
{
    {SYS_HWCFG_I2C_LOGICAL_ADDR_M41T11, SYS_HWCFG_I2C_PHYSICAL_ADDR_M41T11},
    {SYS_HWCFG_I2C_LOGICAL_ADDR_W83782D, SYS_HWCFG_I2C_PHYSICAL_ADDR_W83782D},
    {SYS_HWCFG_I2C_LOGICAL_ADDR_10G_MODULE_1, SYS_HWCFG_I2C_PHYSICAL_ADDR_10G_MODULE_1},
    {SYS_HWCFG_I2C_LOGICAL_ADDR_10G_MODULE_2, SYS_HWCFG_I2C_PHYSICAL_ADDR_10G_MODULE_2},
    {SYS_HWCFG_I2C_LOGICAL_ADDR_SFP_1, SYS_HWCFG_I2C_PHYSICAL_ADDR_SFP_1},
    {SYS_HWCFG_I2C_LOGICAL_ADDR_SFP_2, SYS_HWCFG_I2C_PHYSICAL_ADDR_SFP_2},
    {SYS_HWCFG_I2C_LOGICAL_ADDR_SFP_3, SYS_HWCFG_I2C_PHYSICAL_ADDR_SFP_3},
    {SYS_HWCFG_I2C_LOGICAL_ADDR_SFP_4, SYS_HWCFG_I2C_PHYSICAL_ADDR_SFP_4}
};	
#else /* Thomas added for build error */ 
static I2C_ChannelInfo_T channel_info[SYS_HWCFG_MAX_NBR_OF_I2C_CHANNEL]={0};
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T I2C_Operation(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4);
static int I2C_transaction_config(unsigned char channel_id);
static I2C_Status I2C_do_transaction( I2C_TRANSACTION_MODE act,
                               unsigned char i2c_addr,
                               unsigned char data_addr,
                               int len,
                               char *buffer);
static BOOL_T I2C_GetChannelInfoByLogicalAddr(UI8_T logical_addr, UI8_T *physical_addr, UI8_T *channel_id);
static void i2c_newio(i2c_state_t *state);
static int i2c_send(i2c_state_t *state,
			 unsigned char address,
			 unsigned char secondary_address,
			 unsigned int flags,
			 unsigned short size,
			 unsigned char *dataout);
static int i2c_receive(i2c_state_t *state,
				unsigned char address,
				unsigned char secondary_address,
				unsigned int flags,
				unsigned short size_to_expect,
				unsigned char *datain);
static int i2c_doio(i2c_state_t *state);
static int i2c_read(unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len);
static int i2c_write(unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len);
//static unsigned char i2c_reg_read(unsigned char chip, unsigned char reg);
//static int i2c_reg_write(unsigned char chip, unsigned char reg, unsigned char val);

static UI8_T tempbuffer[128];
static UI8_T *dp_rxbuff;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 
/* FUNCTION NAME: I2C_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: Initializes I2C registers, parameters,
 *                      and buffer descriptors
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  : successful
 *            FALSE : failure
 * NOTES:   Sets up I2C for write transmission to its own station
 *          address.  Interrupts are not enabled.  
 */
BOOL_T I2C_Init(void)
{
	volatile cpm2_map_t *immap = (cpm2_map_t *)CFG_IMMR ;
	volatile cpm_cpm2_t *cp = (cpm_cpm2_t *)&immap->im_cpm;
	volatile i2c_cpm2_t *i2c	= (i2c_cpm2_t *)&immap->im_i2c;
	volatile iic_t *iip;
	ulong rbase, tbase;
	volatile I2C_BD *rxbd, *txbd;
	uint dpaddr;
    //unsigned char varDiv;

        printk("<0>\n [I2C] accton module I2C Init\n");
/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
    	return TRUE;

#ifdef CFG_I2C_INIT_BOARD
	/* call board specific i2c bus reset routine before accessing the   */
	/* environment, which might be in a chip on that bus. For details   */
	/* about this problem see doc/I2C_Edge_Conditions.                  */
	i2c_init_board();
#endif
    /* Initialize Port D IIC pins.
	*/
	immap->im_ioport.iop_ppard |= 0x00030000;
	immap->im_ioport.iop_pdird &= ~0x00030000;
	immap->im_ioport.iop_podrd |= 0x00030000;
	immap->im_ioport.iop_psord |= 0x00030000;

    *(unsigned short *)(&immap->im_dprambase[PROFF_I2C_BASE]) = PROFF_I2C;
    iip = (iic_t *)&immap->im_dprambase[PROFF_I2C];

 //   *(unsigned short *)(&immap->im_dprambase[PROFF_I2C_BASE]) = PROFF_I2C;
//	dpaddr = *((unsigned short*)(&immap->im_dprambase[PROFF_I2C_BASE]));
//	if (dpaddr == 0) {
	    /* need to allocate dual port ram */
	    dpaddr = cpm_dpalloc(  /* 64+ */
		(NUM_RX_BDS * sizeof(I2C_BD)) + (NUM_TX_BDS * sizeof(I2C_BD)) +
		MAX_TX_SPACE + MAX_TX_SPACE , 8);
	//    *((unsigned short*)(&immap->im_dprambase[PROFF_I2C_BASE])) = dpaddr;
//	}

	/*
	 * initialise data in dual port ram:
	 *
	 * 	  dpaddr -> parameter ram (64 bytes)
	 *         rbase -> rx BD         (NUM_RX_BDS * sizeof(I2C_BD) bytes)
	 *         tbase -> tx BD         (NUM_TX_BDS * sizeof(I2C_BD) bytes)
	 *                  tx buffer     (MAX_TX_SPACE bytes)
	 */
    PRINTD(("<0>[I2C] dpaddr = %04x\n", dpaddr));

	// iip = (iic_t *)&immap->im_dprambase[dpaddr];
	memset((void*)iip, 0, sizeof(iic_t));

    
   // memset((void*)(dpaddr+64), 0, 
   //		(NUM_RX_BDS * sizeof(I2C_BD)) + (NUM_TX_BDS * sizeof(I2C_BD)) +
   //		MAX_TX_SPACE );

    rbase = dpaddr ;//+ 64;
	tbase = rbase + NUM_RX_BDS * sizeof(I2C_BD);

    dp_rxbuff = CFG_IMMR + tbase + NUM_TX_BDS * sizeof(I2C_BD) + MAX_TX_SPACE;
    PRINTD(("[I2C] dp_rxbuff is = %04x\n", (int)dp_rxbuff ));
    
	/* Disable interrupts */
	i2c->i2c_i2mod = 0x00;
	i2c->i2c_i2cmr = 0x00;
	i2c->i2c_i2cer = 0xff;
    
	i2c->i2c_i2add = 0xfe;

	/*
	 * Set the I2C BRG Clock division factor from desired i2c rate
	 * and current CPU rate (we assume sccr dfbgr field is 0;
	 * divide BRGCLK by 1)
	 */
	PRINTD(("[I2C] Setting rate...\n"));
    /* initialize I2C specifics:
     * first, initialize the I2C clock (keep below 100khz)
     *
     * MAX_MPU_SPEED is the likely maximum MPU speed for an MPC
     * for the near future.  We could call mpc8xx_clk_value()
     * here to determine the actual speed but to do so causes
     * very slow accesses to SROM contents.  Using MAX_MPU_SPEED
     * instead here is ugly but since the I2C clock frequency is
     * legitimately variable, it works for all the supported
     * speeds and is much faster than calculating the clock speed
     * for every SROM access.
     */

  //  varDiv = (SYS_HWCFG_MAIN_CLOCK_FREQ) / 4; 
  //  varDiv /= 70000;            /* divide by I2C bus clock - 70Kbps */
  //  varDiv /= 2;
  //  varDiv -= 3;
  //  varDiv -= 2; /*for FLT Bit set*/
  //  i2c->i2c_i2brg = varDiv;
    i2c->i2c_i2brg = 0x0f ;
    
//	i2c_setrate (gd->brg_clk, CFG_I2C_SPEED) ;

	/* Set I2C controller in master mode */
	i2c->i2c_i2com = 0x01;

	/* Initialize Tx/Rx parameters */
	iip->iic_rbase = rbase;
	iip->iic_tbase = tbase;
	rxbd = (I2C_BD *)((unsigned char *)&immap->im_dprambase[iip->iic_rbase]) ;
	txbd = (I2C_BD *)((unsigned char *)&immap->im_dprambase[iip->iic_tbase]) ;

	PRINTD(("<0>[I2C] rbase = %04x\n", iip->iic_rbase));
	PRINTD(("<0>[I2C] tbase = %04x\n", iip->iic_tbase));
	PRINTD(("<0>[I2C] rxbd = %08x\n", (int)rxbd));
	PRINTD(("<0>[I2C] txbd = %08x\n", (int)txbd));

	/* Set big endian byte order */
	iip->iic_tfcr = CPMFCR_GBL | CPMFCR_EB; //0x10;
	iip->iic_rfcr = CPMFCR_GBL | CPMFCR_EB; //0x10;

	/* Set maximum receive size. */
	iip->iic_mrblr = I2C_RXTX_LEN;

	iip->iic_rstate = 0;	
    iip->iic_rdp = 0;	
    iip->iic_rbptr = 0;	
    iip->iic_rbc = 0;	
    iip->iic_rxtmp = 0;	
    iip->iic_tstate = 0;	
    iip->iic_tdp = 0;	
    iip->iic_tbptr = 0;	
    iip->iic_tbc = 0;	
    iip->iic_txtmp = 0;


    cp->cp_cpcr = mk_cr_cmd(CPM_CR_I2C_PAGE,
							CPM_CR_I2C_SBLOCK,
							0x00,
							CPM_CR_INIT_TRX) | CPM_CR_FLG;
    do {
		__asm__ __volatile__ ("eieio");
    } while (cp->cp_cpcr & CPM_CR_FLG);

    i2c->i2c_i2mod = 0x06;

	/* Clear events and interrupts */
	i2c->i2c_i2cer = 0xff;
	i2c->i2c_i2cmr = 0x00;
    return TRUE;
}

/* FUNCTION NAME: I2C_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES:
 */
void I2C_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_I2C, I2C_Operation);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME: PHYADDR_ACCESS_Operation
 * PURPOSE:  
 * INPUT:    cmd  -- command
 *              arg1 - arg4 -- different meanings for different cmd case 
 * OUTPUT: arg1 - arg4 -- different meanings for different cmd case 
 * RETURN : 0 --Fail 
 *              1 --Success
 * NOTE:     None.
 */ 
static UI32_T I2C_Operation(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4)
{
    int cmd = L_CVRT_PTR_TO_UINT(arg0);
    int ret;
/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
	return I2C_ERROR;
    
    ret = (UI32_T)I2C_do_transaction((I2C_TRANSACTION_MODE)cmd,L_CVRT_PTR_TO_UINT(arg1),L_CVRT_PTR_TO_UINT(arg2),L_CVRT_PTR_TO_UINT(arg3),(char *)arg4);
    return ret;


}

static int I2C_transaction_config(unsigned char channel_id)
{
	  i2c_state_t state;
      unsigned char buffer[4]={0};
      int rc = 0;

/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
	return I2C_ERROR;

	  i2c_newio(&state);

      buffer[0] = (0x01<<(channel_id));
      rc =  i2c_write(PCA9548, buffer[0],1,buffer,1);
      return rc;
      
/*
      rc = i2c_send(&state,PCA9548,0,I2CF_START_COND | I2CF_STOP_COND,1,buffer);

	  if (rc != 0) {
		PRINTD(("\ni2c_read:pca9548 i2c_send failed (%d)\n", rc));
		return 1;
	  }
      
      return i2c_doio(&state);
*/
}

/* Perform the given I2C transaction, only MASTER_XMIT and MASTER_RCV
 * are implemented.  Both are only in polling mode.
 *
 * en_int controls interrupt/polling mode
 * act is the type of transaction
 * i2c_addr is the I2C address of the slave device
 * data_addr is the address of the data on the slave device
 * len is the length of data to send or receive
 * buffer is the address of the data buffer
 * stop = I2C_NO_STOP, don't signal STOP at end of transaction
 *        I2C_STOP, signal STOP at end of transaction
 * retry is the timeout retry value, currently ignored
 * rsta = I2C_NO_RESTART, this is not continuation of existing transaction
 *        I2C_RESTART, this is a continuation of existing transaction
 */
 
static I2C_Status I2C_do_transaction( I2C_TRANSACTION_MODE act,
                               unsigned char i2c_addr,
                               unsigned char data_addr,
                               int len,
                               char *buffer)
{
  int status;
  unsigned char data_addr_buffer[1];  
  unsigned char channel_id=0;
  unsigned char channel_addr=0;  
    
 // SYSFUN_CopyFromUser(tempbuffer,buffer,len * sizeof (UI8_T));

/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
	return I2C_ERROR;


  SYSFUN_CopyFromUser(dp_rxbuff,buffer,len * sizeof (UI8_T));

  if (I2C_GetChannelInfoByLogicalAddr(i2c_addr, &channel_addr, &channel_id)==FALSE)
  {
        return I2C_ERROR;
  }
        
   data_addr_buffer[0] = data_addr;
   
   status=I2C_transaction_config(channel_id);
   if (status != 0 )                                                      
   {            
       PRINTD(("<0>\n\ri2c PCA9548 set channel error!"));
       return I2C_ERROR;                                                            
   }                                       
   udelay(10000);
   PRINTD(("<0>\n\ri2c PCA9548 set channel success!"));

   if (act == I2C_MASTER_RCV)
   {
        //status =i2c_read(channel_addr,data_addr,1,tempbuffer,len);
        status =i2c_read(channel_addr,data_addr,1,dp_rxbuff,len);

    } /* end receive mode */
    else
    {
        //status = i2c_write(channel_addr,data_addr,1,tempbuffer,len);
        status = i2c_write(channel_addr,data_addr,1,dp_rxbuff,len);
    }  /* end write mode */                       
                                                        
                                                                         
  if (status != 0)
  {
    return I2C_ERROR;
  }
  else
  {
    //SYSFUN_CopyToUser(buffer,tempbuffer,len * sizeof (UI8_T));
    SYSFUN_CopyToUser(buffer,dp_rxbuff,len * sizeof (UI8_T));
   // PRINTD(("<0>\n[I2C] Success!data[0]  = %02x \n", tempbuffer[0] ));
    PRINTD(("<0>\n[I2C] Success!data[0]  = %02x \n", *dp_rxbuff ));
    return I2C_SUCCESS;
  }

}
   
/* FUNCTION NAME : I2C_GetChannelInfoByLogicalAddr
 * PURPOSE : This function translate the I2C logical address to physical address and channel id
 * INPUT   : logical_addr
 * OUTPUT  : physical_addr and channel_id
 * RETUEN  : TRUE/FALSE
 * NOTES   : 
 */
static BOOL_T I2C_GetChannelInfoByLogicalAddr(UI8_T logical_addr, UI8_T *physical_addr, UI8_T *channel_id)
{

    UI8_T i=0;

/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
	return I2C_ERROR;


    for (i=0; i<=SYS_HWCFG_MAX_NBR_OF_I2C_CHANNEL-1; i++)
    {
        if (channel_info[i].logical_addr==logical_addr)        
            break;
        
    }
    if (i>=SYS_HWCFG_MAX_NBR_OF_I2C_CHANNEL)
    {
        return FALSE;/*logical address not found*/
    }
    else
    {
        *physical_addr=channel_info[i].physical_addr;
        *channel_id=i;
 	     PRINTD(("<0>\n[I2C] physical_addr  = %02x ,channel_id = %02x\n", (int) *physical_addr   ,(int)*channel_id  ));
        return TRUE;
    }                
}  

/* FUNCTION NAME: i2c_newio
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : 
 * NOTES:    
 */
static void i2c_newio(i2c_state_t *state)
{
	volatile cpm2_map_t *immap = (cpm2_map_t *)CFG_IMMR ;
	volatile iic_t *iip;
	unsigned int dpaddr;

	PRINTD(("[I2C] i2c_newio\n"));




    dpaddr = *((unsigned short*)(&immap->im_dprambase[PROFF_I2C_BASE]));
	iip = (iic_t *)&immap->im_dprambase[dpaddr];
    
	state->rx_idx = 0;
	state->tx_idx = 0;
	state->rxbd = (void*)&immap->im_dprambase[iip->iic_rbase];
	state->txbd = (void*)&immap->im_dprambase[iip->iic_tbase];
    
	state->tx_space = MAX_TX_SPACE;
	state->tx_buf = (unsigned char*)state->txbd + NUM_TX_BDS * sizeof(I2C_BD);

    state->err_cb = NULL;
	state->cb_data = NULL;

	PRINTD(("<0>[I2C] rxbd = %08x\n", (int)state->rxbd));
	PRINTD(("<0>[I2C] txbd = %08x\n", (int)state->txbd));
	PRINTD(("<0>[I2C] tx_buf = %08x\n", (int)state->tx_buf));

	/* clear the buffer memory */
	memset((char *)state->tx_buf, 0, MAX_TX_SPACE);
}

/* FUNCTION NAME: i2c_send
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : 
 * NOTES:    
 */
static int i2c_send(i2c_state_t *state,
			 unsigned char address,
			 unsigned char secondary_address,
			 unsigned int flags,
			 unsigned short size,
			 unsigned char *dataout)
{
	volatile I2C_BD *txbd;
	int i,j;

	PRINTD(("[I2C] i2c_send add=%02d sec=%02d flag=%02d size=%d\n",
			address, secondary_address, flags, size));
/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
    return 1;

	/* trying to send message larger than BD */
	if (size > I2C_RXTX_LEN)
	  return I2CERR_MSG_TOO_LONG;

	/* no more free bds */
	if (state->tx_idx >= NUM_TX_BDS || state->tx_space < (2 + size))
	  return I2CERR_NO_BUFFERS;

	txbd = (I2C_BD *)state->txbd;

    state->tx_buf = (unsigned char *)(((unsigned int )state->tx_buf  + 15) & ~15);
    txbd->addr = state->tx_buf;

	PRINTD(("[I2C] txbd = %08x\n", (int)txbd));

    if (flags & I2CF_START_COND)
    {
	PRINTD(("[I2C] Formatting addresses...\n"));
	if (flags & I2CF_ENABLE_SECONDARY)
	{
		txbd->length = size + 2;  /* Length of message plus dest addresses */
		txbd->addr[0] = address ;//    << 1;
		txbd->addr[1] = secondary_address;
		i = 2;
	}
	else
	{
		txbd->length = size + 1;  /* Length of message plus dest address */
		txbd->addr[0] = address ;//<< 1;  /* Write destination address to BD */
		i = 1;
	}
    }
    else
    {
	txbd->length = size;  /* Length of message */
	i = 0;
    }

	/* set up txbd */
	txbd->status = BD_SC_READY;
	if (flags & I2CF_START_COND)
	  txbd->status |= BD_I2C_TX_START;
	if (flags & I2CF_STOP_COND)
	  txbd->status |= BD_SC_LAST | BD_SC_WRAP;

	/* Copy data to send into buffer */
	PRINTD(("[I2C] copy data...\n"));
	for(j = 0; j < size; i++, j++)
	  txbd->addr[i] = dataout[j];

	PRINTD(("[I2C] txbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		   txbd->length,
		   txbd->status,
		   txbd->addr[0],
		   txbd->addr[1]));

	/* advance state */
	state->tx_buf += txbd->length;
	state->tx_space -= txbd->length;
	state->tx_idx++;
	state->txbd = (void*)(txbd + 1);

	return 0;
}

/* FUNCTION NAME: i2c_receive
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : 
 * NOTES:    
 */
static int i2c_receive(i2c_state_t *state,
				unsigned char address,
				unsigned char secondary_address,
				unsigned int flags,
				unsigned short size_to_expect,
				unsigned char *datain)
{
	volatile I2C_BD *rxbd, *txbd;
 

	PRINTD(("[I2C] i2c_receive %02d %02d %02d\n", address, secondary_address, flags));
/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
    return 1;


	/* Expected to receive too much */
	if (size_to_expect > I2C_RXTX_LEN)
	  return I2CERR_MSG_TOO_LONG;

	/* no more free bds */
	if (state->tx_idx >= NUM_TX_BDS || state->rx_idx >= NUM_RX_BDS
		 || state->tx_space < 2)
	  return I2CERR_NO_BUFFERS;

	rxbd = (I2C_BD *)state->rxbd;
	txbd = (I2C_BD *)state->txbd;

	PRINTD(("[I2C] rxbd = %08x\n", (int)rxbd));
	PRINTD(("[I2C] txbd = %08x\n", (int)txbd));

  //  state->tx_buf = (unsigned char *)(((unsigned int )state->tx_buf  + 15) & ~15);
  //  txbd->addr = state->tx_buf;
   
	/* set up TXBD for destination address */
  
	if (flags & I2CF_ENABLE_SECONDARY)
	{
		txbd->length = 2;
		txbd->addr[0] = address ;//<< 1;   /* Write data */
		txbd->addr[1] = secondary_address;  /* Internal address */
		txbd->status = BD_SC_READY;
	}
	else
	{
		txbd->length = 1 + size_to_expect;
		txbd->addr[0] = (address /*<< 1*/) | 0x01;
		txbd->status = BD_SC_READY;
		memset(&txbd->addr[1], 0, txbd->length);
	}
    
    /* set up rxbd for reception */
	rxbd->status = BD_SC_EMPTY;
	rxbd->length =  size_to_expect;
	rxbd->addr = datain ; //(unsigned char *)__pa( datain );

    PRINTD(("[I2C] receive buff addr  = %08x\n", (int)datain));
    PRINTD(("[I2C] rxbd->addr  = %08x\n", (int)rxbd->addr));
    
    //dma_cache_inv ( datain , size_to_expect );
    
	txbd->status |= BD_I2C_TX_START;
	if (flags & I2CF_STOP_COND)
	{
		txbd->status |= BD_SC_LAST | BD_SC_WRAP;
		rxbd->status |= BD_SC_WRAP;
	}
/*
	PRINTD(("[I2C] txbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		   txbd->length,
		   txbd->status,
		   txbd->addr[0],
		   txbd->addr[1]));
	PRINTD(("[I2C] rxbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		   rxbd->length,
		   rxbd->status,
		   rxbd->addr[0],
		   rxbd->addr[1]));
*/
	/* advance state */
	state->tx_buf +=  txbd->length;
	state->tx_space -= txbd->length;
	state->tx_idx++;
	state->txbd = (void*)(txbd + 1);
	state->rx_idx++;
	state->rxbd = (void*)(rxbd + 1);

	return 0;
}

/* FUNCTION NAME: i2c_doio
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : 
 * NOTES:    
 */
static int i2c_doio(i2c_state_t *state)
{
	volatile cpm2_map_t *immap = (cpm2_map_t *)CFG_IMMR ;
	volatile iic_t *iip;
	volatile i2c_cpm2_t  *i2c	= (i2c_cpm2_t  *)&immap->im_i2c;
	volatile I2C_BD *txbd, *rxbd;
	int  n, i, b, rxcnt = 0, rxtimeo = 0, txcnt = 0, txtimeo = 0, rc = 0;
	unsigned int dpaddr;

	PRINTD(("[I2C] i2c_doio\n"));

	if (state->tx_idx <= 0 && state->rx_idx <= 0) {
		PRINTD(("[I2C] No I/O is queued\n"));
		return I2CERR_QUEUE_EMPTY;
	}

	dpaddr = *((unsigned short*)(&immap->im_dprambase[PROFF_I2C_BASE]));
	iip = (iic_t *)&immap->im_dprambase[dpaddr];
    
	iip->iic_rbptr = iip->iic_rbase;
	iip->iic_tbptr = iip->iic_tbase;

    iip->iic_mrblr = 1; /* prevent excessive read */ /* for test */

	/* Enable I2C */
	PRINTD(("[I2C] Enabling I2C...\n"));
	i2c->i2c_i2mod |= 0x01;

	/* Begin transmission */
	i2c->i2c_i2com = 0x81;

	/* Loop until transmit & receive completed */

	if ((n = state->tx_idx) > 0) {

		txbd = ((I2C_BD*)state->txbd) - n;
		for (i = 0; i < n; i++) {
			txtimeo += TOUT_LOOP * txbd->length;
			txbd++;
		}

		txbd--; /* wait until last in list is done */

		PRINTD(("[I2C] Transmitting...(txbd=0x%08lx)\n", (ulong)txbd));

		udelay(START_DELAY_US);	/* give it time to start */
		while((txbd->status & BD_SC_READY) && (++txcnt < txtimeo)) {
			udelay(DELAY_US);
			__asm__ __volatile__ ("eieio");
		}
	}

	if (txcnt < txtimeo && (n = state->rx_idx) > 0) {

		rxbd = ((I2C_BD*)state->rxbd) - n;
		for (i = 0; i < n; i++) {
			rxtimeo += TOUT_LOOP * rxbd->length;
			rxbd++;
		}

		rxbd--; /* wait until last in list is done */

		PRINTD(("[I2C] Receiving...(rxbd=0x%08lx)\n", (ulong)rxbd));

		udelay(START_DELAY_US);	/* give it time to start */
		while((rxbd->status & BD_SC_EMPTY) && (++rxcnt < rxtimeo)) {
			udelay(DELAY_US);
			__asm__ __volatile__ ("eieio");
		}
	}

	/* Turn off I2C */
    i2c->i2c_i2mod &= ~0x01;

	if ((n = state->tx_idx) > 0) {
		for (i = 0; i < n; i++) {
			txbd = ((I2C_BD*)state->txbd) - (n - i);
			if ((b = txbd->status & BD_I2C_TX_ERR) != 0) {
				if (state->err_cb != NULL)
					(*state->err_cb)(I2CECB_TX_ERR|b, i,
						state->cb_data);
				if (rc == 0)
					rc = I2CERR_IO_ERROR;
			}
		}
	}

	if ((n = state->rx_idx) > 0) {
		for (i = 0; i < n; i++) {
			rxbd = ((I2C_BD*)state->rxbd) - (n - i);
			if ((b = rxbd->status & BD_I2C_RX_ERR) != 0) {
				if (state->err_cb != NULL)
					(*state->err_cb)(I2CECB_RX_ERR|b, i,
						state->cb_data);
				if (rc == 0)
					rc = I2CERR_IO_ERROR;
			}
		}
	}

	if ((txtimeo > 0 && txcnt >= txtimeo) || (rxtimeo > 0 && rxcnt >= rxtimeo)) {
		if (state->err_cb != NULL)
			(*state->err_cb)(I2CECB_TIMEOUT, -1, state->cb_data);
		if (rc == 0)
			rc = I2CERR_TIMEOUT;
	}
    //PRINTD(("txcnt=0x%08lx ,rxcnt=0x%08lx  \n", (ulong)txcnt ,(ulong)rxcnt ));
	return (rc);
}

/* FUNCTION NAME: i2c_read
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : 
 * NOTES:    
 */
static int i2c_read(unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len)
{
	i2c_state_t state;
	unsigned char xaddr[4];
	int rc;
   // unsigned char w_buffer[257];
/*Add by rosemary.zhang,02/20/2008,i2c can't work now,so close read*/
/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
	return 1;


    xaddr[0] = (addr >> 24) & 0xFF;
	xaddr[1] = (addr >> 16) & 0xFF;
	xaddr[2] = (addr >>  8) & 0xFF;
	xaddr[3] =  addr        & 0xFF;

#ifdef CFG_I2C_EEPROM_ADDR_OVERFLOW
	 /*
	  * EEPROM chips that implement "address overflow" are ones
	  * like Catalyst 24WC04/08/16 which has 9/10/11 bits of address
	  * and the extra bits end up in the "chip address" bit slots.
	  * This makes a 24WC08 (1Kbyte) chip look like four 256 byte
	  * chips.
	  *
	  * Note that we consider the length of the address field to still
	  * be one byte because the extra address bits are hidden in the
	  * chip address.
	  */
	chip |= ((addr >> (alen * 8)) & CFG_I2C_EEPROM_ADDR_OVERFLOW);
#endif
	i2c_newio(&state);


	rc = i2c_send(&state, chip, 0, I2CF_START_COND, alen, &xaddr[4-alen]);
	if (rc != 0) {
		PRINTD(("i2c_read: i2c_send failed (%d)\n", rc));
		return 1;
	}

	rc = i2c_receive(&state, chip, 0, I2CF_STOP_COND, len, buffer);
	if (rc != 0) {
		PRINTD(("i2c_read: i2c_receive failed (%d)\n", rc));
		return 1;
	}

#if 0
	rc = i2c_receive(&state, xaddr[4-alen] , 0, I2CF_STOP_COND, len, buffer);
	if (rc != 0) {
		PRINTD(("i2c_read: i2c_receive failed (%d)\n", rc));
		return 1;
    }
#endif        
	rc = i2c_doio(&state);
	if (rc != 0) {
		PRINTD(("i2c_read: i2c_doio failed (%d)\n", rc));
		return 1;
	}
	return 0;
}

/* FUNCTION NAME: i2c_write
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : 
 * NOTES:    
 */
static int i2c_write(unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len)
{
	i2c_state_t state;
	unsigned char xaddr[4];
	int rc;

/*Add by rosemary.zhang,02/20/2008,i2c can't work now,so close read*/
/*del MACRO: ECN430_FB2,michael.wang 2008-6-25*/
	return 1;

	xaddr[0] = (addr >> 24) & 0xFF;
	xaddr[1] = (addr >> 16) & 0xFF;
	xaddr[2] = (addr >>  8) & 0xFF;
	xaddr[3] =  addr        & 0xFF;

#ifdef CFG_I2C_EEPROM_ADDR_OVERFLOW
	 /*
	  * EEPROM chips that implement "address overflow" are ones
	  * like Catalyst 24WC04/08/16 which has 9/10/11 bits of address
	  * and the extra bits end up in the "chip address" bit slots.
	  * This makes a 24WC08 (1Kbyte) chip look like four 256 byte
	  * chips.
	  *
	  * Note that we consider the length of the address field to still
	  * be one byte because the extra address bits are hidden in the
	  * chip address.
	  */
	chip |= ((addr >> (alen * 8)) & CFG_I2C_EEPROM_ADDR_OVERFLOW);
#endif

	i2c_newio(&state);

	rc = i2c_send(&state, chip, 0, I2CF_START_COND, alen, &xaddr[4-alen]);
	if (rc != 0) {
		PRINTD(("i2c_write: first i2c_send failed (%d)\n", rc));
		return 1;
	}

	rc = i2c_send(&state, 0, 0, I2CF_STOP_COND, len, buffer);
	if (rc != 0) {
		PRINTD(("i2c_write: second i2c_send failed (%d)\n", rc));
		return 1;
	}

	rc = i2c_doio(&state);
	if (rc != 0) {
		PRINTD(("i2c_write: i2c_doio failed (%d)\n", rc));
		return 1;
	}
	return 0;
}
#if 0
/* FUNCTION NAME: i2c_reg_read
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : 
 * NOTES:    
 */
static unsigned char i2c_reg_read(unsigned char chip, unsigned char reg)
{
	unsigned char buf;

	i2c_read(chip, reg, 1, &buf, 1);

	return (buf);
}

/* FUNCTION NAME: i2c_reg_write
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : 
 * NOTES:    
 */
static int i2c_reg_write(unsigned char chip, unsigned char reg, unsigned char val)
{
	return i2c_write(chip, reg, 1, &val, 1);
}
#endif

