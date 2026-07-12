#ifndef MOCK_STORAGE_H
#define MOCK_STORAGE_H

#include <stddef.h>
#include <stdint.h>
#include "storage.h"

#ifdef __cplusplus
extern "C" {
#endif

/* In-memory storage source for tests. Serves bytes from a caller-owned blob;
   a request longer than the blob returns STORAGE_ERR_SHORT without overrunning
   the destination. Flags let a test simulate a missing file or an I/O fault. */
typedef struct mock_storage {
    const uint8_t *blob;  // Backing bytes (caller-owned)
    size_t blob_len;      // Number of bytes available
    uint8_t not_found;    // If non-zero, read() returns STORAGE_ERR_NOT_FOUND
    uint8_t fail_io;      // If non-zero, read() returns STORAGE_ERR_IO
} mock_storage_t;

// Points the mock at a blob and clears the fault flags.
void mock_storage_init(mock_storage_t *m, const uint8_t *blob, size_t blob_len);

// Wires reader to the shared mock ops table with m as its context.
void mock_storage_bind(storage_reader_t *reader, mock_storage_t *m);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_STORAGE_H */
