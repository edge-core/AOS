
#include "alu_client_private.h"

#include "json_object_path.h"

#define ALU_CLIENT_DB_PREFIX                        "aluc"

json_t* alu_client_doc_new()
{
    json_t *doc;

    doc = json_object();

    json_object_path_set_new_2(doc, ALU_CLIENT_DB_PREFIX, "active", json_object());
    json_object_path_set_new_2(doc, ALU_CLIENT_DB_PREFIX, "licenses", json_object());

    return doc;
}

json_t *
alu_client_doc_get_license(
    const alu_client_doc_t *doc,
    const char *lic_no)
{
    json_t *object = json_object_path_get(doc->doc, 0, ALU_CLIENT_DB_PREFIX, "licenses", lic_no, NULL);

    if (object && json_typeof(object) == JSON_OBJECT) {
        return object;
    }

    return NULL;
}

json_t *
alu_client_doc_get_licenses(
    const alu_client_doc_t *doc)
{
    json_t *object = json_object_path_get(doc->doc, 0, ALU_CLIENT_DB_PREFIX, "licenses", NULL);

    if (object && json_typeof(object) == JSON_OBJECT) {
        return object;
    }

    return NULL;
}

int
alu_client_doc_remove_license(
    alu_client_doc_t *doc,
    const char *lic_no)
{
    json_t *licenses;

    licenses = alu_client_doc_get_licenses(doc);

    if (!licenses) {
        return -1;
    }

    json_object_del(licenses, lic_no);
    doc->drity = 1;
    return 0;
}

int
alu_client_doc_write_license(
    alu_client_doc_t *doc,
    const alu_client_license_t *lic)
{
    json_t *json_lic;

    ASSERT(doc);
    ASSERT(lic);

    if (!doc || !lic) {
        return -1;
    }

    json_lic = alu_client_doc_get_license(doc, lic->license_number);

    if (!json_lic) {
        json_lic = json_object();
        if (json_lic == NULL) {
            return -1;
        }

        json_object_path_set_new_3(doc->doc, ALU_CLIENT_DB_PREFIX, "licenses", lic->license_number, json_lic);
    }

    {
        json_t *total_time;

        total_time = json_object_get(json_lic, "total-time");

        if (!total_time || json_typeof(total_time) != JSON_INTEGER ||
            json_integer_value(total_time) != lic->stats.total_time) {
            json_object_set_new(json_lic, "total-time", json_integer(lic->stats.total_time));
            doc->drity = 1;
        }
    }

    {
        json_t *used_time;

        used_time = json_object_get(json_lic, "used-time");

        if (!used_time || json_typeof(used_time) != JSON_INTEGER ||
            json_integer_value(used_time) != lic->stats.used_time) {
            json_object_set_new(json_lic, "used-time", json_integer(lic->stats.used_time));
            doc->drity = 1;
        }
    }

    {
        json_t *last_updated_time;

        last_updated_time = json_object_get(json_lic, "last-updated-time");

        if (!last_updated_time || json_typeof(last_updated_time) != JSON_INTEGER ||
            json_integer_value(last_updated_time) != lic->stats.last_updated_time) {
            json_object_set_new(json_lic, "last-updated-time", json_integer(lic->stats.last_updated_time));
            doc->drity = 1;
        }
    }

    return 0;
}

json_t *
alu_client_doc_get_active(
    const alu_client_doc_t *doc)
{
    json_t *object = json_object_path_get(doc->doc, 0, ALU_CLIENT_DB_PREFIX, "active", NULL);

    if (object && json_typeof(object) == JSON_OBJECT) {
        return object;
    }

    return NULL;
}

int
alu_client_doc_remove_active(
    alu_client_doc_t *doc)
{
    json_t *object = json_object_get(doc->doc, ALU_CLIENT_DB_PREFIX);

    if (object && json_typeof(object) == JSON_OBJECT) {
        if (json_object_get(object, "active")) {
            json_object_del(object, "active");
            doc->drity = 1;
        }

        return 0;
    }

    return -1;
}

int
alu_client_doc_write_active(
    alu_client_doc_t *doc,
    const alu_client_active_license_t *active)
{
    json_t *json_act;
    int rc;

    ASSERT(doc);
    ASSERT(active);

    if (!doc || !active) {
        return -1;
    }

    if (active->valid == 0) {
        alu_client_doc_remove_active(doc);
        return 0;
    }

    if (active->license.trial_time == 1) {
        rc = alu_client_doc_write_license(doc, &active->license);
        if (rc != 0) {
            return rc;
        }
    }

    json_act = alu_client_doc_get_active(doc);

    if (!json_act) {
        json_act = json_object();

        json_object_path_set_new_2(doc->doc, ALU_CLIENT_DB_PREFIX, "active", json_act);
    }

    {
        json_t *lic_no;

        lic_no = json_object_get(json_act, "license-no");

        if (!lic_no || json_typeof(lic_no) != JSON_STRING ||
            strcmp(json_string_value(lic_no), active->license.license_number) != 0) {
            json_object_set_new(json_act, "license-no", json_string(active->license.license_number));
            doc->drity = 1;
        }
    }


    {
        json_t *start_time;

        start_time = json_object_get(json_act, "start-time");

        if (!start_time || json_typeof(start_time) != JSON_INTEGER ||
            json_integer_value(start_time) != active->start_record_time) {
            json_object_set_new(json_act, "start-time", json_integer(active->start_record_time));
            doc->drity = 1;
        }
    }

    return 0;
}

