#include "log.h"
#include "kvm.h"
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
    
    fprintf(log_file, "[0x%llx ", regs.rip);
    vfprintf(log_file, fmt, args);
    fprintf(log_file, "\n");
    va_end(args);
    fflush(log_file);
}

void log_deinit()
{
    fclose(log_file);
}
