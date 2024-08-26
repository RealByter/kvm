#include <err.h>
#include <signal.h>
#include "kvm.h"
#include "gui.h"
#include "log.h"

void handle_sigint(int sig)
{
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);

    gui_init();
    log_init();

    cmos_init();

    if (argc != 2)
    {
        errx(1, "Usage: %s <filename>", argv[0]);
    }

    kvm_init(argv[1]);
    kvm_run();
    kvm_deinit();
    gui_deinit();

    return 0;
}