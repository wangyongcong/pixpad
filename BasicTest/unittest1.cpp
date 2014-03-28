#include "stdafx.h"
#include <vector>
#include "CppUnitTest.h"
#include "vertex_buffer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft{ namespace VisualStudio { namespace CppUnitTestFramework {

	template<> static std::wstring ToString<wyc::xvec3f_t>(const wyc::xvec3f_t &v) {
		std::wstringstream _s;	
		_s << "(" << v.x << "," << v.y << "," << v.z << ")";
		return _s.str();
	}

}}} // end of namespace

namespace BasicTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestVertexBuffer)
		{
			typedef wyc::VERTEX_P3 vertex_t;
			std::vector<vertex_t> vertices;
			std::vector<uint16_t> indices;
			gen_plane(1, 1, vertices, indices);
		}

	};
}