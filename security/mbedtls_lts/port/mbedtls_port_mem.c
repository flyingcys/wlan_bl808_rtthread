#include "mbedtls_port_platform.h"

#include <string.h>

void *mbedtls_port_calloc( size_t nmemb, size_t size )
{
    const size_t sz = nmemb * size;
    void *m = bl_os_malloc( sz );
    if ( m )
        memset( m, 0, sz );
    return( m );
}
