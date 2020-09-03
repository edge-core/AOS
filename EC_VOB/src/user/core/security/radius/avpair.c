
#include "radiusclient.h"
#include "sys_module.h"
#include "l_mm.h"
#include "l_stdlib.h"
#include <string.h>
#include <stdio.h>
#include "sys_cpnt.h"
#include "radius_om.h"

#if (SYS_CPNT_EH == TRUE)
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#endif

/* define AVPAIR_DEBUG_MODE to build Avpair.c with DEBUG version
 * And let following macros print debug messages
 */
#define AVPAIR_DEBUG_MODE

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef AVPAIR_DEBUG_MODE

    #define AVPAIR_TRACE(msg)                       RADIUS_OM_DEBUG_PRINT0(RADIUS_DEBUG_AVPAIR, msg)
    #define AVPAIR_TRACE1(msg, arg)                 RADIUS_OM_DEBUG_PRINT1(RADIUS_DEBUG_AVPAIR, msg, arg)
    #define AVPAIR_TRACE2(msg, arg1, arg2)          RADIUS_OM_DEBUG_PRINT2(RADIUS_DEBUG_AVPAIR, msg, arg1, arg2)
    #define AVPAIR_TRACE3(msg, arg1, arg2, arg3)    RADIUS_OM_DEBUG_PRINT3(RADIUS_DEBUG_AVPAIR, msg, arg1, arg2, arg3)

#else

    #define AVPAIR_TRACE(msg)                       ((void)0)
    #define AVPAIR_TRACE1(msg, arg)                 ((void)0)
    #define AVPAIR_TRACE2(msg, arg1, arg2)          ((void)0)
    #define AVPAIR_TRACE3(msg, arg1, arg2, arg3)    ((void)0)

#endif /* AVPAIR_DEBUG_MODE */

/*
 * Function: rc_avpair_add
 *
 * Purpose: add an attribute-value pair to the given list.
 *
 * Returns: pointer to added a/v pair upon success, NULL pointer upon failure.
 *
 * Remarks: Always appends the new pair to the end of the list.
 *
 */

VALUE_PAIR *rc_avpair_add (VALUE_PAIR **list, int attrid, void *pval, int len)
{
    VALUE_PAIR     *vp;
    int i;

    /* suger, 05-04-2004, reassembling the EAP packet
     * Since the max RADIUS Attribute octets is 253,
     * when the EAP packet length receive from supplicant is larger than 253,
     * it should be divided into multiple consecutive EAP MESSAGE in RADIUS Attribute
     * by length 253 to send to RADIUS server.
     */
    if (len > 253)
    {
        for(i=0; i<(len/253); i++)
        {
            vp = rc_avpair_new (attrid, (char *)pval+253*i, 253);
            if (vp != (VALUE_PAIR *) NULL)
            {
                rc_avpair_insert (list, (VALUE_PAIR *) NULL, vp);
            }
        }
        vp = rc_avpair_new(attrid, (char *)pval + 253 * i, len % 253);
        if (vp != (VALUE_PAIR *) NULL)
        {
            rc_avpair_insert (list, (VALUE_PAIR *) NULL, vp);
        }
    }
    else
    {
        vp = rc_avpair_new (attrid, pval, len);

        if (vp != (VALUE_PAIR *) NULL)
        {
            rc_avpair_insert (list, (VALUE_PAIR *) NULL, vp);
        }
    }

    return vp;
}

/*
 * Function: rc_avpair_assign
 *
 * Purpose: assign the given value to an attribute-value pair.
 *
 * Returns:  0 on success,
 *      -1 on failure.
 *
 */

int rc_avpair_assign (VALUE_PAIR *vp, void *pval, int len)
{
    int result = -1;

    switch (vp->type)
    {
        case PW_TYPE_STRING:

            if (((len == 0) && (strlen ((char *) pval)) > AUTH_STRING_LEN)
                || (len > AUTH_STRING_LEN)) {
         /*                       printf("rc_avpair_assign: bad attribute length");*/
                    return result;
            }

            if (len > 0) {

                    memcpy(vp->strvalue, (char *)pval, len);
                vp->strvalue[len] = '\0';
                vp->lvalue = len;
            } else {
                strncpy (vp->strvalue, (char *) pval, AUTH_STRING_LEN);
                vp->lvalue = strlen((char *) pval);
            }

            result = 0;
            break;

        case PW_TYPE_DATE:
        case PW_TYPE_INTEGER:
        case PW_TYPE_IPADDR:

            vp->lvalue = * (UI32_T *) pval;

            result = 0;
            break;

        case PW_TYPE_IP6ADDR:

            memcpy(vp->strvalue, (char *)pval, SYS_ADPT_IPV6_ADDR_LEN);
            vp->lvalue = SYS_ADPT_IPV6_ADDR_LEN;

            result = 0;
            break;

        default:
/*                      printf("rc_avpair_assign: unknown attribute %d", vp->type);*/
            break;
    }
    return result;
}

/*
 * Function: rc_avpair_new
 *
 * Purpose: make a new attribute-value pair with given parameters.
 *
 * Returns: pointer to generated a/v pair when successful, NULL when failure.
 *
 */

VALUE_PAIR *rc_avpair_new (int attrid, void *pval, int len)
{
    VALUE_PAIR     *vp = (VALUE_PAIR *) NULL;
    DICT_ATTR      *pda;

    if ((pda = rc_dict_getattr (attrid)) == (DICT_ATTR *) NULL)
    {
       /*  printf("rc_avpair_new: unknown attribute %d",attrid);*/
    }
    else
    {
        if ((vp = (VALUE_PAIR *) L_MM_Malloc (sizeof (VALUE_PAIR), L_MM_USER_ID2(SYS_MODULE_RADIUS, RADIUS_TYPE_TRACE_ID_RC_AVPAIR_NEW)))
                            != (VALUE_PAIR *) NULL)
        {
            strncpy (vp->name, pda->name, sizeof (vp->name));
            vp->attribute = attrid;
            vp->next = (VALUE_PAIR *) NULL;
            vp->type = pda->type;
            if (rc_avpair_assign (vp, pval, len) == 0)
            {
                return vp;
            }
#if (SYS_CPNT_EH == TRUE)
            EH_MGR_Handle_Exception(SYS_MODULE_RADIUS, RADIUS_RC_AVPAIR_NEW_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));/*Mercury_V2-00030*/
#endif
            L_MM_Free (vp);
            vp = (VALUE_PAIR *) NULL;
        }
        else
        {
            /*  printf("rc_avpair_new: out of memory");*/
            vp = (VALUE_PAIR *) NULL;
        }
    }
    return vp;
}


VALUE_PAIR *rc_vendor_avpair_gen(VALUE_PAIR *pair, unsigned char *ptr, int length, int vendorpec);

/*
 *
 * Function: rc_avpair_gen
 *
 * Purpose: takes attribute/value pairs from buffer and builds a
 *      value_pair list using allocated memory.
 *
 * Returns: value_pair list or NULL on failure
 */

VALUE_PAIR *rc_avpair_gen (AUTH_HDR *auth)
{
/* suger, 05-12-2004, mark off the debugging code */
    int             length;
//  int             x_len;
    int             attribute;
    int             attrlen;
    UI32_T          lvalue;
//  UI8_T         *x_ptr;
    UI8_T           *ptr;
    DICT_ATTR       *attr;
    VALUE_PAIR      *vp;
    VALUE_PAIR      *pair;
//  UI8_T          hex[3];      /* For hex string conversion. */
//  char            buffer[256];

    /*
     * Extract attribute-value pairs
     */
    ptr = auth->data;
    length = L_STDLIB_Ntoh16((unsigned short) auth->length) - AUTH_HDR_LEN;

    vp = (VALUE_PAIR *) NULL;

    while (length > 0)
    {
        attribute = *ptr++;
        attrlen = *ptr++;
        attrlen -= 2;
        if (attrlen < 0)
        {
            AVPAIR_TRACE("Received attribute with invalid length");
            break;
        }

        if ((attr = rc_dict_getattr (attribute)) == (DICT_ATTR *) NULL)
        {
            /* suger, 05-12-2004, mark off the debugging code,
             * because when receiving a attribute data of large length,
             * this will cause memory corrupt.
             * otherwise we have to enlarge the buffer size.
             */
            /*
            *buffer= '\0';
            for (x_ptr = ptr, x_len = attrlen ;
                x_len > 0 ;
                x_len--, x_ptr++)
            {
                sprintf (hex, "%2.2X", *x_ptr);
                strcat (buffer, hex);
            }
            */
            /*   printf("rc_avpair_gen: received unknown attribute %d of length %d: 0x%s",attribute, attrlen, buffer); */
        }
        else
        {
            /* VSA */
            if (attribute == PW_VENDOR_SPECIFIC)
            {
                int vendorpec;

                if (attrlen < 4)
                {
                    /*RADIUS_OM_DEBUG_PRINT(SECURITY_DEBUG_TYPE_ERR, "Received VSA attribute with invalid length");*/
                    rc_avpair_free(vp);
                    return NULL;
                }
                memcpy(&lvalue, ptr, 4);
                vendorpec = L_STDLIB_Ntoh32(lvalue);

                if (vendorpec == 9) /*cisco*/
                {
                    /*RADIUS_OM_DEBUG_PRINT(SECURITY_DEBUG_TYPE_ERR_TRC, "Received Cisco VSA attribute");*/
                    vp = rc_vendor_avpair_gen(vp, ptr + 4, attrlen - 4, vendorpec);
                }
                else
                {
                    /* Warn and skip over the unknown VSA */
                    /*RADIUS_OM_DEBUG_PRINT1(SECURITY_DEBUG_TYPE_ERR_TRC, "Received VSA attribute with unknown Vendor-Id %d", vendorpec);*/
                }

                /* Next
                 */
                ptr += attrlen;
                length -= attrlen + 2;
                continue;
            }

            pair = (VALUE_PAIR *) L_MM_Malloc(
                sizeof (VALUE_PAIR), L_MM_USER_ID2(SYS_MODULE_RADIUS, RADIUS_TYPE_TRACE_ID_RC_AVPAIR_GEN));

            if (pair ==  NULL)
            {
                AVPAIR_TRACE("Out of Memory");
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception(SYS_MODULE_RADIUS, RADIUS_RC_AVPAIR_GEN_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));/*Mercury_V2-00030*/
#endif
                rc_avpair_free(vp);
                return NULL;
            }
            strncpy (pair->name, attr->name, sizeof(pair->name));
            pair->attribute = attr->value;
            pair->type = attr->type;
            pair->next = (VALUE_PAIR *) NULL;

            switch (attr->type)
            {
            case PW_TYPE_STRING:
                memcpy (pair->strvalue, (char *) ptr, attrlen);
                pair->strvalue[attrlen] = '\0';
                pair->lvalue = attrlen;
                rc_avpair_insert (&vp, (VALUE_PAIR *) NULL, pair);

                AVPAIR_TRACE2("AV-Pair: %s=%s", pair->name, pair->strvalue);
                break;

            case PW_TYPE_INTEGER:
            case PW_TYPE_IPADDR:
                memcpy ((char *) &lvalue, (char *) ptr, sizeof (UI32_T));
                pair->lvalue = L_STDLIB_Ntoh32(lvalue);
                rc_avpair_insert (&vp, (VALUE_PAIR *) NULL, pair);

                AVPAIR_TRACE2("AV-Pair: %s=%lu", pair->name, pair->lvalue);
                break;

            default:
                AVPAIR_TRACE1("%s has unknow type", pair->name);
                L_MM_Free (pair);
                break;
            }

        }
        ptr += attrlen;
        length -= attrlen + 2;
    }
    return (vp);
}

#define VENDOR(x)       (((x) >> 16) & 0xffff)

VALUE_PAIR *rc_vendor_avpair_gen(VALUE_PAIR *pair, unsigned char *ptr, int length, int vendorpec)
{
    int attribute, attrlen;
    UI32_T          lvalue;
    DICT_ATTR      *attr;
    VALUE_PAIR     *rpair;

    if (length < 2) {
        /*RADIUS_OM_DEBUG_PRINT(SECURITY_DEBUG_TYPE_ERR, "Rreceived attribute with invalid length");*/
        rc_avpair_free(pair);
        return NULL;
    }

    attrlen = ptr[1];
    if (length < attrlen) {
        /*RADIUS_OM_DEBUG_PRINT(SECURITY_DEBUG_TYPE_ERR, "Received attribute with invalid length");*/
        rc_avpair_free(pair);
        return NULL;
    }

    /* Advance to the next attribute and process recursively */
    if (length != attrlen) {
        pair = rc_vendor_avpair_gen(pair, ptr + attrlen, length - attrlen, vendorpec);
        if (pair == NULL)
            return NULL;
    }

    /* Actual processing */
    attribute = ptr[0] | (vendorpec << 16);
    ptr += 2;
    attrlen -= 2;

    /* Normal */
    attr = rc_dict_getattr(attribute);
    if (attr == NULL)
    {
#ifdef RADIUS_OM_DEBUG_MODE
        char           buffer[256];
        unsigned char *x_ptr;
        int            x_len;
        char           hex[3];

        buffer[0] = '\0';   /* Initial length. */
        x_ptr = ptr;
        for (x_len = attrlen; x_len > 0; x_len--, x_ptr++) {
            sprintf(hex, "%2.2X", x_ptr[0]);
            strcat(buffer, hex);
    }
        if (vendorpec == 0) {

            /*RADIUS_OM_DEBUG_PRINT3(SECURITY_DEBUG_TYPE_ERR_TRC, "Received unknown attribute %d of length %d: 0x%s",
            attribute, attrlen + 2, buffer);*/
        } else {
            /*RADIUS_OM_DEBUG_PRINT4(SECURITY_DEBUG_TYPE_ERR_TRC, "Received unknown VSA attribute %d, vendor %d of length %d: 0x%s",
                attribute & 0xffff, VENDOR(attribute), attrlen + 2, buffer);*/
        }
#endif /*#ifdef RADIUS_OM_DEBUG_MODE*/

        rc_avpair_free(pair);
        return NULL;
    }

    rpair = (VALUE_PAIR *) L_MM_Malloc(
        sizeof (VALUE_PAIR), L_MM_USER_ID2(SYS_MODULE_RADIUS, RADIUS_TYPE_TRACE_ID_RC_VENDOR_AVPAIR_GEN));

    if (rpair == NULL)
    {
        /*RADIUS_OM_DEBUG_PRINT(SECURITY_DEBUG_TYPE_ERR, "Out of memory\r\n");*/
        rc_avpair_free(pair);
        return NULL;
    }

    memset(rpair, '\0', sizeof(*rpair));

    /* Insert this new pair at the beginning of the list */
    strcpy(rpair->name, attr->name);
    rpair->attribute = attr->value;
    rpair->type = attr->type;
    rpair->next = NULL;
    rc_avpair_insert (&pair, (VALUE_PAIR *) NULL, rpair);

/*printf("%s get VSA name=%s attr=%ld\r\n", __FUNCTION__, attr->name, attr->value);*/

    switch (attr->type)
    {
    case PW_TYPE_STRING:
        memcpy(rpair->strvalue, (char *)ptr, attrlen);
        rpair->strvalue[attrlen] = '\0';
        rpair->lvalue = attrlen;
        break;

    case PW_TYPE_INTEGER:
        if (attrlen != 4) {
            /*RADIUS_OM_DEBUG_PRINT(SECURITY_DEBUG_TYPE_ERR, "Received INT attribute with invalid length");*/
            rc_avpair_free(pair);
            return NULL;
        }
    case PW_TYPE_IPADDR:
        if (attrlen != 4) {
            /*RADIUS_OM_DEBUG_PRINT(SECURITY_DEBUG_TYPE_ERR, "Received IPADDR attribute with invalid length");*/
            rc_avpair_free(pair);
            return NULL;
        }
        memcpy((char *)&lvalue, (char *)ptr, 4);
        rpair->lvalue = L_STDLIB_Ntoh32(lvalue);
        break;

    default:
        /*RADIUS_OM_DEBUG_PRINT1(SECURITY_DEBUG_TYPE_ERR, "%s has unknown type", attr->name);*/
        rc_avpair_free(pair);
        return NULL;
    }

    return pair;
}

/*
 * Function: rc_avpair_get
 *
 * Purpose: Find the first attribute value-pair (which matches the given
 *          attribute) from the specified value-pair list.
 *
 * Returns: found value_pair
 *
 */

VALUE_PAIR *rc_avpair_get (VALUE_PAIR *vp, UI32_T attr)
{
    for (; vp != (VALUE_PAIR *) NULL && vp->attribute != attr; vp = vp->next)
    {
        continue;
    }
    return (vp);
}

/*
 * Function: rc_avpair_insert
 *
 * Purpose: Given the address of an existing list "a" and a pointer
 *      to an entry "p" in that list, add the value pair "b" to
 *      the "a" list after the "p" entry.  If "p" is NULL, add
 *      the value pair "b" to the end of "a".
 *
 */

void rc_avpair_insert (VALUE_PAIR **a, VALUE_PAIR *p, VALUE_PAIR *b)
{
    VALUE_PAIR     *this_node = NULL;
    VALUE_PAIR     *vp;

    if (b->next != (VALUE_PAIR *) NULL)
    {
           /*   printf("rc_avpair_insert: value pair (0x%p) next ptr. (0x%p) not NULL", b, b->next);  */
        abort ();
    }

    if (*a == (VALUE_PAIR *) NULL)
    {
        *a = b;
        return;
    }

    vp = *a;

    if ( p == (VALUE_PAIR *) NULL) /* run to end of "a" list */
    {
        while (vp != (VALUE_PAIR *) NULL)
        {
            this_node = vp;
            vp = vp->next;
        }
    }
    else /* look for the "p" entry in the "a" list */
    {
        this_node = *a;
        while (this_node != (VALUE_PAIR *) NULL)
        {
            if (this_node == p)
            {
                break;
            }
            this_node = this_node->next;
        }
    }

    b->next = this_node->next;
    this_node->next = b;

    return;
}

/*
 * Function: rc_avpair_free
 *
 * Purpose: frees all value_pairs in the list
 *
 */

void rc_avpair_free (VALUE_PAIR *pair)
{
    VALUE_PAIR     *next;

    while (pair != (VALUE_PAIR *) NULL)
    {
        next = pair->next;
        L_MM_Free (pair);
        pair = next;
    }
}

