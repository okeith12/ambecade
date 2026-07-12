#include "mock_storage.h"

static storage_status_t mock_read(void *ctx, const char *path,
                                  uint8_t *buf, size_t len)
{
    mock_storage_t *m = (mock_storage_t *)ctx;
    if (m == NULL || path == NULL || buf == NULL) {
        return STORAGE_ERR_NULL;
    }
    if (m->not_found) {
        return STORAGE_ERR_NOT_FOUND;
    }
    if (m->fail_io) {
        return STORAGE_ERR_IO;
    }
    if (len > m->blob_len) {
        return STORAGE_ERR_SHORT;  // not enough data; do not overrun buf
    }
    for (size_t i = 0u; i < len; i++) {
        buf[i] = m->blob[i];
    }
    return STORAGE_OK;
}

/* Single shared, stateless ops table; all per-instance state lives in ctx. */
static const storage_reader_ops_t mock_ops = {
    .read = mock_read
};

void mock_storage_init(mock_storage_t *m, const uint8_t *blob, size_t blob_len)
{
    if (m == NULL) {
        return;
    }
    m->blob = blob;
    m->blob_len = blob_len;
    m->not_found = 0u;
    m->fail_io = 0u;
}

void mock_storage_bind(storage_reader_t *reader, mock_storage_t *m)
{
    if (reader == NULL || m == NULL) {
        return;
    }
    reader->ops = &mock_ops;
    reader->ctx = m;
}
