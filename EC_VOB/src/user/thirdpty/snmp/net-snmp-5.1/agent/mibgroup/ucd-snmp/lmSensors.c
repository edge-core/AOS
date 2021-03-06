/*
 * This file was generated by mib2c and is intended for use as a mib module
 * for the ucd-snmp snmpd agent. 
 */


/*
 * This should always be included first before anything else 
 */
#include <net-snmp/net-snmp-config.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/*
 * minimal include directives 
 */
#include "util_funcs.h"


#include <time.h>
#include <sensors/sensors.h>

#include "lmSensors.h"

#define N_TYPES      (4)
#define MAX_NAME     (64)
#define MAX_SENSORS  (32)

/*
 * lmSensors_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */


oid             lmSensors_variables_oid[] =
    { 1, 3, 6, 1, 4, 1, 2021, 13, 16 };

/*
 * variable4 lmSensors_variables:
 *   this variable defines function callbacks and type return information 
 *   for the lmSensors mib section 
 */

struct variable4 lmSensors_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix 
     */
#define   LMTEMPSENSORSINDEX    3
    {LMTEMPSENSORSINDEX, ASN_INTEGER, RONLY, var_lmSensorsTable, 3,
     {2, 1, 1}},
#define   LMTEMPSENSORSDEVICE   4
    {LMTEMPSENSORSDEVICE, ASN_OCTET_STR, RONLY, var_lmSensorsTable, 3,
     {2, 1, 2}},
#define   LMTEMPSENSORSVALUE    5
    {LMTEMPSENSORSVALUE, ASN_GAUGE, RONLY, var_lmSensorsTable, 3,
     {2, 1, 3}},
#define   LMFANSENSORSINDEX     8
    {LMFANSENSORSINDEX, ASN_INTEGER, RONLY, var_lmSensorsTable, 3,
     {3, 1, 1}},
#define   LMFANSENSORSDEVICE    9
    {LMFANSENSORSDEVICE, ASN_OCTET_STR, RONLY, var_lmSensorsTable, 3,
     {3, 1, 2}},
#define   LMFANSENSORSVALUE     10
    {LMFANSENSORSVALUE, ASN_GAUGE, RONLY, var_lmSensorsTable, 3,
     {3, 1, 3}},
#define   LMVOLTSENSORSINDEX    13
    {LMVOLTSENSORSINDEX, ASN_INTEGER, RONLY, var_lmSensorsTable, 3,
     {4, 1, 1}},
#define   LMVOLTSENSORSDEVICE   14
    {LMVOLTSENSORSDEVICE, ASN_OCTET_STR, RONLY, var_lmSensorsTable, 3,
     {4, 1, 2}},
#define   LMVOLTSENSORSVALUE    15
    {LMVOLTSENSORSVALUE, ASN_GAUGE, RONLY, var_lmSensorsTable, 3,
     {4, 1, 3}},
#define   LMMISCSENSORSINDEX    18
    {LMMISCSENSORSINDEX, ASN_INTEGER, RONLY, var_lmSensorsTable, 3,
     {5, 1, 1}},
#define   LMMISCSENSORSDEVICE   19
    {LMMISCSENSORSDEVICE, ASN_OCTET_STR, RONLY, var_lmSensorsTable, 3,
     {5, 1, 2}},
#define   LMMISCSENSORSVALUE    20
    {LMMISCSENSORSVALUE, ASN_GAUGE, RONLY, var_lmSensorsTable, 3,
     {5, 1, 3}},
};

typedef struct {
    char            name[MAX_NAME];
    int             value;
} _sensor;

typedef struct {
    int             n;
    _sensor         sensor[MAX_SENSORS];
} _sensor_array;

static _sensor_array sensor_array[N_TYPES];
static clock_t  timestamp;


static int      sensor_init(void);
static void     sensor_load(void);
static void     _sensor_load(clock_t t);

#define CONFIG_FILE_NAME "/etc/sensors.conf"

/*
 * init_lmSensors():
 *   Initialization routine.  This is called when the agent starts up.
 *   At a minimum, registration of your variables should take place here.
 */
void
init_lmSensors(void)
{
    sensor_init();

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("lmSensors", lmSensors_variables, variable4,
                 lmSensors_variables_oid);
}

/*
 * var_lmSensorsTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_lmSensors above.
 */
unsigned char  *
var_lmSensorsTable(struct variable *vp,
                   oid * name,
                   size_t * length,
                   int exact,
                   size_t * var_len, WriteMethod ** write_method)
{
    static long     long_ret;
    static unsigned char string[SPRINT_MAX_LEN];
    int             i;

    int             s_index = name[*length - 1] - 1;
    int             s_type = -1;
    int             n_sensors;

    _sensor         s;

    sensor_load();

    switch (vp->magic) {
    case LMTEMPSENSORSINDEX:
    case LMTEMPSENSORSDEVICE:
    case LMTEMPSENSORSVALUE:
        s_type = 0;
        n_sensors = sensor_array[0].n;
        break;

    case LMFANSENSORSINDEX:
    case LMFANSENSORSDEVICE:
    case LMFANSENSORSVALUE:
        s_type = 1;
        n_sensors = sensor_array[1].n;
        break;

    case LMVOLTSENSORSINDEX:
    case LMVOLTSENSORSDEVICE:
    case LMVOLTSENSORSVALUE:
        s_type = 2;
        n_sensors = sensor_array[2].n;
        break;

    case LMMISCSENSORSINDEX:
    case LMMISCSENSORSDEVICE:
    case LMMISCSENSORSVALUE:
        s_type = 3;
        n_sensors = sensor_array[3].n;
        break;

    default:
        s_type = -1;
        n_sensors = 0;
    }

    if (header_simple_table(vp, name, length, exact,
                            var_len, write_method,
                            n_sensors) == MATCH_FAILED)
        return NULL;

    if (s_type < 0)
        return NULL;

    s = sensor_array[s_type].sensor[s_index];

    switch (vp->magic) {
    case LMTEMPSENSORSINDEX:
    case LMFANSENSORSINDEX:
    case LMVOLTSENSORSINDEX:
    case LMMISCSENSORSINDEX:
        long_ret = s_index;
        return (unsigned char *) &long_ret;

    case LMTEMPSENSORSDEVICE:
    case LMFANSENSORSDEVICE:
    case LMVOLTSENSORSDEVICE:
    case LMMISCSENSORSDEVICE:
        strncpy(string, s.name, SPRINT_MAX_LEN - 1);
        *var_len = strlen(string);
        return (unsigned char *) string;

    case LMTEMPSENSORSVALUE:
    case LMFANSENSORSVALUE:
    case LMVOLTSENSORSVALUE:
    case LMMISCSENSORSVALUE:
        long_ret = s.value;
        return (unsigned char *) &long_ret;

    default:
        ERROR_MSG("Unable to handle table request");
    }

    return NULL;
}

static int
sensor_init(void)
{
    int             res;
    char            filename[] = CONFIG_FILE_NAME;
    clock_t         t = clock();

    FILE           *fp = fopen(filename, "r");
    if (!fp)
        return 1;

    if (res = sensors_init(fp))
        return 2;

    _sensor_load(t);
    return 0;
}

static void
sensor_load(void)
{
    clock_t         t = clock();

    if (t > timestamp + 60)
        _sensor_load(t);

    return;
}

static void
_sensor_load(clock_t t)
{
    const sensors_chip_name *chip;
    const sensors_feature_data *data;
    int             chip_nr = 0;
    int             a = 0;
    int             b = 0;

    int             i;
    for (i = 0; i < N_TYPES; i++)
        sensor_array[i].n = 0;

    while (chip = sensors_get_detected_chips(&chip_nr)) {
        while (data = sensors_get_all_features(*chip, &a, &b)) {
            char           *label;
            double          val;

            if ((data->mode & SENSORS_MODE_R) &&
                (data->mapping == SENSORS_NO_MAPPING) &&
                !sensors_get_label(*chip, data->number, &label) &&
                !sensors_get_feature(*chip, data->number, &val)) {
                int             type;
                float           mul;
                _sensor_array  *array;

                if (strstr(label, "temp")) {
                    type = 0;
                    mul = 1000.0;
                } else if (strstr(label, "fan")) {
                    type = 1;
                    mul = 1.0;
                } else if (strstr(label, "V")) {
                    type = 2;
                    mul = 1000.0;
                } else {
                    type = 3;
                    mul = 1000.0;
                }

                array = &sensor_array[type];

                strncpy(array->sensor[array->n].name, label, MAX_NAME);
                array->sensor[array->n].value = (int) (val * mul);
                array->n++;
            }
        }
    }

    timestamp = t;
}
