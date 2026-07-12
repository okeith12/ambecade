#ifndef STORAGE_H
#define STORAGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Result of a storage operation; STORAGE_OK is success, the rest are failures.
typedef enum storage_status {
    STORAGE_OK = 0,
    STORAGE_ERR_NULL,       // A required pointer (reader, ops, or arg) was NULL
    STORAGE_ERR_NOT_FOUND,  // The requested resource does not exist
    STORAGE_ERR_IO,         // The underlying read failed
    STORAGE_ERR_SHORT,      // Fewer bytes available than requested (malformed/truncated)
    STORAGE_ERR_RANGE       // Destination buffer too small for the request
} storage_status_t;

/* The storage seam: one function pointer for reading a resource. A real board
   fills it with an SD/filesystem read; a mock serves bytes from memory. The
   read must fill exactly len bytes and MUST NOT write past buf[len-1]; if fewer
   than len bytes are available it returns STORAGE_ERR_SHORT. */
typedef struct storage_reader_ops {
    storage_status_t (*read)(void *ctx, const char *path,
                             uint8_t *buf, size_t len);
} storage_reader_ops_t;

// A reader instance: the ops table plus an opaque context passed back to read.
typedef struct storage_reader {
    const storage_reader_ops_t *ops;  // Borrowed function table
    void *ctx;                        // Implementation-defined handle
} storage_reader_t;

// Returns STORAGE_OK only if reader, its ops, and the read pointer are non-NULL.
storage_status_t storage_reader_validate(const storage_reader_t *reader);

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_H */
