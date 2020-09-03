/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_util.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Module for Fileless CGI.
 * CGI utilities.

 * CLASSES AND FUNCTIONS:
 * cgi_ConvertIP: Convert textual IP address to byte array.

 * HISTORY:
 * 1998-11-28 (Tue): Created by Zhong Qiyao.
 * 1998-11-28 (Tue): All functions, Nike Chen (Chen Naiqi).

 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */

//
// TODO: This function only put the library for cgi and call cmnlib ONLY.
//       All CSC specific function move to cgi_lib.
//       (Put convert unit/port to logic port here only.)
//       If the function can be insteaded by std C library, call std C libary
//       directly.
//       If the function just call / by-pass to another CSC function, call the
//       CSC function directly.
//

#include "cgi_coretype.h"
#include "cgi_util.h"
#include "l_stdlib.h"
#include "l_inet.h"
#include "swctrl.h"
#include "trk_mgr.h"
#include "trk_pmgr.h"
#include "stktplg_pmgr.h"
#include "swctrl_pom.h"
#include "stktplg_pom.h"
#include "rule_type.h"

#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_OSPF == TRUE)
#include "netcfg_type.h"
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

static int getHexValue (char ch);

/* FUNCTION NAME: CGI_UTIL_SetXmlFlag
 * PURPOSE: For determing is XML file request.
 * INPUT:  BOOL_T isXMLFile
 * OUTPUT: none
 * RETURN: BOOL_T is_xml_file
 */
//
// FIXME: Make sure what these two variables use for.
//        Remove or move into connection object.
//
static BOOL_T is_xml_file = FALSE ;
static BOOL_T is_ezconfig_file = FALSE ;

int CGI_UTIL_SetXmlFlag(BOOL_T isXMLFile)
{
   return is_xml_file = isXMLFile;
}

int CGI_UTIL_IsXmlFile(BOOL_T *isXMLFile)
{
    return *isXMLFile = is_xml_file;
}

/* FUNCTION NAME: CGI_UTIL_SetEzconfigFlag
 * PURPOSE: For determing is EZConfig program request.
 * INPUT:  BOOL_T isEZCONFIGFile
 * OUTPUT: none
 * RETURN: BOOL_T is_ezconfig_file
 */
int CGI_UTIL_SetEzconfigFlag(BOOL_T isEZCONFIGFile)
{
   return is_ezconfig_file = isEZCONFIGFile;
}

int CGI_UTIL_IsEzconfigFile(BOOL_T *isEZCONFIGFile)
{
    return *isEZCONFIGFile = is_ezconfig_file;
}

/* FUNCTION NAME: CGI_UTIL_StrEsc
 * PURPOSE: Convert string to HTML escaped form.
 * INPUT:  char *szIn - string.
 * OUTPUT: char *szOut - escaped string.
 * RETURN: None
 */
void CGI_UTIL_StrEsc (char *szIn, char *szOut)
{
    char *pOut = szOut, *pIn = szIn;

    while (*pIn != 0)
    {
        /* check special character */
        switch (*pIn)
        {
            case '%': strcpy (pOut, "%25"); pOut += 3; break;
            case '&': strcpy (pOut, "%26"); pOut += 3; break;
            case '#': strcpy (pOut, "%23"); pOut += 3; break;
            case '=': strcpy (pOut, "%3D"); pOut += 3; break;
            case '+': strcpy (pOut, "%2B"); pOut += 3; break;
            case '\\': strcpy (pOut, "%5C"); pOut += 3; break;
            case '\"': strcpy (pOut, "%22"); pOut += 3; break;
            case '\'': strcpy (pOut, "%27"); pOut += 3; break;
            case ' ': strcpy (pOut, "%20"); pOut += 3; break;

            default:  *pOut = *pIn; pOut++; break;
        }

        pIn++;
    }

    *pOut = 0;
}

void cgi_convertDec2Str(unsigned long dec,char *str,int radix)
{
    char ch;
    int mod=0,i=0,j=0;
    char temp[12];
    if (radix<=9 || radix>=17)
        goto IllegalConvert; // JSS we just convert DEC and HEX format

    if (dec == 0)   // nike add for zero case 1998-8-5
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (dec!=0)
    {
        mod=dec%radix;
        if (mod < 10)
            ch='0'+(char)mod;
        else
            ch='7'+(char)mod;
        temp[i++]=ch;
        dec/=radix;
    }

    temp[i]='\0';
    i--;
    for(j=0;i>=0;j++,i--)
    {
        str[i]=temp[j];
    }
IllegalConvert:
    str[j]='\0';
}

/*autwo6*/
int cgi_convertStr2Dec(char *str,unsigned long *dec,int radix)
{
    int i=0;
    *dec = 0;
    if (radix<=9 || radix>=17)
        goto IllegalConvert; // JSS we just convert DEC and HEX format

    if (str[0] == '0' && str[1] == '\0')   // nike add for zero case 1998-8-5
    {
        *dec = 0;
        return 0;
    }

    while (str[i] != '\0')
    {
        if (str[i] >= 'A' && str[i] <= 'F')
            *dec += str[i] - 'A' + 10;
        else if (str[i] >= 'a' && str[i] <= 'f')
            *dec += str[i] - 'a' + 10;
        else if (str[i] >= '0' && str[i] <= '9')
            *dec += str[i] - '0';
        else
            goto IllegalConvert;
        i++;
        *dec = *dec * radix;

    }
    *dec  = *dec / radix;
    return 1;
IllegalConvert:
    *dec = 0;
    return 0;
}

/* FUNCTION NAME: CGI_UTIL_IntToIpStr
 * PURPOSE: Convert int to ip string(4 byte object -> xxx.xxx.xxx.xxx).
 * INPUT:  UI8_T *valueIn - int value.
 * OUTPUT: UI8_T *valueOut - ip string.
 * RETURN: None
 */
void CGI_UTIL_IntToIpStr (UI8_T *valueIn, UI8_T *valueOut)
{
    //
    // FIXME: This function is unnecessary and unsafe.
    //        The type of input / output parameter are even wrong.
    //        Remove this function and call cmnlib
    //
    sprintf((char *) valueOut, "%d.%d.%d.%d",
        valueIn[0], valueIn[1], valueIn[2], valueIn[3]);
}

/* FUNCTION NAME: CGI_UTIL_IpStrToInt
 * PURPOSE: Convert ip string to int (xxx.xxx.xxx.xxx -> 4 byte object).
 * INPUT:  UI8_T *valueIn - ip string.
 * OUTPUT: UI8_T *valueOut - int value.
 * RETURN: TRUE  - convert success
 *         FALSE - convert fail
 */
BOOL_T CGI_UTIL_IpStrToInt (const char *valueIn, UI32_T *valueOut)
{
    int ipAddr[CGI_IPLEN];
    int    i;

    //
    // FIXME: This function is unnecessary and unsafe.
    //        The type of input / output parameter are even wrong.
    //        Remove this function and call cmnlib
    //
    if ((sscanf((char *) valueIn, "%d.%d.%d.%d,",
            &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3])) == 4)
    {
        for (i = 0; i < CGI_IPLEN; i++)
        {
            ((UI8_T *)valueOut)[i] = (UI8_T) ipAddr[i];
        }

        return TRUE;
    }

    return FALSE;
}

/* FUNCTION NAME: CGI_UTIL_HexToMacStr
 * PURPOSE: Convert hex octets value to mac string (6 byte object -> xx-xx-xx-xx-xx-xx).
 * INPUT:  UI8_T *valueIn - hex octets value.
 * OUTPUT: UI8_T *valueOut - mac string.
 * RETURN: None
 */
void CGI_UTIL_HexToMacStr (const UI8_T *valueIn, char *valueOut)
{
    sprintf(valueOut, "%02X-%02X-%02X-%02X-%02X-%02X",
        valueIn[0], valueIn[1], valueIn[2], valueIn[3], valueIn[4], valueIn[5]);
}

/* FUNCTION NAME: CGI_UTIL_MacStrToHex
 * PURPOSE: Convert mac string to hex octets (xx-xx-xx-xx-xx-xx -> 6 byte object).
 * INPUT:  UI8_T *valueIn - mac string.
 * OUTPUT: UI8_T *valueOut - hex octets value.
 * RETURN: TRUE  - convert success
 *         FALSE - convert fail
 */
BOOL_T CGI_UTIL_MacStrToHex (const char *valueIn, UI8_T *valueOut)
{
    UI32_T nMacAddr[CGI_MACHEXLEN];
    int    i;
    //
    // FIXME: sscanf can't verify the format of input string is correct or not
    //
    if (((sscanf(valueIn, "%lX-%lX-%lX-%lX-%lX-%lX",
            &nMacAddr[0], &nMacAddr[1], &nMacAddr[2], &nMacAddr[3], &nMacAddr[4], &nMacAddr[5])) == 6) ||
        ((sscanf(valueIn, "%2lX%2lX%2lX%2lX%2lX%2lX",
            &nMacAddr[0], &nMacAddr[1], &nMacAddr[2], &nMacAddr[3], &nMacAddr[4], &nMacAddr[5])) == 6))
    {
        for (i = 0; i < CGI_MACHEXLEN; i++)
        {
            valueOut[i] = (char) (unsigned char) nMacAddr[i];
        }

        return TRUE;
    }

    return FALSE;
}

/* FUNCTION NAME: CGI_UTIL_StrToHex
 * PURPOSE:
 *     convert the string to its hex octets
 * INPUT:
 *     string
 * OUTPUT:
 *     hex octets
 * RETURN:
 *     1 if success, 0 otherwise
 */
int CGI_UTIL_StrToHex (char *inStr, unsigned char *outHex, int octet_num)
{
    char  *ptr;
    int  c, len;
    int  i, v1, v2;

    if ((inStr==0) || (outHex==0))
       return 0;

    ptr = inStr;
    len = strlen(ptr);

      /* no delimiters, e.g. 000011223344 */

    for (c=0; c<octet_num; c++)
    {
        i = c << 1;
        if (((v1 = getHexValue(ptr[i])) >= 0) &&
            ((v2 = getHexValue(ptr[i+1])) >= 0))
        {
            outHex[c] = (v1 << 4) | v2;
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

/* FUNCTION NAME: CGI_UTIL_StrToUL
 * PURPOSE:
 *     convert the string to unsigned long with 'radix' as base.
 * INPUT:
 *     string, radix
 * RETURN:
 *     unsigned long
 */
UI32_T CGI_UTIL_StrToUL (char *s, int radix)
{
    int i;
    unsigned long n = 0;

    for (i = 0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
        ;       /* skip white space */

    if (s[i] == '+' || s[i] == '-')
    {
        i++;    /* skip sign */
    }

    if (radix == 10)
    {
        for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
        {
            n = 10 * n + s[i] - '0';
        }
    }
    else if (radix == 16)
    {
        if ( (s[i] == '0') && (s[i+1] == 'x' || s[i+1] == 'X') ) // Charles,
           i=i+2;                                                // To skip the "0x" or "0X"


        for (n = 0;
            (s[i] >= '0' && s[i] <= '9') ||
            (s[i] >= 'A' && s[i] <= 'F') ||
            (s[i] >= 'a' && s[i] <= 'f');
            i++)
        {
            if (s[i] >= '0' && s[i] <= '9')
            {
                n = 16 * n + s[i] - '0';
            }
            else if (s[i] >= 'A' && s[i] <= 'F')
            {
                n = 16 * n + s[i] - 'A'+ 10;
            }
            else if (s[i] >= 'a' && s[i] <= 'f')
            {
                n = 16 * n + s[i] - 'a'+ 10;
            }
        }
    }

    return (n);
}

/* FUNCTION NAME: CGI_UTIL_UI32ToCommaStr
 * PURPOSE:  display 27586452 to 27,586,452
 * INPUT:  value     - input value.
 *         dis_len   - input value display lenth.
 *         cb_str    - count of bytes of str.
 * OUTPUT: comma_str - have comma value string.
 * RETURN: The number of characters written, or ¡V1 if an error occurred.
 */
int CGI_UTIL_UI32ToCommaStr(UI32_T value, UI32_T dis_len, char *comma_str, UI32_T cb_str)
{
    int c;
    char buf[32];
    char temp_buf[32];
    char *p;
    char *out = temp_buf;
    int len;

    len = SYSFUN_Snprintf(buf, sizeof(buf), "%lu", value);
    c = 2 - len % 3;

    for (p = buf; *p != 0; p++)
    {
        *out++ = *p;
        if (c == 1)
        {
            *out++ = ',';
        }

        c = (c + 1) % 3;
    }

    *--out = 0;

    return SYSFUN_Snprintf(comma_str, cb_str, "%*s", dis_len, temp_buf);
}

static int getHexValue (char ch)
{
    int  value = -1;
    if (('0' <= ch) && (ch <= '9'))
    {
        value = (int)ch - (int)'0';
    }
    else if (('A' <= ch) && (ch <= 'F'))
    {
        value = (int)ch - (int)'A' + 10;
    }
    else if (('a' <= ch) && (ch <= 'f'))
    {
        value = (int)ch - (int)'a' + 10;
    }
    return value;
}

/* Converts an integer value to its hex character*/
char CGI_UTIL_To_Hex(char code)
{
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

/* FUNCTION NAME: CGI_UTIL_Url_Encode
 * PURPOSE: Returns a url-encoded version of str
 * INPUT:  char *str - string before encoding.
 * OUTPUT: char *out_str - url-encoded version of str.
 * RETURN: None
*/
void CGI_UTIL_Url_Encode(char *str, char *out_str)
{
    char *pstr = str;

    while (*pstr)
    {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
            *out_str++ = *pstr;
        else if (*pstr == ' ')
            *out_str++ = '+';
        else
        {
            *out_str++ = '%';
            *out_str++ = CGI_UTIL_To_Hex(*pstr >> 4);
            *out_str++ = CGI_UTIL_To_Hex(*pstr & 15);
        }
        pstr++;
    }
    *out_str = '\0';
    return;
}

/* FUNCTION NAME: CGI_UTIL_UI64ToStr
 * PURPOSE: Convert 64 bit binary value to string.
 * INPUT:  UI64_T valueIn - 64 bit binary value.
 * OUTPUT: UI8_T *valueOut - string.
 * RETURN: None
 */
void CGI_UTIL_UI64ToStr (UI64_T valueIn, UI8_T *valueOut)
{
    UI32_T nLen = 0;
    UI8_T  arStr[20] = {0};

    memset(arStr, 0, 20);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(valueIn), L_STDLIB_UI64_L32(valueIn), arStr);
    nLen = (UI32_T)L_STDLIB_Trim_Left(arStr, 20);
    strncpy(valueOut, arStr, nLen);
    valueOut[nLen] = '\0';
    return;
}

/* FUNCTION NAME: CGI_UTIL_Base64ToPlain
 * PURPOSE: Convert Base-64 encoding to plaintext.
 * INPUT:  UI8_T buf - plaintext
 * OUTPUT: UI8_T buf - plaintext
 * RETURN: TRUE ; FALSE
 */
BOOL_T CGI_UTIL_Base64ToPlain (UI8_T *buf)
{
    UI8_T  vec[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    int d, i, num, len, j, val;
    int write_idx = 0;
    UI8_T  nw[4],  *p, *c;

    len = strlen(buf);

    if ((len % 4) != 0)
    {
          return FALSE;
    }

    for (i = 0; i < len; i += 4)
    {
        val = 0;
        num = 3;
        c = buf+i;

        if (c[2] == '=')
        {
              num = 1;
        }
      else if (c[3] == '=')
      {
              num = 2;
        }

        for (j = 0; j <= num; j++)
        {
            if (!(p = strchr(vec, c[j])))
            {
                /* not in base64 format */
                return FALSE;
            }

            d = p-vec;
            d <<= (3-j)*6;
            val += d;
        }

        for (j = 2; j >= 0; j--)
        {
            nw[j] = val & 255;
            val >>= 8;
        }

        memcpy(&buf[write_idx], nw, num);
        write_idx += num;
    }

    buf[write_idx] = '\0';
    return TRUE;
}

/* Don't use this function in the future. Use CGI_UTIL_ConvertUserPortToIfindex instead.
 */
BOOL_T CGI_UTIL_UserPortToIfindex (UI32_T nUnit, UI32_T nPort, UI32_T *nIfIndex)
{
    /* trunk */
    if (nUnit == 100)
    {
        if (CGI_UTIL_CheckTrunkExistPort(nPort) != TRUE)
        {
            return FALSE;
        }

        if (SWCTRL_POM_TrunkIDToLogicalPort(nPort, nIfIndex) != TRUE)
        {
            return FALSE;
        }
    }
    /* port */
    else
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (nPort == SYS_ADPT_MGMT_PORT)
        {
            return FALSE;
        }
#endif /* #if (SYS_CPNT_MGMT_PORT == TRUE) */

        if (SWCTRL_POM_UserPortToIfindex(nUnit, nPort, nIfIndex) != SWCTRL_LPORT_NORMAL_PORT)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/* FUNCTION NAME: CGI_UTIL_ConvertUserPortToIfindex
 * PURPOSE: Convert user port to ifindex and get port type.
 * INPUT:  unit - device unit or 100 when inpurt port is trunk id,
 *         port - user port
 * OUTPUT: ifindex_p - interface index
 * RETURN: CGI_TYPE_Lport_Type_T - port type
 */
CGI_TYPE_Lport_Type_T CGI_UTIL_ConvertUserPortToIfindex(UI32_T unit, UI32_T port, UI32_T *ifindex_p)
{
    SWCTRL_Lport_Type_T  lport_type;

    /* trunk
     */
    if (100 == unit)
    {
        if (CGI_UTIL_CheckTrunkExistPort(port) != TRUE)
        {
            return CGI_TYPE_LPORT_UNKNOWN_PORT;
        }

        if (SWCTRL_POM_TrunkIDToLogicalPort(port, ifindex_p) != TRUE)
        {
            return CGI_TYPE_LPORT_UNKNOWN_PORT;
        }

        return CGI_TYPE_LPORT_TRUNK_PORT;
    }

    /* port
     */
#if (SYS_CPNT_MGMT_PORT == TRUE)
    if (SYS_ADPT_MGMT_PORT == port)
    {
        return CGI_TYPE_LPORT_MGMT_PORT;
    }
#endif /* #if (SYS_CPNT_MGMT_PORT == TRUE) */

    lport_type = SWCTRL_POM_UserPortToIfindex(unit, port, ifindex_p);

    switch (lport_type)
    {
        case SWCTRL_LPORT_NORMAL_PORT:
            return CGI_TYPE_LPORT_NORMAL_PORT;

        case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            return CGI_TYPE_LPORT_TRUNK_PORT_MEMBER;

        case SWCTRL_LPORT_UNKNOWN_PORT:
        default:
            break;
    }

    return CGI_TYPE_LPORT_UNKNOWN_PORT;
}

/* FUNCTION NAME: CGI_UTIL_IfindexToUserPortStr
 * PURPOSE: Convert ifindex to user port display string.
 * INPUT:  ifindex -- interface index
 *         cb_str  -- count of bytes of str
 * OUTPUT: str -- display string
 * RETURN: None
 */
BOOL_T CGI_UTIL_IfindexToUserPortStr (UI32_T ifindex, char *str, UI32_T cb_str)
{
    SWCTRL_Lport_Type_T  port_type;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk;

    port_type = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk);

    switch (port_type)
    {
        case SWCTRL_LPORT_NORMAL_PORT:
        case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            SYSFUN_Snprintf(str, cb_str, "Eth %lu / %lu", unit, port);
            break;

        case SWCTRL_LPORT_TRUNK_PORT:
            SYSFUN_Snprintf(str, cb_str, "Trunk %lu", trunk);
            break;

        default:
            return FALSE;
    }

    str[cb_str-1] = '\0';
    return TRUE;
}

/* FUNCTION NAME: CGI_UTIL_IfindexToRestPortStr
 * PURPOSE: Convert ifindex to user port display string.
 * INPUT:  ifindex -- interface index
 * OUTPUT: str -- display string
 * RETURN: None
 */
BOOL_T CGI_UTIL_IfindexToRestPortStr(UI32_T ifindex, char *str)
{
    SWCTRL_Lport_Type_T  port_type;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk;

    port_type = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk);

    switch (port_type)
    {
        case SWCTRL_LPORT_NORMAL_PORT:
        case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            sprintf(str, "eth%lu/%lu", unit, port);
            break;

        case SWCTRL_LPORT_TRUNK_PORT:
            sprintf(str, "trunk%lu", trunk);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

BOOL_T CGI_UTIL_CheckUnitExist (UI32_T nUnit)
{
    return STKTPLG_POM_UnitExist(nUnit);
}

BOOL_T CGI_UTIL_CheckPortExist (UI32_T nUnit, UI32_T nPort)
{
    UI32_T nIfIndex = 1;

    /* For ES5508-08-00054, if SYS_CPNT_MGMT_PORT == TRUE */
#if (SYS_CPNT_MGMT_PORT == TRUE)
    if (nPort == SYS_ADPT_MGMT_PORT)
    {
        return FALSE;
    }
#endif /* #if (SYS_CPNT_MGMT_PORT == TRUE) */

    if (SWCTRL_POM_UserPortToIfindex(nUnit, nPort, &nIfIndex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL_T CGI_UTIL_CheckPortExistNoTrunk (UI32_T nUnit, UI32_T nPort)
{
    UI32_T nIfIndex = 1;

    /* For ES5508-08-00054, if SYS_CPNT_MGMT_PORT == TRUE */
#if (SYS_CPNT_MGMT_PORT == TRUE)
    if (nPort == SYS_ADPT_MGMT_PORT)
    {
        return FALSE;
    }
#endif /* #if (SYS_CPNT_MGMT_PORT == TRUE) */

    if (SWCTRL_POM_UserPortToIfindex(nUnit, nPort, &nIfIndex) != SWCTRL_LPORT_NORMAL_PORT)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL_T CGI_UTIL_CheckTrunkExist (UI32_T nTrunk)
{
    TRK_MGR_TrunkEntry_T trunk_entry;
    memset(&trunk_entry, 0, sizeof(TRK_MGR_TrunkEntry_T));

    trunk_entry.trunk_index = nTrunk;
    return TRK_PMGR_GetTrunkEntry(&trunk_entry);
}

BOOL_T CGI_UTIL_CheckTrunkExistPort (UI32_T nTrunk)
{
    if (CGI_UTIL_CheckTrunkExist (nTrunk) != TRUE)
    {
        return FALSE;
    }

    if (TRK_PMGR_GetTrunkMemberCounts(nTrunk) == 0)
    {
        return FALSE;
    }

    return TRUE;
}

void CGI_UTIL_CheckAnyIPV6Addr (UI8_T *buff, UI8_T *ipv6, UI32_T pfx_len)
{
    UI32_T  i, ret =0;
    char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1];

    if (buff != NULL)
    {
        /* improve here */
        for (i =0; i < SYS_ADPT_IPV6_ADDR_LEN; i++)
        {
            if (ipv6 [i] != 0)
                ret =1;
        }

        if (ret == 0)
            strcpy (buff, "Any");
        else
        {
            L_INET_Ntop(L_INET_AF_INET6, (void *) ipv6, ipv6_addr_str, sizeof(ipv6_addr_str));

            /* may need to improve for dst-prefix-length */
            if (pfx_len == RULE_TYPE_MAX_IPV6_PREFIX_LEN)
            {
                sprintf (buff, "%s", ipv6_addr_str);

            }
            else
            {   /* ipv6-adr/prefix-len */
                sprintf (buff, "%s/%lu", ipv6_addr_str, pfx_len);
            }
        }
    }
}

/* FUNCTION NAME: CGI_UTIL_ReplaceString
 * PURPOSE: replace a char to a specified string in result string
 * INPUT:  source - source string
 *         srch - which char to replace
 *         tgstr - replace char to specified string
 * OUTPUT: result
 * RETURN: none
 */
void CGI_UTIL_ReplaceString(char *source, char srch, char *tgstr, char *result)
{
    int  i, j;
    int  tglen = strlen(tgstr);

    j = 0;

    for(i = 0; i < strlen(source); i++)
    {
        if (source[i] == srch)
        {
            memcpy(&result[j], tgstr, tglen);
            j += tglen;
        }
        else
        {
            result[j] = source[i];
            j++;
        }
    }
}

BOOL_T CGI_UTIL_StrToNonprintable(char *str, UI8_T *om)
{
   UI32_T  i;
   char    buff[3];

   if (strlen(str) == SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN*2)
   {
      for (i = 0 ; i <SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN; i++, om++)
      {
         buff[0] = *(str);
         buff[1] = *(str+1);
         buff[2] = 0;

         *om = (UI8_T)CGI_UTIL_StrToUL(buff,16);
         str += 2;
      }

      *om = 0;
   }
   else
   {
      return FALSE;
   }

   return TRUE;
}

#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_OSPF == TRUE)
/* FUNCTION NAME: CGI_UTIL_ConvertAreaToFormat
 * PURPOSE: Convert OSPF area ID to UI32 format
 * INPUT:  UI8_T buf - input string
 * OUTPUT: UI32_T areaId, areaFormat
 * RETURN: none
 */
void CGI_UTIL_ConvertAreaToFormat (UI8_T *buf, UI32_T *areaId, UI32_T *areaFormat)
{
    int ipAddr[CGI_IPLEN] = {0};

    if ((sscanf(buf, "%d.%d.%d.%d,",
            &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3])) == 4)
    {
        L_INET_Aton(buf, areaId);
        *areaFormat = NETCFG_TYPE_OSPF_AREA_ID_FORMAT_ADDRESS;
    }
    else
    {
        *areaId = atol(buf);
        *areaFormat = NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DECIMAL;
    }
}

#endif /* #if (SYS_CPNT_ROUTING == TRUE) */
#endif /*#if (SYS_CPNT_OSPF == TRUE) */

// TODO: Use std C library instead. -- strchr()
//       Also need to check the caller, why need this function ?
BOOL_T CGI_UTIL_IsMeetDelemiter(char c, char delemiters[])
{
   UI32_T i = 0;

   for(; delemiters[i] != 0; i ++)
   {
      if (c == delemiters[i])
         return TRUE;
   }

   return FALSE;
}

/* PURPOSE: Get a string from a line
 * INPUT:   s:     Starting address to search
 * OUTPUT:  Token: Buffer of the found token
 * RETURN:  The address that follow the got string, if 0 => no next string
 * NOTE:
 */
char *CGI_UTIL_GetToken (char *s, char *Token, char delemiters[])
{

   int  state = 0;

   for( ; *s !=0 || *s == 0x0a || *s == 0x0d ; s++)
   {
      switch ( state )
      {
         case 0:   /* skip while space */
         if ( CGI_UTIL_IsMeetDelemiter(*s, delemiters) )
         {
            continue;
         }
         else
         {
            state = 1;
            *Token++ = *s;
            break;
         }

         case 1:   /* character occures */
         if ( CGI_UTIL_IsMeetDelemiter(*s, delemiters) )
         {
            char *s_op = s + 1;
            *Token = NULL;

            for(; ; s_op++) /* search if non-space exists */
            {
               if ( *s_op == NULL || *s_op == 0x0a || *s_op == 0x0d )
                  return 0;
               else if (!CGI_UTIL_IsMeetDelemiter(*s_op, delemiters))
                  break;
            }
            return s;
         }
         else
         {
            *Token++ = *s;
            break;
         }
      }
   }
   return 0; /* if (*s == 0) occures */
}

BOOL_T CGI_UTIL_GetLowerUpperValue(char *buff, UI32_T *lower_val, UI32_T *upper_val, UI32_T *err_idx)
{
   UI32_T i;
   UI32_T case_val = 0;

   for(i=0; buff[i]!= 0; i++)
   {
      switch(case_val)
      {
      case 0:                  /* find lower value */
         if (isdigit(buff[i]))
         {
            *lower_val = atoi(buff+i);
            *upper_val = atoi(buff+i);
            case_val = 1;

            for(;;i++)
            {
               if (!isdigit(buff[i]))
               {
                  i--;
                  break;
               }
            }
         }
         else /*not digit*/
         {
            *err_idx = i;
            return FALSE;
         }
         break;


      case 1:                  /* find dash */
         if (buff[i] == '-')
         {
            case_val = 2;
            *upper_val = WAIT_FOR_VALUE;
         }
         else
         {
            *err_idx = i; /*not dash*/
            return FALSE;
         }
         break;


      case 2:                /* find upper value */
         if (isdigit(buff[i]))
         {
            *upper_val = atoi(buff+i);
            case_val = 3;

            for(;;i++)
            {
               if (!isdigit(buff[i]))
               {
                  i--;
                  break;
               }
            }
         }
         else /*not digit*/
         {
            *err_idx = i;
            return FALSE;
         }
         break;

      case 3:                /* any thing else? */
         *err_idx = i;
         return FALSE;
      }
   }

   if (*upper_val == WAIT_FOR_VALUE)
      return FALSE;
   else
   {
      if (*upper_val < *lower_val)
         return FALSE;
      else
         return TRUE;
   }
}

#if (SYS_CPNT_CFM == TRUE)
void CGI_UTIL_ParseCfmVlanList(char *input_p, UI32_T *primary_vid, UI32_T *vid_num, UI8_T *vid_list)
{
    UI32_T  err_idx=0;
    UI32_T  lower_val = 0, upper_val = 0;
    UI32_T  vlan=0;
    char    Token[CGI_CFM_DEF_MAX_BUFSIZE] = {0};
    char    delemiters[2] = { 0 };
    BOOL_T  first_vid=TRUE;

    delemiters[0] = ',';

    do
    {
       memset(Token, 0, CGI_CFM_DEF_MAX_BUFSIZE);

       input_p = CGI_UTIL_GetToken(input_p, Token, delemiters);

       if (!CGI_UTIL_GetLowerUpperValue(Token, &lower_val, &upper_val, &err_idx))
           break;

       for(vlan=lower_val; vlan<=upper_val; vlan++)
       {
           if (TRUE == first_vid)
           {
               *primary_vid=vlan;
               first_vid=FALSE;
           }

           vid_list[(UI32_T)((vlan-1)/8)] |= (1 << (7 - ((vlan-1)%8)));

           (*vid_num)++;
       }

    }while ((input_p != 0) && !isspace(*input_p));
}
#endif /* #if (SYS_CPNT_CFM == TRUE) */

/* FUNCTION NAME: CGI_UTIL_GetPortMax
 * PURPOSE: Get the max port number
 * INPUT:   unit - device unit
 * OUTPUT:  port_number_p - max port number
 * RETURN:  TRUE ; FALSE
 * NOTE:
 */
BOOL_T CGI_UTIL_GetPortMax (UI32_T unit, UI32_T *port_number_p)
{
    STKTPLG_MGR_Switch_Info_T   switch_info;

    memset(&switch_info, 0, sizeof(STKTPLG_MGR_Switch_Info_T));
    switch_info.sw_unit_index = unit;

    if (STKTPLG_PMGR_GetSwitchInfo(&switch_info) != TRUE)
    {
        return FALSE;
    }

    *port_number_p = (UI32_T)switch_info.sw_port_number;
    return TRUE;
}

/* FUNCTION NAME: CGI_UTIL_IsInteger
 * PURPOSE: Check the string is a integer.
 * INPUT:   str_p - pointer of input string (ASCII format)
 * RETURN:  TRUE ; FALSE
 * NOTE:
 */
BOOL_T CGI_UTIL_IsInteger (const char *str_p)
{
    const char *p = str_p;

    if (0 == p)
    {
       return FALSE;
    }

    if (('-' == *p) || ('+' == *p))
    {
        ++p;
    }

    if (0 == *p)
    {
       return FALSE;
    }

    for (; 0 != *p; ++p)
    {
       if (L_STDLIB_IsDigit(*p) != TRUE)
       {
           return FALSE;
       }
    }

    return TRUE;
}

//
// TODO: This function is not good. Need to check more.
//       this check shall be moved to getToken
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
UI16_T CGI_UTIL_CheckNullStr(char *buff)
{
    while ((*buff == ' ') || (*buff == 0x09))
    {
        buff++;
    }

    if (*buff == '\0')
    {
        return 1; /* null string */
    }
    else
    {
        return 0; /* not null string */
    }
}

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
void CGI_UTIL_ConvertTextToHex(UI8_T *input, UI32_T input_len, char *out_buf, size_t out_buf_len)
{
    size_t  i;
    int     len;
    size_t  total_len = out_buf_len;
    char    *p = out_buf;

    for (i = 0, len = 0; i < input_len; ++ i) {
        len = snprintf(p, total_len, "%02x", input[i]);

        if (len <= 0) {
            break;
        }

        ASSERT(len <= total_len);
        if (total_len < len) {
            break;
        }

        total_len -= len;
        p += len;

        if (total_len == 0) {
            break;
        }
    }

    *p = '\0';
}

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
BOOL_T CGI_UTIL_InterfaceIdToEth(const char *str, UI32_T *unit, UI32_T *port)
{
    char *p;
    if (strlen(str) < 6)
        return FALSE;
    if (memcmp(str, "eth", 3) != 0)
        return FALSE;
    if (NULL == (p = strchr(str, '/')))
        return FALSE;
    *unit = atoi(str + 3);
    *port = atoi(p + 1);
    if (*unit == 0 || *port == 0)
        return FALSE;
    return TRUE;
}

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
BOOL_T CGI_UTIL_InterfaceIdToLport(const char *str, UI32_T *lport)
{
    SWCTRL_Lport_Type_T lport_type;
    UI32_T unit = 0, port = 0;
    UI32_T trunk_id = 0;

    if (TRUE == CGI_UTIL_InterfaceIdToEth(str, &unit, &port))
    {
        lport_type = SWCTRL_POM_UserPortToIfindex(unit, port, lport);
        return TRUE;
    }

    if (strlen(str) < strlen("trunkx"))
    {
        return FALSE;
    }

    if (memcmp(str, "trunk", 5) != 0)
    {
        return FALSE;
    }

    trunk_id = atoi(str + 5);
    return SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, lport);
}

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
void CGI_UTIL_UrlDecode(char *dst, const char *src)
{
    char a, b;
    while (*src)
    {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b)))
        {
            if (a >= 'a')
                a -= 'a'-'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';

            if (b >= 'a')
                b -= 'a'-'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        }
        else if (*src == '+')
        {
            *dst++ = ' ';
            src++;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

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
UI32_T CGI_UTIL_MaskToPrefix(UI8_T *mask_p)
{
    int i, j;
    UI32_T prefix = 0;
    UI8_T mask_byte;

    /* BODY
     */
    for(i=0; i<SYS_ADPT_MAC_ADDR_LEN; i++)
    {
        mask_byte = mask_p[i];

        for(j=0; j<8;j++)
        {
            if( mask_byte & 0x80)
            {
                prefix++;
                mask_byte = mask_byte << 1;
            }
            else
            {
                break;
            }
        } /* j */
    } /* i */

    return prefix;
}  /* End of CGI_UTIL_MaskToPrefix */

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
void CGI_UTIL_PrefixToMask(UI32_T prefix, UI8_T *mask_p)
{
    UI8_T *pnt_p;
    UI8_T maskbit[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
    int bit;
    int offset;

    if (prefix > 48)
    {
        return;
    }

    pnt_p = (UI8_T *) mask_p;

    offset = (prefix / 8);
    bit = (prefix % 8);

    while (offset--)
    {
        *pnt_p++ = 0xff;
    }

    if (bit)
        *pnt_p = maskbit[bit];

    return;
}

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
BOOL_T CGI_UTIL_ExecLinuxSetCommand(char *cmd_p)
{
    FILE* fp_p;
    char buff_ar[CGI_UTIL_LINUX_RETURN_BUFFER_SIZE] = {0};

    fp_p = popen(cmd_p, "w");

    if (fp_p == NULL)
    {
        printf("ERROR, can execute command %s\r\n", cmd_p);
        return FALSE;
    }

    while (fgets(buff_ar, CGI_UTIL_LINUX_RETURN_BUFFER_SIZE, fp_p) != NULL)
    {
        printf("%s", buff_ar);
    }

    pclose(fp_p);
    return TRUE;
} //CGI_UTIL_ExecLinuxSetCommand

