#include <linux/module.h>
#include <linux/mm_types.h>

struct folio check_type(void)
{
	return (struct folio) {};
}

MODULE_LICENSE("GPL");
