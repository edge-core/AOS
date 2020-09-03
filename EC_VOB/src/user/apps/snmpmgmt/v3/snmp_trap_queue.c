#include <string.h>

#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "snmp_mgr.h"

/******************************************************************
 *
 * All functions with prefix SNMP_MGR_TRAP_QUEUE_BUF_* MUST be called
 * after buffer is protected by get the trap queue semaphore
 *
 ******************************************************************/
static void   SNMP_MGR_ShmTrapQueueBufInit(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *buf);
static void   SNMP_MGR_ShmTrapQueueBufReset(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *buf);
static UI32_T SNMP_MGR_GetShmTrapQueueBufDataSize(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *buf);
static int    SNMP_MGR_ShmTrapQueueBufWrite(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *shmbuf, UI8_T *buf,
                                             int size, int flag);
static int    SNMP_MGR_ShmTrapQueueBufRead(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *shmbuf, UI32_T offset,
                                             UI8_T *buf, int size, int flag);

static int                        shm_trapq_dbg_write = 0;
static int                        shm_trapq_dbg_read  = 0;

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_GetDynamicDataSize
 * ---------------------------------------------------------------------
 *  FUNCTION: Get the size of the dynamic data block
 *
 *  INPUT    : trap_type
 *  OUTPUT   : NONE.
 *  RETURN   : data size
 *  NOTE     :
 * ---------------------------------------------------------------------
 */
UI32_T
SNMP_MGR_GetDynamicDataSize(
    UI32_T trap_type)
{
    switch (trap_type)
    {
        /* five of the Six Early Traps (no enterprise)
         */
        case TRAP_EVENT_LINK_STATE:
            return sizeof (TRAP_EVENT_LinkStateTrap_T);
            break;
        case TRAP_EVENT_LINK_DOWN:
        case TRAP_EVENT_LINK_UP:
            return sizeof(TRAP_EVENT_LinkTrap_T);
            break;

        case TRAP_EVENT_CRAFT_PORT_LINK_DOWN:
        case TRAP_EVENT_CRAFT_PORT_LINK_UP:
            return sizeof(TRAP_EVENT_CraftLinkTrap_T);
            break;

        /* Spanning Tree Protocol MIB
         */
        case TRAP_EVENT_TOPOLOGY_CHANGE:
            return sizeof(UI32_T);

        case TRAP_EVENT_TOPOLOGY_CHANGE_RECEIVE:
            return sizeof(TRAP_EVENT_XstpRxTC_T);

        /* Accton private MIB
         */
        case TRAP_EVENT_SW_POWER_STATUS_CHANGE_TRAP:
            return sizeof(TRAP_EVENT_PowerStatusChangeTrap_T);
            break;

        case TRAP_EVENT_SW_ACTIVE_POWER_CHANGE_TRAP:
            return sizeof(TRAP_EVENT_ActivePowerChangeTrap_T);
            break;

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
        case TRAP_EVENT_SFP_RX_POWER_HIGH_ALARM:
        case TRAP_EVENT_SFP_RX_POWER_LOW_ALARM:
        case TRAP_EVENT_SFP_RX_POWER_HIGH_WARNING:
        case TRAP_EVENT_SFP_RX_POWER_LOW_WARNING:
        case TRAP_EVENT_SFP_TX_POWER_HIGH_ALARM:
        case TRAP_EVENT_SFP_TX_POWER_LOW_ALARM:
        case TRAP_EVENT_SFP_TX_POWER_HIGH_WARNING:
        case TRAP_EVENT_SFP_TX_POWER_LOW_WARNING:
        case TRAP_EVENT_SFP_TEMP_HIGH_ALARM:
        case TRAP_EVENT_SFP_TEMP_LOW_ALARM:
        case TRAP_EVENT_SFP_TEMP_HIGH_WARNING:
        case TRAP_EVENT_SFP_TEMP_LOW_WARNING:
        case TRAP_EVENT_SFP_VOLTAGE_HIGH_ALARM:
        case TRAP_EVENT_SFP_VOLTAGE_LOW_ALARM:
        case TRAP_EVENT_SFP_VOLTAGE_HIGH_WARNING:
        case TRAP_EVENT_SFP_VOLTAGE_LOW_WARNING:
        case TRAP_EVENT_SFP_CURRENT_HIGH_ALARM:
        case TRAP_EVENT_SFP_CURRENT_LOW_ALARM:
        case TRAP_EVENT_SFP_CURRENT_HIGH_WARNING:
        case TRAP_EVENT_SFP_CURRENT_LOW_WARNING:
        case TRAP_EVENT_SFP_TEMP_ALARMWARN_CEASE:
        case TRAP_EVENT_SFP_VOLTAGE_ALARMWARN_CEASE:
        case TRAP_EVENT_SFP_CURRENT_ALARMWARN_CEASE:
        case TRAP_EVENT_SFP_TX_POWER_ALARMWARN_CEASE:
        case TRAP_EVENT_SFP_RX_POWER_ALARMWARN_CEASE:
            return sizeof(TRAP_EVENT_SfpThresholdAlarmWarnTrap_T);
            break;
#endif

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
        case TRAP_EVENT_SW_ALARM_INPUT:
            return sizeof(TRAP_EVENT_alarmMgt_T);
            break;
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
        case TRAP_EVENT_MAJOR_ALARM:
        case TRAP_EVENT_MINOR_ALARM:
            return sizeof(TRAP_EVENT_alarmMgt_T);
            break;

        case TRAP_EVENT_MAJOR_ALARM_RECOVERY:
            return sizeof(TRAP_EVENT_MajorAlarmRecovery_T);
            break;
#endif

        /* RMON MIB
         */
        case TRAP_EVENT_RISING_ALARM:
        case TRAP_EVENT_FALLING_ALARM:
            return sizeof(TRAP_EVENT_RisingFallingAlarmTrap_T);
            break;

        /* Accton private MIB
         */
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
        case TRAP_EVENT_PORT_SECURITY_TRAP:
            return sizeof(TRAP_EVENT_PortSecurityTrap_T);
            break;
#endif /* #if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE) */

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        case TRAP_EVENT_LOOPBACK_TEST_FAILURE_TRAP:
            return sizeof(TRAP_EVENT_LoopBackFailureTrap_T);
            break;
#endif /* #if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE) */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case TRAP_EVENT_FAN_FAILURE:
        case TRAP_EVENT_FAN_RECOVER:
            return sizeof(TRAP_EVENT_FanTrap_T);
#endif /* #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case TRAP_EVENT_THERMAL_RISING:
            return sizeof(TRAP_EVENT_thermalRising_T);
            break;

        case TRAP_EVENT_THERMAL_FALLING:
            return sizeof(TRAP_EVENT_thermalFalling_T);
            break;
#endif /* #if (SYS_CPNT_THERMAL_DETECT == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
        case TRAP_EVENT_IPFILTER_REJECT_TRAP:
            return sizeof(TRAP_EVENT_IpFilterRejectTrap_T);
            break;

        case TRAP_EVENT_IPFILTER_INET_REJECT_TRAP:
            return sizeof(TRAP_EVENT_IpFilterInetRejectTrap_T);
            break;
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

#if (SYS_CPNT_SMTP == TRUE)
        case TRAP_EVENT_SMTP_CONN_FAILURE_TRAP:
            return sizeof(TRAP_EVENT_swSmtpConnFailureTrap_T);
            break;
#endif /* #if (SYS_CPNT_SMTP == TRUE) */

#if (SYS_CPNT_POE == TRUE)
        case TRAP_EVENT_PETH_PSE_PORT_ON_OFF_TRAP:
            return sizeof(TRAP_EVENT_PethPsePortOnOffTrap_T);
            break;

        case TRAP_EVENT_PETH_MAIN_POWER_USAGE_ON_TRAP:
            return sizeof(TRAP_EVENT_PethMainPowerUsageOnTrap_T);
            break;

        case TRAP_EVENT_PETH_MAIN_POWER_USAGE_OFF_TRAP:
            return sizeof(TRAP_EVENT_PethMainPowerUsageOffTrap_T);
            break;

#endif  /* #if (SYS_CPNT_POE == TRUE) */

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
        /* 3Com private MIB
         */
        case TRAP_EVENT_SECURE_ADDRESS_LEARNED:
            return sizeof(TRAP_EVENT_secureAddressLearned_T);
            break;

        case TRAP_EVENT_SECURE_VIOLATION2:
            return sizeof(TRAP_EVENT_secureViolation2_T);
            break;

        case TRAP_EVENT_SECURE_LOGIN_FAILURE:
            return sizeof(TRAP_EVENT_secureLoginFailure_T);
            break;

        case TRAP_EVENT_SECURE_LOGON:
            return sizeof(TRAP_EVENT_secureLogon_T);
            break;

        case TRAP_EVENT_SECURE_LOGOFF:
            return sizeof(TRAP_EVENT_secureLogoff_T);
            break;
#endif  /* (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32) */

        /* Accton private MIB
         */
        case TRAP_EVENT_TCN:
            return sizeof(TRAP_EVENT_tcn_T);
            break;

        case TRAP_EVENT_LLDP_REM_TABLES_CHANGED:
            return sizeof(TRAP_EVENT_lldpRemTablesChange_T);
            break;

#if (SYS_CPNT_EFM_OAM == TRUE)
        /* OAM MIB
         */
        case TRAP_EVENT_DOT3_OAM_THRESHOLD:
            return sizeof(TRAP_EVENT_dot3OamThreshold_T);
            break;

        case TRAP_EVENT_DOT3_OAM_NON_THRESHOLD:
            return sizeof(TRAP_EVENT_dot3OamNonThreshold_T);
            break;
#endif /* #if (SYS_CPNT_EFM_OAM == TRUE) */

        /* Accton private MIB
         */
#if (SYS_CPNT_ATC_STORM == TRUE)
        case TRAP_EVENT_BCAST_STORM_ALARM_FIRE:
            return sizeof(TRAP_EVENT_BcastStormAlarmFire_T);
            break;

        case TRAP_EVENT_BCAST_STORM_ALARM_CLEAR:
            return sizeof(TRAP_EVENT_BcastStormAlarmClear_T);
            break;

        case TRAP_EVENT_BCAST_STORM_TC_APPLY:
            return sizeof(TRAP_EVENT_BcastStormTcApply_T);
            break;

        case TRAP_EVENT_BCAST_STORM_TC_RELEASE:
            return sizeof(TRAP_EVENT_BcastStormTcRelease_T);
            break;

        case TRAP_EVENT_MCAST_STORM_ALARM_FIRE:
            return sizeof(TRAP_EVENT_McastStormAlarmFire_T);
            break;

        case TRAP_EVENT_MCAST_STORM_ALARM_CLEAR:
            return sizeof(TRAP_EVENT_McastStormAlarmClear_T);
            break;

        case TRAP_EVENT_MCAST_STORM_TC_APPLY:
            return sizeof(TRAP_EVENT_McastStormTcApply_T);
            break;

        case TRAP_EVENT_MCAST_STORM_TC_RELEASE:
            return sizeof(TRAP_EVENT_McastStormTcRelease_T);
            break;
#endif /* #if (SYS_CPNT_ATC_STORM == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
        case TRAP_EVENT_STP_BPDU_GUARD_PORT_SHUTDOWN_TRAP:
            return sizeof(TRAP_EVENT_stpBpduGuardPortShutdownTrap_T);
            break;
#endif

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
        case TRAP_EVENT_NETWORK_ACCESS_PORT_LINK_DETECTION_TRAP:  /* 96 */
            return sizeof(TRAP_EVENT_networkAccessPortLinkDetectionTrap_T);
            break;
#endif  /* #if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

#if (SYS_CPNT_CFM == TRUE)
        case TRAP_EVENT_CFM_MEP_UP:  /* 97 */
            return sizeof(TRAP_EVENT_CfmMepUp_T);
            break;

        case TRAP_EVENT_CFM_MEP_DOWN:
            return sizeof(TRAP_EVENT_CfmMepDown_T);
            break;

        case TRAP_EVENT_CFM_CONFIG_FAIL:
            return sizeof(TRAP_EVENT_CfmConfigFail_T);
            break;

        case TRAP_EVENT_CFM_LOOP_FIND:
            return sizeof(TRAP_EVENT_CfmLoopFind_T);
            break;

        case TRAP_EVENT_CFM_MEP_UNKNOWN:
            return sizeof(TRAP_EVENT_CfmMepUnknown_T);
            break;

        case TRAP_EVENT_CFM_MEP_MISSING:
            return sizeof(TRAP_EVENT_CfmMepMissing_T);
            break;

        case TRAP_EVENT_CFM_MA_UP:  /* 103 */
            return sizeof(TRAP_EVENT_CfmMaUp_T);
            break;
#endif  /* (SYS_CPNT_CFM == TRUE) */

#if (SYS_CPNT_CFM == TRUE)
        /* CFM MIB
         */
        case TRAP_EVENT_CFM_FAULT_ALARM:
            return sizeof(TRAP_EVENT_CfmFaultAlarm_T);
            break;
#endif  /* #if (SYS_CPNT_CFM == TRUE) */

#if (SYS_CPNT_XFER_AUTO_UPGRADE==TRUE)
        /* autoUpgradeTrap(104)
         */
        case TRAP_EVENT_XFER_AUTO_UPGRADE:
            return sizeof(TRAP_EVENT_xferAutoUpgrade_T);
            break;
#endif

/* PENDING: The following 08 code will be generally used (no "#if"-check)
 * in ES4552MA-HPoE-7LF-LN-01322 and ASF4612MMS-FLF-08-00752.
 * At the same time, ID's 139 and 140, and their associated structures,
 * will be removed.
 */
#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_08)
        /* swAuthenticationFailureWithTimeAndInetAddress(105)
         * swAuthenticationSuccessWithTimeAndInetAddress(106)
         */
        case TRAP_EVENT_AUTH_FAILURE:
            return sizeof(TRAP_EVENT_swAuth_T);
            break;

        case TRAP_EVENT_AUTH_SUCCESS:
            return sizeof(TRAP_EVENT_swAuth_T);
            break;
#endif  /* (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_08) */

        /* dhcpRogueServerAttackTrap(114)
         * DHCP client sends a trap when receiving a packet from a rogue server.
         */
        case TRAP_EVENT_DHCP_ROGUE_SERVER_ATTACK:
            return sizeof(TRAP_EVENT_DhcpRogueServerAttack_T);
            break;

#if (SYS_CPNT_LLDP_MED == TRUE)
        /* LLDP-MED MIB
         */
        case TRAP_EVENT_LLDP_MED_TOPOLOGY_CHANGE_DETECTED:
            return sizeof(TRAP_EVENT_lldpMedTopologyChange_T);
            break;
#endif

        /* log, but no trap
         */
        case TRAP_EVENT_XSTP_PORT_STATE_CHANGE:
            return sizeof(TRAP_EVENT_XstpPortStateChange_T);
            break;

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
        case TRAP_EVENT_MAC_NOTIFY:
            return sizeof(TRAP_EVENT_MacNotify_T);
            break;
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */
        /* Accton private MIB
         */
#if (SYS_CPNT_BGP == TRUE)
        case TRAP_EVENT_BGP_ESTABLISHED_NOTIFICATION:
            return sizeof(TRAP_EVENT_BgpEstablishedNotificationTrap_T);
        case TRAP_EVENT_BGP_BACKWARD_TRANS_NOTIFICATION:
            return sizeof(TRAP_EVENT_BgpBackwardTransNotificationTrap_T);
#endif

        case TRAP_EVENT_XSTP_ROOT_BRIDGE_CHANGED:
            return sizeof(TRAP_EVENT_XstpRootBridgeChanged_T);
            break;
#if(SYS_CPNT_SYNCE == TRUE)
        case TRAP_EVENT_SYNCE_SSM_RX:
            return sizeof(TRAP_EVENT_SyncESsmRx);
            break;
        case TRAP_EVENT_SYNCE_CLOCK_SRC:
            return sizeof(TRAP_EVENT_SyncEClkSrc);
            break;
#endif
#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
        case TRAP_EVENT_SW_ALARM_INPUT_RECOVER:
            return sizeof(TRAP_EVENT_alarmMgt_T);
            break;
#endif

        case TRAP_EVENT_USERAUTH_AUTHENTICATION_FAILURE:
        case TRAP_EVENT_USERAUTH_AUTHENTICATION_SUCCESS:
        case TRAP_EVENT_LOGIN:
        case TRAP_EVENT_LOGOUT:
            return sizeof(TRAP_EVENT_UserInfo_T);
            break;

        case TRAP_EVENT_FILE_COPY:
            return (sizeof(TRAP_EVENT_XferFileCopy_T));
            break;

        case TRAP_EVENT_USERAUTH_CREATE_USER:
        case TRAP_EVENT_USERAUTH_DELETE_USER:
        case TRAP_EVENT_USERAUTH_MODIFY_USER_PRIVILEGE:
            return (sizeof(TRAP_EVENT_UserauthAccount_T));
            break;

        default:
            return 0;
            break;
    }
    return 0;
}

UI32_T
SNMP_MGR_GetTrapDataSize(TRAP_EVENT_TrapData_T *trap)
{
    UI32_T  size = 0;

    if (!trap)
        return 0;

    size = SNMP_MGR_GetDynamicDataSize(trap->trap_type);

    switch (trap->trap_type) {
        case TRAP_EVENT_LINK_STATE:
            if (IF_BITMAP_IS_MAPPED(trap->u.link_state.if_bitmap.if_count))
                size += IF_BITMAP_ARRAY_SIZE(trap->u.link_state.if_bitmap.if_count);
            break;
        default:
            break;
    }

    return size;
}


/* Initialize trap queue buffer by clear all header fields
 * to zero and set magic word
 */
static void
SNMP_MGR_ShmTrapQueueBufInit(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *buf)
{
    if (!buf || (buf->magic == SNMP_MGR_SHM_TRAP_QUEUE_BUF_MAGIC))
        return;

    memset(buf, 0, sizeof (*buf));
    buf->magic = SNMP_MGR_SHM_TRAP_QUEUE_BUF_MAGIC;
    buf->size  = SNMP_MGR_SHM_TRAP_QUEUE_BUF_SIZE;
}

/*
 * Calculate and return real data size current in buffer.
 */
static UI32_T
SNMP_MGR_GetShmTrapQueueBufDataSize(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *buf)
{
    UI32_T      wr_off, rd_off;
    UI32_T      size = 0;

    if (!SNMP_MGR_SHM_TRAP_QUEUE_BUF_IS_VALID(buf))
        return 0;

    wr_off = buf->wr_off;
    rd_off = buf->rd_off;

    if (wr_off > rd_off) {
        /*
         *   |__________|~~~~~~~~~|__________|
         * base      rd_off     wr_off        limit
         */
        size = (UI32_T)(wr_off - rd_off);
    } else if (wr_off < rd_off) {
        /*
         *   |~~~~~~~~~~|_________|~~~~~~~~~~|
         * base      wr_off    rd_off         limit
         */
        size = (UI32_T)(buf->size - rd_off + wr_off);
    } else if (buf->flags & SNMP_MGR_SHM_TRAP_QUEUE_FULL) {
        /*
         *   |~~~~~~~~~~|~~~~~~~~~~~~~~~~~~~~|
         * base      wr_off == rd_off         limit
         */
        size = buf->size;
    }

    return size;
}

/* reset all trap queue buffer header fields to zero */
static void
SNMP_MGR_ShmTrapQueueBufReset(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *buf)
{
    if (!SNMP_MGR_SHM_TRAP_QUEUE_BUF_IS_VALID(buf))
        return;

    buf->entries = 0;
    // shmbuf->flags &= ~(SNMP_MGR_SHM_TRAP_QUEUE_FULL | SNMP_MGR_SHM_TRAP_QUEUE_ERROR);
    buf->flags   = 0;
    buf->errcnt  = 0;
    buf->wr_off  = 0;
    buf->rd_off  = 0;
    buf->mtime   = SYSFUN_GetSysTick();
}

/*
 * Write data into trap queue buffer
 * shmbuf   pointer of trap queue buffer in shared memory
 * buf      data to be written to shmbuf
 *          If 'buf' is specified as NULL, it means just update internal
 *          data writing position for next writing (or free some space
 *          for next writing), no any data is written into buffer 'shmbuf'
 * size     size of data in input buffer 'buf'
 * flag     flags to indicate how to handle exceptions while here is no
 *          enough free space for new writing data. Two flags are defined
 *          now:
 *          - SNMP_MGR_SHM_TRAP_QUEUE_FORCE_WRITE
 *            This flag indicate force writing data to 'shmbuf' even if
 *            the left free space can't store all new data ('size' bytes).
 *            This means the input data will be truncated if over-written
 *            is allowed.
 *
 *          - SNMP_MGR_SHM_TRAP_QUEUE_OVER_WRITE
 *            This flag indicate whether wipe oldest records to free more
 *            space to store new input data. The more space freed automatically
 *            may exceed the needed space due to the free is done by free one
 *            or more oldest records.
 *            This flag must use together with SNMP_MGR_SHM_TRAP_QUEUE_FORCE_WRITE.
 *
 * Return value:
 *  < 0       error occured
 *  >= 0      bytes write into shared memory buffer 'shmbuf'
 */
static int
SNMP_MGR_ShmTrapQueueBufWrite(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *shmbuf, UI8_T *buf, int size, int flag)
{
    UI32_T      space;

    if (!SNMP_MGR_SHM_TRAP_QUEUE_BUF_IS_VALID(shmbuf) || (size < 0))
        return -1;

    if (!size)
        return 0;

    space = shmbuf->size - SNMP_MGR_GetShmTrapQueueBufDataSize(shmbuf);
    if (space < (UI32_T)size) {
        if (!(flag & SNMP_MGR_SHM_TRAP_QUEUE_OVER_WRITE)) {
            if (!space) {       /* no space to write            */
                ++shmbuf->errcnt;
                return -1;
            }
            size = (int)space;  /* truncate data to force write */
            goto write_data;
        }

        /* need to overwrite exist entries if no enough space */
        if (shmbuf->size < (UI32_T)size) {
            ++shmbuf->errcnt;
            return -1;          /* no enough space even if overwrite */
        }

        if (shmbuf->size == (UI32_T)size) {
            SNMP_MGR_ShmTrapQueueBufReset(shmbuf);
            space = shmbuf->size;
            goto write_data;
        }

        /* overwrite exist entries, discard oldest
         * entries to get more spaces
         */
        while ((space < (UI32_T)size) && shmbuf->entries) {
            UI16_T  nbytes;

            (void)SNMP_MGR_ShmTrapQueueBufRead(shmbuf, 0, (UI8_T *)&nbytes, (int)(sizeof (nbytes)), SNMP_MGR_SHM_TRAP_QUEUE_PEEK);
            (void)SNMP_MGR_ShmTrapQueueBufRead(shmbuf, 0, NULL, (int)nbytes, 0);
            --shmbuf->entries;
            space += nbytes;
        }
        shmbuf->flags |= SNMP_MGR_SHM_TRAP_QUEUE_OVERLAP;
    }

write_data:
    if (buf && space) {
        UI8_T * base   = (UI8_T *)(shmbuf + 1);
        UI32_T  wr_off = shmbuf->wr_off;

        if ((wr_off + size) <= shmbuf->size) {
            memmove((void *)(base + wr_off), (void *)buf, size);
        } else {
            size_t  len1 = (size_t)(shmbuf->size - wr_off);
            size_t  len2 = (size_t)(size - len1);

            memmove((void *)(base + wr_off), (void *)buf, len1);
            memmove((void *)base, (void *)(buf + len1), len2);
        }
    }

    shmbuf->wr_off = (shmbuf->wr_off + size) % shmbuf->size;
    if (shmbuf->wr_off == shmbuf->rd_off)
        shmbuf->flags |= SNMP_MGR_SHM_TRAP_QUEUE_FULL;
    shmbuf->mtime = SYSFUN_GetSysTick();

    return size;
}

/*
 * Hack data of trap queue buffer
 * shmbuf   pointer of trap queue buffer in shared memory
 * buf      data to be written to shmbuf
 *          If 'buf' is specified as NULL, it means just update internal
 *          data writing position for next writing (or free some space
 *          for next writing), no any data is written into buffer 'shmbuf'
 * size     size of data in input buffer 'buf'
 *
 * Return value:
 *  < 0       error occured
 *  >= 0      bytes write into shared memory buffer 'shmbuf'
 *
 * NOTE:
 *          Due to this function is used to hack exist data, so the modification
 *          timestamp of queue buffer can't be changed!
 */
static int
SNMP_MGR_ShmTrapQueueBufHack(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *shmbuf, UI32_T offset, UI8_T *buf, int size)
{
    UI32_T      datalen = 0;

    if (!SNMP_MGR_SHM_TRAP_QUEUE_BUF_IS_VALID(shmbuf) || (size < 0))
        return -1;

    if (!size)
        return 0;

    /*
     * Treat the buffer as a flat address space. Only exist data can
     * be hacked, this means write-offset is the limit mark of data
     * hacking.
     *
     *   |<----- real data length ---->|
     *   |         |<- data can hack ->|
     *   | offset  |<- size ->|        |
     *   |_________|~~~~~~~~~~~~~~~~~~~|_____________|
     *  rd       rd_off               wr               limit
     *
     */
    if ((datalen = SNMP_MGR_GetShmTrapQueueBufDataSize(shmbuf)) < (offset + size))
        return -1;  /* hack data exceed limit */

    if (buf) {
        UI8_T * base   = (UI8_T *)(shmbuf + 1);
        UI32_T  wr_off = (shmbuf->rd_off + offset) % shmbuf->size;

        if ((wr_off + size) <= shmbuf->size) {
            memmove((void *)(base + wr_off), (void *)buf, size);
        } else {
            size_t  len1 = (size_t)(shmbuf->size - wr_off);
            size_t  len2 = (size_t)(size - len1);

            memmove((void *)(base + wr_off), (void *)buf, len1);
            memmove((void *)base, (void *)(buf + len1), len2);
        }
    }

    return size;
}

/*
 * Read data from trap queue buffer
 * shmbuf   pointer of trap queue buffer in shared memory
 * offset   offset from current read position to start reading data
 *          This 'offset' is usefull while peeking data from buffer
 * buf      data to keep data read from 'shmbuf'
 *          If 'buf' is specified as NULL, it means just update internal
 *          data reading position for next reading,
 * size     size of data to be read into buffer 'buf'
 *          It is the caller's responisiblity to ensure the destination
 *          buffer 'buf' has enough space to keep 'size' bytes data
 * flag     flags to indicate how to read data.
 *          Currently only one special flag is defined:
 *          - 0
 *            If no flag is specified , use 0 to indicate it is a normal
 *            reading operation to update internal state and data read
 *            position for next reading.
 *          - SNMP_MGR_SHM_TRAP_QUEUE_PEEK
 *            Peek data from trap queue buffer in shared memory without
 *            change its internal state and data read position.
 *
 * Return value:
 *  < 0     error occured
 *  >= 0    how many bytes of data been read into 'buf'
 *          If here is less data in 'shmbuf' than given size 'size', only
 *          available data is read into buffer without any wait.
 *          So it is always less than or equal required bytes for reading 'size'
 */
static int
SNMP_MGR_ShmTrapQueueBufRead(SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *shmbuf, UI32_T offset,  UI8_T *buf, int size, int flag)
{
    UI32_T      datalen = 0;

    if (!SNMP_MGR_SHM_TRAP_QUEUE_BUF_IS_VALID(shmbuf) || (size < 0))
        return -1;

    /*
     * Treat the buffer as a flat address space
     *   |<----- real data length ---->|
     *   |         |<- data can read ->|
     *   | offset  |<- size ->|        |
     *   |_________|~~~~~~~~~~~~~~~~~~~|_____________|
     *  rd       rd_off               wr               limit
     *
     */
    if (!size || ((datalen = SNMP_MGR_GetShmTrapQueueBufDataSize(shmbuf)) <= offset))
        return 0;                   /* here is no data to read  */

    datalen -= offset;              /* data length can be read  */
    if (datalen > (UI32_T)size)     /* real data length to read */
        datalen = size;

    if (buf) {
        UI8_T *     base   = (UI8_T *)(shmbuf + 1);
        UI32_T      rd_off = (shmbuf->rd_off + offset) % shmbuf->size;

        if ((rd_off + datalen) <= shmbuf->size) {
            memmove((void *)buf, (void *)(base + rd_off), (size_t)datalen);
        } else {
            size_t  len1 = (size_t)(shmbuf->size - rd_off);
            size_t  len2 = (size_t)(datalen - len1);

            memmove((void *)buf, (void *)(base + rd_off), len1);
            memmove((void *)(buf + len1), (void *)(base), len2);
        }
    }

    if (!(flag & SNMP_MGR_SHM_TRAP_QUEUE_PEEK)) {
        shmbuf->rd_off = (shmbuf->rd_off + offset + datalen) % shmbuf->size;
        if (datalen && (shmbuf->flags & (SNMP_MGR_SHM_TRAP_QUEUE_FULL | SNMP_MGR_SHM_TRAP_QUEUE_OVERLAP)))
            shmbuf->flags &= ~(SNMP_MGR_SHM_TRAP_QUEUE_FULL | SNMP_MGR_SHM_TRAP_QUEUE_OVERLAP);
        shmbuf->mtime = SYSFUN_GetSysTick();
    }

    return (int)(datalen);
}

/*
 * Initialize trap queue
 *      - create synchronization lock 'sem' semaphore
 *      - create and initialize trap queue buffer in shared memory
 * Return value:
 *      < 0     error occured
 *      0       success
 */
int
SNMP_MGR_ShmTrapQueueInit(SNMP_MGR_SHM_TRAP_QUEUE_T *queue)
{
    if (!queue)
        return -1;

    if (!SNMP_MGR_SHM_TRAP_QUEUE_IS_VALID(queue)) {
        queue->buf = (SNMP_MGR_SHM_TRAP_QUEUE_BUF_T *) SYSRSC_MGR_GetShMem(SYSRSC_MGR_SNMP_TRAP_SHMEM_SEGID);
        SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SNMP_TRAP, &queue->sem);
        if (queue->buf) {
            SNMP_MGR_ShmTrapQueueBufInit(queue->buf);
        }
    }

    return 0;
}

void
SNMP_MGR_ShmTrapQueueReset(SNMP_MGR_SHM_TRAP_QUEUE_T *queue)
{
    if (!SNMP_MGR_SHM_TRAP_QUEUE_IS_VALID(queue))
        return;

    SNMP_MGR_SHM_TRAP_QUEUE_LOCK(queue);
    SNMP_MGR_ShmTrapQueueBufReset(queue->buf);
    SNMP_MGR_SHM_TRAP_QUEUE_UNLOCK(queue);
}

SNMP_MGR_SHM_TRAP_QUEUE_T *
SNMP_MGR_GetShmTrapQueue(void)
{
    static SNMP_MGR_SHM_TRAP_QUEUE_T     queue = { (UI32_T)(-1), NULL };

    if ((queue.sem == (UI32_T)(-1)) || !queue.buf)
        SNMP_MGR_ShmTrapQueueInit(&queue);

    return &queue;
}

UI32_T
SNMP_MGR_GetShmTrapQueueEntryCount(SNMP_MGR_SHM_TRAP_QUEUE_T *queue)
{
    UI32_T      count = 0;

    if (SNMP_MGR_SHM_TRAP_QUEUE_IS_VALID(queue)) {
        SNMP_MGR_SHM_TRAP_QUEUE_LOCK(queue);
        count = queue->buf->entries;
        SNMP_MGR_SHM_TRAP_QUEUE_UNLOCK(queue);
    }

    return count;
}

/*
 * Read queued trap data from buffer in shared memory
 * queue        The trap queue of trap data
 * trap         Buffer or array to store returned trap data
 * n            number of trap data to retrieve from queue buffer
 * flag         flags to indicate how to read data.
 *              Currently only one special flag is defined:
 *              - 0
 *                If no flag is specified , use 0 to indicate it is a normal
 *                reading operation to update internal state and data read
 *                position for next reading.
 *              - SNMP_MGR_SHM_TRAP_QUEUE_PEEK
 *                Peek data from trap queue buffer in shared memory without
 *                change its internal state and data read position.
 *                If no flag is specified , use 0 to indicate it is a normal
 *                reading operation to update internal state and data read
 *                position for next reading.
 *
 * return value:
 *  < 0     error occurred
 *  >= 0    number of entries returned in 'buf', it may less than given 'n'
 *          if the here is no more entries
 *  It is the caller's responsibility to ensure the buffer space pointed by
 *  'trap' can hold 'n' trap entries
 *
 * The normal use of this function is:
 * AMS_TRAP_EVENT_TrapData_T        traps[SNMP_MGR_NTRAPS];
 * SNMP_MGR_ShmTrapQueueRead(queue, traps, SNMP_MGR_NTRAPS, 0);
 * SNMP_MGR_ShmTrapQueueRead(queue, traps, SNMP_MGR_NTRAPS, SNMP_MGR_SHM_TRAP_QUEUE_PEEK);
 */
int
SNMP_MGR_ShmTrapQueueRead(SNMP_MGR_SHM_TRAP_QUEUE_T *queue, TRAP_EVENT_TrapData_T *trap, int n, int flag)
{
    SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_T  entry;
    int                 offset;
    int                 nx, ret;

    if (!SNMP_MGR_SHM_TRAP_QUEUE_IS_VALID(queue) || (n < 0))
        return -1;

    if (n == 0)
        return 0;

    SNMP_MGR_SHM_TRAP_QUEUE_LOCK(queue);

    // dprintf("%s() [1]: buf rd_off %lu, wr_off %lu, entries %lu\n", __func__,
    //         queue->buf->rd_off, queue->buf->wr_off, queue->buf->entries);

    for (nx = 0, offset = 0; nx < n; ++nx, offset += entry.size) {
        memset(&entry, 0, sizeof (entry));
        /* read entry header until SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_T::content.comm.len */
        if ((ret = SNMP_MGR_ShmTrapQueueBufRead(queue->buf, offset, (UI8_T *)&entry,
                                       (SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE + 1),
                                       SNMP_MGR_SHM_TRAP_QUEUE_PEEK)) <= 0) {
            break;
        }
        if (trap && (ret > 0)) {
            int     data_len, data_off;

            memset(trap, 0, sizeof (*trap));
            /*
             *  trap->vr_id           = 0;
             *  trap->vrf_id          = 0;
             */
            trap->trap_time           = (UI32_T)entry.time;
            trap->remainRetryTimes    = (UI32_T)entry.retries;
            trap->trap_type           = (UI32_T)entry.type;
            trap->community_specified = (BOOL_T)((entry.flags & SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_COMMUNITY) != 0);
            trap->flag                = entry.flags;
            trap->community[0]        = 0;

            if (trap->community_specified) {
                int     comm_len = (int)entry.content.comm.len;
                int     comm_off = offset + SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE + 1;

                if ((ret = SNMP_MGR_ShmTrapQueueBufRead(queue->buf, (UI32_T)comm_off,
                                               (UI8_T *)trap->community,
                                               comm_len, SNMP_MGR_SHM_TRAP_QUEUE_PEEK)) < 0) {
                    break;
                }
                data_len = entry.size - SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE - 1 - comm_len;
                data_off = offset + SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE + 1 + comm_len;
            } else {
                data_len = entry.size - SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE;
                data_off = offset + SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE;
            }

            if ((ret = SNMP_MGR_ShmTrapQueueBufRead(queue->buf, (UI32_T)data_off,
                                           (UI8_T *)&trap->u, data_len,
                                           SNMP_MGR_SHM_TRAP_QUEUE_PEEK)) < 0) {
                break;
            }

            ++trap; /* read and translate next entry to trap data */
        }
    }

    if (nx) {
        if (!(flag & SNMP_MGR_SHM_TRAP_QUEUE_PEEK)) {
            (void) SNMP_MGR_ShmTrapQueueBufRead(queue->buf, 0, NULL, offset, 0);
            if (queue->buf->entries > nx) {
                queue->buf->entries -= nx;
            } else {
                queue->buf->entries = 0;
            }
            if (shm_trapq_dbg_read) {
                dprintf("%s() [2]: buf rd_off %lu, wr_off %lu, read %d entries, left %lu entries\n", __func__,
                        queue->buf->rd_off, queue->buf->wr_off, nx, queue->buf->entries);
            }
        } else {
            if (shm_trapq_dbg_read) {
                dprintf("%s() [3]: buf rd_off %lu, wr_off %lu, entries %lu\n", __func__,
                        queue->buf->rd_off, queue->buf->wr_off, queue->buf->entries);
            }
        }
    }
    SNMP_MGR_SHM_TRAP_QUEUE_UNLOCK(queue);

    return nx;
}

int
SNMP_MGR_ShmTrapQueueGetUnloggedEntries(SNMP_MGR_SHM_TRAP_QUEUE_T *queue, TRAP_EVENT_TrapData_T *trap, int n)
{
    static SNMP_MGR_SHM_TRAP_QUEUE_BUF_T    qbuf = {
        SNMP_MGR_SHM_TRAP_QUEUE_BUF_MAGIC,  /* magic        */
        SNMP_MGR_SHM_TRAP_QUEUE_BUF_SIZE,   /* size         */
        0,                                  /* max_entries  */
        0,                                  /* max_size     */
        0,                                  /* entries      */
        0,                                  /* flags        */
        0,                                  /* errcnt       */
        0,                                  /* wr_off       */
        0,                                  /* rd_off       */
        0                                   /* mtime        */
    };
    static UI32_T                   last_offset = 0;

    /////////////////////////////////////////////////////////////

    SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_T entry;
    UI32_T                          datasize;
    UI32_T                          offset;
    int                             nx, ret;

    if (!trap || !n)
        return 0;

    if (!SNMP_MGR_SHM_TRAP_QUEUE_IS_VALID(queue))
        return 0;

    SNMP_MGR_SHM_TRAP_QUEUE_LOCK(queue);

    datasize = SNMP_MGR_GetShmTrapQueueBufDataSize(queue->buf);
    if (memcmp((void *)&qbuf, (void *)queue->buf, sizeof (qbuf))) {
        offset = last_offset = 0;
        memcpy((void *)&qbuf, (void *)queue->buf, sizeof (qbuf));
    } else {
        offset = last_offset;
    }

    for (nx = 0; ((offset < datasize) && (nx < n)); offset += entry.size) {
        memset(&entry, 0, sizeof (entry));
        /* read entry header until SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_T::content.comm.len */
        if ((ret = SNMP_MGR_ShmTrapQueueBufRead(queue->buf, offset, (UI8_T *)&entry,
                                       (SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE + 1),
                                       SNMP_MGR_SHM_TRAP_QUEUE_PEEK)) <= 0) {
            break;
        }
        if (ret > 0) {
            int     data_len, data_off;

            if (entry.flags & SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOGGED)
                continue;

            memset(trap, 0, sizeof (*trap));
            trap->trap_time           = (UI32_T)entry.time;
            trap->remainRetryTimes    = (UI32_T)entry.retries;
            trap->trap_type           = (UI32_T)entry.type;
            trap->community_specified = (BOOL_T)((entry.flags & SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_COMMUNITY) != 0);
            /* trap->flag */
            if(entry.flags & SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOG_ONLY)
                trap->flag = TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY;
            else if (entry.flags & SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_TRAP_ONLY)
                trap->flag = TRAP_EVENT_SEND_TRAP_OPTION_TRAP_ONLY;
            else
                trap->flag = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;
            trap->community[0]        = 0;

            if (trap->community_specified) {
                int     comm_len = (int)entry.content.comm.len;
                int     comm_off = offset + SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE + 1;

                if ((ret = SNMP_MGR_ShmTrapQueueBufRead(queue->buf, (UI32_T)comm_off,
                                               (UI8_T *)trap->community,
                                               comm_len, SNMP_MGR_SHM_TRAP_QUEUE_PEEK)) < 0) {
                    break;
                }
                data_len = entry.size - SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE - 1 - comm_len;
                data_off = offset + SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE + 1 + comm_len;
            } else {
                data_len = entry.size - SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE;
                data_off = offset + SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE;
            }

            if ((ret = SNMP_MGR_ShmTrapQueueBufRead(queue->buf, (UI32_T)data_off,
                                           (UI8_T *)&trap->u, data_len,
                                           SNMP_MGR_SHM_TRAP_QUEUE_PEEK)) < 0) {
                break;
            }

            /*
             * XXX NOTE:
             *
             * Mark this entry as logged immediately. If send log message
             * to syslog task failed, then this log was lost.
             *
             * Due to SNMP_MGR_TrapLog() do not return any info about
             * log success of failure, so this behavior seems ok.
             *
             */
            entry.flags |= SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOGGED;
            if (SNMP_MGR_ShmTrapQueueBufHack(queue->buf, offset, (UI8_T *)&entry,
                            SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE) < 0) {
                dprintf("%s(): hack entry at offset %d failed\n", __func__, offset);
                dprintf("%s(): entry content\n", __func__);
                dprintf("   entry.type %d\n",  (int)entry.type);
                dprintf("   entry.flags %d\n", (int)entry.flags);
            }

            /* wait for a while to let syslog task to handle this logs */
            ++nx;
        }
    }

    last_offset = offset;
    SNMP_MGR_SHM_TRAP_QUEUE_UNLOCK(queue);

    return nx;
}

/*
 * Write trap data into trap queue buffer
 * queue        The queue to which write the trap data in
 * trap         Trap data to be written into queue
 * flag         Flags to indicate how to handle exceptions while here is no
 *              enough free space for new writing data. Two flags are defined
 *              now:
 *              - SNMP_MGR_SHM_TRAP_QUEUE_FORCE_WRITE
 *                This flag indicate force writing data to 'shmbuf' even if
 *                the left free space can't store all new data ('size' bytes).
 *                This means the input data will be truncated if over-written
 *                is allowed.
 *
 *              - SNMP_MGR_SHM_TRAP_QUEUE_OVER_WRITE
 *                This flag indicate whether wipe oldest records to free more
 *                space to store new input data. The more space freed automatically
 *                may exceed the needed space due to the free is done by free one
 *                or more oldest records.
 *                This flag must use together with SNMP_MGR_SHM_TRAP_QUEUE_FORCE_WRITE.
 *
 * Return value:
 *  < 0         error occured
 *  >= 0        bytes write into shared memory buffer 'shmbuf'
 */
int
SNMP_MGR_ShmTrapQueueWrite(SNMP_MGR_SHM_TRAP_QUEUE_T *queue, TRAP_EVENT_TrapData_T *trap, int flag)
{
    SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_T     entry;
    UI32_T                              space, buf_datasize;
    int                                 total_size, comm_size, data_size;
    int                                 ret = 0;

    if (!SNMP_MGR_SHM_TRAP_QUEUE_IS_VALID(queue))
        return -1;

    if (!trap)
        return 0;

    /* some trap do not initialize community string */
    if (!trap->community_specified) {
        trap->community[0] = 0;
    }

    // data_size  = (int)SNMP_MGR_GetDynamicDataSize(trap->trap_type);
    data_size  = (int)SNMP_MGR_GetTrapDataSize(trap);
    /* community size includes the header length byte and terminate NUL character */
    comm_size  = (int)(trap->community_specified ? (strlen((char *)trap->community) + 2) : 0);
    total_size = (int)(SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE + comm_size + data_size);

    if (comm_size && ((comm_size - 2) > SYS_ADPT_MAX_COMM_STR_NAME_LEN)) {
        dprintf("%s(): invalid community string %s of trap %lu\n", __func__,
                (char *)trap->community, trap->trap_type);
        return -1;
    }

#if 0
    if (shm_trapq_dbg_write) {
        dprintf("%s(): trap %lu, data %d, community %s, total %d\n", __func__,
                trap->trap_type, data_size, (char *)trap->community, total_size);
    }
#endif

    SNMP_MGR_SHM_TRAP_QUEUE_LOCK(queue);
    buf_datasize = SNMP_MGR_GetShmTrapQueueBufDataSize(queue->buf);
    space        = queue->buf->size - buf_datasize;

    // dprintf("%s() [1]: buf rd_off %lu, wr_off %lu, space %lu, entries %lu\n", __func__,
    //         queue->buf->rd_off, queue->buf->wr_off, space, queue->buf->entries);

    if (space < (UI32_T)total_size) {
        dprintf("%s(): no enough free space for trap %lu size %d\n",
                __func__, trap->trap_type, data_size);
        if (  !(flag & SNMP_MGR_SHM_TRAP_QUEUE_OVER_WRITE)
            || (queue->buf->size < (UI32_T)total_size)) {
            ret = -1;
            goto write_exit;
        }
    }

    memset(&entry, 0, sizeof (entry));
    entry.size    = (UI16_T)total_size;
    entry.type    = (UI16_T)trap->trap_type;
    entry.time    = (UI32_T)trap->trap_time;
    entry.retries = (UI8_T)trap->remainRetryTimes;
    /* entry.flags
     * Note: TRAP_EVENT_SEND_TRAP_OPTION_XXX is not bit-mapped
     *       should use "==" instead of "&" to compare
     */
    if (trap->flag == TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY)
    {
        entry.flags |= SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOG_ONLY;
    }
    else if (trap->flag == TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP)
    {
        entry.flags |= SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOG_AND_TRAP;
    }
    else if (trap->flag == TRAP_EVENT_SEND_TRAP_OPTION_TRAP_ONLY)
    {
        entry.flags |= SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_TRAP_ONLY;
    }
    else if (trap->flag == TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT)
    {
        entry.flags |= SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_DEFAULT;
    }
    else
    {
        ret = -1;
        goto write_exit;
    }

    if (trap->community_specified) {
        entry.flags           |= SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_COMMUNITY;
        entry.content.comm.len = (UI8_T)(comm_size - 1); /* include terminate NUL */
        ret += SNMP_MGR_ShmTrapQueueBufWrite(queue->buf, (UI8_T *)&entry, (int)(SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE + 1), flag);
        ret += SNMP_MGR_ShmTrapQueueBufWrite(queue->buf, (UI8_T *)trap->community, (int)entry.content.comm.len, flag);
    } else {
        ret += SNMP_MGR_ShmTrapQueueBufWrite(queue->buf, (UI8_T *)&entry, (int)SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE, flag);
    }
    ret += SNMP_MGR_ShmTrapQueueBufWrite(queue->buf, (UI8_T *)&trap->u, (int)data_size, flag);

    ++queue->buf->entries;
    if (queue->buf->entries > queue->buf->max_entries)
        queue->buf->max_entries = queue->buf->entries;

    buf_datasize += total_size;
    if (buf_datasize > queue->buf->max_size)
        queue->buf->max_size = buf_datasize;

    // SYSFUN_SendEvent(SYSFUN_GetMsgQOwner(ipcmsgq_handle), SNMP_MGR_EVENT_TRAP_ARRIVAL);
    // SYSFUN_SendEvent(snmp_taskId, SNMP_MGR_EVENT_TRAP_ARRIVAL);
    if (shm_trapq_dbg_write) {
        dprintf("%s() [2]: buf rd_off %lu, wr_off %lu, entries %lu\n", __func__,
                queue->buf->rd_off, queue->buf->wr_off, queue->buf->entries);

        if (ret != total_size) {
            dprintf("%s(): write data to buffer error, total %d, write %d\n",
                    __func__, total_size, ret);
        }
    }

write_exit:
    SNMP_MGR_SHM_TRAP_QUEUE_UNLOCK(queue);
    return ret;
}
