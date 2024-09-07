#include "log.h"
#include "kvm.h"
#include "components/seabios_log.h"
#include <stdio.h>
#include <stdarg.h>

FILE *log_file;

void log_init()
{
    log_file = fopen("log.txt", "w");
    if (!log_file)
        err(1, "Failed to open log file");
}

void log_print(const char *fmt, ...)
{  
    va_list args;
    va_start(args, fmt);

    struct kvm_regs regs;
    kvm_get_regs(&regs);
    
    // FILE* seabios_log = seabios_log_get_file();
    fprintf(log_file, "[0x%llx ", regs.rip);
    // fprintf(seabios_log, "[0x%llx ", regs.rip);
    // va_list args_copy;
    // va_copy(args_copy, args);
    vfprintf(log_file, fmt, args);
    // vfprintf(seabios_log, fmt, args_copy);
    fprintf(log_file, "\n");
    // fprintf(seabios_log, "\n");
    va_end(args);
    fflush(log_file);
}

void log_deinit()
{
    fclose(log_file);
}
