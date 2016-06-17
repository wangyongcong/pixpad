#include "logger.h"

// global logger
extern wyc::CLogger *g_log;

// macro for logging
#define log_debug(fmt,...) (g_log->debug(fmt,__VA_ARGS__))
#define log_info(fmt,...) (g_log->info(fmt,__VA_ARGS__))
#define log_warn(fmt,...) (g_log->warn(fmt,__VA_ARGS__))
#define log_error(fmt,...) (g_log->error(fmt,__VA_ARGS__))
#define log_fatal(fmt,...) (g_log->fatal(fmt,__VA_ARGS__))

