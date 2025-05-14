#include "linux/module.h"
#include "linux/seqlock.h"

seqcount_spinlock_t exists;

MODULE_LICENSE("GPL");
