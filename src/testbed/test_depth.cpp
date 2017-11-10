#pragma once
#include <fstream>
#include "test.h"
#include "tinyply.h"

class CTestDepth : public CTest
{
public:
	virtual void run() 
	{
		std::ifstream fs("res/torus.ply");
		tinyply::PlyFile file(fs);
		for (auto e : file.get_elements())
		{
			for (auto p : e.properties)
			{
				auto &prop = tinyply::PropertyTable[p.propertyType];
			}
		}

	}

private:
};

REGISTER_NEW_TEST(CTestDepth)
