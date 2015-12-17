#include "stdafx.h"
#include "CppUnitTest.h"

#include "any_stride_iterator.h"
#include "vertex_buffer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ViewUnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(any_stride_iterator)
		{
			using namespace wyc;
			
			typedef struct {
				float x, y, z;
			} Vec3;
			constexpr int cnt = 8;
			Vec3 verts[cnt];
			typedef CAnyStrideIterator<Vec3> iterator;

			// constructors
			iterator iter1;
			iterator iter2(verts, sizeof(Vec3));
			iterator iter3(iter2);
			iterator iter4 = iter2;
			Assert::IsTrue(iter1 != iter2);
			iter1 = iter2;
			Assert::IsTrue(iter1 == iter2);
			Assert::IsTrue(iter2 == iter3);
			Assert::IsTrue(iter2 == iter4);
			Assert::IsTrue(iter3 == iter4);

			// operation
			Assert::IsTrue(iter1++ == iter2++);
			Assert::IsTrue(iter1-- == iter2--);
			++iter1;
			++iter2;
			Assert::IsTrue(iter1 == iter2);
			--iter1;
			--iter2;
			Assert::IsTrue(iter1 == iter2);
			iter1 += 3;
			iter2 += 3;
			Assert::IsTrue(iter1 == iter2);
			iter1 -= 2;
			iter2 -= 2;
			Assert::IsTrue(iter1 == iter2);
			Assert::IsTrue(iter1 + 4 == iter2 + 4);
			Assert::IsTrue(iter1 - 4 == iter2 - 4);

			// comparision
			iter1 = iter2 + 1;
			Assert::IsTrue(iter1 != iter2);
			Assert::IsTrue(iter1 > iter2);
			Assert::IsTrue(iter2 < iter1);
			Assert::IsTrue(iter1 >= iter2);
			Assert::IsTrue(iter2 <= iter1);
			iter1 = iter2;
			Assert::IsTrue(iter1 == iter2);
			Assert::IsTrue(iter1 >= iter2);
			Assert::IsTrue(iter2 <= iter1);

			// data access
			float x, y, z;
			x = 0;
			y = 1;
			z = 1;
			iterator beg(verts, sizeof(Vec3));
			iterator end(verts + cnt);
			for (auto it = beg; it != end; ++it)
			{
				*it = { x, y, z };
				x = y;
				y = z;
				z = x + y;
			}
			x = 0;
			y = 1;
			z = 1;
			int idx = 0;
			for (auto it = beg; it != end; ++it, ++idx)
			{
				Assert::IsTrue(idx < cnt);
				Assert::IsTrue(verts[idx].x == x && verts[idx].y == y && verts[idx].z == z);
				Assert::IsTrue(it->x == x && it->y == y && it->z == z);
				x = y;
				y = z;
				z = x + y;
			}
			Assert::IsTrue(idx == cnt);
		}

		TEST_METHOD(vertex_buffer)
		{
			using namespace wyc;

			typedef struct {
				float x, y, z;
			} Vec3;

			typedef struct {
				Vec3 pos;
				Vec3 color;
				Vec3 normal;
			} Vertex;

			constexpr size_t cnt = 8;
			
			CVertexBuffer vb;
			Assert::IsTrue(vb.size() == 0);
			Assert::IsTrue(vb.vertex_size() == 0);
			auto beg = vb.begin(), end = vb.end();
			Assert::IsTrue(beg == end);

			// set attribute
			vb.set_attribute(ATTR_POSITION, 3);
			vb.set_attribute(ATTR_COLOR, 3);
			vb.set_attribute(ATTR_NORMAL, 3);
			vb.resize(cnt);
			Assert::IsTrue(vb.vertex_size() == sizeof(Vertex));
			Assert::IsTrue(vb.size() == cnt);
			auto pos = vb.get_attribute(ATTR_POSITION);
			Assert::IsTrue(pos);
			auto color = vb.get_attribute(ATTR_COLOR);
			Assert::IsTrue(color);
			auto normal = vb.get_attribute(ATTR_NORMAL);
			Assert::IsTrue(normal);
			auto tangent = vb.get_attribute(ATTR_TANGENT);
			Assert::IsFalse(tangent);

			fibonacci<Vec3, CAttributeArray<false>::iterator>(pos.begin(), pos.end());

			vb.clear();
			Assert::IsTrue(vb.size() == 0);
			Assert::IsTrue(vb.vertex_size() == 0);
		}

		template<typename T, typename Iter>
		void fibonacci(Iter beg, Iter end)
		{
			float x, y, z;
			x = 0;
			y = 1;
			z = 1;
			for (auto it = beg; it != end; ++it)
			{
				*it = T{ x, y, z };
				x = y;
				y = z;
				z = x + y;
			}
		}

	};
}