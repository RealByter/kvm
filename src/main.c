#include <err.h>
#include <signal.h>
#include <stddef.h>
#include "kvm.h"
#include "gui.h"
#include "log.h"
#include "io_manager.h"
#include "components/cmos.h"
#include "components/a20.h"
#include "components/pci.h"
#include "components/seabios_info.h"
#include "components/seabios_log.h"
#include "components/com.h"
#include "components/dma.h"
#include "components/pic.h"
#include "components/pit.h"
#include "components/ps2.h"
#include "components/ata.h"

void handle_sigint(int sig)
{
    exit(0);
}

void null_handle(exit_io_info_t *io, uint8_t *base)
{
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        errx(1, "Usage: %s <bios> <kernel> <harddisk>", argv[0]);
    }

    signal(SIGINT, handle_sigint);

    // gui_init();
    log_init();
    ata_init_disks(argv[2], argv[3]);

    io_manager_register(cmos_init, cmos_handle, 0x70, 0x71);
    io_manager_register(NULL, a20_handle, 0x92, 0x92);
    io_manager_register(pci_init, pci_handle, 0xcf8, 0xcff);
    io_manager_register(seabios_log_init, seabios_log_handle, 0x402, 0x402);
    io_manager_register(NULL, seabios_info_handle, 0x510, 0x511);
    io_manager_register(com_init, com_handle, 0x3f8, 0x3ff);
    // ignore the other com ports
    io_manager_register(NULL, null_handle, 0x2f8, 0x2ff);
    io_manager_register(NULL, null_handle, 0x3e8, 0x3ef);
    io_manager_register(NULL, null_handle, 0x2e8, 0x2ef);
    io_manager_register(NULL, dma_handle_master, 0xc0, 0xde);
    io_manager_register(NULL, dma_handle_slave, 0x00, 0x0f);
    io_manager_register(pic_init_master, pic_handle_master, 0x20, 0x21);
    io_manager_register(pic_init_slave, pic_handle_slave, 0xa0, 0xa1);
    io_manager_register(pit_init, pit_handle, 0x40, 0x43);
    io_manager_register(ps2_init, ps2_handle, 0x60, 0x60);
    io_manager_register(NULL, ps2_handle, 0x64, 0x64);
    io_manager_register(ata_init_primary, ata_handle_io_primary, 0x1f0, 0x1f7);
    io_manager_register(ata_init_secondary, ata_handle_io_secondary, 0x170, 0x177);
    io_manager_register(NULL, ata_handle_control_primary, 0x3f6, 0x3f7);
    io_manager_register(NULL, ata_handle_control_secondary, 0x376, 0x377);

    ata_deinit_disks();
    kvm_init(argv[1]);
    kvm_run();
    kvm_deinit();
    // gui_deinit();

    return 0;
}