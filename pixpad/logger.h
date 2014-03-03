#ifndef WYC_HEADER_LOGGER
#define WYC_HEADER_LOGGER

namespace wyc
{

void init_logger(const char *log_file_name);

void print(const char *message);

}; // end of namespace wyc

#endif // WYC_HEADER_LOGGER