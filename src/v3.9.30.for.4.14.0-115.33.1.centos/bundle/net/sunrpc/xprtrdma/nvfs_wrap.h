#ifndef __NVFS_WRAP_H__
#define __NVFS_WRAP_H__

#ifdef CONFIG_NVFS
#define NVFS_FRWR

#include "nvfs.h"
#include "nvfs_rpc_rdma.h"

static inline int _alter_ib_dma_map_sg(struct ib_device *dev,
				       struct scatterlist *sg, int nents,
				       enum dma_data_direction direction)
{
	int ret;
	bool is_nvfs_io = false;

	ret = rpcrdma_nvfs_map_data(dev->dma_device, sg, nents, direction,
				    &is_nvfs_io);
	if (ret == -EIO) {
		/* Fail caller completely */
		return 0;
	}

	if (is_nvfs_io) {
		/* GPU memory */
		dprintk("rpcrdma_nvfs_map_data device"
			" %s mr->mr_sg: %p, nents: %d, direction: %d\n",
			dev->name, sg, nents, direction);
		return ret;
	}

	/* Fallback (not GPU memory) */
	return ib_dma_map_sg(dev, sg, nents, direction);
}

#define ib_dma_map_sg _alter_ib_dma_map_sg

static inline void _alter_ib_dma_unmap_sg(struct ib_device *dev,
					  struct scatterlist *sg, int nents,
					  enum dma_data_direction direction)
{
	int ret;

	ret = rpcrdma_nvfs_unmap_data(dev->dma_device, sg, nents, direction);
	if (ret > 0) {
		dprintk("rpcrdma_nvfs_unmap_data device"
			" %s mr->mr_sg: %p, nents: %d, direction: %d\n",
			dev->name, sg, nents, direction);
		return;
	}

	/* Fallback (not GPU memory) */
	ib_dma_unmap_sg(dev, sg, nents, direction);
}

#define ib_dma_unmap_sg _alter_ib_dma_unmap_sg

#endif

#endif
