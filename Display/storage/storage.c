/* storage seam helper.

   The reader contract is just data (the ops table in storage.h); the only
   behavior here is validation, run once before use so a partially wired reader
   fails loudly instead of crashing on a null function pointer. */

#include "storage.h"

storage_status_t storage_reader_validate(const storage_reader_t *reader)
{
    if (reader == NULL || reader->ops == NULL) {
        return STORAGE_ERR_NULL;
    }
    if (reader->ops->read == NULL) {
        return STORAGE_ERR_NULL;
    }
    return STORAGE_OK;
}
