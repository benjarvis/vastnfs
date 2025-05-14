#include <linux/sysctl.h>

#ifdef IMPL_COMPAT_SYSCTL_NUMBERS
const int compat_sysctl_vals[] = { 0, 1, INT_MAX };
EXPORT_SYMBOL(compat_sysctl_vals);
#endif
