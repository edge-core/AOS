#ifndef _DEV_SWDRV_PMGR_H_
#define _DEV_SWDRV_PMGR_H_
#include "sys_type.h"
#include "dev_swdrv.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_VXLAN == TRUE)
    #include "l_inet.h"
#endif

#if (SYS_CPNT_POE == TRUE)

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
#include <cpss/generic/dragonite/cpssGenDragonite.h>
#endif

#endif

enum
{
    DEV_SWDRV_PMGR_RESP_RESULT_FAIL,
};

#define DEV_SWDRV_PMGR_REQ_CMD_SIZE       sizeof(((DEV_SWDRV_PMGR_IPCMSG_T *)0)->type.cmd)
#define DEV_SWDRV_PMGR_RESP_RESULT_SIZE   sizeof(((DEV_SWDRV_PMGR_IPCMSG_T *)0)->type)

#define DEV_SWDRV_PMGR_MAX_NBR_OF_PHY_REG_OP 10

typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
    }type;

    union
    {
        union
        {
            struct
            {
                UI32_T module_slot_index;
                UI8_T module_id;
            }req;
            struct
            {
            }resp;
        }reconfig_module                             ;
        union
        {
            struct
            {
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }reconfig_module_byport                      ;
        union
        {
            struct
            {
                BOOL_T include_cross_bar;
            }req;
            struct
            {
            }resp;
        }resetondemand                               ;
#if 0   /* nobody use this function */
        union
        {
            struct
            {
                DEV_SWDRV_Device_Port_Mapping_T unit_port_to_device_port_mapping_table[][SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
            }req;
            struct
            {
            }resp;
        }setdeviceportmapping                        ;
#endif
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }getlocalunitchipsetnumber                   ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI32_T module_id;
                UI32_T device_id;
                UI32_T phy_port;
            }resp;
        }logical2phydeviceportid                     ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI32_T module_id;
                UI32_T device_id;
                UI32_T phy_id;
            }resp;
        }logical2phydeviceid                         ;
        union
        {
            struct
            {
                UI32_T module_id;
                UI32_T device_id;
                UI32_T phy_port;
            }req;
            struct
            {
                UI32_T unit;
                UI32_T port;
            }resp;
        }physical2logicalport                        ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T module_id;
            }req;
            struct
            {
                UI32_T device_id;
            }resp;
        }getlocaldeviceidfrommoduleid                ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T device_id;
            }req;
            struct
            {
                UI32_T module_id;
            }resp;
        }getmoduleidfromlocaldeviceid                ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableportadmin                             ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disableportadmin                            ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }linkscanregister                           ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }disableallportadmin                         ;
        union
        {
            struct
            {
            }req;
            struct
            {
            UI32_T unit ;
            }resp;
        }getunitid                                  ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T speed_duplex;
            }req;
            struct
            {
            }resp;
        }setportcfgspeedduplex                       ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableportautoneg                           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disableportautoneg                          ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableportcfgflowctrl                       ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disableportcfgflowctrl                      ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setportcfgflowctrl                          ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T capability;
            }req;
            struct
            {
            }resp;
        }setportautonegcapability                    ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                BOOL_T status;
            }req;
            struct
            {
            }resp;
        }setminigbicportledstatus;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI32_T port_type;
            }resp;
        }getporttype                                 ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }setportrestartautonego                      ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T remote_fault;
            }req;
            struct
            {
            }resp;
        }setportautonegoremotefaultadvertisement     ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T remote_fault;
            }resp;
        }getportautonegoremotefaultadvertisement     ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T state;
            }resp;
        }getportlinkpartnerautonegosignalingstate    ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T state;
            }resp;
        }getportautonegoprocessstate                 ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T capabilities;
            }resp;
        }getportlinkpartnerautonegocapa              ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T remote_fault;
            }resp;
        }getportlinkpartnerautonegoremotefault       ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T state;
            }resp;
        }getportjabberstate                          ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T counter;
            }resp;
        }getportfalsecarriersensecounter             ;
        union
        {
            struct
            {
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setstamode                                  ;
        union
        {
            struct
            {
                UI32_T mstid;
                UI32_T vlan_count;
                UI16_T vlan_list[SYS_ADPT_MAX_VLAN_ID];
                UI32_T unit_id;
                UI32_T port;
                UI32_T state;
            }req;
            struct
            {
            }resp;
        }setportstastate                             ;
        union
        {
            struct
            {
                UI32_T mstidx;
                UI32_T unit_id;
                UI32_T port;
                UI32_T state;
            }req;
            struct
            {
            }resp;
        }setportstastatewithmstidx                   ;
        union
        {
            struct
            {
                UI32_T vid;
                UI32_T mstidx;
            }req;
            struct
            {
            }resp;
        }addvlantostawithmstidx                      ;
        union
        {
            struct
            {
                UI32_T vid;
                UI32_T mstidx;
            }req;
            struct
            {
            }resp;
        }deletevlanfromstawithmstidx                 ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T pvid;
            }req;
            struct
            {
            }resp;
        }setportpvid                                 ;
        union
        {
            struct
            {
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }createvlan                                  ;
        union
        {
            struct
            {
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }destroyvlan                                 ;
        union
        {
            struct
            {
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }setglobaldefaultvlan                        ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }deleteportuntaggedvlanset                   ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }addportuntaggedvlanset                      ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }deleteportvlanset                           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }addportvlanset                              ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T vid;
                UI8_T port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            }req;
            struct
            {
            }resp;
        }setporttovlanmemberset                      ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T vid;
                UI8_T  port_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
            }req;
            struct
            {
            }resp;
        }setporttovlanuntaggedset                    ;
        union
        {
            struct
            {
                UI32_T status;
                UI32_T mtu;
            }req;
            struct
            {
            }resp;
        }setsystemmtu                       ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T mtu;
            }req;
            struct
            {
            }resp;
        }setportmtu                       ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T untagged_max_frame_sz;
                UI32_T tagged_max_frame_sz;
            }resp;
        }getportmaxframesize                       ;
        union
        {
            struct
            {
                UI32_T status;
            }req;
            struct
            {
            }resp;
        }setportdot1qtunnelmtu                       ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI8_T mode;
            }req;
            struct
            {
            }resp;
        }setportdot1qtunnelmode                      ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI8_T  mode;
            }resp;
        }getportdot1qtunnelmodeoflocalport           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T tpid;
            }req;
            struct
            {
            }resp;
        }setportdot1qtunneltpid                      ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI32_T tpid;
            }resp;
        }getportdot1qtunneltpidoflocalport           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableingressfilter                         ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disableingressfilter                        ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }admitonlyvlantaggedframes                   ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }admitonlyvlanuntaggedframes                   ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }admitallframes                              ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }enabletrapunspecifiedtagframe               ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }disabletrapunspecifiedtagframe              ;
        union
        {
            struct
            {
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }addcputovlan                                ;
        union
        {
            struct
            {
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }deletecpufromvlan                           ;
        union
        {
            struct
            {
                SYS_TYPE_Uport_T source_port;
                SYS_TYPE_Uport_T rx_to_sniffer_port;
                SYS_TYPE_Uport_T tx_to_sniffer_port;
            }req;
            struct
            {
            }resp;
        }setportmirroring                            ;
        union
        {
            struct
            {
                SYS_TYPE_Uport_T source_port;
                SYS_TYPE_Uport_T dest_port;
            }req;
            struct
            {
            }resp;
        }deleteportmirroring                         ;
        union
        {
            struct
            {
                SYS_TYPE_Uport_T source_port;
            }req;
            struct
            {
            }resp;
        }enableportmirroring                         ;
        union
        {
            struct
            {
                SYS_TYPE_Uport_T source_port;
            }req;
            struct
            {
            }resp;
        }disableportmirroring                        ;
        union
        {
            struct
            {
                UI32_T trunk_id;
            }req;
            struct
            {
            }resp;
        }createtrunk                                 ;
        union
        {
            struct
            {
                UI32_T trunk_id;
                UI8_T mode;
            }req;
            struct
            {
            }resp;
        }createtrunkwithmode                         ;
        union
        {
            struct
            {
                UI32_T load_balance_mode;
            }req;
            struct
            {
            }resp;
        }settrunkloadbalancemode                     ;
        union
        {
            struct
            {
                UI32_T trunk_id;
            }req;
            struct
            {
            }resp;
        }destroytrunk                                ;
        union
        {
            struct
            {
                UI32_T trunk_id;
                UI32_T port_member_count;
                SYS_TYPE_Uport_T port_member_list[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
            }req;
            struct
            {
            }resp;
        }settrunkportmembers                         ;
        union
        {
            struct
            {
                UI32_T trunk_id;
                UI8_T mode;
            }req;
            struct
            {
            }resp;
        }settrunkbalancemode                         ;
        union
        {
            struct
            {
                UI32_T trunk_id;
            }req;
            struct
            {
                UI8_T mode;
            }resp;
        }gettrunkbalancemodeoflocalunit              ;
        union
        {
            struct
            {
                UI8_T            mac[6];
                UI32_T           vid;
                UI32_T           trunk_id;
                SYS_TYPE_Uport_T trunk_member;
            }req;
            struct
            {
            }resp;
        }addmulticastaddrtotrunkmember               ;
        union
        {
            struct
            {
                UI8_T            mac[6];
                UI32_T           vid;
                UI32_T           trunk_id;
                SYS_TYPE_Uport_T trunk_member;
            }req;
            struct
            {
            }resp;
        }deletemulticastaddrtotrunkmember            ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }enableigmptrap                              ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }disableigmptrap                             ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T threshold;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setbroadcaststormcontrolthreshold           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T threshold;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setmulticaststormcontrolthreshold           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T threshold;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setunknownunicaststormcontrolthreshold      ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enablebroadcaststormcontrol                 ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disablebroadcaststormcontrol                ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enablemulticaststormcontrol                 ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disablemulticaststormcontrol                ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableunknownunicaststormcontrol            ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disableunknownunicaststormcontrol           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T mode;
            }req;
            struct
            {
                UI32_T granularity;
            }resp;
        }getportstormgranularity                    ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T priority;
            }req;
            struct
            {
            }resp;
        }setportuserdefaultpriority                  ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI8_T dot1p_to_cos_mapping[8];
            }req;
            struct
            {
            }resp;
        }setprioritymapping                          ;
        union
        {
            struct
            {
                UI8_T dot1p_to_cos_mapping[8];
            }req;
            struct
            {
            }resp;
        }setprioritymappingpersystem                 ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T number_of_queue;
            }req;
            struct
            {
            }resp;
        }setnumberoftrafficclassbyport               ;
        union
        {
            struct
            {
                UI32_T number_of_queue;
            }req;
            struct
            {
            }resp;
        }setnumberoftrafficclass;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI8_T mapping[8];
            }req;
            struct
            {
            }resp;
        }setstackingportprioritymapping              ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }enablejumboframe                            ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }disablejumboframe                           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI32_T speed_duplex;
            }resp;
        }getportoperspeedduplex                      ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                DEV_SWDRV_PortTableStats_T PortStats    ;
            }resp;
        }getallportstatus                           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI32_T link_status;
            }resp;
        }getportlinkstatus                           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                DEV_SWDRV_PortTableStats_T PortStats    ;
            }resp;
        }getallportlinkstatus                           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                DEV_SWDRV_PortTableStats_T PortStats    ;
            }resp;
        }getallportoperspeedduplex                           ;
        union
        {
            struct
            {
                UI32_T device_id;
                UI32_T phy_port;
            }req;
            struct
            {
                UI32_T link_status;
            }resp;
        }getportlinkstatusbydeviceid                 ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI32_T flow_ctrl;
            }resp;
        }getportflowctrl                             ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                DEV_SWDRV_PortTableStats_T PortStats    ;
            }resp;
        }getallportflowctrl                          ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }linkscan_update                             ;
        union
        {
            struct
            {
                I32_T unit_id;
            }req;
            struct
            {
            }resp;
        }enableipmc                                  ;
        union
        {
            struct
            {
                I32_T unit_id;
            }req;
            struct
            {
            }resp;
        }disableipmc                                 ;
        union
        {
            struct
            {
                I32_T unit_id;
            }req;
            struct
            {
            }resp;
        }ipmc_enablesourceportcheck                  ;
        union
        {
            struct
            {
                I32_T unit_id;
            }req;
            struct
            {
            }resp;
        }ipmc_disablesourceportcheck                 ;
        union
        {
            struct
            {
                I32_T unit_id;
            }req;
            struct
            {
            }resp;
        }ipmc_enablesourceipsearch                   ;
        union
        {
            struct
            {
                I32_T unit_id;
            }req;
            struct
            {
            }resp;
        }ipmc_disablesourceipsearch                  ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableportlearning                          ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disableportlearning                         ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                BOOL_T learning;
                BOOL_T to_cpu;
                BOOL_T drop;
            }req;
            struct
            {
            }resp;
        }setportlearningstatus;
        union
        {
            struct
            {
                UI32_T vid;
                BOOL_T learning;
            }req;
            struct
            {
            }resp;
        }setvlanlearningstatus;
        union
        {
            struct
            {
                UI8_T frame_type;
            }req;
            struct
            {
                UI32_T bcm_frame_type;
            }resp;
        }frametypeconvert                            ;
        union
        {
            struct
            {
                DEV_SWDRV_PRVLAN_PER_PORT_T prtable;
                UI8_T ftype;
                UI32_T etype;
            }req;
            struct
            {
                UI8_T index;
            }resp;
        }findprtableentry;
        union
        {
            struct
            {
                UI32_T  unit_id;
                UI32_T  port;
                UI32_T  group_index;
                UI32_T  vlan_id;
                UI32_T  nbr_of_type_protocol;
                UI8_T   frame_type[SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_GROUP_ENTRY];
                UI8_T   protocol_value[SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_GROUP_ENTRY][SYS_ADPT_1V_MAX_PROTOCOL_VALUE_BUFFER_LENGTH];
                #if (SYS_CPNT_PROTOCOL_VLAN_PORT_SUPPORT_PRIORITY == TRUE)
                UI8_T   priority;
                #endif
            }req;
            struct
            {
            }resp;
        }adddot1vprotocolportentry                   ;
        union
        {
            struct
            {
                UI32_T  unit_id;
                UI32_T  port;
                UI32_T  group_index;
                UI32_T  vlan_id;
                UI32_T  nbr_of_type_protocol;
                UI8_T   frame_type[SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_GROUP_ENTRY];
                UI8_T   protocol_value[SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_GROUP_ENTRY][SYS_ADPT_1V_MAX_PROTOCOL_VALUE_BUFFER_LENGTH];
            }req;
            struct
            {
            }resp;
        }deldot1vprotocolportentry                   ;
        union
        {
            struct
            {
                UI32_T  frame_type;
                UI8_T   protocol_value[SYS_ADPT_1V_MAX_PROTOCOL_VALUE_BUFFER_LENGTH];
                UI32_T  group_index;
            }req;
            struct
            {
            }resp;
        }setdot1vprotocolgroupentry                  ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T forcedmode;
                UI32_T mau_type;
                UI32_T speed;
            }req;
            struct
            {
            }resp;
        }setportcomboforcedmode                      ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }shutdownswitch                              ;
        union
        {
            struct
            {
                Stacking_Info_T stack_info;
            }req;
            struct
            {
            }resp;
        }updateherculesuctable                       ;
        union
        {
            struct
            {
                Stacking_Info_T stack_info;
            }req;
            struct
            {
            }resp;
        }configtopologyinfo                          ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T device_id;
                UI32_T phy_port;
            }req;
            struct
            {
                UI32_T unit;
                UI32_T port;
            }resp;
        }physical2logicalportbyunitid_deviceid       ;
        union
        {
            struct
            {
            }req;
            struct
            {
                UI32_T stackchipdeviceid;
            }resp;
        }getstackchipdeviceid                        ;
        union
        {
            struct
            {
                UI32_T src_unit;
                UI32_T dst_unit;
            }req;
            struct
            {
                UI32_T port;
                UI32_T dst_mod;
            }resp;
        }getdstunitinfo                              ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }higigremoteloopbackset                      ;
        union
        {
            struct
            {
                UI32_T module_id;
            }req;
            struct
            {
                UI32_T phy_port;
            }resp;
        }get5670linkport                             ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI32_T copper_energy_detect;
            }resp;
        }getcopperenergydetect                       ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }enableumcastiptrap                          ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }disableumcastiptrap                         ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }enableumcastmactrap                         ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }disableumcastmactrap                        ;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T auto_neg;
            }req;
            struct
            {
            }resp;
        }setfiberportautoneg                         ;
        union
        {
            struct
            {
                UI32_T port;
                UI32_T attribute;
            }req;
            struct
            {
            }resp;
        }setportforwardingattribute                  ;
        union
        {
            struct
            {
                UI32_T unitid;
            }req;
            struct
            {
                UI32_T  frabicStkport;
            }resp;
        }getfabricportbyunitposition                 ;
        union
        {
            struct
            {
                UI32_T unit;
            }req;
            struct
            {
                UI32_T frabicStkport;
            }resp;
        }getfabricporttodstunit                      ;
        union
        {
            struct
            {
            }req;
            struct
            {
                UI32_T  mastermoduleid;
            }resp;
        }getmastermoduleid                           ;
        union
        {
            struct
            {
                UI32_T unit_id;
                BOOL_T mainbrd_option;
            }req;
            struct
            {
                UI32_T moduleid;
            }resp;
        }getmoduleinfobyunit                         ;
        union
        {
            struct
            {
            }req;
            struct
            {
                UI32_T cross_bar_id;
            }resp;
        }getlocalcrossbardeviceid                    ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }enablestackingchipallport                   ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }vid_break                                   ;
        union
        {
            struct
            {
                UI32_T device_id;
            }req;
            struct
            {
                UI32_T device_id;
            }resp;
        }getnextswitchchipid                         ;
        union
        {
            struct
            {
                UI8_T mac_address[6];
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }addipv6multicastaddress                     ;
        union
        {
            struct
            {
                UI8_T mac_address[6];
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }delipv6multicastaddress                     ;
#if (SYS_CPNT_MLDSNP == TRUE)
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }enablemldtrap2cpu                           ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }disablemldtrap2cpu                          ;
#endif
        union
        {
            struct
            {
                BOOL_T is_flood;
            }req;
            struct
            {
            }resp;
        }enablendtrap2cpu                            ;
        union
        {
            struct
            {
                BOOL_T is_flood;
            }req;
            struct
            {
            }resp;
        }disablendtrap2cpu                           ;
        union
        {
            struct
            {
            }req;
            struct
            {
                UI32_T stacking_port_bmap;
            }resp;
        }getforwardingstackingportbitmap             ;
        union
        {
            struct
            {
                int flag;
            }req;
            struct
            {
            }resp;
        }receive_all                                 ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }setblockcpuforwardingattr                   ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI16_T mode;
            }req;
            struct
            {
            }resp;
        }set1000basetporttestmode                    ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI16_T mode;
            }resp;
        }get1000basetporttestmode                    ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI16_T transmitter_type;
            }req;
            struct
            {
            }resp;
        }set1000basettransmittertype                 ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
                UI16_T transmitter_type;
            }resp;
        }get1000basettransmittertype                 ;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setport1000basetforcemode;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T autoMDIXmode;
            }resp;
        }getautomdixmode                             ;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }init;
		union
        {
			struct
            {
            	UI32_T device_id;
				UI32_T reg_addr;
				UI32_T data;
            }req;
			struct
			{
			}resp;
        }socpciwrite;
#if (SYS_CPNT_POWER_SAVE == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                BOOL_T status;
                BOOL_T link_status;
            }req;
            struct
            {
            }resp;
        }setpowersave;
#endif
#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
        union
        {
            struct
            {
            	UI32_T unit;
                UI32_T port;
            }req;
			struct
			{
                            DEV_SWDRV_CableDiagResult_T diag_result;
			}resp;
        }getcablediag;

#if (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                DEV_SWDRV_CableDiagResultGeneral_T diag_result;
            }resp;
        }getcablediagresult;
#endif
#endif
        union
        {
            struct
            {
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableinternalloopback;
        union
        {
            struct
            {
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disableinternalloopback;
        union
        {
            struct
            {
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }isportloopbacking;
        union
        {
            struct
            {
                UI32_T device_id;
                UI32_T reg_addr;
                UI32_T reg_val;
                UI32_T reg_len;
            }req;
            struct
            {
            }resp;
        }writeregister;
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
        union
        {
            struct
            {
                BOOL_T is_stacking_port;
            }req;
            struct
            {
            }resp;
        }setstackingport;
#endif
        union
        {
            struct
            {
                BOOL_T is_init;
            }req;
            struct
            {
            }resp;
        }updateportmapping;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setportauthmode;
#if (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setportmdix;
#endif
#if (SYS_CPNT_MAC_VLAN == TRUE)
        struct
        {
            UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN];
            UI16_T vid;
            UI8_T priority;
        }setmacvlanentry;
        struct
        {
            UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN];
        }deletemacvlanentry;
#endif
        union
        {
            struct
            {
                BOOL_T state;
            }req;
            struct
            {
            }resp;
        }setEapolFramePassThrough;
#if (SYS_CPNT_RSPAN == TRUE)
        union
        {
            struct
            {
                SYS_TYPE_Uport_T target_port;
                UI16_T tpid;
                UI16_T vlan;
            }req;
            struct
            {
            }resp;
        }setrspanvlantag;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disablerspanportlearning;
        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enablerspanportlearning;
        union
        {
            struct
            {
                UI8_T port;
                BOOL_T is_increase;
            }req;
            struct
            {
            }resp;
        }modifymaxframesize;
#endif /* end of #if (SYS_CPNT_RSPAN == TRUE) */

        union
        {
            struct
            {
            	UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T port_media;
            }resp;
        }getportmediaactive;

        union
        {
            struct
            {
                BOOL_T to_cpu;
                BOOL_T flood;
            }req;
            struct
            {
            }resp;
        } trap_packet;

        union
        {
            struct
            {
                BOOL_T to_cpu;
                BOOL_T drop;
            }req;

            struct
            {
            }resp;
        }trap_igmp_packet;

        union
        {
            struct
            {
                BOOL_T to_cpu;
                BOOL_T drop;
            }req;

            struct
            {
            }resp;
        }trap_mld_packet;

        union
        {
            struct
            {
                BOOL_T to_cpu;
                BOOL_T flood;
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        } trap_packet_byvlan;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI8_T reg;
                UI16_T data;
            }req;
            struct
            {
            }resp;
        }PhyRegisterWrite;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI8_T reg;
            }req;
            struct
            {
                UI16_T data;
            }resp;
        }PhyRegisterRead;

        union
        {
            struct
            {
                UI32_T dev_slv_id;
                UI32_T reg_addr;
                UI32_T value;
            }req;
            struct
            {
            }resp;
        }TwsiRegisterWrite;

        union
        {
            struct
            {
                UI32_T dev_slv_id;
                UI32_T reg_addr;
            }req;
            struct
            {
                UI32_T value;
            }resp;
        }TwsiRegisterRead;

        union
        {
            struct
            {
                UI8_T dev_slv_id;
                UI8_T type;
                UI8_T validOffset;
                UI32_T offset;
                UI8_T moreThen256;
                UI8_T data[0xFF];
                UI8_T data_len;
            }req;
            struct
            {
            }resp;
        }TwsiDataWrite;

        union
        {
            struct
            {
                UI32_T dev_slv_id;
                UI8_T type;
                UI8_T validOffset;
                UI32_T offset;
                UI8_T moreThen256;
                UI8_T data_len;
            }req;
            struct
            {
                UI8_T data[0xFF];
            }resp;
        }TwsiDataRead;

#if (SYS_CPNT_POE == TRUE)

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
        union
        {
            struct
            {
                UI32_T event;
                UI32_T unit;
                UI32_T port;
                UI32_T data;
            }req;
            struct
            {
                CPSS_GEN_DRAGONITE_DATA_STC dragonite;
            }resp;
        }ProcessDragoniteData;
#endif

        union
        {
            struct
            {
                UI8_T I2C_Address;
                UI16_T Flags;
                UI8_T Txdata[0xFF];
                UI16_T num_write_length;
                void* UserParam;
            }req;
            struct
            {
            }resp;
        }TwsiWrite;

        union
        {
            struct
            {
                UI8_T I2C_Address;
                UI16_T Flags;
                UI16_T number_of_bytes_to_read;
                void* UserParam;
            }req;
            struct
            {
                UI8_T Rxdata[0xFF];
            }resp;
        }TwsiRead;

        union
        {
            struct
            {
                UI8_T pTxdata[0xFF];
                UI16_T num_write_length;
            }req;
            struct
            {
                I32_T pDevice_error;
            }resp;
        }MSCCPOEWrite;

        union
        {
            struct
            {
                UI16_T num_read_length;
            }req;
            struct
            {
                UI8_T pRxdata[0xFF];
                I32_T pDevice_error;
            }resp;
        }MSCCPOERead;
#endif

#if (SYS_CPNT_VLAN_MIRROR == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        } vlan_mirror;
#endif
#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                BOOL_T mode;
            }req;
            struct
            {
            }resp;
        } mac_mirror;
#endif

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                BOOL_T enable;
            }req;
            struct
            {
            }resp;
        } enable_port;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T old_vid;
                UI32_T new_vid;
                UI32_T priority;
                BOOL_T add_tag;
                BOOL_T enable;
            }req;
            struct
            {
            }resp;
        } vlan_translation;

        union
        {
            struct
            {
                UI32_T device_id;
                UI32_T phy_addr;
                UI32_T dev_addr;
                UI32_T phy_reg_addr;
            }req;
            struct
            {
                UI16_T reg_value;
            }resp;
        } getxmiiphyreg;

        union
        {
            struct
            {
                UI32_T device_id;
                UI32_T phy_addr;
                UI32_T dev_addr;
                UI32_T phy_reg_addr;
                UI16_T reg_value;
            }req;
            struct
            {
            }resp;
        } setxmiiphyreg;

        union
        {
            struct
            {
                UI32_T device_id;
                UI32_T phy_addr;
                UI32_T phy_port;
                UI32_T phy_reg;
                UI32_T page_reg;
                UI32_T page_val;
                UI32_T page_mask;
            }req;
            struct
            {
                UI16_T data;
            }resp;
        } getmiiphyregbypage;

        union
        {
            struct
            {
                BOOL_T  is_enabled;
            }req;
            struct
            {
            }resp;
        } setraandrrtrap;

        union
        {
            struct
            {
                UI32_T unit_id;
                UI32_T port;
                DEV_SWDRV_PHY_Reg_Operation_T phy_reg_ops[DEV_SWDRV_PMGR_MAX_NBR_OF_PHY_REG_OP];
                UI32_T num_of_ops;
            }req;
            struct
            {
            }resp;
        } dophyregtransactions;

        union
        {
            struct
            {
                 BOOL_T to_cpu;
                 BOOL_T drop;
            }req;
            struct
            {
            }resp;
       }set_packet_trap;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN];
            }req;
            struct
            {
            }resp;
        } port_mac_addr;

#if (SYS_CPNT_PFC == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                BOOL_T rx_en;
                BOOL_T tx_en;
                UI16_T pri_en_vec;
            }req;
            struct
            {
            }resp;
        } port_pfc_status;
#endif

#if (SYS_CPNT_MAC_IN_MAC == TRUE)
        union
        {
            struct
            {
                DEV_SWDRV_MimServiceInfo_T mim;
                BOOL_T is_valid;
            }req;
            struct
            {
                UI32_T hw_idx;
            }resp;
        } mim_service;

        union
        {
            struct
            {
                DEV_SWDRV_MimPortInfo_T mim_port;
                BOOL_T is_valid;
            }req;
            struct
            {
                UI32_T hw_idx;
            }resp;
        } mim_service_port;
#endif

        union
        {
            struct
            {
                UI32_T pkt_type;
                UI32_T rate;
            }req;
            struct
            {
            }resp;
        } cpu_rate_limit;

        union
        {
            struct
            {
             UI32_T unit;
             UI32_T port;
            }req;
            struct
            {
                DEV_SWDRV_PortAbility_T ability;
            }resp;
        } port_ability;

        union
        {
            struct
            {
            }req;
            struct
            {
                UI32_T temperature;
            }resp;
        } getporttemperature;

        union
        {
            struct
            {
                UI32_T packet_per_second;
            }req;

            struct
            {
            }resp;
        }setcpuportrate;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI8_T egr_blk_uport_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
            }req;

            struct
            {
            }resp;
        }setportegressblock;
        union
        {
            struct
            {
                UI32_T vid;
                DEV_SWDRV_VlanFloodingType_T type;
                BOOL_T flood;
            }req;
            struct
            {
            }resp;
        }setvlanflooding;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableportopenflowmode;

        union
        {
            struct
            {
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }deleteallportfromvlan;

        union
        {
            struct
            {
                UI32_T is_enable;
            }req;
            struct
            {
            }resp;
        }staticmacpkt2cpu;
#if (SYS_CPNT_VXLAN == TRUE)
        union
        {
            struct
            {
                UI32_T vfi_id;
                UI32_T l3_inf_id;
                UI32_T unit;
                UI32_T port;
                UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
                UI32_T match_type;
            }req;
            struct
            {
                UI32_T vxlan_port;
            }resp;
        }createvxlanaccessport;
        union
        {
            struct
            {
                UI32_T          vfi_id;
                UI32_T          udp_port;
                BOOL_T          is_mc;
                L_INET_AddrIp_T l_vtep;
                L_INET_AddrIp_T r_vtep;
                BOOL_T          is_ecmp;
                void            *nh_hw_info;
            }req;
            struct
            {
                UI32_T vxlan_port;
            }resp;
        }createvxlannetworkport;
        union
        {
            struct
            {
                UI32_T vfi_id;
                UI32_T vxlan_port_id;
                BOOL_T is_ecmp;
            }req;
            struct
            {

            }resp;
        }destroyvxlanport;
        union
        {
            struct
            {
                UI32_T l3_inf_id;
                UI32_T unit;
                UI32_T port;
                UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
                BOOL_T is_mc;
            }req;
            struct
            {
                void *nh_hw_info;
            }resp;
        }createvxlannexthop;
        union
        {
            struct
            {
                void *nh_hw_info;
            }req;
            struct
            {

            }resp;
        }destroyvxlannexthop;
        union
        {
            struct
            {
                UI32_T vfi_id;
                UI32_T vxlan_port_id;
                void   *nh_hw_info;
            }req;
            struct
            {

            }resp;
        }addvxlanecmpnexthop;
        union
        {
            struct
            {
                UI32_T vfi_id;
                UI32_T vxlan_port_id;
                void   *nh_hw_info;
            }req;
            struct
            {

            }resp;
        }removevxlanecmpnexthop;
        union
        {
            struct
            {
                UI32_T vni;
            }req;
            struct
            {
                UI32_T vfi;
                UI32_T bcast_group;
            }resp;
        }createvni;
        union
        {
            struct
            {
               UI32_T bcast_group;
               UI32_T vxlan_port;
               BOOL_T is_add;
            }req;
            struct
            {

            }resp;
        }addvteptobcast;
        union
        {
            struct
            {
                UI32_T udp_port;
            }req;
            struct
            {

            }resp;
        }setvxlanudpport;
        union
        {
            struct
            {
                BOOL_T is_enable;
                BOOL_T is_random_src_port;
            }req;
            struct
            {

            }resp;
        }setvxlanstatus;
        union
        {
            struct
            {
                UI32_T  unit;
                UI32_T  port;
                BOOL_T  is_acc_port;
                BOOL_T  is_enable;
            }req;
            struct
            {

            }resp;
        }setvxlanstatusport;
        union
        {
            struct
            {
                DEV_SWDRV_VxlanVpnInfo_T    vpn_info;
                BOOL_T                      is_add;
            }req;
            struct
            {
                DEV_SWDRV_VxlanVpnInfo_T    vpn_info;
            }resp;
        }setvxlanvpn;

        union
        {
            struct
            {
                UI32_T vxlan_port_id;
                BOOL_T is_learning;
            }req;
            struct
            {

            }resp;
        }setvxlanportlearning;
#endif /*#if (SYS_CPNT_VXLAN == TRUE)*/

#if (SYS_CPNT_HASH_SELECTION == TRUE)
        union
        {
            struct
            {
                DEV_SWDRV_HashSelBlockInfo_T block_info;
            }req;
            struct
            {

            }resp;
        } sethashselection;

        union
        {
            struct
            {
                UI8_T hw_block_index;
            }req;
            struct
            {

            }resp;
        } setecmphash;
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                BOOL_T status;
            }req;
            struct
            {
            }resp;
        }setportstatusforlicense;

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
               UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setswtichingmode;
#endif /*#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)*/

#if (SYS_CPNT_SWCTRL_FEC == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T fec_mode;
            }req;
            struct
            {
            }resp;
        }setportfec;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
                UI32_T fec_mode;
            }resp;
        }getportfecstatus;
#endif

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }updateportsfptxenable;

#if (SYS_CPNT_WRED == TRUE)
       union
       {
         struct
         {
           UI32_T unit_id;
           UI32_T port;     
           DEV_SWDRV_RandomDetect_T v;
         }req;
         struct
         {         
         }resp;
       }random_detect;
#endif
    }data;

}DEV_SWDRV_PMGR_IPCMSG_T;

enum
{
    DEV_SWDRV_IPCCMD_RECONFIG_MODULE                                    ,
    DEV_SWDRV_IPCCMD_RECONFIG_MODULE_BYPORT                             ,
    DEV_SWDRV_IPCCMD_RESETONDEMAND                                      ,
    DEV_SWDRV_IPCCMD_SETDEVICEPORTMAPPING                               ,
    DEV_SWDRV_IPCCMD_GETLOCALUNITCHIPSETNUMBER                          ,
    DEV_SWDRV_IPCCMD_LOGICAL2PHYDEVICEPORTID                            ,
    DEV_SWDRV_IPCCMD_LOGICAL2PHYDEVICEID                                ,
    DEV_SWDRV_IPCCMD_PHYSICAL2LOGICALPORT                               ,
    DEV_SWDRV_IPCCMD_GETLOCALDEVICEIDFROMMODULEID                       ,
    DEV_SWDRV_IPCCMD_GETMODULEIDFROMLOCALDEVICEID                       ,
    DEV_SWDRV_IPCCMD_ENABLEHGPORTADMIN                                  ,
    DEV_SWDRV_IPCCMD_LINKSCAN_REGISTER                                  ,
    DEV_SWDRV_IPCCMD_UPDATEPORTSFPTXENABLE                              ,
    DEV_SWDRV_IPCCMD_ENABLEPORTADMIN                                    ,
    DEV_SWDRV_IPCCMD_DISABLEPORTADMIN                                   ,
    DEV_SWDRV_IPCCMD_DISABLEALLPORTADMIN                                ,
    DEV_SWDRV_IPCCMD_SETPORTCFGSPEEDDUPLEX                              ,
    DEV_SWDRV_IPCCMD_ENABLEPORTAUTONEG                                  ,
    DEV_SWDRV_IPCCMD_DISABLEPORTAUTONEG                                 ,
    DEV_SWDRV_IPCCMD_ENABLEPORTCFGFLOWCTRL                              ,
    DEV_SWDRV_IPCCMD_DISABLEPORTCFGFLOWCTRL                             ,
    DEV_SWDRV_IPCCMD_SETPORTCFGFLOWCTRL                                 ,
    DEV_SWDRV_IPCCMD_SETPORTAUTONEGCAPABILITY                           ,
    DEV_SWDRV_IPCCMD_SETMINIGBICPORTLEDSTATUS                           ,
    DEV_SWDRV_IPCCMD_GETPORTTYPE                                        ,
    DEV_SWDRV_IPCCMD_SETPORTRESTARTAUTONEGO                             ,
    DEV_SWDRV_IPCCMD_SETPORTAUTONEGOREMOTEFAULTADVERTISEMENT            ,
    DEV_SWDRV_IPCCMD_GETPORTAUTONEGOREMOTEFAULTADVERTISEMENT            ,
    DEV_SWDRV_IPCCMD_GETPORTLINKPARTNERAUTONEGOSIGNALINGSTATE           ,
    DEV_SWDRV_IPCCMD_GETPORTAUTONEGOPROCESSSTATE                        ,
    DEV_SWDRV_IPCCMD_GETPORTLINKPARTNERAUTONEGOCAPA                     ,
    DEV_SWDRV_IPCCMD_GETPORTLINKPARTNERAUTONEGOREMOTEFAULT              ,
    DEV_SWDRV_IPCCMD_GETPORTJABBERSTATE                                 ,
    DEV_SWDRV_IPCCMD_GETPORTFALSECARRIERSENSECOUNTER                    ,
    DEV_SWDRV_IPCCMD_SETSTAMODE                                         ,
    DEV_SWDRV_IPCCMD_SETPORTSTASTATE                                    ,
    DEV_SWDRV_IPCCMD_SETPORTSTASTATEWITHMSTIDX                          ,
    DEV_SWDRV_IPCCMD_ADDVLANTOSTAWITHMSTIDX                             ,
    DEV_SWDRV_IPCCMD_DELETEVLANFROMSTAWITHMSTIDX                        ,
    DEV_SWDRV_IPCCMD_SETPORTPVID                                        ,
    DEV_SWDRV_IPCCMD_CREATEVLAN                                         ,
    DEV_SWDRV_IPCCMD_DESTROYVLAN                                        ,
    DEV_SWDRV_IPCCMD_SETGLOBALDEFAULTVLAN                               ,
    DEV_SWDRV_IPCCMD_DELETEPORTUNTAGGEDVLANSET                          ,
    DEV_SWDRV_IPCCMD_ADDPORTUNTAGGEDVLANSET                             ,
    DEV_SWDRV_IPCCMD_DELETEPORTVLANSET                                  ,
    DEV_SWDRV_IPCCMD_ADDPORTVLANSET                                     ,
    DEV_SWDRV_IPCCMD_SETPORTTOVLANMEMBERSET                             ,
    DEV_SWDRV_IPCCMD_SETPORTTOVLANUNTAGGEDSET                           ,
    DEV_SWDRV_IPCCMD_SETSYSTEMMTU                                       ,
    DEV_SWDRV_IPCCMD_SETPORTMTU                                         ,
    DEV_SWDRV_IPCCMD_GETPORTMAXFRAMESIZE                                ,
    DEV_SWDRV_IPCCMD_SETPORTDOT1QTUNNELMTU                              ,
    DEV_SWDRV_IPCCMD_SETPORTDOT1QTUNNELMODE                             ,
    DEV_SWDRV_IPCCMD_GETPORTDOT1QTUNNELMODEOFLOCALPORT                  ,
    DEV_SWDRV_IPCCMD_SETPORTDOT1QTUNNELTPID                             ,
    DEV_SWDRV_IPCCMD_GETPORTDOT1QTUNNELTPIDOFLOCALPORT                  ,
    DEV_SWDRV_IPCCMD_ENABLEINGRESSFILTER                                ,
    DEV_SWDRV_IPCCMD_DISABLEINGRESSFILTER                               ,
    DEV_SWDRV_IPCCMD_ADMITONLYVLANTAGGEDFRAMES                          ,
    DEV_SWDRV_IPCCMD_ADMITONLYVLANUNTAGGEDFRAMES                        ,
    DEV_SWDRV_IPCCMD_ADMITALLFRAMES                                     ,
    DEV_SWDRV_IPCCMD_ENABLETRAPUNSPECIFIEDTAGFRAME                      ,
    DEV_SWDRV_IPCCMD_DISABLETRAPUNSPECIFIEDTAGFRAME                     ,
    DEV_SWDRV_IPCCMD_ADDCPUTOVLAN                                       ,
    DEV_SWDRV_IPCCMD_DELETECPUFROMVLAN                                  ,
    DEV_SWDRV_IPCCMD_SETPORTMIRRORING                                   ,
    DEV_SWDRV_IPCCMD_DELETEPORTMIRRORING                                ,
    DEV_SWDRV_IPCCMD_ENABLEPORTMIRRORING                                ,
    DEV_SWDRV_IPCCMD_DISABLEPORTMIRRORING                               ,
#if (SYS_CPNT_VLAN_MIRROR == TRUE)
    DEV_SWDRV_IPCCMD_ADDVLANMIRROR                                      ,
    DEV_SWDRV_IPCCMD_DELETEVLANMIRROR                                   ,
#endif
#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
    DEV_SWDRV_IPCCMD_SETDESTPORTFORMACMIRROR                            ,
#endif
    DEV_SWDRV_IPCCMD_SETDESTPORTFORACLMIRROR                            , /* SYS_CPNT_ACL_MIRROR */
    DEV_SWDRV_IPCCMD_CREATETRUNK                                        ,
    DEV_SWDRV_IPCCMD_CREATETRUNKWITHMODE                                ,
    DEV_SWDRV_IPCCMD_SETTRUNKLOADBALANCEMODE                            ,
    DEV_SWDRV_IPCCMD_DESTROYTRUNK                                       ,
    DEV_SWDRV_IPCCMD_SETTRUNKPORTMEMBERS                                ,
    DEV_SWDRV_IPCCMD_SETTRUNKBALANCEMODE                                ,
    DEV_SWDRV_IPCCMD_GETTRUNKBALANCEMODEOFLOCALUNIT                     ,
    DEV_SWDRV_IPCCMD_ADDMULTICASTADDRTOTRUNKMEMBER                      ,
    DEV_SWDRV_IPCCMD_DELETEMULTICASTADDRTOTRUNKMEMBER                   ,
    DEV_SWDRV_IPCCMD_ENABLEIGMPTRAP                                     ,
    DEV_SWDRV_IPCCMD_DISABLEIGMPTRAP                                    ,
    DEV_SWDRV_IPCCMD_TRAPIGMPTOCPU                                      ,
    DEV_SWDRV_IPCCMD_SETBROADCASTSTORMCONTROLTHRESHOLD                  ,
    DEV_SWDRV_IPCCMD_SETMULTICASTSTORMCONTROLTHRESHOLD                  ,
    DEV_SWDRV_IPCCMD_SETUNKNOWNUNICASTSTORMCONTROLTHRESHOLD             ,
    DEV_SWDRV_IPCCMD_ENABLEBROADCASTSTORMCONTROL                        ,
    DEV_SWDRV_IPCCMD_DISABLEBROADCASTSTORMCONTROL                       ,
    DEV_SWDRV_IPCCMD_ENABLEMULTICASTSTORMCONTROL                        ,
    DEV_SWDRV_IPCCMD_DISABLEMULTICASTSTORMCONTROL                       ,
    DEV_SWDRV_IPCCMD_ENABLEUNKNOWNUNICASTSTORMCONTROL                   ,
    DEV_SWDRV_IPCCMD_DISABLEUNKNOWNUNICASTSTORMCONTROL                  ,
    DEV_SWDRV_IPCCMD_GETPORTSTORMGRANULARITY                            ,
    DEV_SWDRV_IPCCMD_SETPORTUSERDEFAULTPRIORITY                         ,
    DEV_SWDRV_IPCCMD_SETPRIORITYMAPPING                                 ,
    DEV_SWDRV_IPCCMD_SETPRIORITYMAPPINGPERSYSTEM                        ,
    DEV_SWDRV_IPCCMD_SETNUMBEROFTRAFFICCLASSBYPORT                      ,
    DEV_SWDRV_IPCCMD_SETNUMBEROFTRAFFICCLASS                            ,
    DEV_SWDRV_IPCCMD_SETSTACKINGPORTPRIORITYMAPPING                     ,
    DEV_SWDRV_IPCCMD_ENABLEJUMBOFRAME                                   ,
    DEV_SWDRV_IPCCMD_DISABLEJUMBOFRAME                                  ,
    DEV_SWDRV_IPCCMD_GETPORTOPERSPEEDDUPLEX                             ,
    DEV_SWDRV_IPCCMD_GETALLPORTOPERSPEEDDUPLEX                          ,
    DEV_SWDRV_IPCCMD_GETALLPORTLINKSTATUS                               ,
    DEV_SWDRV_IPCCMD_GETPORTLINKSTATUS                                  ,
    DEV_SWDRV_IPCCMD_GETALLPORTSTATUS                                   ,
    DEV_SWDRV_IPCCMD_GETPORTLINKSTATUSBYDEVICEID                        ,
    DEV_SWDRV_IPCCMD_GETPORTFLOWCTRL                                    ,
    DEV_SWDRV_IPCCMD_GETALLPORTFLOWCTRL                                 ,
    DEV_SWDRV_IPCCMD_LINKSCAN_UPDATE                                    ,
    DEV_SWDRV_IPCCMD_ENABLEIPMC                                         ,
    DEV_SWDRV_IPCCMD_DISABLEIPMC                                        ,
    DEV_SWDRV_IPCCMD_IPMC_ENABLESOURCEPORTCHECK                         ,
    DEV_SWDRV_IPCCMD_IPMC_DISABLESOURCEPORTCHECK                        ,
    DEV_SWDRV_IPCCMD_IPMC_ENABLESOURCEIPSEARCH                          ,
    DEV_SWDRV_IPCCMD_IPMC_DISABLESOURCEIPSEARCH                         ,
    DEV_SWDRV_IPCCMD_ENABLEPORTLEARNING                                 ,
    DEV_SWDRV_IPCCMD_DISABLEPORTLEARNING                                ,
    DEV_SWDRV_IPCCMD_SETPORTLEARNINGSTATUS                              ,
    DEV_SWDRV_IPCCMD_FRAMETYPECONVERT                                   ,
    DEV_SWDRV_IPCCMD_FINDPRTABLEENTRY                                   ,
    DEV_SWDRV_IPCCMD_ADDDOT1VPROTOCOLPORTENTRY                          ,
    DEV_SWDRV_IPCCMD_DELDOT1VPROTOCOLPORTENTRY                          ,
    DEV_SWDRV_IPCCMD_SETDOT1VPROTOCOLGROUPENTRY                         ,
    DEV_SWDRV_IPCCMD_SETPORTCOMBOFORCEDMODE                             ,
    DEV_SWDRV_IPCCMD_SHUTDOWNSWITCH                                     ,
    DEV_SWDRV_IPCCMD_UPDATEHERCULESUCTABLE                              ,
    DEV_SWDRV_IPCCMD_CONFIGTOPOLOGYINFO                                 ,
    DEV_SWDRV_IPCCMD_PHYSICAL2LOGICALPORTBYUNITID_DEVICEID              ,
    DEV_SWDRV_IPCCMD_GETUNITID                                          ,
    DEV_SWDRV_IPCCMD_GETSTACKCHIPDEVICEID                               ,
    DEV_SWDRV_IPCCMD_GETDSTUNITINFO                                     ,
    DEV_SWDRV_IPCCMD_HIGIGREMOTELOOPBACKSET                             ,
    DEV_SWDRV_IPCCMD_GET5670LINKPORT                                    ,
    DEV_SWDRV_IPCCMD_GETCOPPERENERGYDETECT                              ,
    DEV_SWDRV_IPCCMD_ENABLEUMCASTIPTRAP                                 ,
    DEV_SWDRV_IPCCMD_DISABLEUMCASTIPTRAP                                ,
    DEV_SWDRV_IPCCMD_ENABLEUMCASTMACTRAP                                ,
    DEV_SWDRV_IPCCMD_DISABLEUMCASTMACTRAP                               ,
    DEV_SWDRV_IPCCMD_SETFIBERPORTAUTONEG                                ,
    DEV_SWDRV_IPCCMD_SETPORTFORWARDINGATTRIBUTE                         ,
    DEV_SWDRV_IPCCMD_GETFABRICPORTBYUNITPOSITION                        ,
    DEV_SWDRV_IPCCMD_GETFABRICPORTTODSTUNIT                             ,
    DEV_SWDRV_IPCCMD_GETMASTERMODULEID                                  ,
    DEV_SWDRV_IPCCMD_GETMODULEINFOBYUNIT                                ,
    DEV_SWDRV_IPCCMD_GETLOCALCROSSBARDEVICEID                           ,
    DEV_SWDRV_IPCCMD_ENABLESTACKINGCHIPALLPORT                          ,
    DEV_SWDRV_IPCCMD_VID_BREAK                                          ,
    DEV_SWDRV_IPCCMD_GETNEXTSWITCHCHIPID                                ,
    DEV_SWDRV_IPCCMD_ADDIPV6MULTICASTADDRESS                            ,
    DEV_SWDRV_IPCCMD_DELIPV6MULTICASTADDRESS                            ,
    DEV_SWDRV_IPCCMD_ENABLEMLDTRAP2CPU                                  ,
    DEV_SWDRV_IPCCMD_DISABLEMLDTRAP2CPU                                 ,
    DEV_SWDRV_IPCCMD_TRAPMLDTOCPU                                       ,
    DEV_SWDRV_IPCCMD_TRAPND2CPU                                         ,
    DEV_SWDRV_IPCCMD_ENABLENDTRAP2CPU                                   ,
    DEV_SWDRV_IPCCMD_DISABLENDTRAP2CPU                                  ,
    DEV_SWDRV_IPCCMD_GETFORWARDINGSTACKINGPORTBITMAP                    ,
    DEV_SWDRV_IPCCMD_RECEIVE_ALL                                        ,
    DEV_SWDRV_IPCCMD_SETBLOCKCPUFORWARDINGATTR                          ,
    DEV_SWDRV_IPCCMD_SET1000BASETPORTTESTMODE                           ,
    DEV_SWDRV_IPCCMD_GET1000BASETPORTTESTMODE                           ,
    DEV_SWDRV_IPCCMD_SET1000BASETTRANSMITTERTYPE                        ,
    DEV_SWDRV_IPCCMD_GET1000BASETTRANSMITTERTYPE                        ,
    DEV_SWDRV_IPCCMD_SETPORT1000BASETFORCEMODE                          ,
    DEV_SWDRV_IPCCMD_GETAUTOMDIXMODE                                    ,
    DEV_SWDRV_IPCCMD_INIT                                               ,
    DEV_SWDRV_IPCCMD_CREATE_INTERCSC_RELATION                           ,
    DEV_SWDRV_IPCCMD_SOCPCIWRITE                                        ,
    DEV_SWDRV_IPCCMD_SETPOWERSAVE                                       ,
    DEV_SWDRV_IPCCMD_GETCABLEDIAG                                       ,
    DEV_SWDRV_IPCCMD_GETCABLEDIAGRESULT                                 ,
    DEV_SWDRV_IPCCMD_ENABLEINTERNALLOOPBACK                             ,
    DEV_SWDRV_IPCCMD_DISABLEINTERNALLOOPBACK                            ,
    DEV_SWDRV_IPCCMD_ISPORTLOOPBACKING                                  ,
    DEV_SWDRV_IPCCMD_WRITEREGISTER                                      ,
    DEV_SWDRV_IPCCMD_SETSTACKINGPORT                                    ,
    DEV_SWDRV_IPCCMD_SETPORTAUTHMODE                                    ,
    DEV_SWDRV_IPCCMD_UPDATEPORTMAPPING                                  ,
#if (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
    DEV_SWDRV_IPCCMD_SETPORTMDIX                                  		,
#endif
    DEV_SWDRV_IPCCMD_SETMACVLANENTRY                                    ,
    DEV_SWDRV_IPCCMD_DELETEMACVLANENTRY                                 ,
    DEV_SWDRV_IPCCMD_SETIPSUBNETVLANENTRY                               ,
    DEV_SWDRV_IPCCMD_DELETEIPSUBNETVLANENTRY                            ,
    DEV_SWDRV_IPCCMD_SETEAPOLFRAMEPASSTHROUGH                           ,
#if (SYS_CPNT_RSPAN == TRUE)
    DEV_SWDRV_IPCCMD_SETRSPANVLANTAG                                    ,
    DEV_SWDRV_IPCCMD_DISABLERSPANPORTLEARNING                           ,
    DEV_SWDRV_IPCCMD_ENABLERSPANPORTLEARNING                            ,
    DEV_SWDRV_IPCCMD_MODIFYMAXFRAMESIZE                                 ,
#endif /* end of #if (SYS_CPNT_RSPAN == TRUE) */
    DEV_SWDRV_IPCCMD_GETPORTMEDIAACTIVE                                 ,
    DEV_SWDRV_IPCCMD_TRAPUNKNOWNIPMCASTTOCPU                            ,
    DEV_SWDRV_IPCCMD_TRAPUNKNOWNIPV6MCASTTOCPU                          ,
#if 0 /* not used */
    DEV_SWDRV_IPCCMD_PHYREGISTERWRITE                                   ,
    DEV_SWDRV_IPCCMD_PHYREGISTERREAD                                    ,
#endif /* #if 0 */
    DEV_SWDRV_IPCCMD_PHYLIGHTPORTLEDNORMAL                              ,

#if (SYS_CPNT_POE == TRUE)

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    DEV_SWDRV_IPCCMD_PROCESSDRAGONITEDATA                               ,
#endif

#endif

    DEV_SWDRV_IPCCMD_TWSIINIT                                           ,
    DEV_SWDRV_IPCCMD_TWSIREGISTERWRITE                                  ,
    DEV_SWDRV_IPCCMD_TWSIREGISTERREAD                                   ,
    DEV_SWDRV_IPCCMD_TWSIDATAWRITE                                      ,
    DEV_SWDRV_IPCCMD_TWSIDATAREAD                                       ,
    DEV_SWDRV_IPCCMD_TWSIWRITE                                          ,
    DEV_SWDRV_IPCCMD_TWSIREAD                                           ,
    DEV_SWDRV_IPCCMD_MSCCPOEINIT                                        ,
    DEV_SWDRV_IPCCMD_MSCCPOEWRITE                                       ,
    DEV_SWDRV_IPCCMD_MSCCPOEREAD                                        ,
    DEV_SWDRV_IPCCMD_MSCCPOEEXIT                                        ,
    DEV_SWDRV_IPCCMD_SETOAMPDUTOCPU                                     ,
    DEV_SWDRV_IPCCMD_SETOAMLOOPBACK                                     ,
    DEV_SWDRV_IPCCMD_SETPORTINGVLANXLATEFORQINQ                         ,
    DEV_SWDRV_IPCCMD_SETPORTEGRVLANXLATEFORQINQ                         ,
    DEV_SWDRV_IPCCMD_SETEGRESSPACKETBLOCKFORBC                          ,
    DEV_SWDRV_IPCCMD_SETEGRESSPACKETBLOCKFORUMC                         ,
    DEV_SWDRV_IPCCMD_SETEGRESSPACKETBLOCKFORUUC                         ,
    DEV_SWDRV_IPCCMD_GETXMIIPHYREG                                      ,
    DEV_SWDRV_IPCCMD_SETXMIIPHYREG                                      ,
    DEV_SWDRV_IPCCMD_GETMIIPHYREGBYPAGE                                 ,
    DEV_SWDRV_IPCCMD_SETRAANDRRTRAP2CPU                                 ,
    DEV_SWDRV_IPCCMD_TRAPPTPTOCPU                                       ,
    DEV_SWDRV_IPCCMD_SETPTPSTATUS                                       ,
    DEV_SWDRV_IPCCMD_GETTXTSDATATIMESTAMP                               ,
    DEV_SWDRV_IPCCMD_DOPHYREGTRANSACTIONS                               ,
    DEV_SWDRV_IPCCMD_SETPORTMACADDR                                     ,
    DEV_SWDRV_IPCCMD_SETPORTPFCSTATUS                                   , /* SYS_CPNT_PFC */
    DEV_SWDRV_IPCCMD_UPDATEPFCPRIMAP                                    , /* SYS_CPNT_PFC */
    DEV_SWDRV_IPCCMD_SETMIMSERVICE                                      , /* SYS_CPNT_MAC_IN_MAC */
    DEV_SWDRV_IPCCMD_SETMIMSERVICEPORT                                  , /* SYS_CPNT_MAC_IN_MAC */
    DEV_SWDRV_IPCCMD_SETMIMSERVICEPORTLEARNINGSTATUSFORSTATIONMOVE      , /* SYS_CPNT_MAC_IN_MAC & SYS_CPNT_IAAS */
    DEV_SWDRV_IPCCMD_SETCPURATELIMIT                                    ,
    DEV_SWDRV_IPCCMD_GETPORTABILITY                                     ,
    DEV_SWDRV_IPCCMD_GETPORTTEMPERATURE                                 ,
    DEV_SWDRV_IPCCMD_SETCPUPORTRATE                                     ,
    DEV_SWDRV_IPCCMD_SETVLANLEARNINGSTATUS                               ,
    DEV_SWDRV_IPCCMD_SETPORTEGRESSBLOCK                                 ,
    DEV_SWDRV_IPCCMD_SETVLANFLOODING                                    ,
    DEV_SWDRV_IPCCMD_ENABLEPORTOPENFLOWMODE                             ,
    DEV_SWDRV_IPCCMD_DELETEALLPORTFROMVLAN                              ,
    DEV_SWDRV_IPCCMD_STATICMACMOVEPKTTOCPU                              ,
    DEV_SWDRV_IPCCMD_SETVXLANSTATUS                                     ,
    DEV_SWDRV_IPCCMD_SETUDPPORT                                         ,
    DEV_SWDRV_IPCCMD_CREATEVTEP                                         ,
    DEV_SWDRV_IPCCMD_DESTROYVTEP                                        ,
    DEV_SWDRV_IPCCMD_ADDVTEPINTOMCASTGROUP                              ,
    DEV_SWDRV_IPCCMD_SETVXLANVPN                                        ,
    DEV_SWDRV_IPCCMD_SETVXLANSTATUSPORT                                 ,
    DEV_SWDRV_IPCCMD_SETVXLANPORTLEARNING                               ,
    DEV_SWDRV_IPCCMD_CREATE_VXLAN_ACCESS_PORT,
    DEV_SWDRV_IPCCMD_CREATE_VXLAN_NETWORK_PORT,
    DEV_SWDRV_IPCCMD_DESTROY_VXLAN_PORT,
    DEV_SWDRV_IPCCMD_CREATE_VXLAN_NEXTHOP,
    DEV_SWDRV_IPCCMD_DESTROY_VXLAN_NEXTHOP,
    DEV_SWDRV_IPCCMD_ADD_VXLAN_ECMP_NEXTHOP,
    DEV_SWDRV_IPCCMD_REMOVE_VXLAN_ECMP_NEXTHOP,
#if (SYS_CPNT_HASH_SELECTION == TRUE)
    DEV_SWDRV_IPCCMD_SETHASHSELECTIONBLOCK                              ,
    DEV_SWDRV_IPCCMD_UNBINDHASHSELFORECMP                               ,
    DEV_SWDRV_IPCCMD_SETHASHSELECTIONFORECMP                            ,
#endif
    DEV_SWDRV_IPCCMD_SETPORTSTATUSFORLICENSE                            ,
#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
    DEV_SWDRV_IPCCMD_SETSWITCHINGMODE                                            ,
#endif
    DEV_SWDRV_IPCCMD_SETPORTFEC                                         , /* SYS_CPNT_SWCTRL_FEC */
    DEV_SWDRV_IPCCMD_GETPORTFECSTATUS                                   , /* SYS_CPNT_SWCTRL_FEC */
    DEV_SWDRV_IPCCMD_SETRANDOMDETECT                                  ,
};

BOOL_T DEV_SWDRV_PMGR_ResetOnDemand(BOOL_T include_cross_bar);
#if 0
void   DEV_SWDRV_PMGR_SetDevicePortMapping(DEV_SWDRV_Device_Port_Mapping_T unit_port_to_device_port_mapping_table[][SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);
#endif
UI32_T DEV_SWDRV_PMGR_GetLocalUnitChipsetNumber(void);
BOOL_T DEV_SWDRV_PMGR_Logical2PhyDevicePortID(UI32_T unit_id, UI32_T port, UI32_T *module_id, UI32_T *device_id, UI32_T *phy_port);
BOOL_T DEV_SWDRV_PMGR_Logical2PhyDeviceID(UI32_T unit_id, UI32_T port, UI32_T *module_id, UI32_T *device_id, UI32_T *phy_id);
BOOL_T DEV_SWDRV_PMGR_Physical2LogicalPort(UI32_T module_id, UI32_T device_id, UI32_T phy_port, UI32_T *unit, UI32_T *port);
BOOL_T DEV_SWDRV_PMGR_GetLocalDeviceIDFromModuleID(UI32_T unit, UI32_T module_id, UI32_T *device_id);
BOOL_T DEV_SWDRV_PMGR_GetModuleIdFromLocalDeviceId(UI32_T unit, UI32_T device_id, UI32_T *module_id);
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_UpdatePortSfpTxEnable
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function will configure the sfp tx enable setting kept in the
 *           shadow database to the hardware device.
 * INPUT   : unit_id - in which unit
 *           port    - which port wants to be configured
 * OUTPUT  : None
 * RETURN  : TRUE    - Success
 *           FALSE   - If a given (unit, port) is not available
 * NOTE    : 1. This function needs to be called when the insertion of a QSFP
 *              transceiver is detected. However, it is no harm to called when
 *              other types of a transceiver is inserted.
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_UpdatePortSfpTxEnable(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_EnablePortAdmin(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisablePortAdmin(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisableAllPortAdmin(void);
BOOL_T DEV_SWDRV_PMGR_SetPortCfgSpeedDuplex(UI32_T unit_id, UI32_T port, UI32_T speed_duplex);
BOOL_T DEV_SWDRV_PMGR_EnablePortAutoNeg(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisablePortAutoNeg(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_EnablePortCfgFlowCtrl(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisablePortCfgFlowCtrl(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_SetPortCfgFlowCtrl(UI32_T unit_id, UI32_T port, UI32_T mode);
BOOL_T DEV_SWDRV_PMGR_SetPortAutoNegCapability(UI32_T unit_id, UI32_T port, UI32_T capability);
BOOL_T DEV_SWDRV_PMGR_SetMiniGbicPortLEDStatus(UI32_T unit_id, UI32_T port, BOOL_T status);
BOOL_T DEV_SWDRV_PMGR_GetPortType(UI32_T unit_id, UI32_T port, UI32_T *port_type);
#if (SYS_CPNT_MAU_MIB == TRUE) || (SYS_CPNT_SYNCE == TRUE)
BOOL_T DEV_SWDRV_PMGR_SetPortRestartAutoNego(UI32_T unit, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_SetPortAutoNegoRemoteFaultAdvertisement (UI32_T unit, UI32_T port, UI32_T remote_fault);
BOOL_T DEV_SWDRV_PMGR_GetPortAutoNegoRemoteFaultAdvertisement(UI32_T unit, UI32_T port, UI32_T *remote_fault);
BOOL_T DEV_SWDRV_PMGR_GetPortLinkPartnerAutoNegoSignalingState (UI32_T unit, UI32_T port, UI32_T *state);
BOOL_T DEV_SWDRV_PMGR_GetPortAutoNegoProcessState (UI32_T unit, UI32_T port, UI32_T *state);
BOOL_T DEV_SWDRV_PMGR_GetPortLinkPartnerAutoNegoCapa(UI32_T unit, UI32_T port, UI32_T *capabilities);
BOOL_T DEV_SWDRV_PMGR_GetPortLinkPartnerAutoNegoRemoteFault(UI32_T unit, UI32_T port, UI32_T *remote_fault);
BOOL_T DEV_SWDRV_PMGR_GetPortJabberState (UI32_T unit, UI32_T port, UI32_T *state);
BOOL_T DEV_SWDRV_PMGR_GetPortFalseCarrierSenseCounter (UI32_T unit, UI32_T port, UI32_T *counter);
#endif
BOOL_T DEV_SWDRV_PMGR_SetSTAMode(UI32_T mode);
BOOL_T DEV_SWDRV_PMGR_SetPortSTAState(UI32_T mstid, UI32_T vlan_count, UI16_T *vlan_list, UI32_T unit_id,
                                 UI32_T port, UI32_T state);
BOOL_T DEV_SWDRV_PMGR_SetPortSTAStateWithMstidx(UI32_T mstidx, UI32_T unit_id, UI32_T port, UI32_T state);
BOOL_T DEV_SWDRV_PMGR_AddVlanToSTAWithMstidx(UI32_T vid, UI32_T mstidx);
BOOL_T DEV_SWDRV_PMGR_DeleteVlanFromSTAWithMstidx(UI32_T vid, UI32_T mstidx);
BOOL_T DEV_SWDRV_PMGR_SetPortPVID(UI32_T unit_id, UI32_T port, UI32_T pvid);
BOOL_T DEV_SWDRV_PMGR_CreateVlan(UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_DestroyVlan(UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_SetGlobalDefaultVlan(UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_DeletePortUntaggedVlanSet(UI32_T unit_id, UI32_T port, UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_AddPortUntaggedVlanSet(UI32_T unit_id, UI32_T port, UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_DeletePortVlanSet(UI32_T unit_id, UI32_T port, UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_AddPortVlanSet(UI32_T unit_id, UI32_T port, UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_SetPortToVlanMemberSet(UI32_T unit_id, UI32_T vid, UI8_T *port_list);
BOOL_T DEV_SWDRV_PMGR_SetPortToVlanUntaggedSet(UI32_T unit_id, UI32_T vid, UI8_T *port_list);
BOOL_T DEV_SWDRV_PMGR_SetSystemMTU(UI32_T status,UI32_T mtu);
BOOL_T DEV_SWDRV_PMGR_SetPortMTU(UI32_T unit,UI32_T port,UI32_T mtu);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_GetPortMaxFrameSize
 * -------------------------------------------------------------------------
 * PURPOSE : to get max frame size of port
 * INPUT   : unit
 *           port
 * OUTPUT  : untagged_max_frame_sz_p - max frame size for untagged frames
 *           tagged_max_frame_sz_p   - max frame size for tagged frames
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_GetPortMaxFrameSize(UI32_T unit, UI32_T port, UI32_T *untagged_max_frame_sz_p, UI32_T *tagged_max_frame_sz_p);

BOOL_T DEV_SWDRV_PMGR_EnableIngressFilter(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisableIngressFilter(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_AdmitOnlyVlanTaggedFrames(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_AdmitOnlyVlanUntaggedFrames(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_AdmitAllFrames(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_EnableTrapUnspecifiedTagFrame();
BOOL_T DEV_SWDRV_PMGR_DisableTrapUnspecifiedTagFrame();
BOOL_T DEV_SWDRV_PMGR_AddCpuToVlan(UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_DeleteCpuFromVlan(UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_SetPortMirroring(SYS_TYPE_Uport_T source_port,
                                  SYS_TYPE_Uport_T rx_to_sniffer_port,
                                  SYS_TYPE_Uport_T tx_to_sniffer_port);
BOOL_T DEV_SWDRV_PMGR_DeletePortMirroring(SYS_TYPE_Uport_T source_port,SYS_TYPE_Uport_T dest_port);
BOOL_T DEV_SWDRV_PMGR_EnablePortMirroring(SYS_TYPE_Uport_T source_port);
BOOL_T DEV_SWDRV_PMGR_DisablePortMirroring(SYS_TYPE_Uport_T source_port);
#if (SYS_CPNT_VLAN_MIRROR == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_AddVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will add the vlan mirror and destination port
 * INPUT   : unit -- which unit to set
 *           port -- which destination port to set
 *           vid  -- which vlan-id add to source mirrored table
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_AddVlanMirror(UI32_T unit, UI32_T port, UI32_T vid);

/*------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_DeleteVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will delete the vlan mirror and destination port
 * INPUT   : unit -- which unit to set
 *           port -- which destination port to set
 *           vid  -- which vlan-id add to source mirrored table
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : A destination port shall be removed when source vlan mirror has empty
 *------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_DeleteVlanMirror(UI32_T unit, UI32_T port, UI32_T vid);
#endif /* End of #if (SYS_CPNT_VLAN_MIRROR == TRUE) */

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetDestPortForMacMirror
 * -------------------------------------------------------------------------
 * PURPOSE : This function sets destination port for MAC based MIRROR
 * INPUT   : unit -- in which unit
 *           port -- which port to monitor
 *           mode -- TRUE: set, FALSE: remove
 * OUTPUT  : none
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Failed
 * NOTE    : none
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetDestPortForMacMirror(UI32_T unit, UI32_T port, BOOL_T mode);
#endif

#if (SYS_CPNT_ACL_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetDestPortForAclMirror
 * -------------------------------------------------------------------------
 * PURPOSE : This function sets destination port for ACL based MIRROR
 * INPUT   : unit -- in which unit
 *           port -- which port to monitor
 *           mode -- TRUE: set, FALSE: remove
 * OUTPUT  : none
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Failed
 * NOTE    : none
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetDestPortForAclMirror(UI32_T unit, UI32_T port, BOOL_T mode);
#endif

BOOL_T DEV_SWDRV_PMGR_CreateTrunk(UI32_T trunk_id);
BOOL_T DEV_SWDRV_PMGR_CreateTrunkwithMode(UI32_T trunk_id, UI8_T mode);
BOOL_T DEV_SWDRV_PMGR_SetTrunkLoadBalanceMode(UI32_T load_balance_mode);
BOOL_T DEV_SWDRV_PMGR_DestroyTrunk(UI32_T trunk_id);
BOOL_T DEV_SWDRV_PMGR_SetTrunkPortMembers(UI32_T trunk_id,
                                     UI32_T port_member_count,
                                     SYS_TYPE_Uport_T *port_member_list);
BOOL_T DEV_SWDRV_PMGR_SetTrunkBalanceMode(UI32_T trunk_id, UI8_T mode);
BOOL_T DEV_SWDRV_PMGR_GetTrunkBalanceModeofLocalUnit(UI32_T trunk_id, UI8_T * mode);
BOOL_T DEV_SWDRV_PMGR_AddMulticastAddrToTrunkMember(UI8_T            *mac,
                                               UI32_T           vid,
                                               UI32_T           trunk_id,
                                               SYS_TYPE_Uport_T trunk_member);
BOOL_T DEV_SWDRV_PMGR_DeleteMulticastAddrToTrunkMember(UI8_T            *mac,
                                                  UI32_T           vid,
                                                  UI32_T           trunk_id,
                                                  SYS_TYPE_Uport_T trunk_member);
BOOL_T DEV_SWDRV_PMGR_EnableIgmpTrap(void);
BOOL_T DEV_SWDRV_PMGR_DisableIgmpTrap();
BOOL_T DEV_SWDRV_PMGR_TrapIGMPToCPU(BOOL_T to_cpu, BOOL_T drop);
BOOL_T DEV_SWDRV_PMGR_SetBroadcastStormControlThreshold(UI32_T unit_id, UI32_T port, UI32_T threshold, UI32_T mode);
BOOL_T DEV_SWDRV_PMGR_SetMulticastStormControlThreshold(UI32_T unit_id, UI32_T port, UI32_T threshold, UI32_T mode);
BOOL_T DEV_SWDRV_PMGR_SetUnknownUnicastStormControlThreshold(UI32_T unit_id, UI32_T port, UI32_T threshold, UI32_T mode);
BOOL_T DEV_SWDRV_PMGR_EnableBroadcastStormControl(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisableBroadcastStormControl(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_EnableMulticastStormControl(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisableMulticastStormControl(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_EnableUnknownUnicastStormControl(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisableUnknownUnicastStormControl(UI32_T unit_id, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_GetPortStormGranularity(UI32_T unit, UI32_T port, UI32_T mode, UI32_T *storm_granularity);
BOOL_T DEV_SWDRV_PMGR_SetPortUserDefaultPriority(UI32_T unit_id, UI32_T port, UI32_T priority);
BOOL_T DEV_SWDRV_PMGR_SetPriorityMapping(UI32_T unit_id, UI32_T port, UI8_T *dot1p_to_cos_mapping);
BOOL_T DEV_SWDRV_PMGR_SetPriorityMappingPerSystem(UI8_T *dot1p_to_cos_mapping);
BOOL_T DEV_SWDRV_PMGR_SetNumberOfTrafficClassByPort(UI32_T unit_id, UI32_T port, UI32_T number_of_queue);
BOOL_T DEV_SWDRV_PMGR_SetNumberOfTrafficClass(UI32_T number_of_queue);    /* whole system */
BOOL_T DEV_SWDRV_PMGR_SetStackingPortPriorityMapping(UI32_T unit_id, UI32_T port, UI8_T mapping[8]);
BOOL_T DEV_SWDRV_PMGR_EnableJumboFrame(void);
BOOL_T DEV_SWDRV_PMGR_DisableJumboFrame(void);
BOOL_T DEV_SWDRV_PMGR_GetPortOperSpeedDuplex(UI32_T unit_id, UI32_T port, UI32_T *speed_duplex);
BOOL_T DEV_SWDRV_PMGR_GetPortLinkStatus(UI32_T unit_id, UI32_T port, UI32_T *link_status);
BOOL_T DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId(UI32_T device_id, UI32_T phy_port, UI32_T *link_status);
BOOL_T DEV_SWDRV_PMGR_EnableHgPortAdmin();
BOOL_T DEV_SWDRV_PMGR_GetPortFlowCtrl(UI32_T unit_id, UI32_T port, UI32_T *flow_ctrl);
void DEV_SWDRV_PMGR_LinkScan_Update();
BOOL_T DEV_SWDRV_PMGR_EnableIPMC(I32_T unit_id);
BOOL_T DEV_SWDRV_PMGR_DisableIPMC(I32_T unit_id);
BOOL_T DEV_SWDRV_PMGR_IPMC_EnableSourcePortCheck(I32_T unit_id);
BOOL_T DEV_SWDRV_PMGR_IPMC_DisableSourcePortCheck(I32_T unit_id);
BOOL_T DEV_SWDRV_PMGR_IPMC_EnableSourceIPSearch(I32_T unit_id);
BOOL_T DEV_SWDRV_PMGR_IPMC_DisableSourceIPSearch(I32_T unit_id);
BOOL_T DEV_SWDRV_PMGR_EnablePortLearning(UI32_T unit, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_DisablePortLearning(UI32_T unit, UI32_T port);
BOOL_T DEV_SWDRV_PMGR_SetPortLearningStatus(UI32_T unit, UI32_T port, BOOL_T learning, BOOL_T to_cpu, BOOL_T drop);
BOOL_T DEV_SWDRV_PMGR_SetVlanLearningStatus(UI32_T vid, BOOL_T learning);
BOOL_T  DEV_SWDRV_PMGR_AddDot1vProtocolPortEntry(   UI32_T  unit_id,
                                               UI32_T  port,
                                               UI32_T  group_index,
                                               UI32_T  vlan_id,
                                               UI32_T  nbr_of_type_protocol,
                                               UI8_T   *frame_type,
                                               #if (SYS_CPNT_PROTOCOL_VLAN_PORT_SUPPORT_PRIORITY == TRUE)
                                               UI8_T   priority,
                                               #endif
                                               UI8_T   (*protocol_value)[DEV_SWDRV_MAX_1V_PROTOCOL_VALUE_LENGTH]
                                            );
BOOL_T  DEV_SWDRV_PMGR_DelDot1vProtocolPortEntry(   UI32_T  unit_id,
                                               UI32_T  port,
                                               UI32_T  group_index,
                                               UI32_T  vlan_id,
                                               UI32_T  nbr_of_type_protocol,
                                               UI8_T   *frame_type,
                                               UI8_T   (*protocol_value)[DEV_SWDRV_MAX_1V_PROTOCOL_VALUE_LENGTH]
                                           );
BOOL_T  DEV_SWDRV_PMGR_SetDot1vProtocolGroupEntry(  UI32_T  frame_type,
                                               UI8_T   *protocol_value,
                                               UI32_T  group_index
                                            );
#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
BOOL_T DEV_SWDRV_PMGR_SetPortComboForcedMode(UI32_T unit, UI32_T port, UI32_T forcedmode, UI32_T mau_type, UI32_T speed);
#endif
void DEV_SWDRV_PMGR_ShutdownSwitch(void);

#ifndef INCLUDE_DIAG
BOOL_T  DEV_SWDRV_PMGR_UpdateHerculesUCTable(Stacking_Info_T *stack_info);
BOOL_T  DEV_SWDRV_PMGR_ConfigTopologyInfo(Stacking_Info_T *stack_info);
#endif
BOOL_T DEV_SWDRV_PMGR_Physical2LogicalPortByUnitID_DeviceID(UI32_T unit_id, UI32_T device_id, UI32_T phy_port, UI32_T *unit, UI32_T *port);
BOOL_T DEV_SWDRV_PMGR_GetStackChipDeviceID(UI32_T *stackchipdeviceid);
void  DEV_SWDRV_PMGR_GetDstUnitInfo(UI32_T src_unit,UI32_T dst_unit,UI32_T *port,UI32_T *dst_mod);
void DEV_SWDRV_PMGR_HiGigRemoteLoopbackSet(UI32_T unit, UI32_T port, UI32_T mode);
#ifdef BLANC
BOOL_T DEV_SWDRV_PMGR_Get5670linkport(UI32_T module_id, UI32_T *phy_port);
#endif
BOOL_T DEV_SWDRV_PMGR_GetCopperEnergyDetect(UI32_T unit_id, UI32_T port, UI32_T *copper_energy_detect);
BOOL_T DEV_SWDRV_PMGR_EnableUMCASTIpTrap(void);
BOOL_T DEV_SWDRV_PMGR_DisableUMCASTIpTrap(void);
#ifndef INCLUDE_DIAG
BOOL_T DEV_SWDRV_PMGR_EnableUMCASTMacTrap(void);
BOOL_T DEV_SWDRV_PMGR_DisableUMCASTMacTrap(void);
#endif
BOOL_T DEV_SWDRV_PMGR_SetFiberPortAutoNeg(UI32_T unit, UI32_T port, UI32_T auto_neg);
#if 0
BOOL_T DEV_SWDRV_PMGR_SetPortForwardingAttribute(UI32_T port, UI32_T attribute);
#endif
void  DEV_SWDRV_PMGR_GetFabricportByUnitPosition(UI32_T unitid,UI32_T  *frabicStkport);
void  DEV_SWDRV_PMGR_GetFabricPortToDstUnit(UI32_T unit,UI32_T *frabicStkport);
BOOL_T  DEV_SWDRV_PMGR_GetMasterModuleID(UI32_T  *mastermoduleid);
BOOL_T  DEV_SWDRV_PMGR_GetModuleInfoByUnit(UI32_T unit_id,BOOL_T mainbrd_option,UI32_T *moduleid);
BOOL_T  DEV_SWDRV_PMGR_GetLocalCrossBarDeviceId(UI32_T *cross_bar_id);
#if 0
BOOL_T DEV_SWDRV_PMGR_EnableStackingChipAllPort(void);
#endif
void DEV_SWDRV_PMGR_VID_BREAK(void);
BOOL_T DEV_SWDRV_PMGR_GetNextSwitchChipId(UI32_T *device_id);
BOOL_T DEV_SWDRV_PMGR_AddIPv6MulticastAddress(UI8_T mac_address[], UI32_T vid);
BOOL_T DEV_SWDRV_PMGR_DelIPv6MulticastAddress(UI8_T mac_address[], UI32_T vid);
#if (SYS_CPNT_MLDSNP == TRUE)
BOOL_T DEV_SWDRV_PMGR_EnableMLDTrap2CPU(void);
BOOL_T DEV_SWDRV_PMGR_DisableMLDTrap2CPU(void);
BOOL_T DEV_SWDRV_PMGR_TrapMLDToCPU(BOOL_T to_cpu, BOOL_T drop);
#endif
#if (SYS_CPNT_NDSNP == TRUE)
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_TrapNDToCPU
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function enable trap ND packet to CPU
 * INPUT   : to_cpu    --  send packet to cpu or not
 *           drop      --  drop packet or not
 * OUTPUT  : None
 * RETURN  : TRUE  --  get next succuess
             FALSE --  don't exist the next chip id
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_TrapNDToCPU(BOOL_T to_cpu, BOOL_T drop);

#endif

#if (SYS_CPNT_IPV6_RA_GUARD_TRAP_BY_REG == TRUE)
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetRaAndRrTrap2CPU
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function eanble/disable RA/RR packet to CPU
 * INPUT   : is_enabled - TRUE to enable
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetRaAndRrTrap2CPU(
    BOOL_T  is_enabled);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_TRAP_BY_REG == TRUE) */

void DEV_SWDRV_PMGR_GetForwardingStackingPortBitMap(UI32_T *stacking_port_bmap);
void    DEV_SWDRV_PMGR_Receive_All(int flag);
#if (SYS_CPNT_CPU_INTERFACE_CPU_JOIN_VLAN == TRUE)
BOOL_T DEV_SWDRV_PMGR_SetBlockCPUForwardingAttr();
#endif
BOOL_T DEV_SWDRV_PMGR_Set1000BaseTPortTestMode(UI32_T unit_id,
                                          UI32_T port,
                                          UI16_T mode);
BOOL_T DEV_SWDRV_PMGR_Get1000BaseTPortTestMode(UI32_T unit_id,
                                          UI32_T port,
                                          UI16_T *mode);
BOOL_T DEV_SWDRV_PMGR_Set1000BaseTTransmitterType(UI32_T unit_id,
                                             UI32_T port,
                                             UI16_T transmitter_type);
BOOL_T DEV_SWDRV_PMGR_Get1000BaseTTransmitterType(UI32_T unit_id,
                                             UI32_T port,
                                             UI16_T *transmitter_type);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetPort1000BaseTForceMode
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function sets the specified giga-phy mode for a given port.
 *           This function forces a given port to operate on the specified giga-phy mode
 * INPUT   : device_id          -- in which device
 *           phy_port           -- which phy port to set
 *           force_mode         -- available modes for a given port are listed below
 *                                 VAL_portMasterSlaveModeCfg_master
 *                                 VAL_portMasterSlaveModeCfg_slave
 *                                 VAL_portMasterSlaveModeCfg_auto
 * OUTPUT  : None
 * RETURN  : TRUE               -- Success
 *           FALSE              -- If a given (unit, port) is not available, or it can not support
 *                                 the specified giga-phy mode
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetPort1000BaseTForceMode(UI32_T device_id, UI32_T phy_port, UI32_T mode);


BOOL_T DEV_SWDRV_PMGR_GetAutoMDIXmode(UI32_T unit, UI32_T port, UI32_T *autoMDIXmode);
void DEV_SWDRV_PMGR_InitiateProcessResource();
void DEV_SWDRV_PMGR_Create_InterCSC_Relation();
I32_T DEV_SWDRV_PMGR_SocPciWrite(UI32_T device_id, UI32_T reg_addr, UI32_T data) ;
BOOL_T DEV_SWDRV_PMGR_UpdatePortMapping(BOOL_T is_init);

#if (SYS_CPNT_POWER_SAVE == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetPortPowerSave
 *------------------------------------------------------------------------|
 * FUNCTION: Set the power saving status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetPortPowerSave(UI32_T unit, UI32_T port, BOOL_T status, BOOL_T link_status);
#endif

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_SWDRV_PMGR_GetCableDiag
 *------------------------------------------------------------------------|
 * FUNCTION: Execute and return the cable diagnositc informaiton
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_GetCableDiag(UI32_T unit, UI32_T port, DEV_SWDRV_CableDiagResult_T *diag_result);

#if (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_SWDRV_PMGR_GetCableDiagResult
 *------------------------------------------------------------------------|
 * FUNCTION: Get the cable diagnositc informaiton after testing
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_GetCableDiagResult(UI32_T unit, UI32_T port, DEV_SWDRV_CableDiagResultGeneral_T *diag_result);
#endif
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_EnableInternalLoopback
 *------------------------------------------------------------------------------
 * PURPOSE  : Enable Internal Loopback
 * INPUT    : port - port.
 *
 * OUTPUT   : None.
 * RETURN   : TRUE : SUCCESS, FALSE: FAIL.
 * NOTES    : None.
 *------------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_EnableInternalLoopback(UI32_T port);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_DisableInternalLoopback
 *------------------------------------------------------------------------------
 * PURPOSE  : Disable Internal Loopback
 * INPUT    : port - port.
 *
 * OUTPUT   : None.
 * RETURN   : TRUE : SUCCESS, FALSE: FAIL.
 * NOTES    : None.
 *------------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_DisableInternalLoopback(UI32_T port);

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_IsPortLoopbacking
 *------------------------------------------------------------------------------
 * PURPOSE  : Disable Internal Loopback
 * INPUT    : port - port.
 *
 * OUTPUT   : None.
 * RETURN   : TRUE : SUCCESS, FALSE: FAIL.
 * NOTES    : None.
 *------------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_IsPortLoopbacking(UI32_T port);
#endif

#ifdef BCM_ROBO_SUPPORT
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_SWDRV_PMGR_WriteRegister
 *------------------------------------------------------------------------|
 * FUNCTION: Set chip's register value
 * INPUT   : device_id   -- chip unit number
 *           reg_addr    -- register address
 *           reg_val     -- register value
 *           reg_len     -- register len (in byte)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_WriteRegister(UI32_T device_id, UI32_T reg_addr, UI32_T reg_val, UI32_T reg_len);
#endif
BOOL_T DEV_SWDRV_PMGR_SetStackingPort(BOOL_T is_stacking_port);

/*Add for 802.1x -- xiongyu 20090109*/
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_SetPortAuthMode
 * -------------------------------------------------------------------------------------------
 * PURPOSE : Set the 802.1X control mode
 * INPUT   : unit_id            -- which unit to set
 *           port               -- which port to set
 *           mode           -- control mode
 * OUTPUT  : None
 * RETURN  : TRUE               -- Success
 *           FALSE              -- Set error.
 * NOTE    :
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetPortAuthMode(UI32_T unit_id, UI32_T port, UI32_T mode);


#if (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
BOOL_T DEV_PMGR_SWDRV_SetMDIXMode(UI32_T unit_id, UI32_T port, UI32_T medium);
#endif

#if (SYS_CPNT_MAC_VLAN == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetMacVlanEntry
 * -------------------------------------------------------------------------
 * FUNCTION:
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetMacVlanEntry(UI8_T *mac_address, UI16_T vid, UI8_T priority);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_DeleteMacVlanEntry
 * -------------------------------------------------------------------------
 * FUNCTION:
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_DeleteMacVlanEntry(UI8_T *mac_address);
#endif

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetEapolFramePassThrough
 * -------------------------------------------------------------------------------------------
 * PURPOSE : To set EAPOL frames pass through (pass through means not trapped to CPU)
 * INPUT   : state (TRUE/FALSE)
 * OUTPUT  : None
 * RETURN  : TRUE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetEapolFramePassThrough(BOOL_T state);

#if(SYS_CPNT_MLDSNP == TRUE)
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_mld_packet_trap
 * -------------------------------------------------------------------------------------------
 * PURPOSE : Set mld packet trap to cpu
 * INPUT   : enabled (TRUE/FALSE)
 * OUTPUT  : None
 * RETURN  : TRUE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_mld_packet_trap(BOOL_T enabled);
#endif

#if (SYS_CPNT_RSPAN == TRUE)
/* -----------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetRspanVlanTag
 * -----------------------------------------------------------------------------
 * PURPOSE : Set VLAN for egressing mirrored packets on a port.
 * INPUT   : target_port -- mirror-to port to set (-1 for all ports)
 *           tpid        -- tag protocol id (tpid and vid are equal 0, meaning
 *                          to disable RSPAN)
 *           vlan        -- virtual lan number (tpid and vid are equal 0,
 *                          meaning to disable RSPAN)
 * OUTPUT  : None
 * RETURN  : TRUE        -- Success
 *           FALSE       -- If the specified specific-port is not available, or
 *                          the ASIC can not support port mirroring function
 * NOTE    : For now RSPAN doesn't support stacking.
 * -----------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetRspanVlanTag(SYS_TYPE_Uport_T target_port, UI16_T tpid, UI16_T vlan);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_DisableRspanPortLearning
 * -----------------------------------------------------------------------------
 * FUNCTION: This function will disable port mac learning.
 * INPUT   : unit -- in which unit
 *           port -- in which port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : For now RSPAN doesn't support stacking. The reason not to use
 *           SWDRV_DisablePortLearning is because there will be different ways
 *           to handle remote parts.
 * -----------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_DisableRspanPortLearning(UI32_T unit_id, UI32_T port);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_EnableRspanPortLearning
 * -----------------------------------------------------------------------------
 * FUNCTION: This function will enable port mac learning.
 * INPUT   : unit -- in which unit
 *           port -- in which port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : For now RSPAN doesn't support stacking. The reason not to use
 *           SWDRV_DisablePortLearning is because there will be different ways
 *           to handle remote parts.
 * -----------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_EnableRspanPortLearning(UI32_T unit_id, UI32_T port);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_ModifyMaxFrameSize
 * -----------------------------------------------------------------------------
 * FUNCTION: This function will modify maximum fram size of RSPAN
 * INPUT   : port -- in which port
 *           is_increase -- TRUE:add RSPAN tag size (4 bytes)
 *                       -- FALSE:delete RSPAN tag size (4 bytes)
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : For now RSPAN doesn't support stacking. The reason not to use
 *           SWDRV_DisablePortLearning is because there will be different ways
 *           to handle remote parts.
 * -----------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_ModifyMaxFrameSize(UI8_T port, BOOL_T is_increase);
#endif /* end of #if (SYS_CPNT_RSPAN == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_GetPortMediaActive
 * -------------------------------------------------------------------------
 * FUNCTION: This function check which media is active.
 * INPUT   : device_id
 *           phy_port
 * OUTPUT  : port_media_p -- DEV_SWDRV_PORT_MEDIA_COPPER
 *                           DEV_SWDRV_PORT_MEDIA_FIBER
 *                           DEV_SWDRV_PORT_MEDIA_UNKNOWN
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_GetPortMediaActive(UI32_T unit, UI32_T port, UI32_T *port_media_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_TrapUnknownIpMcastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ip multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           vid = 0 -- global setting, vid = else -- which VLAN ID to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_TrapUnknownIpMcastToCPU(BOOL_T to_cpu, BOOL_T flood, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_TrapUnknownIpv6McastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ipv6 multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           vid = 0 -- global setting, vid = else -- which VLAN ID to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_TrapUnknownIpv6McastToCPU(BOOL_T to_cpu, BOOL_T flood, UI32_T vid);

#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_REMOTE_LB_PASSIVELY_BY_RULE != TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_SetOamPduToCpu
 * -------------------------------------------------------------------------
 * PURPOSE : This function will trap EFM OAM PDU to CPU
 * INPUT   : unit  -- which unit.
 *           port  -- which port.
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetOamPduToCpu(UI32_T unit, UI32_T port, BOOL_T enable);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_SetOamLoopback
 * -------------------------------------------------------------------------
 * PURPOSE : This function will enable EFM OAM Loopback
 * INPUT   : unit  -- which unit.
 *           port  -- which port.
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetOamLoopback(UI32_T unit, UI32_T port, BOOL_T enable);
#endif /* (SYS_CPNT_EFM_OAM_REMOTE_LB_PASSIVELY_BY_RULE != TRUE) */
#endif /* (SYS_CPNT_EFM_OAM == TRUE) */

/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_SetCpuRateLimit
 * -------------------------------------------------------------------------
 * PURPOSE : To configure CPU rate limit
 * INPUT   : pkt_type  -- SWDRV_PKTTYPE_XXX
 *           rate      -- in pkt/s. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetCpuRateLimit(UI32_T pkt_type, UI32_T rate);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_GetPortAbility
 * -------------------------------------------------------------------------
 * PURPOSE : To get port abilities
 * INPUT   : unit
 *           port
 * OUTPUT  : ability_p
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_GetPortAbility(UI32_T unit, UI32_T port, DEV_SWDRV_PortAbility_T *ability_p);

#if 0 /* not used */
BOOL_T DEV_SWDRV_PMGR_PhyRegisterWrite(UI32_T unit, UI32_T port, UI8_T reg, UI16_T data);
BOOL_T DEV_SWDRV_PMGR_PhyRegisterRead(UI32_T unit, UI32_T port, UI8_T reg, UI16_T* data);
#endif /* #if 0 */
BOOL_T DEV_SWDRV_PMGR_PhyLightPortLedNormal();

BOOL_T DEV_SWDRV_PMGR_TwsiInit();
BOOL_T DEV_SWDRV_PMGR_TwsiRegisterWrite(UI32_T dev_slv_id, UI32_T reg_addr, UI32_T value);
BOOL_T DEV_SWDRV_PMGR_TwsiRegisterRead(UI32_T dev_slv_id, UI32_T reg_addr, UI32_T* value);
BOOL_T DEV_SWDRV_PMGR_TwsiDataWrite(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);
BOOL_T DEV_SWDRV_PMGR_TwsiDataRead(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);
#if (SYS_CPNT_POE == TRUE)

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
BOOL_T DEV_SWDRV_PMGR_ProcessDragoniteData(UI32_T event, UI32_T unit, UI32_T port, UI32_T data, CPSS_GEN_DRAGONITE_DATA_STC *dragonitep);
#endif

BOOL_T DEV_SWDRV_PMGR_TwsiWrite(UI8_T I2C_Address, UI16_T Flags, const UI8_T* Txdata, UI16_T num_write_length, void* UserParam);
BOOL_T DEV_SWDRV_PMGR_TwsiRead(UI8_T I2C_Address, UI16_T Flags, UI8_T* Rxdata, UI16_T number_of_bytes_to_read, void* UserParam);
BOOL_T DEV_SWDRV_PMGR_MSCC_POE_Init();
BOOL_T DEV_SWDRV_PMGR_MSCC_POE_Write(UI8_T* pTxdata, UI16_T num_write_length, I32_T* pDevice_error);
BOOL_T DEV_SWDRV_PMGR_MSCC_POE_Read(UI8_T* pRxdata, UI16_T num_read_length, I32_T* pDevice_error);
BOOL_T DEV_SWDRV_PMGR_MSCC_POE_Exit();
#endif

#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_Reconfig_Module
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function reconfig HW PHY singal when plug in module.
 * INPUT   : module_slot_index  -- module slot index
 *           module_id;         -- module ID
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_Reconfig_Module(UI32_T module_slot_index, UI8_T module_id);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_Reconfig_Module_ByPprt
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function reconfig HW PHY singal for a specified port.
 * INPUT   : port
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_Reconfig_Module_ByPprt(UI32_T port);
#endif /*end of SYS_CPNT_10G_MODULE_SUPPORT */

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_GetXMIIPhyReg
 * -------------------------------------------------------------------------------------------
 * PURPOSE : Get the register value of the specified XG Phy through
 *           XMII (10-Gigabit Media Independent Interface).
 * INPUT   : device_id      -  The device id of the XG Phy port
 *           phy_addr       -  PHY address
 *           dev_addr       -  Device address
 *           phy_reg_addr   -  PHY register address
 * OUTPUT  : reg_value_p    -  The register value got from XMII
 * RETURN  : TRUE - Success, FALSE - Failed
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_GetXMIIPhyReg(UI32_T device_id, UI32_T phy_addr, UI32_T dev_addr, UI32_T phy_reg_addr, UI16_T *reg_value_p);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetXMIIPhyReg
 * -------------------------------------------------------------------------------------------
 * PURPOSE : Set the register value of the specified XG Phy through
 *           XMII (10-Gigabit Media Independent Interface).
 * INPUT   : device_id      -  The device id of the XG Phy port
 *           phy_addr       -  PHY address
 *           dev_addr       -  Device address
 *           phy_reg_addr   -  PHY register address
 *           reg_value      -  The register value to be set through XMII
 * OUTPUT  : None
 * RETURN  : TRUE - Success, FALSE - Failed
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetXMIIPhyReg(UI32_T device_id, UI32_T phy_addr, UI32_T dev_addr, UI32_T phy_reg_addr, UI16_T reg_value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_GetMIIPhyRegByPage
 * -------------------------------------------------------------------------
 * FUNCTION: This function will Read Phy Register by specified Page
 * INPUT   : device_id - device id
 *           phy_port  - PHY port id
 *           phy_addr  - PHY address
 *           phy_reg   - PHY register to read
 *           page_reg  - page register of the PHY
 *           page_val  - page value to be written to the page register of the PHY
 *           page_mask - the mask used together with page_val when writing page
 *                       register of the PHY, only the bit with value 1 will be
 *                       written to the page register.
 * OUTPUT  : data_p    - Register value read from PHY
 * RETURN  : TRUE/FALSE
 * NOTE    : Must not call this function if PHY do not have a page register
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_GetMIIPhyRegByPage(UI32_T device_id, UI32_T phy_addr,
    UI32_T phy_port, UI32_T phy_reg, UI32_T page_reg, UI32_T page_val, UI32_T page_mask, UI16_T *data_p);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_DoPHYRegsTransactions
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function will do phy register write operations by the given arguments
 *           atomically
 * INPUT   : unit_id     -- which unit to set
 *           port        -- which port to set
 *           phy_reg_ops -- an array which contains the phy reg operations to be performed
 *                          as a transaction
 *           num_of_ops  -- number of element in phy_reg_ops[]
 * OUTPUT  : None
 * RETURN  : TRUE        -- successfully
 *           FALSE       -- errors occur when doing phy register write operations
 * NOTE    : Number of element in phy_reg_ops cannot be greater than DEV_SWDRV_PMGR_MAX_NBR_OF_PHY_REG_OP
 *           Always switch page register back to page 0
 *           if phy_reg_ops[] changes the page register.
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_DoPHYRegsTransactions(UI32_T unit_id, UI32_T port, DEV_SWDRV_PHY_Reg_Operation_T phy_reg_ops[], UI32_T num_of_ops);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetPortMacAddr
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function configure source MAC address of chip port.
 * INPUT   : unit
 *           port
 *           mac_addr
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetPortMacAddr(UI32_T unit, UI32_T port, UI8_T *mac_addr);

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetPortPfcStatus
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function configure PFC status.
 * INPUT   : unit
 *           port
 *           rx_en      -- enable/disable PFC response
 *           tx_en      -- enable/disable PFC triggering
 *           pri_en_vec -- bitmap of enable status per priority
 *                         set bit to enable PFC; clear to disable.
 * OUTPUT  : None
 * RETURN  : TRUE  --  get next succuess
             FALSE --  don't exist the next chip id
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetPortPfcStatus(UI32_T unit, UI32_T port, BOOL_T rx_en, BOOL_T tx_en, UI16_T pri_en_vec);

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_UpdatePfcPriMap
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function update PFC priority to queue mapping.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_UpdatePfcPriMap(void);
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_MAC_IN_MAC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_PMGR_SWDRV_SetMimService
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create/destroy a MiM service instance.
 * INPUT   : mim_p            -- MiM service instance info.
 *           is_valid         -- TRUE to create/update; FALSE to destroy.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetMimService(DEV_SWDRV_MimServiceInfo_T *mim_p, BOOL_T is_valid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetMimServicePort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add/delete member port to a MiM service instance.
 * INPUT   : mim_port_p       -- MiM port info.
 *           is_valid         -- TRUE to add; FALSE to delete.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetMimServicePort(DEV_SWDRV_MimPortInfo_T *mim_port_p, BOOL_T is_valid);

#if (SYS_CPNT_IAAS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_SetMimServicePortLearningStatusForStationMove
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set MiM port learning status
 *              for station move handling
 * INPUT    :   learning
 *              to_cpu
 *              drop
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetMimServicePortLearningStatusForStationMove(BOOL_T learning, BOOL_T to_cpu, BOOL_T drop);
#endif /* (SYS_CPNT_IAAS == TRUE) */
#endif /* (SYS_CPNT_MAC_IN_MAC == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_GetAllPortAverageTemperature
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get average temperature from PHY chipsets.
 * INPUT   : None.
 * OUTPUT  : temperature -- The average temperature from PHY chipsets.
 * RETURN  : True: Successfully.
 *           False: If not available / Failed.
 * NOTE    : The feature will be supported by following PHY chipset.
 *           1. 88E1540 family:
 *              - 88E1543
 *              - 88E1545
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_GetAllPortAverageTemperature(UI32_T *temperature);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetCpuPortRate
 * -------------------------------------------------------------------------
 * FUNCTION:
 * INPUT   : packet per second       -- cpu port rx rate
 * OUTPUT  : None
 * RETURN  : TRUE:
 *           FALSE:
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetCpuPortRate(UI32_T packet_per_second);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetPortEgressBlock
 * -------------------------------------------------------------------------
 * FUNCTION: To set egress block ports.
 * INPUT   : unit
 *           port
 *           egr_blk_uport_list - uport list to block.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetPortEgressBlock(
    UI32_T unit,
    UI32_T port,
    UI8_T egr_blk_uport_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_SetVlanFlooding
 * -------------------------------------------------------------------------
 * PURPOSE : This function will drop and disable unknown UC/MC/BC flooding.
 *           1. Drop unicast packets in the VLAN that miss the L2 lookup and
 *              do not copy to CPU. Multicast packets must not be dropped.
 *           2. Block flooding of unknown unicast, unknown multicast and
 *              broadcast by default.
 * INPUT   : vid   -- which VLAN
 *           type  -- Uknown UC, Uknown MC or BC
 *           flood -- TRUE = flooding, FALSE = not flooding
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_SetVlanFlooding(UI32_T vid, DEV_SWDRV_VlanFloodingType_T type, BOOL_T flood);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_EnablePortOpenFlowMode
 * -------------------------------------------------------------------------
 * PURPOSE : This function will configure the following settings for
 *           specified port to meet OpenFlow requirements.
 *           1. Turn off flow control on each port.
 *           2. Disable MAC learning on new addresses and copy SLF packet
 *              to CPU.
 *           3. Disable MAC learning on station moves and copy SLF packet
 *              to CPU.
 *           4. Disable IPv4 Multicast
 *           5. Disable IPv6 Multicast
 *           6. Discard untagged traffic
 *           7. Filter traffic for a VLAN which is not a member of a port.
 * INPUT   : unit -- which unit
 *           port -- which port to enable OpenFlow mode
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_EnablePortOpenFlowMode(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_DeleteAllPortFromVlan
 * -------------------------------------------------------------------------
 * PURPOSE : Remove all ports from specified VLAN
 * INPUT   : vid -- which VLAN to remove all members
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_DeleteAllPortFromVlan(UI32_T vid);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_StaticMacMovePktToCpu
 * -------------------------------------------------------------------------
 * PURPOSE : The operation for static MAC address port move event.
 * INPUT   : is_enable -- TRUE will trap packet to CPU. FASLE will not.
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_StaticMacMovePktToCpu(UI32_T is_enable);

#if (SYS_CPNT_VXLAN == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetVxlanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable vxlan globally.
 * INPUT   : is_enable          - TRUE to enable
 *           is_random_src_port - TRUE to configure random src port also
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetVxlanStatus(
    BOOL_T  is_enable,
    BOOL_T  is_random_src_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetVxlanStatusPort
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable vxlan status for a port.
 * INPUT   : unit        - unit to set
 *           port        - port to set
 *           is_acc_port - TRUE if it's access port
 *           is_enable   - TRUE if enable
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : not support trunk !!!
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetVxlanStatusPort(
    UI32_T  unit,
    UI32_T  port,
    BOOL_T  is_acc_port,
    BOOL_T  is_enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetVxlanUdpPort
 * -------------------------------------------------------------------------
 * FUNCTION: To set udp dst port for vxlan globally.
 * INPUT   : udp_port - dst port to conifgure
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetVxlanUdpPort(UI32_T udp_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetVxlanVpn
 * -------------------------------------------------------------------------
 * FUNCTION: To add/del VPN for VXLAN
 * INPUT   : vpn_info_p - key for add is vnid
 *                        key for del is vfi/bc_group
 *           is_add     - TRUE to add
 * OUTPUT  : vpn_info_p - vfi/bc_group for add
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetVxlanVpn(
    DEV_SWDRV_VxlanVpnInfo_T    *vpn_info_p,
    BOOL_T                      is_add);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_AddVtepIntoMcastGroup
 * -------------------------------------------------------------------------
 * FUNCTION: To add/del a vxlan port to mcast group.
 * INPUT   : bcast_group - bcast group to set
 *           vxlan_port  - vxlan port to set
 *           unit        - unit to set
 *           port        - port to set (trunk id if unit is 0)
 *           is_add      - TRUE to add
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_AddVtepIntoMcastGroup(
    UI32_T  bcast_group,
    UI32_T  vxlan_port,
    BOOL_T  is_add);

BOOL_T DEV_SWDRV_PMGR_CreateVxlanAccessPort(
    UI32_T              vfi_id,
    UI32_T              l3_inf_id,
    UI32_T              unit,
    UI32_T              port,
    UI8_T               mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T              match_type,
    UI32_T              *vxlan_port_p);

BOOL_T DEV_SWDRV_PMGR_CreateVxlanNetworkPort(
    UI32_T                      vfi_id,
    UI32_T              udp_port,
    BOOL_T              is_mc,
    L_INET_AddrIp_T     *l_vtep_p,
    L_INET_AddrIp_T     *r_vtep_p,
    BOOL_T                      is_ecmp,
    void                        *nh_hw_info,
    UI32_T              *vxlan_port_p);

BOOL_T DEV_SWDRV_PMGR_DestroyVxlanPort(
    UI32_T  vfi_id,
    UI32_T  vxlan_port_id,
    BOOL_T  is_ecmp);

BOOL_T DEV_SWDRV_PMGR_CreateVxlanNexthop(
    UI32_T              l3_inf_id,
    UI32_T  unit,
    UI32_T              port,
    UI8_T               mac[SYS_ADPT_MAC_ADDR_LEN],
    BOOL_T              is_mc,
    void                **nh_hw_info_pp);

BOOL_T DEV_SWDRV_PMGR_DestroyVxlanNexthop(void *nh_hw_info);

BOOL_T DEV_SWDRV_PMGR_DeleteVxlanNexthop(void *nh_hw_info);

BOOL_T DEV_SWDRV_PMGR_AddVxlanEcmpNexthop(
    UI32_T                      vfi_id,
    UI32_T                      vxlan_port_id,
    void                        *nh_hw_info);

BOOL_T DEV_SWDRV_PMGR_RemoveVxlanEcmpNexthop(
    UI32_T                      vfi_id,
    UI32_T                      vxlan_port_id,
    void                        *nh_hw_info);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetVxlanPortLearning
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable MAC learning for VXLAN port.
 * INPUT   : vxlan_port_id - Which VXLAN logical port.
 *           is_learning -- TRUE to enable MAC learning.
 *                          FALSE to disable MAC learning.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetVxlanPortLearning(
    UI32_T vxlan_port_id,
    BOOL_T is_learning);

#endif /*#if (SYS_CPNT_VXLAN == TRUE)*/

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetHashSelBlock
 * -------------------------------------------------------------------------
 * FUNCTION: To set hash-selection block
 * INPUT   : block_info_p - block info
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetHashSelBlock(DEV_SWDRV_HashSelBlockInfo_T *block_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_UnBindHashSelForECMP
 * -------------------------------------------------------------------------
 * FUNCTION: Unbind hash-selection for ECMP
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_UnBindHashSelForECMP();

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetHashSelectionForECMP
 * -------------------------------------------------------------------------
 * FUNCTION: To set hash-selection block index for ECMP
 * INPUT   : hw_block_index - the index of hash-selection block for ECMP
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetHashSelectionForECMP(UI8_T hw_block_index);
#endif
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetPortStatusForLicense
 * -------------------------------------------------------------------------
 * FUNCTION: To set set port administration status
 * INPUT   : unit
 *           port
 *           status  - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetPortStatusForLicense(UI32_T unit, UI32_T port, BOOL_T status);

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetSwitchingMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set switching mode
 * INPUT   : unit     - unit to set
 *           port     - port to set
 *           mode     - VAL_swctrlSwitchModeSF
 *                      VAL_swctrlSwitchModeCT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetSwitchingMode(
    UI32_T  unit,
    UI32_T  port,
    UI32_T mode);
#endif /*#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)*/

#if (SYS_CPNT_SWCTRL_FEC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetPortFec
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable FEC
 * INPUT   : unit
 *           port
 *           fec_mode - VAL_portFecMode_XXX
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetPortFec(UI32_T unit, UI32_T port, UI32_T fec_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_GetPortFecStatus
 * -------------------------------------------------------------------------
 * FUNCTION: To get FEC status
 * INPUT   : unit
 *           port
 * OUTPUT  : fec_mode_p - VAL_portFecMode_XXX
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_GetPortFecStatus(UI32_T unit, UI32_T port, UI32_T *fec_mode_p);
#endif

#if(SYS_CPNT_WRED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DEV_SWDRV_PMGR_RandomDetect
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set port ecn marking percentage
 * INPUT    :   unit       - which unit
 *              port       - which port
 *              value      - what percantage
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   percentage =0 means disable
 * ------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRV_PMGR_RandomDetect(UI32_T unit, UI32_T port, DEV_SWDRV_RandomDetect_T value);
#endif

#endif /*#ifndef _DEV_SWDRV_PMGR_H_*/

