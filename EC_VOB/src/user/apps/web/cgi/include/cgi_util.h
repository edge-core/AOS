/* ----------------------------------------------------------------------
 * FILE NAME: cg_util.h

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K.

 * ABSTRACT:
 * This is part of the embedded program for ES3524.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Module for Fileless CGI.
 * This is the header file for utility.

 * CLASSES AND FUNCTIONS:

 * cgi_ConvertIP: Convert IP address to byte array.

 * USAGE:

 * HISTORY:
 * 1998-08-12 (Wed): Created by Daniel K. Chung.

 * COPYRIGHT (C) Accton Technology Corporation, 1998.
 * ---------------------------------------------------------------------- */
#ifndef CGI_UTIL_H
#define CGI_UTIL_H

#include "sys_type.h"
#include "cgi.h"

#if __cplusplus
extern "C" {
#endif

#define CGI_UTIL_SET_FLAG(V,F)        (V) = (V) | (F)
#define CGI_UTIL_CHECK_FLAG(V,F)      ((V) & (F))
#define CGI_UTIL_UNSET_FLAG(V,F)      (V) = (V) & ~(F)

#define CGI_UTIL_LINUX_CMD_SIZE  256
#define CGI_UTIL_LINUX_RETURN_BUFFER_SIZE  256

/* FUNCTION NAME: CGI_UTIL_SetXmlFlag
 * PURPOSE: For determing is XML file request.
 * INPUT:  BOOL_T isXMLFile
 * OUTPUT: none
 * RETURN: BOOL_T is_xml_file
 */
int CGI_UTIL_SetXmlFlag(BOOL_T isXMLFile);
int CGI_UTIL_IsXmlFile(BOOL_T *isXMLFile);

 /* FUNCTION NAME: CGI_UTIL_StrEsc
 * PURPOSE: Convert string to HTML escaped form.
 * INPUT:  char *szIn - string.
 * OUTPUT: char *szOut - escaped string.
 * RETURN: None
 */
void CGI_UTIL_StrEsc (char *szIn, char *szOut);


void cgi_convertDec2Str(unsigned long dec,char *str,int radix);
int cgi_convertStr2Dec(char *str,unsigned long *dec,int radix);


/* FUNCTION NAME: CGI_UTIL_IntToIpStr
 * PURPOSE: Convert int to ip string(4 byte object -> xxx.xxx.xxx.xxx).
 * INPUT:  UI8_T *valueIn - int value.
 * OUTPUT: UI8_T *valueOut - ip string.
 * RETURN: None
 */
void CGI_UTIL_IntToIpStr (UI8_T *valueIn, UI8_T *valueOut);

/* FUNCTION NAME: CGI_UTIL_IpStrToInt
 * PURPOSE: Convert ip string to int (xxx.xxx.xxx.xxx -> 4 byte object).
 * INPUT:  UI8_T *valueIn - ip string.
 * OUTPUT: UI8_T *valueOut - int value.
 * RETURN: TRUE  - convert success
 *         FALSE - convert fail
 */
BOOL_T CGI_UTIL_IpStrToInt (const char *valueIn, UI32_T *valueOut);

/* FUNCTION NAME: CGI_UTIL_HexToMacStr
 * PURPOSE: Convert hex octets value to mac string (6 byte object -> xx-xx-xx-xx-xx-xx).
 * INPUT:  UI8_T *valueIn - hex octets value.
 * OUTPUT: UI8_T *valueOut - mac string.
 * RETURN: None
 */
void CGI_UTIL_HexToMacStr (const UI8_T *valueIn, char *valueOut);

/* FUNCTION NAME: CGI_UTIL_MacStrToHex
 * PURPOSE: Convert mac string to hex octets (xx-xx-xx-xx-xx-xx -> 6 byte object).
 * INPUT:  UI8_T *valueIn - mac string.
 * OUTPUT: UI8_T *valueOut - hex octets value.
 * RETURN: TRUE  - convert success
 *         FALSE - convert fail
 */
BOOL_T CGI_UTIL_MacStrToHex (const char *valueIn, UI8_T *valueOut);


/* FUNCTION NAME: CGI_UTIL_UI64ToStr
 * PURPOSE: Convert 64 bit binary value to string.
 * INPUT:  UI64_T valueIn - 64 bit binary value.
 * OUTPUT: UI8_T *valueOut - string.
 * RETURN: None
 */
void CGI_UTIL_UI64ToStr(UI64_T valueIn, UI8_T *valueOut);

/* FUNCTION NAME: CGI_UTIL_Url_Encode
 * PURPOSE: Returns a url-encoded version of str
 * INPUT:  char *str - string before encoding.
 * OUTPUT: char *out_str - url-encoded version of str.
 * RETURN: None
*/
void CGI_UTIL_Url_Encode(char *str, char *out_str);

/* FUNCTION NAME: CGI_UTIL_Base64ToPlain
 * PURPOSE: Convert Base-64 encoding to plaintext.
 * INPUT:  UI8_T buf - plaintext
 * OUTPUT: UI8_T buf - plaintext
 * RETURN: TRUE ; FALSE
 */
BOOL_T CGI_UTIL_Base64ToPlain (UI8_T *buf);

/* Don't use this function in the future. Use CGI_UTIL_ConvertUserPortToIfindex instead.
 */
BOOL_T CGI_UTIL_UserPortToIfindex (UI32_T nUnit, UI32_T nPort, UI32_T *nIfIndex);

/* FUNCTION NAME: CGI_UTIL_ConvertUserPortToIfindex
 * PURPOSE: Convert user port to ifindex and get port type.
 * INPUT:  unit - device unit or 100 when inpurt port is trunk id,
 *         port - user port
 * OUTPUT: ifindex_p - interface index
 * RETURN: CGI_TYPE_Lport_Type_T - port type
 */
CGI_TYPE_Lport_Type_T CGI_UTIL_ConvertUserPortToIfindex(UI32_T unit, UI32_T port, UI32_T *ifindex_p);

/* FUNCTION NAME: CGI_UTIL_IfindexToUserPortStr
 * PURPOSE: Convert ifindex to user port display string.
 * INPUT:  ifindex -- interface index
 *         cb_str  -- count of bytes of str
 * OUTPUT: str -- display string
 * RETURN: None
 */
BOOL_T CGI_UTIL_IfindexToUserPortStr (UI32_T ifindex, char *str, UI32_T cb_str);
BOOL_T CGI_UTIL_IfindexToRestPortStr(UI32_T ifindex, char *str);

BOOL_T CGI_UTIL_CheckUnitExist (UI32_T nUnit);
BOOL_T CGI_UTIL_CheckPortExist (UI32_T nPort, UI32_T nUnit);
BOOL_T CGI_UTIL_CheckPortExistNoTrunk (UI32_T nPort, UI32_T nUnit);
BOOL_T CGI_UTIL_CheckTrunkExist (UI32_T nTrunk);
BOOL_T CGI_UTIL_CheckTrunkExistPort (UI32_T nTrunk);

void CGI_UTIL_CheckAnyIPV6Addr (UI8_T *buff, UI8_T *ipv6, UI32_T pfx_len);
int CGI_UTIL_StrToHex (char *inStr, unsigned char *outHex, int octet_num);
UI32_T CGI_UTIL_StrToUL (char *s, int radix);

int CGI_UTIL_UI32ToCommaStr(UI32_T value, UI32_T dis_len, char *comma_str, UI32_T cb_str);

//static int getHexValue (char ch);

/* FUNCTION NAME: CGI_UTIL_ReplaceString
 * PURPOSE: replace some char of a string
 * INPUT:  char *str - source string
 *         char srch - which char to replace
 *         char *tgstr - replace char to specified string
 * OUTPUT: char *result
 * RETURN: none
 */
void CGI_UTIL_ReplaceString(char *str, char srch, char *tgstr, char *result);

BOOL_T CGI_UTIL_StrToNonprintable(char *str, UI8_T *om);

//
// FIXME: cgi_util shall not have any SYS_CPNT
//        input data shall include area format
//
#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_OSPF == TRUE)
/* FUNCTION NAME: CGI_UTIL_ConvertAreaToFormat
 * PURPOSE: Convert OSPF area ID to UI32 format
 * INPUT:  UI8_T buf - input string
 * OUTPUT: UI32_T areaId, areaFormat
 * RETURN: none
 */
void CGI_UTIL_ConvertAreaToFormat (UI8_T *buf, UI32_T *areaId, UI32_T *areaFormat);
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */
#endif /*#if (SYS_CPNT_OSPF == TRUE) */

//
// FIXME: use strchr to compare
//
BOOL_T CGI_UTIL_IsMeetDelemiter(char c, char delemiters[]);

//
// FIXME: regularize input data (e.g. "1-1,2-10" ),
//        if data format is incorrect, reject this request
//
char *CGI_UTIL_GetToken (char *s, char *Token, char delemiters[]);

//
// FIXME: use strcspn to parse
//
#define WAIT_FOR_VALUE 65535
BOOL_T CGI_UTIL_GetLowerUpperValue(char *buff, UI32_T *lower_val, UI32_T *upper_val, UI32_T *err_idx);

//
// FIXME: cgi_util shall not have any SYS_CPNT
//
#if (SYS_CPNT_CFM == TRUE)
#define CGI_CFM_DEF_MAX_BUFSIZE   300

void CGI_UTIL_ParseCfmVlanList(char *input_p, UI32_T *primary_vid, UI32_T *vid_num, UI8_T *vid_list);
#endif /* #if (SYS_CPNT_CFM == TRUE) */

/* FUNCTION NAME: CGI_UTIL_GetPortMax
 * PURPOSE: Get the max port number
 * INPUT:   unit - device unit
 * OUTPUT:  port_number_p - max port number
 * RETURN:  TRUE ; FALSE
 * NOTE:
 */
BOOL_T CGI_UTIL_GetPortMax (UI32_T unit, UI32_T *port_number_p);

/* FUNCTION NAME: CGI_UTIL_StrIsSignedDigit
 * PURPOSE: Check the string is a signed digit.
 * INPUT:   str_p - pointer of input string (ASCII format)
 * RETURN:  TRUE ; FALSE
 * NOTE:
 */
BOOL_T CGI_UTIL_IsInteger (const char *str_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_UTIL_CheckNullStr
 *------------------------------------------------------------------------------
 * PURPOSE:  Check if input is a null str.
 * INPUT:    buff
 * OUTPUT:   None.
 * RETURN:   1               -- buff is a null string.
 *           0               -- buff is not a null string.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
UI16_T CGI_UTIL_CheckNullStr(char *buff);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_UTIL_ConvertTextToHex
 *------------------------------------------------------------------------------
 * PURPOSE:  Convert text to a hexadecimal string.
 * INPUT:    input          -- text
 *           input_len      -- input text's length
 * OUTPUT:   out_buf        -- output buffer for save converted hexadecimal string
 *           out_buf_len    -- output buffer's length
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
void CGI_UTIL_ConvertTextToHex(UI8_T *input, UI32_T input_len, char *out_buf, size_t out_buf_len);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_UTIL_InterfaceIdToEth
 *------------------------------------------------------------------------------
 * PURPOSE:  Parsing interface ID (eth1/1) to get unit and port.
 * INPUT:    str         -- The interface ID
 * OUTPUT:   unit        -- The unit value
 *           port        -- The port value
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
BOOL_T CGI_UTIL_InterfaceIdToEth(const char *str, UI32_T *unit, UI32_T *port);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_UTIL_InterfaceIdToLport
 *------------------------------------------------------------------------------
 * PURPOSE:  Parsing interface ID (eth1/1) to get lport.
 * INPUT:    str         -- The interface ID
 * OUTPUT:   lport       -- The ifindex
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
BOOL_T CGI_UTIL_InterfaceIdToLport(const char *str, UI32_T *lport);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_UTIL_UrlDecode
 *------------------------------------------------------------------------------
 * PURPOSE:  Returns a url-decoded version of str
 * INPUT:    src        -- string before decoding.
 * OUTPUT:   dst        -- url-decoded version of str.
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
void CGI_UTIL_UrlDecode(char *dst, const char *src);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_UTIL_MaskToPrefix
 *------------------------------------------------------------------------------
 * PURPOSE:  Returns a prefix length of specified mask address
 * INPUT:    mask_p -- specified mask address.
 * OUTPUT:   None.
 * RETURN:   prefix length.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
UI32_T CGI_UTIL_MaskToPrefix(UI8_T *mask_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_UTIL_PrefixToMask
 *------------------------------------------------------------------------------
 * PURPOSE:  Convert a prefix length to mask address
 * INPUT:    prefix -- prefix length.
 * OUTPUT:   mask_p -- mask address.
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
void CGI_UTIL_PrefixToMask(UI32_T prefix, UI8_T *mask_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_UTIL_ExecLinuxSetCommand
 *------------------------------------------------------------------------------
 * PURPOSE:  Send set command to linux shell
 * INPUT:    cmd_p -- set command
 * OUTPUT:   None --
 * RETURN:   TRUE/FALSE.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
BOOL_T CGI_UTIL_ExecLinuxSetCommand(char *cmd_p);

#if __cplusplus
}
#endif

#endif /* CGI_UTIL_H */
