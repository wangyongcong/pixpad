#pragma once
#include "test.h"
#include "mesh.h"

class CTestDepth : public CTest
{
public:
	virtual void run() 
	{
		std::string ply_file;
		if (!get_param("model", ply_file)) {
			log_error("no model");
			return;
		}
		auto mesh = std::make_shared<wyc::CMesh>();
		if (!mesh->load_ply(ply_file)) {
			return;
		}
		log_info("succeed!");
	}

private:
};

REGISTER_NEW_TEST(CTestDepth)
