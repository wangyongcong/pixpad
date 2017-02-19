#pragma once
#include "stdafx.h"
#include <cmath>
#include "test.h"
#include "image.h"
#include "spw_renderer.h"


class CTestBox : public CTest
{
public:
	static CTest* create() {
		return new CTestBox();
	}
	virtual void run() {
		printf("draw box");
	}
};
