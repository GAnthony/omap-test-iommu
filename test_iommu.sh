set -o xtrace
dmesg | egrep iommu

insmod iommu_test.ko name=omap-iommu.0 pa=0x0 da=0xa0000000 count=5
cat /sys/kernel/debug/iommu/mmu0_dsp1/pagetable
rmmod iommu_test.ko

insmod iommu_test.ko name=omap-iommu.2 pa=0x95000000 da=0xa0000000 count=5
cat /sys/kernel/debug/iommu/mmu0_dsp2/pagetable
rmmod iommu_test.ko

insmod iommu_test.ko name=omap-iommu.4 pa=0x0 da=0xa1000000 count=5
cat /sys/kernel/debug/iommu/mmu_ipu1/pagetable
rmmod iommu_test.ko

insmod iommu_test.ko name=omap-iommu.5 pa=0x0 da=0xa2000000 count=5
cat /sys/kernel/debug/iommu/mmu_ipu2/pagetable
rmmod iommu_test.ko
