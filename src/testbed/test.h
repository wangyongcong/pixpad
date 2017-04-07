#pragma once
#include <string>
#include <tclap/CmdLine.h>

class CTest
{
public:
	static CTest* new_test() {
		return nullptr;
	}
	virtual ~CTest() {
	}
	virtual void init(int argc, char *argv[]) {
	}
	virtual void run() {
	}
};
