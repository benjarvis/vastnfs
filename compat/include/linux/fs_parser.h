#ifndef __COMPAT_LINUX_FS_PARSER_H__
#define __COMPAT_LINUX_FS_PARSER_H__

#ifdef COMPAT_DETECT_INCLUDE_FS_CONTEXT
#include_next <linux/fs_parser.h>

#if !defined(COMPAT_DETECT_FS_PARAM_DESCRIPTION) && \
    !defined(COMPAT_DETECT_FS_PARAM_DESCRIPTION_ENUMS)
#define __fsparam_compat(a, b, c, d, null) \
	__fsparam(a, b, c, d, null)
#define fsparam_enum_compat(a, c, array) \
	fsparam_enum(a, c, array)
#else
/* 2709c957a8ef4fb00f21acb306e3bd6bcf80c81f: fs_parse: get rid of ->enums */
#define __fsparam_compat(a, b, c, d, null) \
	__fsparam(a, b, c, d)
#define fsparam_enum_compat(a, c, array) \
	fsparam_enum(a, c)
#endif

#if defined(COMPAT_DETECT_FS_PARAMETER_TYPE)
#define fsparam_type_is_flag fs_param_is_flag
#else
#define fsparam_type_is_flag NULL
#endif

#else

#include <linux/fs_parser_backport.h>

#define __fsparam_compat(a, b, c, d, null) \
	__fsparam(a, b, c, d, null)
#define fsparam_enum_compat(a, c, array) \
	fsparam_enum(a, c, array)
#define fsparam_type_is_flag NULL

#endif

#endif
