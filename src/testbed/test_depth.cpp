#pragma once
#include "test.h"
#include "mesh.h"

class CTestDepth : public CTest
{
public:
	virtual void run() 
	{
		const char *ply_file = "res/torus.ply";
		auto mesh = std::make_shared<wyc::CMesh>();
		if (!mesh->load_ply(ply_file)) {
			return;
		}
		log_info("succeed!");
	}

private:
};

REGISTER_NEW_TEST(CTestDepth)
