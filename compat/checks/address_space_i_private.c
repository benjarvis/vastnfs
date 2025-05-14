#include <linux/module.h>
#include <linux/fs.h>

/*
 * v6.8-rc1~215^2~23 "fs: Rename mapping private members"
 */
void* test_dummy(struct address_space *ptr)
{
	return ptr->i_private_data;
}

MODULE_LICENSE("GPL");
