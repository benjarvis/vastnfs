#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>

#define MODULE_PREFIX rpcrdma

#include "../bundle/net/sunrpc/xprtrdma/nvfs.h"

int nvstub_dma_map_sg_attrs(struct device *device,
			    struct scatterlist *sglist,
			    int nents,
			    enum dma_data_direction dma_dir,
			    unsigned long attrs)
{
	return NVFS_CPU_REQ;
}

int nvstub_dma_unmap_sg(struct device *device,
			struct scatterlist *sglist,
			int nents,
			enum dma_data_direction dma_dir)
{
	return 0;
}


bool nvstub_is_gpu_page(struct page *page)
{
	return true;
}

unsigned int nvstub_gpu_index(struct page *page)
{
	return (page_to_pfn(page) >> 4) & 3;
}

unsigned int nvstub_device_priority(struct device *dev, unsigned int gpu_index)
{
	struct pci_dev *pdev;

	if (!dev_is_pci(dev))
		return 0;

	pdev = to_pci_dev(dev);
	if (!pdev || !pdev->bus)
		return 0;

	if (pdev->bus->number == 0x6) {
		switch (gpu_index) {
		case 0: return 2;
		case 1: return 2;
		case 2: return 1;
		case 3: return 1;
		}
	} else if (pdev->bus->number == 0x7) {
		switch (gpu_index) {
		case 0: return 1;
		case 1: return 1;
		case 2: return 2;
		case 3: return 2;
		}
	}

	return 0;
}

struct nvfs_dma_rw_ops ops = {
	.ft_bmap = nvfs_ft_prep_sglist | nvfs_ft_map_sglist |
		nvfs_ft_is_gpu_page | nvfs_ft_device_priority,
	.nvfs_is_gpu_page = nvstub_is_gpu_page,
	.nvfs_dma_map_sg_attrs = nvstub_dma_map_sg_attrs,
	.nvfs_dma_unmap_sg = nvstub_dma_unmap_sg,
	.nvfs_gpu_index = nvstub_gpu_index,
	.nvfs_device_priority = nvstub_device_priority,
};

static int __init init_nvstub(void)
{
	printk(KERN_DEBUG "nvstub: loaded\n");
	return rpcrdma_register_nvfs_dma_ops(&ops);
}

static void __exit exit_nvstub(void)
{
	printk(KERN_DEBUG "nvstub: unloading\n");
	rpcrdma_unregister_nvfs_dma_ops();
}

module_init(init_nvstub);
module_exit(exit_nvstub);

MODULE_AUTHOR("Dan Aloni <dan.aloni@vastdata.com>");
MODULE_LICENSE("GPL");
