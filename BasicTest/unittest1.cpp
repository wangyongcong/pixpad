#include "stdafx.h"
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
			using namespace wyc;
			typedef xvertex_buffer<VERTEX_P3C3> vertex_buffer_t;
			vertex_buffer_t buffer;
			size_t size = 100;
			buffer.storage(size);
			Assert::AreEqual(buffer.size(), size);
			Assert::IsNotNull(buffer.get_data());
			vertex_buffer_t::vertex_t v;
			v.position = xvec3f_t(0, 0, 0);
			v.color = xvec3f_t(0, 0, 0);
			buffer[0] = v;
			void *position_ptr = buffer.get_attr<VERTEX_POSITION>();
			void *color_ptr = buffer.get_attr<VERTEX_COLOR>();
			Assert::AreEqual(v.position, *(xvec3f_t*)position_ptr);
			Assert::AreEqual(v.color, *(xvec3f_t*)color_ptr);
			buffer.release();
			Assert::AreEqual(buffer.size(),size_t(0));
			Assert::IsNull(buffer.get_data());
		}

	};
}