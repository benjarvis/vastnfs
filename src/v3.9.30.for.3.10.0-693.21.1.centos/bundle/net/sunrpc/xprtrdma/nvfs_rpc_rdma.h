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

#ifndef NVFS_RPCRDMA_H
#define NVFS_RPCRDMA_H

#ifdef NVFS_FRWR
static inline int rpcrdma_nvfs_map_data(struct device *dev, struct scatterlist *sg,
                                      int nents, enum dma_data_direction dma_dir,
                                      bool *is_nvfs_io)
{
	int count;

        *is_nvfs_io = false;
        count = 0;
	if (nvfs_get_ops()) {
		count = nvfs_ops->nvfs_dma_map_sg_attrs(dev,
				sg,
				nents,
				dma_dir,
				DMA_ATTR_NO_WARN);

		if (unlikely((count == NVFS_IO_ERR))) {
			nvfs_put_ops();
		        return -EIO;
		}

                if (unlikely(count == NVFS_CPU_REQ)) {
                        nvfs_put_ops();
		        return 0;
                }
		*is_nvfs_io = true;
        }
        return count;
}
#endif

static inline bool rpcrdma_nvfs_unmap_data(struct device *dev, struct scatterlist *sg,
                                         int nents, enum dma_data_direction dma_dir)

{
        int count;

        if (nvfs_ops != NULL) {
                count = nvfs_ops->nvfs_dma_unmap_sg(dev, sg, nents,
                                dma_dir);
                if (count > 0) {
                        nvfs_put_ops();
                        return true;
                }
        }
        return false;
}

static inline unsigned int rpcrdma_nvfs_device_priority(struct device *dev, unsigned int gpu_index)
{
	if (nvfs_get_ops()) {
                unsigned int r = nvfs_ops->nvfs_device_priority(dev, gpu_index);
		nvfs_put_ops();
		return r;
	}

        return 0;
}

static inline unsigned int rpcrdma_nvfs_gpu_index(struct page *page)
{
	if (nvfs_get_ops()) {
		unsigned int r = nvfs_ops->nvfs_gpu_index(page);
		nvfs_put_ops();
		return r;
	}

        return 0;
}

static inline bool rpcrdma_nvfs_is_gpu_page(struct page *page)
{
	if (nvfs_get_ops()) {
                bool r = nvfs_ops->nvfs_is_gpu_page(page);
		nvfs_put_ops();
		return r;
	}

        return false;
}

#endif /* NVFS_RPCRDMA_H */
