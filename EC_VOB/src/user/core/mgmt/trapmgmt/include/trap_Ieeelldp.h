#ifndef TRAP_IEEE_LLDP_H
#define TRAP_IEEE_LLDP_H
int crt_lldpRemTablesChange_trap(EBUFFER_T*, OCTET_T*, char*, unsigned, UINT_32_T, UINT_32_T, UINT_32_T, UINT_32_T, UINT_32_T);
int crt_lldpRemTablesChange_trapV2(EBUFFER_T*, char*, int, INT_32_T, UINT_32_T, UINT_32_T, UINT_32_T, UINT_32_T, UINT_32_T);

#endif