#include <err.h>
#include <signal.h>
#include <stddef.h>
#include "kvm.h"
#include "gui.h"
#include "log.h"
#include "io_manager.h"
#include "devices/cmos.h"
#include "devices/a20.h"
#include "devices/pci.h"
#include "devices/seabios_info.h"
#include "devices/seabios_log.h"
#include "devices/com.h"
#include "devices/dma.h"
#include "devices/pic.h"

void handle_sigint(int sig)
{
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        errx(1, "Usage: %s <filename>", argv[0]);
    }

    signal(SIGINT, handle_sigint);

    gui_init();
    log_init();

    io_manager_register(cmos_init, cmos_handle, 0x70, 0x71);
    io_manager_register(NULL, a20_handle, 0x92, 0x92);
    io_manager_register(pci_init, pci_handle, 0xcf8, 0xcff);
    io_manager_register(seabios_log_init, seabios_log_handle, 0x402, 0x402);
    io_manager_register(NULL, seabios_info_handle, 0x510, 0x511);
    io_manager_register(NULL, com_handle, 0x3f8, 0x3f8);
    io_manager_register(NULL, dma_handle_master, 0xc0, 0xde);
    io_manager_register(NULL, dma_handle_slave, 0x00, 0x0f);
    io_manager_register(NULL, pic_handle_master, 0x20, 0x21);
    io_manager_register(NULL, pic_handle_slave, 0xa0, 0xa1);

    kvm_init(argv[1]);
    kvm_run();
    kvm_deinit();
    gui_deinit();

    return 0;
}