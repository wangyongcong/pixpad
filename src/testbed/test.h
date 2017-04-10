#pragma once
#include <string>

class CTest
{
public:
	static CTest* new_test() {
		return nullptr;
	}
	virtual ~CTest() {
	}
	virtual void init() {
	}
	virtual void run() {
	}
};
