#include "linux/module.h"
#include "linux/fs_context.h"

void *check_type(struct fs_context *ctx)
{
	return (void *)&ctx->log;
}

MODULE_LICENSE("GPL");
