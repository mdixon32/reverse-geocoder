#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rg_handle rg_handle;

rg_handle* rg_create_default(void);
void rg_destroy(rg_handle* handle);

const char* rg_reverse_geocode(rg_handle* handle, double latitude, double longitude);
const char* rg_reverse_geocode_state(rg_handle* handle, double latitude, double longitude);
size_t rg_country_count(rg_handle* handle);

#ifdef __cplusplus
}
#endif
