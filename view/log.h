#include "logger.h"

// global logger
extern wyc::xlogger *g_log;

// macro for logging
#define debug(fmt,...) (g_log->debug(fmt,__VA_ARGS__))
#define  info(fmt,...) (g_log->info (fmt,__VA_ARGS__))
#define  warn(fmt,...) (g_log->warn (fmt,__VA_ARGS__))
#define error(fmt,...) (g_log->error(fmt,__VA_ARGS__))
#define fatal(fmt,...) (g_log->fatal(fmt,__VA_ARGS__))

