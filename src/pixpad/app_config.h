#pragma once

class AppConfig
{
public:
	// do not create instance
	AppConfig() = delete;
	// static configuration
	static const char *app_name;
	static int window_width;
	static int window_height;

};
