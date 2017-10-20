#ifndef __HEADER_CONFIG
#define __HEADER_CONFIG

class AppConfig
{
public:
	// do not create instance
	AppConfig() = delete;
	// static configuration
	static const char *app_name;
	static const int window_width;
	static const int window_height;

};

#endif // __HEADER_CONFIG
