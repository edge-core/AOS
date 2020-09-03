#include "radiusclient.h"


static DICT_ATTR dict_attr_table[]={
{"User-Name",           1,PW_TYPE_STRING},
{"Password",            2,PW_TYPE_STRING},
{"CHAP-Password",       3,PW_TYPE_STRING},
{"NAS-IP-Address",      4,PW_TYPE_IPADDR},
{"NAS-Port-Id",         5,PW_TYPE_INTEGER},
{"Service-Type",        6,PW_TYPE_INTEGER},
{"Framed-Protocol",     7,PW_TYPE_INTEGER},
{"Framed-IP-Address",   8,PW_TYPE_IPADDR},
{"Framed-IP-Netmask",   9,PW_TYPE_IPADDR},
{"Framed-Routing",      10,PW_TYPE_INTEGER},
{"Filter-Id",           11,PW_TYPE_STRING},
{"Framed-MTU",          12,PW_TYPE_INTEGER},
{"Framed-Compression",  13,PW_TYPE_INTEGER},
{"Login-IP-Host",       14,PW_TYPE_IPADDR},
{"Login-Service",       15,PW_TYPE_INTEGER},
{"Login-TCP-Port",      16,PW_TYPE_INTEGER},
{"Reply-Message",       18,PW_TYPE_STRING},
{"Callback-Number",     19,PW_TYPE_STRING},
{"Callback-Id",         20,PW_TYPE_STRING},
{"Framed-Route",        22,PW_TYPE_STRING},
{"Framed-IPX-Network",  23,PW_TYPE_IPADDR},
{"State",               24,PW_TYPE_STRING},
{"Vendor-Specific",     26,PW_TYPE_STRING},
{"Session-Timeout",     27,PW_TYPE_INTEGER},
{"Idle-Timeout",        28,PW_TYPE_INTEGER},
{"Termination-Action",  29,PW_TYPE_INTEGER},
{"Called-Station-Id",   30,PW_TYPE_STRING},
{"Calling-Station-Id",  31,PW_TYPE_STRING},
{"NAS-Identifier",      32,PW_TYPE_STRING},
{"Acct-Status-Type",    40,PW_TYPE_INTEGER},
{"Acct-Delay-Time",     41,PW_TYPE_INTEGER},
{"Acct-Input-Octets",   42,PW_TYPE_INTEGER},
{"Acct-Output-Octets",  43,PW_TYPE_INTEGER},
{"Acct-Session-Id",     44,PW_TYPE_STRING},
{"Acct-Authentic",      45,PW_TYPE_INTEGER},
{"Acct-Session-Time",   46,PW_TYPE_INTEGER},
{"Acct-Input-Packets",  47,PW_TYPE_INTEGER},
{"Acct-Output-Packets", 48,PW_TYPE_INTEGER},
{"Acct-Terminate-Cause",49,PW_TYPE_INTEGER},
{"NAS-Port-Type",       61,PW_TYPE_INTEGER},
{"Port-Limit",          62,PW_TYPE_INTEGER},
{"Tunnel-Type",         64,PW_TYPE_INTEGER},
{"Tunnel-Medium-Type",  65,PW_TYPE_INTEGER},
{"Connect-Info",        77,PW_TYPE_STRING},
/* add for EAP */
{"EAP-Message",         79,PW_TYPE_STRING},
{"Message-Authenticator",80,PW_TYPE_STRING},
{"Tunnel-Private-Group",81,PW_TYPE_STRING},
{"NAS-Port-ID",         87,PW_TYPE_STRING},

{"NAS-IPv6-Address",    95,PW_TYPE_IP6ADDR},

/* vendor specific*/
/* cisco(9)*/
{"cisco-avpair",        ((9<<16)|1),PW_TYPE_STRING},
};
static int num_attr=((sizeof(dict_attr_table))/(sizeof(dict_attr_table[0])));

/*
 * Function: rc_dict_getattr
 *
 * Purpose: Return the full attribute structure based on the
 *      attribute id number.
 *
 */

DICT_ATTR *rc_dict_getattr (int attribute)
{
/*  DICT_ATTR      *attr;*/
        int i;
        for(i=0;i<num_attr;i++)
         {
           if(dict_attr_table[i].value== attribute)
              return &dict_attr_table[i];
         }
        return NULL;

/*
    attr = dictionary_attributes;
    while (attr != (DICT_ATTR *) NULL)
    {
        if (attr->value == attribute)
        {
            return (attr);
        }
        attr = attr->next;
    }
    return ((DICT_ATTR *) NULL);
*/
}

