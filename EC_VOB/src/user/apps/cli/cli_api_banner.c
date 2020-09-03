#include "cli_def.h"
#include "cli_api.h"
#include "cli_lib.h"
#include "cli_io.h"

#if (SYS_CPNT_CLI_BANNER == TRUE)
#include "cli_banner.h"
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

UI32_T CLI_API_Banner_Configure(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)
    char string_ar[BANNER_MAX_NOTE_DATA_LENGTH+1]={0};

    {
        CLI_LIB_PrintStr("\r\nCompany: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);

        if(FALSE == CLI_BANNER_SetCompany(string_ar))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set company.\r\n");
#endif
        }
    }

    {
        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nResponsible department: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);

        if(FALSE == CLI_BANNER_SetDepartment(string_ar))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set department.\r\n");
#endif
        }
    }

    {
        BannerManagerInfo_T manager_info;

        memset(string_ar, 0, sizeof(string_ar));

        CLI_LIB_PrintStr("\r\nName and telephone to Contact the management people");
        CLI_LIB_PrintStr("\r\nManager1 name: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(manager_info.name_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\n phone number: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(manager_info.phone_number_ar, string_ar);

        if(FALSE == CLI_BANNER_SetManagerInfo(1, &manager_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
        }

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nManager2 name: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(manager_info.name_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\n phone number: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(manager_info.phone_number_ar, string_ar);

        if(FALSE == CLI_BANNER_SetManagerInfo(2, &manager_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
        }

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nManager3 name: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(manager_info.name_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\n phone number: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(manager_info.phone_number_ar, string_ar);

        if(FALSE == CLI_BANNER_SetManagerInfo(3, &manager_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
        }
    }

    {
        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nThe physical location of the equipment.");
        CLI_LIB_PrintStr("\r\nCity and street address: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);

        if(FALSE == CLI_BANNER_SetEquipmentLocation(string_ar))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set equipment location.\r\n");
#endif
        }
    }

    {
        BannerEquipmentInfo_T equipment_info;

        memset(string_ar, 0, sizeof(string_ar));

        CLI_LIB_PrintStr("\r\nInformation about this equipment:");
        CLI_LIB_PrintStr("\r\nManufacturer: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(equipment_info.manufacturer_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nID: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(equipment_info.manufacturer_id_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nFloor: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(equipment_info.floor_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nRow: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(equipment_info.row_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nRack: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(equipment_info.rack_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nShelf in this rack: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(equipment_info.self_rack_ar, string_ar);

        if(FALSE == CLI_BANNER_SetEquipmentInfo(&equipment_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set equipment information.\r\n");
#endif
        }
    }

    {
        BannerDCPowerInfo_T dc_power_info;

        memset(string_ar, 0, sizeof(string_ar));

        CLI_LIB_PrintStr("\r\nInformation about DC power supply.");
        CLI_LIB_PrintStr("\r\nFloor: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(dc_power_info.floor_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nRow: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(dc_power_info.row_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nRack: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(dc_power_info.rack_ar, string_ar);

        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nElectrical circuit: : ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        strcpy(dc_power_info.electrical_circuit_ar, string_ar);

        if(FALSE == CLI_BANNER_SetDCPowerInfo(&dc_power_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set DC power information.\r\n");
#endif
        }
    }

    {
        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nNumber of LP: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);

        if(FALSE == CLI_BANNER_SetLPNumber(string_ar))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set LP number.\r\n");
#endif
        }
    }

    {
        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nPosition of the equipment in the MUX: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);

        if(FALSE == CLI_BANNER_SetMUX(string_ar))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set MUX.\r\n");
#endif
        }
    }

    {
        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nIP LAN: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);

        if(FALSE == CLI_BANNER_SetIpLan(string_ar))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set IP LAN.\r\n");
#endif
        }
    }

    {
        memset(string_ar, 0, sizeof(string_ar));
        CLI_LIB_PrintStr("\r\nNote: ");
        CLI_PARS_ReadLine( string_ar, BANNER_MAX_NOTE_DATA_LENGTH, FALSE, FALSE);
        CLI_LIB_PrintNullStr(1);
        CLI_LIB_PrintStr("\r\n");

        if(FALSE == CLI_BANNER_SetNote(string_ar))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set note.\r\n");
#endif
        }
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Company(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    char company_ar[BANNER_MAX_DATA_LENGTH+1] = {0};

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_COMPANY:
           strcpy(company_ar, arg[0]);
           break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_COMPANY:
           break;
        default:
           return CLI_ERR_INTERNAL;
    }

    if(FALSE == CLI_BANNER_SetCompany(company_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set company.\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Department(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)
    char department_ar[BANNER_MAX_DATA_LENGTH+1] = {0};

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_DEPARTMENT:
           strcpy(department_ar, arg[0]);
           break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_DEPARTMENT:
           break;
        default:
           return CLI_ERR_INTERNAL;
    }

    if(FALSE == CLI_BANNER_SetDepartment(department_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set department.\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Manager_Info(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    BannerManagerInfo_T manager_info;

    memset(&manager_info, 0, sizeof(BannerManagerInfo_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_MANAGERINFO:
        {
            strcpy(manager_info.name_ar, arg[1]);
            strcpy(manager_info.phone_number_ar, arg[3]);

            if(FALSE == CLI_BANNER_SetManagerInfo(1, &manager_info))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
            }
            if(arg[4]!=NULL)
            {
                strcpy(manager_info.name_ar, arg[5]);
                strcpy(manager_info.phone_number_ar, arg[7]);

                if(FALSE == CLI_BANNER_SetManagerInfo(2, &manager_info))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                }
            }

            if(arg[8]!=NULL)
            {
                strcpy(manager_info.name_ar, arg[9]);
                strcpy(manager_info.phone_number_ar, arg[11]);

                if(FALSE == CLI_BANNER_SetManagerInfo(3, &manager_info))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                }
            }
        }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_MANAGERINFO:
        {
            UI32_T i=0;

            if(arg[0]!=NULL)
            {
                if(arg[0][4]=='1')
                {
                    if(arg[1]!=NULL)
                    {
                        if(arg[1][4]=='2')
                        {
                            if(arg[2]!=NULL)
                            {
                                for(i=1;i<4;i++)
                                {
                                    if(FALSE == CLI_BANNER_SetManagerInfo(i, &manager_info))
                                    {
#if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
#else
                                        CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                                    }
                                }
                            }
                            else
                            {
                                for(i=1;i<3;i++)
                                {
                                    if(FALSE == CLI_BANNER_SetManagerInfo(i, &manager_info))
                                    {
#if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
#else
                                        CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                                    }
                                }
                            }
                        }
                        else if(arg[1][4]=='3')
                        {
                            if(FALSE == CLI_BANNER_SetManagerInfo(1, &manager_info))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                            }
                            if(FALSE == CLI_BANNER_SetManagerInfo(3, &manager_info))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                            }
                        }
                    }
                    else
                    {
                        if(FALSE == CLI_BANNER_SetManagerInfo(1, &manager_info))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                        }
                    }
                }
                else if(arg[0][4]=='2')
                {
                    if(arg[1]!=NULL)
                    {
                        for(i=2;i<4;i++)
                        {
                            if(FALSE == CLI_BANNER_SetManagerInfo(i, &manager_info))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                            }
                        }
                    }
                    else
                    {
                        if(FALSE == CLI_BANNER_SetManagerInfo(2, &manager_info))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                        }
                    }
                }
                else if(arg[0][4]=='3')
                {
                    if(FALSE == CLI_BANNER_SetManagerInfo(3, &manager_info))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                    }
                }

            }
            else
            {
                for(i=1;i<4;i++)
                {
                    if(FALSE == CLI_BANNER_SetManagerInfo(i, &manager_info))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to set manager information.\r\n");
#endif
                    }
                }

            }
        }
            break;
        default:
           return CLI_ERR_INTERNAL;
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Equipment_Location(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    char equipment_location_ar[BANNER_MAX_DATA_LENGTH+1] = {0};

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_EQUIPMENTLOCATION:
           strcpy(equipment_location_ar ,arg[0]);
           break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_EQUIPMENTLOCATION:
           break;

        default:
           return CLI_ERR_INTERNAL;
    }

    if(!CLI_BANNER_SetEquipmentLocation(equipment_location_ar))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set equipment location.\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Equipment_Info(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    BannerEquipmentInfo_T equipment_info;

    memset(&equipment_info, 0, sizeof(BannerEquipmentInfo_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_EQUIPMENTINFO:
           strcpy(equipment_info.manufacturer_id_ar, arg[1]);
           strcpy(equipment_info.floor_ar, arg[3]);
           strcpy(equipment_info.row_ar, arg[5]);
           strcpy(equipment_info.rack_ar, arg[7]);
           strcpy(equipment_info.self_rack_ar, arg[9]);
           strcpy(equipment_info.manufacturer_ar, arg[11]);
           break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_EQUIPMENTINFO:
        {
            /* arg[0] be NULL means clear all fileds of entry
             * use memset(&equipment_info, 0, sizeof(BannerEquipmentInfo_T)); to set
             */

            if(NULL != arg[0])
            {
                UI32_T pos=0;

                if(!CLI_BANNER_GetEquipmentInfo(&equipment_info))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to get DCPowerInfo\r\n");
#endif
                }

                while(arg[pos]!=NULL)
                {
                    if(arg[pos][0]=='m'||arg[pos][0]=='M')
                    {
                        if(arg[pos][12]=='-')
                        {
                            equipment_info.manufacturer_id_ar[0] = '\0';
                        }
                        else
                        {
                            equipment_info.manufacturer_ar[0] = '\0';
                        }
                    }
                    else if(arg[pos][0]=='f'||arg[pos][0]=='F')
                    {
                        equipment_info.floor_ar[0] = '\0';
                    }
                    else if(arg[pos][0]=='r'||arg[pos][0]=='R')
                    {
                        if(arg[pos][1]=='o'||arg[pos][1]=='O')
                        {
                            equipment_info.row_ar[0] = '\0';
                        }
                        else /* if(arg[pos][1]=='a'||arg[pos][1]=='A') */
                        {
                            equipment_info.rack_ar[0] = '\0';
                        }
                    }
                    else /* if(arg[pos][0]=='s'||arg[pos][0]=='S') */
                    {
                        equipment_info.self_rack_ar[0] = '\0';
                    }
                    pos++;
                }/*end while*/

            } /* if(NULL != arg[pos]) */

        }
        break;

    default:
       return CLI_ERR_INTERNAL;
    }

    if(!CLI_BANNER_SetEquipmentInfo(&equipment_info))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set equipmentInfo\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Dc_Power_Info(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    BannerDCPowerInfo_T dc_power_info;

    memset(&dc_power_info, 0, sizeof(BannerDCPowerInfo_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_DCPOWERINFO:
           strcpy(dc_power_info.floor_ar, arg[1]);
           strcpy(dc_power_info.row_ar, arg[3]);
           strcpy(dc_power_info.rack_ar, arg[5]);
           strcpy(dc_power_info.electrical_circuit_ar, arg[7]);
           break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_DCPOWERINFO:
        {
            /* arg[0] be NULL means clear all fileds of entry
             * use memset(&dc_power_info, 0, sizeof(BannerDCPowerInfo_T)); to set
             */

            if(NULL != arg[0])
            {
                UI32_T pos=0;

                if(!CLI_BANNER_GetDCPowerInfo(&dc_power_info))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to get DCPowerInfo\r\n");
#endif
                }

                while(arg[pos]!=NULL)
                {
                    if(arg[pos][0]=='f'||arg[pos][0]=='F')
                    {
                        dc_power_info.floor_ar[0] = '\0';
                    }
                    else if(arg[pos][0]=='r'||arg[pos][0]=='R')
                    {
                        if(arg[pos][1]=='o'||arg[pos][1]=='O')
                        {
                            dc_power_info.row_ar[0] = '\0';
                        }
                        else /* if(arg[pos][1]=='a'||arg[pos][1]=='A') */
                        {
                            dc_power_info.rack_ar[0] = '\0';
                        }
                    }
                    else /* if(arg[pos][0]=='e'||arg[pos][0]=='E') */
                    {
                        dc_power_info.electrical_circuit_ar[0] = '\0';
                    }
                    pos++;
                }/*end-while*/
            } /* if(NULL != arg[pos]) */
        }
        break;

    default:
       return CLI_ERR_INTERNAL;
    }

    if(!CLI_BANNER_SetDCPowerInfo(&dc_power_info))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set DCPowerInfo\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Ip_Lan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    char ip_lan_ar[BANNER_MAX_DATA_LENGTH+1] = {0};

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_IPLAN:
           strcpy(ip_lan_ar, arg[0]);
           break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_IPLAN:
           break;

        default:
           return CLI_ERR_INTERNAL;
    }

    if(!CLI_BANNER_SetIpLan(ip_lan_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set IP LAN.\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Lp_Number(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    char lp_number_ar[BANNER_MAX_DATA_LENGTH+1] = {0};

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_LPNUMBER:
           strcpy(lp_number_ar, arg[0]);
           break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_LPNUMBER:
           break;

        default:
           return CLI_ERR_INTERNAL;
    }

    if(!CLI_BANNER_SetLPNumber(lp_number_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set LP number.\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Mux(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    char  mux_ar[BANNER_MAX_DATA_LENGTH+1] = {0};

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_MUX:
           strcpy(mux_ar, arg[0]);
           break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_MUX:
           break;

        default:
           return CLI_ERR_INTERNAL;
    }

    if(!CLI_BANNER_SetMUX(mux_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set MUX.\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Configure_Note(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    char note_ar[BANNER_MAX_NOTE_DATA_LENGTH+1] = {0};

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_BANNER_CONFIGURE_NOTE:
           strcpy(note_ar, arg[0]);
           break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_BANNER_CONFIGURE_NOTE:
           break;

        default:
           return CLI_ERR_INTERNAL;
    }

    if(!CLI_BANNER_SetNote(note_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set Note\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Banner(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_BANNER == TRUE)

    UI32_T line_num=0;
    char string_ar[BANNER_MAX_NOTE_DATA_LENGTH+1] = {0};
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if(!CLI_BANNER_GetCompany(string_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get Company\r\n");
#endif
    }
    PROCESS_MORE(string_ar);

    PROCESS_MORE("\r\nWARNING - MONITORED ACTIONS AND ACCESSES\r\n");

    if(!CLI_BANNER_GetDepartment(string_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get Department\r\n");
#endif
    }

    sprintf(buff, "%s\r\n", string_ar);
    PROCESS_MORE(buff);
    PROCESS_MORE("\r\n");

     {
         UI32_T i=0;
         BannerManagerInfo_T manager_info;
         memset(&manager_info, 0, sizeof(BannerManagerInfo_T));

         while(i<3)
         {
             i++;
            if(!CLI_BANNER_GetManagerInfo(i, &manager_info))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to get manager information.\r\n");
#endif
            }

            if(strlen(manager_info.name_ar)==0)
            {
                continue;
            }

            sprintf(buff, "%s - %s\r\n",manager_info.name_ar,manager_info.phone_number_ar);
            PROCESS_MORE(buff);

        }
    }

    PROCESS_MORE("\r\nStation's information:\r\n");

    if(!CLI_BANNER_GetEquipmentLocation(string_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get Equipment Location\r\n");
#endif
    }
    sprintf(buff, "%s\r\n\r\n",string_ar);
    PROCESS_MORE(buff);

    {
        BannerEquipmentInfo_T equipment_info;
        memset(&equipment_info, 0, sizeof(BannerEquipmentInfo_T));

        if(!CLI_BANNER_GetEquipmentInfo(&equipment_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to get EquipmentInfo\r\n");
#endif
        }
        if(strlen(equipment_info.manufacturer_ar)>0 || strlen(equipment_info.manufacturer_id_ar)>0)
        {
            sprintf(buff, " %s- %s\r\n",equipment_info.manufacturer_ar,equipment_info.manufacturer_id_ar);
            PROCESS_MORE(buff);
        }

        PROCESS_MORE("Floor / Row / Rack / Sub-Rack\r\n");
        sprintf(buff, " %s/ %s / %s / %s\r\n",equipment_info.floor_ar,equipment_info.row_ar,equipment_info.rack_ar,equipment_info.self_rack_ar);
        PROCESS_MORE(buff);
    }

    {
        BannerDCPowerInfo_T dc_power_info;
        memset(&dc_power_info, 0, sizeof(BannerDCPowerInfo_T));

        if(!CLI_BANNER_GetDCPowerInfo(&dc_power_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to get EquipmentInfo\r\n");
#endif
        }
        PROCESS_MORE("DC power supply:\r\n");
        PROCESS_MORE("Power Source A: Floor / Row / Rack / Electrical circuit\r\n");
        sprintf(buff, " %s/ %s / %s / %s\r\n",dc_power_info.floor_ar,dc_power_info.row_ar,dc_power_info.rack_ar,dc_power_info.electrical_circuit_ar);
        PROCESS_MORE(buff);

    }

    if(!CLI_BANNER_GetLPNumber(string_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get LPNumber\r\n");
#endif
    }
    sprintf(buff, "Number of LP: %s\r\n",string_ar);
    PROCESS_MORE(buff);

    if(!CLI_BANNER_GetMUX(string_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get MUX\r\n");
#endif
    }
    sprintf(buff, "Position MUX: %s\r\n",string_ar);
    PROCESS_MORE(buff);

    if(!CLI_BANNER_GetIpLan(string_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get IpLan\r\n");
#endif
    }
    sprintf(buff, "IP LAN: %s\r\n",string_ar);
    PROCESS_MORE(buff);

    if(!CLI_BANNER_GetNote(string_ar))
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get Note\r\n");
#endif
    }
    sprintf(buff, "Note: %s\r\n",string_ar);
    PROCESS_MORE(buff);

#endif  /* #if (SYS_CPNT_CLI_BANNER == TRUE) */

    return CLI_NO_ERROR;
}
