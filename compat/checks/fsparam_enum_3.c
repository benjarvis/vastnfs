#include <linux/module.h>
#include <linux/fs_parser.h>

#include "../../compat.h"

/* LAYER: 2 */

enum {
	Opt_comp
};

#if defined(COMPAT_DETECT_FS_PARAMETER_ENUM)
#define FS_PARAMETER_ENUM fs_parameter_enum
#else
#define FS_PARAMETER_ENUM constant_table
#endif

static const struct FS_PARAMETER_ENUM _param_compr[] = {
	{"none",	1 },
};

static const struct fs_parameter_spec param_specs[] = {
	fsparam_enum	("compr",	Opt_comp, _param_compr),
	{}
};

MODULE_LICENSE("GPL");
