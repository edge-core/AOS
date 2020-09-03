#include "alu_client_private.h"

#include "json_object_path.h"
#include "sys_time.h"

#define ALU_CLIENT_DB_PREFIX                        "aluc"

static void
alu_client_storage_aes_init_key(
    u_char *key,
    size_t key_len,
    size_t count,
    u_char out_key[SHA256_DIGEST_LENGTH],
    u_char out_iv[SHA256_DIGEST_LENGTH]
);

static u_char *
alu_client_storage_aes_encript(
   u_char *key,
   size_t key_len,
   const u_char *plaintext,
   size_t plaintext_len,
   size_t *_ciphertext_len
);

static u_char *
alu_client_storage_aes_decrypt(
    u_char *key,
    size_t key_len,
    const u_char *ciphertext,
    size_t ciphertext_len,
    size_t *_plaintext_len
);

static json_t *
alu_client_storage_unpack_doc_v1(
    alu_client_ctx_t *ctx,
    const bson_t *bson_file
);

static uint8_t *
alu_client_storage_pack_doc_v1(
    alu_client_ctx_t *ctx,
    size_t *len
);

static void
alu_client_storage_aes_init_key(
    u_char *key,
    size_t key_len,
    size_t count,
    u_char out_key[SHA256_DIGEST_LENGTH],
    u_char out_iv[SHA256_DIGEST_LENGTH])
{
    size_t i;

    u_char tmp[SHA256_DIGEST_LENGTH * 2];

    SHA256(key, key_len, tmp);
    SHA256(tmp, SHA256_DIGEST_LENGTH, &tmp[SHA256_DIGEST_LENGTH]);

    for (i = 0; i < count; ++i) {
        SHA256(&tmp[SHA256_DIGEST_LENGTH], SHA256_DIGEST_LENGTH, tmp);
        SHA256(tmp, SHA256_DIGEST_LENGTH, &tmp[SHA256_DIGEST_LENGTH]);
    }

    memcpy(out_iv, tmp, SHA256_DIGEST_LENGTH);
    memcpy(out_key, &tmp[SHA256_DIGEST_LENGTH], SHA256_DIGEST_LENGTH);
}

static u_char *
alu_client_storage_aes_encript(
    u_char *key,
    size_t key_len,
    const u_char *plaintext,
    size_t plaintext_len,
    size_t *_ciphertext_len)
{
    u_char             *ciphertext = NULL;
    int                 ciphertext_len;
    EVP_CIPHER_CTX      en;

    u_char              aes_key[SHA256_DIGEST_LENGTH];
    u_char              aes_iv[SHA256_DIGEST_LENGTH];

    alu_client_storage_aes_init_key(key, key_len, 5, aes_key, aes_iv);

    EVP_CIPHER_CTX_init(&en);
    if (EVP_EncryptInit_ex(&en, EVP_aes_256_cbc(), NULL, aes_key, aes_iv) == 0) {
        return NULL;
    }

    ciphertext_len = plaintext_len;
    ciphertext = aes_encrypt(&en, (unsigned char *)plaintext, &ciphertext_len);

    if (ciphertext) {
        *_ciphertext_len = ciphertext_len;
    }

    EVP_CIPHER_CTX_cleanup(&en);

    return ciphertext;
}

static u_char *
alu_client_storage_aes_decrypt(
    u_char *key,
    size_t key_len,
    const u_char *ciphertext,
    size_t ciphertext_len,
    size_t *_plaintext_len)
{
    u_char             *ret = NULL;
    int                 plaintext_len;

    EVP_CIPHER_CTX      de;

    u_char              aes_key[SHA256_DIGEST_LENGTH];
    u_char              aes_iv[SHA256_DIGEST_LENGTH];

    alu_client_storage_aes_init_key(key, key_len, 5, aes_key, aes_iv);

    EVP_CIPHER_CTX_init(&de);
    if (EVP_DecryptInit_ex(&de, EVP_aes_256_cbc(), NULL, aes_key, aes_iv) == 0) {
        return NULL;
    }

    plaintext_len = ciphertext_len;
    ret = aes_decrypt(&de, (unsigned char *)ciphertext, &plaintext_len);
    if (ret) {
        *_plaintext_len = plaintext_len;
    }

    EVP_CIPHER_CTX_cleanup(&de);

    return ret;
}

static const char *
alu_client_storage_get_version(
    const bson_t *bson_file)
{
    bson_iter_t iter;

    ASSERT(bson_file);

    if (bson_file &&
        bson_iter_init(&iter, bson_file) &&
        bson_iter_find(&iter, "Version") &&
        BSON_ITER_HOLDS_UTF8(&iter)) {
        uint32_t len;
        return bson_iter_utf8(&iter, &len);
    }

    return NULL;
}

static json_t *
alu_client_storage_unpack_doc_v1(
    alu_client_ctx_t *ctx,
    const bson_t *bson_file)
{
    if (bson_file) {
        bson_iter_t iter;
        const char *data_algorithm;

        if (!bson_iter_init(&iter, bson_file)) {
            return NULL;
        }

        if (!bson_iter_find(&iter, "DataAlgorithm") ||
            !BSON_ITER_HOLDS_UTF8(&iter)) {
            return NULL;
        }

        data_algorithm = bson_iter_utf8(&iter, NULL);
        if (strcmp(data_algorithm, "aesEncryption") != 0) {
            return NULL;
        }

        if (bson_iter_find(&iter, "Data") &&
            BSON_ITER_HOLDS_BINARY(&iter)) {

            bson_subtype_t subtype;
            uint32_t encrypted_bson_doc_len;
            const uint8_t *encrypted_bson_doc;

            bson_iter_binary(&iter, &subtype, &encrypted_bson_doc_len, &encrypted_bson_doc);

            {
                alu_keys_t keys;

                size_t  raw_bson_len;
                u_char *raw_bson_doc;

                json_t *doc = NULL;

                memset(&keys, 0, sizeof(keys));
                alu_get_keys(&keys.rsa[0].public_key, &keys.rsa[1].public_key, &keys.shared_key);
                keys.shared_key_len = strlen((char *)keys.shared_key);

                raw_bson_doc = alu_client_storage_aes_decrypt((u_char*)keys.shared_key, keys.shared_key_len, encrypted_bson_doc, encrypted_bson_doc_len, &raw_bson_len);

                // TODO: set_error

                if (raw_bson_doc) {
                    bson_t *bson_doc;

                    bson_doc = bson_new_from_data(raw_bson_doc, raw_bson_len);
                    // TODO: set_error

                    if (bson_doc) {
                        char *json_doc;

                        json_doc = bson_as_json(bson_doc, NULL);

                        if (json_doc) {
                            json_error_t error;
                            doc = json_loads(json_doc, 0, &error);

                            bson_free(json_doc);
                        }

                        bson_destroy(bson_doc);
                    }

                    free(raw_bson_doc);
                }

                return doc;
            }
        }
    }

    return NULL;
}

// pack doc to encrypted bson format
// use free() function to free returned buffer
static uint8_t *
alu_client_storage_pack_doc_v1(
    alu_client_ctx_t *ctx,
    size_t *len)
{
    char *json_doc = NULL;

    bson_error_t error;
    bson_t *bson_doc = NULL;

    alu_keys_t keys;

    u_char *encrypted_bson_doc = NULL;

    json_doc = json_dumps(ctx->storage.data.doc, JSON_COMPACT);
    if (!json_doc) {
        goto error;
    }

    bson_doc = bson_new_from_json((const uint8_t *)json_doc, strlen(json_doc), &error);
    if (!bson_doc) {
        goto error;
    }

    memset(&keys, 0, sizeof(keys));
    alu_get_keys(&keys.rsa[0].public_key, &keys.rsa[1].public_key, &keys.shared_key);
    keys.shared_key_len = strlen((char *)keys.shared_key);

    encrypted_bson_doc = alu_client_storage_aes_encript((u_char*)keys.shared_key, keys.shared_key_len, bson_get_data(bson_doc), bson_doc->len, len);
    if (!encrypted_bson_doc) {
        goto error;
    }

    if (0) {
    error:
        encrypted_bson_doc = NULL;
    }

    if (bson_doc) {
        bson_destroy(bson_doc);
    }

    if (json_doc) {
        free(json_doc);
    }

    return encrypted_bson_doc;
}

// dump storage to bson buffer
bson_t *
alu_client_storage_dumpb_v1(
    alu_client_ctx_t *ctx)
{
    size_t  encrypted_bson_doc_len;
    u_char *encrypted_bson_doc = NULL;

    bson_t *bson_file;

    uint64_t last_write_time = 0;

    bson_file = bson_new();
    if (!bson_file) {
        return NULL;
    }

    if (!bson_append_utf8(bson_file, "Version", -1, "1.0.0", -1)) {
        goto error;
    }

    last_write_time = alu_client_time_get_up_time();
    if (!bson_append_int64(bson_file, "LastWriteTime", -1, last_write_time)) {
        goto error;
    }

    if (!bson_append_utf8(bson_file, "DataAlgorithm", -1, "aesEncryption", -1)) {
        goto error;
    }

    encrypted_bson_doc = alu_client_storage_pack_doc_v1(ctx, &encrypted_bson_doc_len);
    if (!encrypted_bson_doc) {
        goto error;
    }

    if (!bson_append_binary(bson_file, "Data", -1, BSON_SUBTYPE_BINARY, encrypted_bson_doc, encrypted_bson_doc_len)) {
        goto error;
    }

    if (!bson_append_utf8(bson_file, "SignatureAlgorithm", -1, "sha", -1)) {
        goto error;
    }

    {
        const uint8_t *data = bson_get_data(bson_file);
        u_char tmp[SHA256_DIGEST_LENGTH * 2];
        u_char sig[SHA256_DIGEST_LENGTH];

        SHA256(data, bson_file->len, tmp);
        SHA256((u_char*)"hello", 5, &tmp[SHA256_DIGEST_LENGTH]);
        SHA256(tmp, sizeof(tmp), sig);

        if (!bson_append_binary(bson_file, "Signature", -1, BSON_SUBTYPE_BINARY, sig, sizeof(sig))) {
            goto error;
        }
    }

    if (0) {
    error:
        bson_destroy(bson_file);
        bson_file = NULL;
    }

    if (encrypted_bson_doc) {
        free(encrypted_bson_doc);
    }

    return bson_file;
}

// load storage from bson buffer
int
alu_client_storage_loadb_v1(
    alu_client_ctx_t *ctx,
    const bson_t *bson_file)
{
    json_t *new_doc;

    bson_iter_t iter;

    uint32_t len;
    const char *version;

    uint16_t new_major_ver;
    uint16_t new_minor_ver;
    uint16_t new_micro_ver;
    uint64_t new_last_write_time;

    ASSERT(bson_file);

    if (!bson_iter_init(&iter, bson_file)) {
        return -1;
    }

    if (!bson_iter_find(&iter, "Version") || !BSON_ITER_HOLDS_UTF8(&iter)) {
        return -1;
    }

    version = bson_iter_utf8(&iter, &len);
    if (!version) {
        return -1;
    }

    new_major_ver = (uint16_t)atol(version);
    version = strchr(version, '.');
    if (!version) {
        return -1;
    }

    version += 1;

    new_minor_ver = (uint16_t)atol(version);
    version = strchr(version, '.');
    if (!version) {
        return -1;
    }

    version += 1;
    new_micro_ver = (uint16_t)atol(version);

    ASSERT(new_major_ver == 1 && new_minor_ver == 0 && new_micro_ver == 0);

    // Should be 1.0.X
    if (new_major_ver != 1 && new_minor_ver != 0) {
        return -1;
    }

    if (!bson_iter_find(&iter, "LastWriteTime") || !BSON_ITER_HOLDS_INT64(&iter)) {
        return -1;
    }

    new_last_write_time = bson_iter_int64(&iter);

    new_doc = alu_client_storage_unpack_doc_v1(ctx, bson_file);

    if (!new_doc) {
        return -1;
    }

    if (ctx->storage.data.doc) {
        json_decref(ctx->storage.data.doc);
    }

    ctx->storage.major_version = new_major_ver;
    ctx->storage.minor_version = new_minor_ver;
    ctx->storage.micro_version = new_micro_ver;

    ctx->storage.last_write_time = new_last_write_time;

    ctx->storage.data.doc = new_doc;

    if (0) {
        char *s = json_dumps(new_doc, JSON_INDENT(2));
        free(s);
    }

    return 0;
}

bson_t *
alu_client_storage_dumpb(
    alu_client_ctx_t *ctx)
{
    bson_t *bson_file;

    if (ctx->storage.major_version == 1 &&
        ctx->storage.minor_version == 0 &&
        ctx->storage.micro_version == 0)
    {
        bson_file = alu_client_storage_dumpb_v1(ctx);
    }
    else {
        ASSERT(0);

        bson_file = alu_client_storage_dumpb_v1(ctx);
    }

    return bson_file;
}

int
alu_client_storage_loadb(
    alu_client_ctx_t *ctx,
    const uint8_t *data,
    int64_t length)
{
    bson_t *bson_file;
    const char *version;
    int rc;

    bson_file = bson_new_from_data(data, (size_t)length);
    if (!bson_file) {
        return -1;
    }

    version = alu_client_storage_get_version(bson_file);
    if (version && !strcmp(version, "1.0.0")) {
        rc = alu_client_storage_loadb_v1(ctx, bson_file);
    }
    else {
        rc = -1;
    }

    if (bson_file) {
        bson_free(bson_file);
    }

    return rc;
}



