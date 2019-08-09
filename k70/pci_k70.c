#include "main.h"
#include "pci_k70.h"

static struct pci_device_id pci_k70_tbl[] = {
    { PCI_DEVICE(0x1172, 0x4258) },
    { PCI_DEVICE(0x10ee, 0x7028) },
    { 0, }
};

MODULE_DEVICE_TABLE(pci, pci_k70_tbl);

static int
pci_k70_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id)
{
    i32 ret;

    if (pci_enable_device(pdev)) {
        ret = PCI_K70_ENABLE_DEVICE_FAILED;
        goto  error;
    }

    if (pci_request_regions(pdev, "pci_k70")) {
        ret = PCI_K70_RESOURCE_FAILED;
        goto error;
    }

    if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM)) {
        ret = PCI_K70_RESOURCE_ERR;
        goto error;
    }

    mconfig->pci_mem = __va(pci_resource_start(pdev, 0));
    mconfig->pci_mems = pci_resource_len(pdev, 0);

    pci_set_master(pdev);

    PRINT("pci k70 mem: 0x%p size = 0x%x bus no=%d\n", mconfig->pci_mem, mconfig->pci_mems,
        pdev->bus->number);
    return 0;

error:
    pci_release_regions(pdev);
    pci_disable_device(pdev);
    return ret;
}

static void
pci_k70_remove(struct pci_dev  *pdev)
{
    pci_release_regions(pdev);
    pci_disable_device(pdev);

    PRINT("pci k70 remove: %p\n", pdev);
    return;
}

struct pci_driver pci_k70_driver = {
    .name       = "k70",
    .id_table   = pci_k70_tbl,
    .probe      = pci_k70_probe,
    .remove     = pci_k70_remove
};