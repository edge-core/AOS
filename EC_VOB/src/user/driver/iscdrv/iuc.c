/* Module Name: IUC.C
 * Purpose: 
 *      This module provides an interface for Inter-Unit-Communucation(which IUC stands for)
 *      between stacked switch units. IUC relay packet from ISC to NIC and passing received 
 *      packet from NIC to ISC.
 * Notes: 
 * History:                                                               
 *    
 * Copyright(C)      Accton Corporation, 2005                   
 */
#include <string.h>
#include <stdio.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_stdlib.h"
#include "l_mm.h"
#include "iuc.h"
#include "dev_nicdrv_type.h"
#include "dev_nicdrv_pmgr.h"
#include "sys_dflt.h"
#include "sys_module.h"
#include "stktplg_om.h"
#include "lan_type.h"
#include "backdoor_mgr.h"
#include "isc_om.h"

/* Here, we treat IUC and ISC as a component. So in IUC, declare used ISC functions
 * as extern functions and call them directly.
 */
extern void ISC_PacketIncoming(UI32_T rx_port, L_MM_Mref_Handle_T *mem_ref);
extern LAN_TYPE_PacketType_T ISC_GetPacketType(UI8_T *data_p);

/* NAMING CONSTANT DECLARACTIONS
 */
#define STK_TPID        0x8100

#ifdef SYS_ADPT_VLAN_INTERNAL_MANAGEMENT_VLAN
    #define STK_VLAN_ID     SYS_ADPT_VLAN_INTERNAL_MANAGEMENT_VLAN
#else                       
    #define STK_VLAN_ID     0
#endif

#define IUC_ETHERNET_TYPE           DEV_NICDRV_IUC_ETHERNET_TYPE 
#define IUC_ETHERNET_HEADER_LEN     sizeof(IUC_EthernetHeader_T)

#define IUC_INVALID_PORT_ID  0xFF

/* DATA TYPE DECLARACTIONS
 */
typedef char MAC_T[6];

typedef struct  {
    MAC_T     d_mcaddr;     /* destination MAC address */
    MAC_T     s_mcaddr;     /* source MAC address      */
    UI16_T    tpid;         /* tag control             */
    UI16_T    vtag;         /* vlan tag                */
    UI16_T    type;         /* ethernet type/length    */
    UI8_T     reserved[2];  /* make the size of IUC_EthernetHeader_T multiple of 4 bytes */   
} __attribute__((packed, aligned(1)))IUC_EthernetHeader_T;


/* LOCAL FUNCTION DECLARATIONS
 */
static BOOL_T IUC_GetDrvUnitMac(UI32_T drv_unit, MAC_T mac_addr);
void   IUC_PacketArrival(I32_T device_id, UI32_T rx_port, void *cookie, UI32_T reason, L_MM_Mref_Handle_T *mem_ref);

LAN_TYPE_PacketType_T IUC_GetPacketType(UI8_T *data_p,UI32_T rx_port);

/* backdoor functions
 */
static UI32_T IUC_BD_InputInteger(void);
static void IUC_BD_DisplayPacket(L_MM_Mref_Handle_T *mem_ref);
static void IUC_BD_DisplayDataByHex(UI8_T *data, UI32_T length);
static void IUC_BD_BackdoorMenu(void);
static void IUC_BD_SendSamplePacket(void);

/* STATIC VARIABLES DECLARATIONS
 */
/* base mac address that use to compute specify unit's mac
 */
static const MAC_T  mac_base_unit_addr = {0x00,0x10,0x18,0xBE,0xEF,0x00}; 

/* fill in the DA when send packet to next unit by crossbar
 */
static const MAC_T  cpu_mac_addr       = {0x00,0x00,0xf1,0x0d,0x00,0x00};

/* multicast mac address that will fill in DA when send packet to all stack unit by crossbar
 */
static const MAC_T  mac_multicast      = {0x01,0xFF,0xFF,0xFF,0xFF,0xFF};

static BOOL_T iuc_bd_capture_received;
static BOOL_T iuc_bd_capture_transmitted;
static BOOL_T iuc_bd_show;
static UI32_T iuc_bd_rx_numeber[10]={0};

/* EXPORTED SUBPROGRAM BODIES
 */ 
 
/* FUNCTION NAME:   IUC_Init
 * PURPOSE: 
 *          Initial system variables of IUC
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:   
 */
void IUC_Init(void)
{   
    iuc_bd_capture_received = FALSE;
    iuc_bd_capture_transmitted = FALSE;
        
    return;
}  


/* FUNCTION NAME : IUC_Create_InterCSC_Relation
 * PURPOSE: 
 *          This function initiates all function pointer registration operations.
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          Register a callback function IUC_PacketArrival() to NIC driver
 */
void IUC_Create_InterCSC_Relation(void)
{
    //DEV_NICDRV_Register_RecvPacket_Handler(DEV_NICDRV_PROTOCOL_IUC,(void*)IUC_PacketArrival);
    //DEV_NICDRV_Register_GetPacketTypeFunc((void *)IUC_GetPacketType);
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("IUC", 
                    SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, IUC_BD_BackdoorMenu);  
    
    return;
}

  
/* FUNCTION NAME:   IUC_SendPacket
 * PURPOSE: 
 *          Send an IUC packet
 * INPUT:   
 *          mem_ref          -- points to the data block that holds the packet content
 *          dest_unit        -- unit_id or IUC_STACK_UNIT_NEXT, IUC_STACK_UNIT_ALL
 *          Uplink_Downlink  -- LAN_TYPE_TX_UP_LINK: Uplink
 *                              LAN_TYPE_TX_DOWN_LINK: Downlink
 *                              LAN_TYPE_TX_UP_DOWN_LINKs: Uplink and Downlink 
 *          priority         -- the send packet priority
 * OUTPUT:  
 *          None
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 */
BOOL_T  IUC_SendPacket(L_MM_Mref_Handle_T *mem_ref, UI32_T dst_unit, UI8_T Uplink_Downlink, UI32_T priority)
{
    UI32_T                  pdu_len;
    IUC_EthernetHeader_T    *enet_hdr;

    enet_hdr = (IUC_EthernetHeader_T *)L_MM_Mref_MovePdu (mem_ref, (I32_T)(0-IUC_ETHERNET_HEADER_LEN), &pdu_len);
    
    /* Get source mac addr in Ethernet Header
     */
    IUC_GetDrvUnitMac (ISC_OM_GetMyDrvUnitId(), enet_hdr->s_mcaddr);
    
    if( dst_unit == IUC_STACK_UNIT_NEXT )       /* send to next hop */
    {
        memcpy(enet_hdr->d_mcaddr,cpu_mac_addr, sizeof(MAC_T));
        dst_unit = DEV_NICDRV_CPU_PKT;
    }
    else if ( dst_unit == IUC_STACK_UNIT_ALL )  /* send to all unit */
    {
        memcpy(enet_hdr->d_mcaddr, mac_multicast, sizeof(MAC_T) );
        /* Should make sure that DEV_NICDRV_SendIUCPacket() implement the
         * operation of sending multicast packets efficiently, instead of 
         * sending them individually when dst_unit = DEV_NICDRV_MC_PKT
         */
        dst_unit = DEV_NICDRV_MC_PKT;
    }
    else
    {
        IUC_GetDrvUnitMac (dst_unit, enet_hdr->d_mcaddr);
    }    
        
    enet_hdr->tpid = L_STDLIB_Hton16(STK_TPID);  /* VLAN tag */
    enet_hdr->vtag =  L_STDLIB_Hton16(STK_VLAN_ID | priority<<13);
    enet_hdr->type =  L_STDLIB_Hton16(IUC_ETHERNET_TYPE);
    
    if(iuc_bd_capture_transmitted == TRUE)
    {
        printf("\n");
        printf("========================IUC_SendPacket=====================\n");
        printf("*     Shown before passing to DEV_NICDRV_SendIUCPacket    *\n");
        printf("===========================================================\n");
        IUC_BD_DisplayPacket(mem_ref);
    }  
    
    return(DEV_NICDRV_PMGR_SendIUCPacket(mem_ref, dst_unit, Uplink_Downlink, priority));
}

/* FUNCTION NAME:   IUC_GetDrvUnitMac
 * PURPOSE: 
 *          compute mac address of specified unit
 * INPUT:   
 *          drv_unit    -- the driver unit number
 * OUTPUT:  
 *          mac_addr    -- computed mac address of drv_unit
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 *         1. This is used to compute the destination unit mac address which will be fill
 *            in IUC Ethernet header's DA
 */         
static BOOL_T IUC_GetDrvUnitMac(UI32_T drv_unit, MAC_T mac_addr)
{
    memcpy(mac_addr, mac_base_unit_addr, sizeof(MAC_T));
    mac_addr[5] += (drv_unit - 1);
    
    return TRUE;
}


/* FUNCTION NAME: IUC_PacketArrival
 * PURPOSE: 
 *          
 * INPUT:   
 *          mem_ref   -- points to the data block that holds the packet content
 *          device_id -- the device id where packet is received
 *          rx_port   -- the rx port of recieve packet device
 *          cookie    -- not use
 *          reason    -- not use
 *          size      -- not use
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:  1. This function is register to DEV_NICDRV as a callback function
 *         2. The packet must be freed in this function before return
 */
void IUC_PacketArrival(I32_T device_id, UI32_T rx_port, void *cookie, UI32_T reason, L_MM_Mref_Handle_T *mem_ref)
{
    UI32_T                  pdu_len;
    IUC_EthernetHeader_T    *iuc_ether_hdr;
    
    iuc_ether_hdr = (IUC_EthernetHeader_T *)L_MM_Mref_GetPdu(mem_ref, &pdu_len);
    if (NULL == iuc_ether_hdr)
    {
        SYSFUN_Debug_Printf("%s:mem_ref == NULL\n", __FUNCTION__);
        return;
    }
    
    if(iuc_ether_hdr->type != IUC_ETHERNET_TYPE)
    {
        L_MM_Mref_Release(&mem_ref);
        return;
    }
        
    L_MM_Mref_MovePdu (mem_ref, IUC_ETHERNET_HEADER_LEN, &pdu_len);
    
    if(iuc_bd_capture_received == TRUE)
    {
        printf("\n");
        printf("========================IUC_PacketArrival==================\n");
        printf("*         Shown before passing to ISC_PacketIncoming      *\n");
        printf("===========================================================\n");
        IUC_BD_DisplayPacket(mem_ref);
    }  

    ISC_PacketIncoming (rx_port, mem_ref);
        
    return;
}
    

/* FUNCTION NAME: IUC_GetPacketType
 * PURPOSE: 
 *          determine the packet type of input packet
 * INPUT:   
 *          data_p    -- points to the data block that holds the packet content
 * OUTPUT:  
 *          None
 * RETURN:  
 *          One of LAN_TYPE_PacketType_E
 * NOTES:   
 *          1. It is register to DEV_NICDRV to callback
 */
LAN_TYPE_PacketType_T IUC_GetPacketType(UI8_T *data_p,UI32_T rx_port)
{
    IUC_EthernetHeader_T    *iuc_ether_hdr = (IUC_EthernetHeader_T*)data_p;
    UI16_T     tag_info;
    UI32_T     dev_id, uplink_stackingport, downlink_stackingport;
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
   UI32_T state,uplinkport,downlinkport;
#endif

    if (NULL == data_p)
    {
        iuc_bd_rx_numeber[0]++;
        return LAN_TYPE_INVALID_PACKET_TYPE;
    }

    if (STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_UP_LINK, &dev_id, &uplink_stackingport) == FALSE)
    {
        uplink_stackingport = IUC_INVALID_PORT_ID;
    }
    if (STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_DOWN_LINK, &dev_id, &downlink_stackingport) == FALSE)
    {
        downlink_stackingport = IUC_INVALID_PORT_ID;
    }

    tag_info = L_STDLIB_Ntoh16(iuc_ether_hdr->vtag) & 0xfff;

#ifdef SYS_ADPT_VLAN_INTERNAL_MANAGEMENT_VLAN
    /*PKT tag is 4094,but it received not from stk port ,so pkt is invalid*/
    if( tag_info == STK_VLAN_ID &&
       ((rx_port != uplink_stackingport) && (rx_port != downlink_stackingport)))
       {
        iuc_bd_rx_numeber[1]++;
        if(iuc_bd_show)
         printf("\r\n %s %d,rx_port %ld ,tag %d",__FUNCTION__,__LINE__,rx_port,tag_info);
       return LAN_TYPE_INVALID_PACKET_TYPE;
       }
#endif

/*if the pkt is not from stacking pkt, we think it is normal pkt*/
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    if((rx_port != uplink_stackingport) && (rx_port != downlink_stackingport))
    {
        iuc_bd_rx_numeber[2]++;
        if(iuc_bd_show)
            printf("\r\n %s %d,rx_port %ld ,tag %d",__FUNCTION__,__LINE__,rx_port,tag_info);
        return LAN_TYPE_LOCAL_NETWORK_PACKET;
    }
#else
    return LAN_TYPE_LOCAL_NETWORK_PACKET;
#endif

/*the pkt is for stacking port*/  
    if(iuc_ether_hdr->type == IUC_ETHERNET_TYPE)
    {
       
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
     /*if stacking is not enable,no stacking port,but pkt tag is 4094,drop it*/
      if(!STKTPLG_POM_GetStackingPortInfo(&state,&uplinkport,&downlinkport))
       {
        iuc_bd_rx_numeber[3]++;
       return LAN_TYPE_INVALID_PACKET_TYPE;
       }

      if(!state)/*stacking is not enable,it is a normal port*/
      {
#ifdef SYS_ADPT_VLAN_INTERNAL_MANAGEMENT_VLAN

        if(tag_info == STK_VLAN_ID)
         {
         iuc_bd_rx_numeber[4]++;
         return LAN_TYPE_INVALID_PACKET_TYPE;
         }
        else
#endif
        {
        iuc_bd_rx_numeber[5]++;
        return LAN_TYPE_LOCAL_NETWORK_PACKET;
        }
      }
      else
#endif 
      {/*it is a stacking port,but the pkt is normal pkt*/
        if(tag_info != STK_VLAN_ID)
          {
          iuc_bd_rx_numeber[6]++;
          return LAN_TYPE_INVALID_PACKET_TYPE;
          }
        else/*it is a stacking port,but the pkt is stacking pkt*/
         return ISC_GetPacketType(data_p+IUC_ETHERNET_HEADER_LEN);
      }
    }
    else
    {
        iuc_bd_rx_numeber[7]++;
        return LAN_TYPE_LOCAL_NETWORK_PACKET;
    }
}


/* FUNCTION NAME:   ICU_GetIUCEthHeaderLen
 * PURPOSE: 
 *          Get the IUC_ETHERNET_HEADER_LEN of IUC packet
 * INPUT:   
 *          None
 * OUTPUT:  
 *          length
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 */
BOOL_T ICU_GetIUCEthHeaderLen(UI16_T *length)
{
    if (NULL == length)
        return FALSE;

    *length = (UI16_T)IUC_ETHERNET_HEADER_LEN;
    return TRUE;
}


/* FUNCTION NAME: IUC_BD_InputInteger
 * PURPOSE: 
 *          get an integer from console
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          The integer key in by user
 * NOTES:   
 */
static UI32_T IUC_BD_InputInteger(void)
{
    #define ASCII_0  0x30
    #define ASCII_9  0x39
    
    UI32_T ret_int = 0;
    int ch;

    while (TRUE)
    {
        if ((ch = BACKDOOR_MGR_GetChar()) == EOF)
        {
            printf("%s: BACKDOOR_MGR_GetChar fail\n",__FUNCTION__);
            break;
        }
        if (ch == '\n')
            break;

        if ((ch < ASCII_0) || (ch > ASCII_9))
            continue;
        ch -= ASCII_0;
        BACKDOOR_MGR_Printf("%d", ch);
        
        ret_int = ret_int * 10 + ch;
    }
    BACKDOOR_MGR_Printf("\n");

    return ret_int;
}


/* FUNCTION NAME: IUC_BD_DisplayPacket
 * PURPOSE: 
 *          display packet data in L_MM_Mref_Handle_T structure
 * INPUT:   
 *          mem_ref    -- target L_MM_Mref_Handle_T
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:   
 */
static void IUC_BD_DisplayPacket(L_MM_Mref_Handle_T *mem_ref)
{
    UI8_T     *pkt;    
    UI32_T    pkt_len;
         
    pkt = L_MM_Mref_GetPdu(mem_ref, &pkt_len);

    BACKDOOR_MGR_Printf("payload: \n");
    IUC_BD_DisplayDataByHex(pkt,pkt_len);
    
    BACKDOOR_MGR_Printf("\n Report done! \n");
    return;
}
        

/* FUNCTION NAME: IUC_BD_DisplayDataByHex
 * PURPOSE: 
 *          Print out raw data in HEX format
 * INPUT:   
 *          data   -- pointer to the data
 *            length -- data length that want to print
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:   
 */
static void IUC_BD_DisplayDataByHex(UI8_T *data, UI32_T length)
{
    #define BYTES_IN_GROUP  4
    #define BYTES_IN_LINE   (3*BYTES_IN_GROUP)

    UI32_T i;

    for (i=0; i<length; i++)
    {
        if ((i%BYTES_IN_LINE) == 0)
        {
            BACKDOOR_MGR_Printf("%04lx:",i);
        }
        if (((i%BYTES_IN_GROUP) == 0) && ((i%BYTES_IN_LINE) != 0))
        {
            BACKDOOR_MGR_Printf("  ");
        }
        if ((i%BYTES_IN_GROUP) == 0)
        {
            BACKDOOR_MGR_Printf("0x");
        }
        BACKDOOR_MGR_Printf("%02x ", data[i]);
        if (((i+1)%BYTES_IN_LINE) == 0)
        {
            BACKDOOR_MGR_Printf("\r\n");
        }
    }
}


/* FUNCTION NAME: IUC_BD_BackdoorMenu
 * PURPOSE: 
 *          Main menu of backdoor
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:   
 */
static void IUC_BD_BackdoorMenu(void)
{
#define ASCII_0  0x30
#define ASCII_9  0x39
    
    int ch;
    BOOL_T eof = FALSE;

  
    while (!eof)
    {
        BACKDOOR_MGR_Printf("\n 0. Exit\n");
        BACKDOOR_MGR_Printf(" 1. IUC_BD_SendSamplePacket\n");
        BACKDOOR_MGR_Printf(" 2. Toggle Show/Hide received packet"); if(iuc_bd_capture_received) printf("(Show):\n"); else printf("(Hide):\n");  
        BACKDOOR_MGR_Printf(" 3. Toggle Show/Hide transmitted packet"); if(iuc_bd_capture_transmitted) printf("(Show):\n"); else printf("(Hide):\n");    
        BACKDOOR_MGR_Printf(" select = ");
        
        if ((ch = BACKDOOR_MGR_GetChar()) == EOF)
        {
            printf("%s: BACKDOOR_MGR_GetChar fail\n",__FUNCTION__);
            break;
        }
        
        if ((ch < ASCII_0) || (ch > ASCII_9))
            continue;
            
        ch -= ASCII_0;
        
        BACKDOOR_MGR_Printf("%d\n",ch);
        
        switch (ch)
        {
            case 0:
                eof=TRUE;
                break;
            case 1:
                IUC_BD_SendSamplePacket();
                break;
            case 2:
                iuc_bd_capture_received = !iuc_bd_capture_received;
                break;
            case 3:
                iuc_bd_capture_transmitted = !iuc_bd_capture_transmitted;
                break;
            default:
                break;               
        }
    }
    
    return;
}


/* FUNCTION NAME: IUC_BD_SendSamplePacket
 * PURPOSE: 
 *          Send a IUC packet to target unit
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:   
 */
void IUC_BD_ShowBDCounter()
{
  int i = 0;
  printf("==============================================");
  for(i=0;i<10;i++)
  {
    if(iuc_bd_rx_numeber[i]!=0)
    printf("\r\n %d counter %ld",i,iuc_bd_rx_numeber[i]);
  }
   printf("==============================================");
}

 void IUC_BD_ClearBDCounter()
{
  memset(iuc_bd_rx_numeber,0,sizeof(iuc_bd_rx_numeber));
}

static void IUC_BD_SendSamplePacket(void)
{
    UI8_T               unit, Uplink_Downlink;  
    UI8_T               *req_frame;    
    UI16_T              dst_unit;
    UI16_T              iuc_sdu_len;
    UI16_T              index;     
    UI32_T              frame_len; 
    L_MM_Mref_Handle_T  *mem_ref;        

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("================================================\r\n");  
    BACKDOOR_MGR_Printf("||                                            ||\r\n");      
    BACKDOOR_MGR_Printf("||          IUC Send Packet Test              ||\r\n");
    BACKDOOR_MGR_Printf("||                                            ||\r\n");         
    BACKDOOR_MGR_Printf("================================================\r\n");
    BACKDOOR_MGR_Printf("\nDestination driver unit number (0: next, %u: means all) :", SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK+1);
    
    unit = IUC_BD_InputInteger();
    
    switch (unit)
    {
        case 0:
            dst_unit = IUC_STACK_UNIT_NEXT;
            break;
            
        case SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK+1:
            dst_unit = IUC_STACK_UNIT_ALL;
            break;
            
        default:
            dst_unit = unit;
            break;
    }  
    
    BACKDOOR_MGR_Printf("\nDo you want send to 1.Uplink 2.Downlink 3.Uplink and Downlink ?");    
    
    Uplink_Downlink = IUC_BD_InputInteger();   
        
    if(Uplink_Downlink < 1 || Uplink_Downlink >3)
    {
        BACKDOOR_MGR_Printf("input is out of range! Do nothing!\n"); 
        return;
    }  
    
    BACKDOOR_MGR_Printf(" Packet content size=>");
    iuc_sdu_len = IUC_BD_InputInteger();
    
    if(iuc_sdu_len == 0)    
    {
        BACKDOOR_MGR_Printf("Lenght is 0, send nothing..\n");
        return;
    }        
    else if(iuc_sdu_len > (SYS_ADPT_MAX_FRAME_SIZE - IUC_ETHERNET_HEADER_LEN - 4 /* CRC */))
    {
        BACKDOOR_MGR_Printf("Packet size is too large\n");
        return;
    }
    
    if(NULL == (mem_ref = L_MM_AllocateTxBuffer(iuc_sdu_len, 
                                                L_MM_USER_ID(SYS_MODULE_IUC, 0, 0))))                                                    
    {
        BACKDOOR_MGR_Printf("Not enough memory to send this packet!\n");
        return;    
    } 
    
    req_frame = L_MM_Mref_MovePdu (mem_ref, (I32_T)(IUC_ETHERNET_HEADER_LEN - SYS_ADPT_ISC_LOWER_LAYER_HEADER_LEN), &frame_len);
    for(index = 0; index<iuc_sdu_len; index++)
        req_frame[index] = index;
        
    if (TRUE == IUC_SendPacket(mem_ref, dst_unit, Uplink_Downlink, SYS_DFLT_STK_TPLG_PACKET_TO_CPU_PRIORITY)) 
        BACKDOOR_MGR_Printf(" Send successfully");
    else
        BACKDOOR_MGR_Printf(" Send unsuccessfully");
        
    return;    
}/* IUC_BD_SendSamplePacket */

