/*******************************************************************************
    License-Identifier: GPL-2.0
    Copyright (c) 2020 NVIDIA Corporation

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

        The above copyright notice and this permission notice shall be
        included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#ifdef CONFIG_NVFS
#define MODULE_PREFIX rpcrdma
#include "nvfs.h"

struct nvfs_dma_rw_ops *nvfs_ops = NULL;

atomic_t nvfs_shutdown = ATOMIC_INIT(1);

DEFINE_PER_CPU(long, nvfs_n_ops);

// must have for compatability
#define NVIDIA_FS_COMPAT_FT(ops) \
      (NVIDIA_FS_CHECK_FT_SGLIST_PREP(ops) && \
       NVIDIA_FS_CHECK_FT_SGLIST_DMA(ops) && \
       NVIDIA_FS_CHECK_FT_GPU_PAGE(ops) && \
       NVIDIA_FS_CHECK_FT_DEVICE_PRIORITY(ops))

// protected via nvfs_module_mutex
int REGISTER_FUNC (struct nvfs_dma_rw_ops *ops)
{
	if (NVIDIA_FS_COMPAT_FT(ops)) {
	      nvfs_ops = ops;
	      atomic_set(&nvfs_shutdown, 0);
	      return 0;
	} else
	      return -ENOTSUPP;

}
EXPORT_SYMBOL(REGISTER_FUNC);

// protected via nvfs_module_mutex
void UNREGISTER_FUNC (void)
{
        (void) atomic_cmpxchg(&nvfs_shutdown, 0, 1);
        do{
                msleep(NVFS_HOLD_TIME_MS);
        } while (nvfs_count_ops());
        nvfs_ops = NULL;
}
EXPORT_SYMBOL(UNREGISTER_FUNC);
#endif
