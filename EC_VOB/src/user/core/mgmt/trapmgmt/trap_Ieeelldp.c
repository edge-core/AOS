#include <asn1.h>
#include <buffer.h>
#include <snmpdefs.h>
/* Trap V1*/
static OIDC_T ID_lldpMIB[] = { 1, 0, 8802, 1, 1, 2 };
static int len_lldpMIB = 6;
static OIDC_T ID_lldpStatsRemTablesInserts[] = { 1, 0, 8802, 1, 1, 2, 1, 2, 2, 0 };
static int len_lldpStatsRemTablesInserts = 10;

static OIDC_T ID_lldpStatsRemTablesDeletes[] = { 1, 0, 8802, 1, 1, 2, 1, 2, 3, 0 };
static int len_lldpStatsRemTablesDeletes = 10;

static OIDC_T ID_lldpStatsRemTablesDrops[] = { 1, 0, 8802, 1, 1, 2, 1, 2, 4, 0 };
static int len_lldpStatsRemTablesDrops = 10;

static OIDC_T ID_lldpStatsRemTablesAgeouts[] = { 1, 0, 8802, 1, 1, 2, 1, 2, 5, 0 };
static int len_lldpStatsRemTablesAgeouts = 10;


/*****************************************************************************
 ****
 ****
 **** Trap creation function for lldpRemTablesChange trap
 ****
 **** DESCRIPTION:
 **** A lldpRemTablesChange notification is sent when the value of
 **** lldpStatsRemTableLastChangeTime changes. It can be utilized by an NMS to
 **** trigger LLDP remote systems table maintenance polls. Note that
 **** transmission of lldpRemTablesChange notifications are throttled by the
 **** agent, as specified by the 'lldpNotificationInterval' object.
 ****
 **** Returns length of encoded packet, 0 on SNMP failure,
 **** or -1 if an instance is too long.
 ****
 ****
 *****************************************************************************
 */

int crt_lldpRemTablesChange_trap(
	EBUFFER_T	*ebuff,
	OCTET_T		*local_ip,
	char		*community,
	unsigned	comlen,
	UINT_32_T	timestamp,
	UINT_32_T	AlldpStatsRemTablesInserts,	/* Gauge */
	UINT_32_T	AlldpStatsRemTablesDeletes,	/* Gauge */
	UINT_32_T	AlldpStatsRemTablesDrops,	/* Gauge */
	UINT_32_T	AlldpStatsRemTablesAgeouts)	/* Gauge */
{
	SNMP_PKT_T	*trap_pkt;

	/* Create the SNMP packet structure. */
	trap_pkt = SNMP_Create_Trap(VERSION_RFC1157, comlen, community,
				    len_lldpMIB, ID_lldpMIB,
				    local_ip,
				    6,	/* enterpriseSpecific(6) */
				    1,
				    timestamp,
				    4);

	/* Verify that SNMP_Create_Trap succeeded. */
	if (trap_pkt == (SNMP_PKT_T*)0)
		return 0;

	/* Bind the variables in the trap. */
	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 0, len_lldpStatsRemTablesInserts, ID_lldpStatsRemTablesInserts, VT_GAUGE, AlldpStatsRemTablesInserts) == -1)
		goto fail;

	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 1, len_lldpStatsRemTablesDeletes, ID_lldpStatsRemTablesDeletes, VT_GAUGE, AlldpStatsRemTablesDeletes) == -1)
		goto fail;

	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 2, len_lldpStatsRemTablesDrops, ID_lldpStatsRemTablesDrops, VT_GAUGE, AlldpStatsRemTablesDrops) == -1)
		goto fail;

	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 3, len_lldpStatsRemTablesAgeouts, ID_lldpStatsRemTablesAgeouts, VT_GAUGE, AlldpStatsRemTablesAgeouts) == -1)
		goto fail;

	/* Encode the packet as bytes into the user-supplied buffer. */
	if (SNMP_Encode_Packet(trap_pkt, ebuff) == -1)
		goto fail;

	/* Free the SNMP packet structure. */
	SNMP_Free(trap_pkt);

	/* Return the number of bytes used in the buffer. */
	return EBufferUsed(ebuff);

fail:
	/* Come here on failure during bind or encode. */
	SNMP_Free(trap_pkt);
	return 0;
}

/* Trap V2 */
static OIDC_T ID_sysUpTime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };
static int len_sysUpTime = 9;
static OIDC_T ID_snmpTrapOID[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
static int len_snmpTrapOID = 11;
static OIDC_T ID_lldpRemTablesChange[] = { 1, 0, 8802, 1, 1, 2, 0, 0, 1 };
static int len_lldpRemTablesChange = 9;

/*****************************************************************************
 ****
 ****
 **** Trap creation function for lldpRemTablesChange trap
 ****
 **** DESCRIPTION:
 **** A lldpRemTablesChange notification is sent when the value of
 **** lldpStatsRemTableLastChangeTime changes. It can be utilized by an NMS to
 **** trigger LLDP remote systems table maintenance polls. Note that
 **** transmission of lldpRemTablesChange notifications are throttled by the
 **** agent, as specified by the 'lldpNotificationInterval' object.
 ****
 **** Returns length of encoded packet, 0 on SNMP failure,
 **** or -1 if an instance is too long.
 ****
 ****
 *****************************************************************************
 */

int crt_lldpRemTablesChange_trapV2(
	EBUFFER_T	*ebuff,
	char		*community,
	int		comlen,
	INT_32_T	request_id,
	UINT_32_T	timestamp,
	UINT_32_T	AlldpStatsRemTablesInserts,	/* Gauge */
	UINT_32_T	AlldpStatsRemTablesDeletes,	/* Gauge */
	UINT_32_T	AlldpStatsRemTablesDrops,	/* Gauge */
	UINT_32_T	AlldpStatsRemTablesAgeouts)	/* Gauge */
{
	SNMP_PKT_T	*trap_pkt;

	/* Create the SNMP packet structure. */
	trap_pkt = SNMP_Create_Request2(TRAP2_PDU, SNMP_VERSION_2,
				   comlen, community, request_id,
				   6, 0, 0);

	/* Verify that SNMP_Create_V2_Request succeeded. */
	if (trap_pkt == (SNMP_PKT_T*)0)
		return 0;

	/* Bind the variables in the trap. */
	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 0, len_sysUpTime, ID_sysUpTime, VT_TIMETICKS, timestamp) == -1)
		goto fail;

	if (SNMP_Bind_Object_ID(trap_pkt, 1, len_snmpTrapOID, ID_snmpTrapOID, len_lldpRemTablesChange, ID_lldpRemTablesChange) == -1)
		goto fail;

	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 2, len_lldpStatsRemTablesInserts, ID_lldpStatsRemTablesInserts, VT_GAUGE, AlldpStatsRemTablesInserts) == -1)
		goto fail;

	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 3, len_lldpStatsRemTablesDeletes, ID_lldpStatsRemTablesDeletes, VT_GAUGE, AlldpStatsRemTablesDeletes) == -1)
		goto fail;

	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 4, len_lldpStatsRemTablesDrops, ID_lldpStatsRemTablesDrops, VT_GAUGE, AlldpStatsRemTablesDrops) == -1)
		goto fail;

	if (SNMP_Bind_Unsigned_Integer(trap_pkt, 5, len_lldpStatsRemTablesAgeouts, ID_lldpStatsRemTablesAgeouts, VT_GAUGE, AlldpStatsRemTablesAgeouts) == -1)
		goto fail;

	/* Encode the packet as bytes into the user-supplied buffer. */
	if (SNMP_Encode_Packet(trap_pkt, ebuff) == -1)
		goto fail;

	/* Free the SNMP packet structure. */
	SNMP_Free(trap_pkt);

	/* Return the number of bytes used in the buffer. */
	return EBufferUsed(ebuff);

fail:
	/* Come here on failure during bind or encode. */
	SNMP_Free(trap_pkt);
	return 0;
}