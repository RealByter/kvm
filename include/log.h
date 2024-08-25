#ifndef LOG_H
#define LOG_H

#define LOG_DEFINE(module_name) \
    static const char *current_module_name = module_name; \

#define LOG_MSG(fmt, ...) \
    log_print("- %s] " fmt, current_module_name, ##__VA_ARGS__)

void log_init();
void log_print(const char *fmt, ...);
void log_deinit();


#endif