#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/of_platform.h>

#include <linux/iommu.h>
#include <linux/omap-iommu.h>
#include <linux/platform_data/iommu-omap.h>

MODULE_LICENSE("GPL");

/* load-time options */
char *name = "omap-iommu.1";
unsigned int pa = 0x95000000;
unsigned int da = 0xA0000000;
int len = 0x00100000;
int count = 1;

module_param(name, charp, 0);
module_param(count, int, 0);
module_param(pa, uint, 0);
module_param(da, uint, 0);
module_param(len, int, 0);

static struct device *dev;
static struct iommu_domain *domain;

static int iommu_fault(struct iommu_domain *domain, struct device *dev,
			unsigned long iova, int flags, void *token)
{
	dev_err(dev, "iommu fault: da 0x%x flags 0x%x\n", iova, flags);

	/*
	 * Let the iommu core know we're not really handling this fault;
	 * we just used it as a recovery trigger.
	 */
	return -ENOSYS;
}

static void __exit iommu_test_cleanup(void)
{
	pr_info("%s: iommu_test_cleanup entered\n", __func__);
	if (dev && domain) {
		iommu_detach_device(domain, dev);
		iommu_domain_free(domain);
	}
}

static int __init iommu_test_init(void)
{
	int i, ret = 0;
	const char *dev_name;

	pr_info("%s: iommu_test_init entered\n", __func__);

	/* Get dev from name: */
	dev = bus_find_device_by_name(&platform_bus_type, NULL, name);
	if (!dev) {
		pr_info("%s: Unable to find device: %s\n", __func__, name);
		return ret;
	}

	/* Enable IOMMU:  */
	dev_info(dev, "Enabling IOMMU...\n");
	if (!iommu_present(dev->bus)) {
		dev_dbg(dev, "iommu not found\n");
		return ret;
	}

	domain = iommu_domain_alloc(dev->bus);
	if (!domain) {
		dev_err(dev, "can't alloc iommu domain\n");
		return -ENOMEM;
	}

	iommu_set_fault_handler(domain, iommu_fault, NULL);

	/*
	 * omap_iommu_attach_device() expects dev->archdata.iommu->name to be
	 * set.  This is set by a remoteproc platform function, but here we
	 * need to set it manually:
	 */
	dev_name = ((struct iommu_platform_data *)dev->platform_data)->name;
	dev_info(dev, "dev->platform_data->name: %s\n", dev_name);
	((struct omap_iommu_arch_data *)dev->archdata.iommu)->name = dev_name;

	dev_info(dev, "dev->archdata.iommu->name: %s\n",
	((struct omap_iommu_arch_data *)dev->archdata.iommu)->name);

	ret = iommu_attach_device(domain, dev);
	if (ret) {
		dev_err(dev, "can't attach iommu device: %d\n", ret);
		goto free_domain;
	}

	/* Setup 'count' mappings:  */
	for (i = 0; i < count; i++) {
		pa += len; da += len;
		dev_info(dev, "Mapping pa 0x%x, da 0x%x, len 0x%x\n",
				pa, da, len);

		ret = iommu_map(domain, da, pa, len, 0);
		if (ret)
			dev_err(dev, "failed to map pa to da: %d\n", ret);
	}
	return 0;

free_domain:
	iommu_domain_free(domain);
	return ret;
}

module_init(iommu_test_init);
module_exit(iommu_test_cleanup);
