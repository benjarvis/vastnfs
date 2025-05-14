#include <linux/module.h>
#include <linux/ftrace.h>

struct pt_regs *test_func(struct ftrace_regs *fregs)
{
	return ftrace_get_regs(fregs);

}

MODULE_LICENSE("GPL");
