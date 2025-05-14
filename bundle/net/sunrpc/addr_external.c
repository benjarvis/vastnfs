#define SUNRPC_ADDR_EXTERNAL
// We use private rebuild of `addr.c` so that the actual `addr.c` can export
// functions with signatures like the original sunrpc addr.c, because some
// external modules may depend on them.
#include "addr.c"
