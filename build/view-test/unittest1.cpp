#include "stdafx.h"
#include "CppUnitTest.h"

#include "any_stride_iterator.h"
#include "vertex_buffer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ViewUnitTest
{	
	struct Vec3 {
		float x, y, z;
		bool operator == (const Vec3 &other) const
		{
			return x == other.x && y == other.y && z == other.z;
		}
		std::wstring to_string() const {
			std::wstringstream ss;
			ss << x << "," << y << "," << z;
			return ss.str();
		}
	};

	struct Vertex {
		Vec3 pos;
		Vec3 color;
		Vec3 normal;
		bool operator = (const Vertex &other) const
		{
			return pos == other.pos && color == other.color && normal == other.normal;
		}
		inline std::wstring to_string() const {
			return L"pos:(), color:(), normal:()";
		}
	};

	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(any_stride_iterator)
		{
			using namespace wyc;
			
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
			iterator beg(verts, sizeof(Vec3));
			iterator end(verts + cnt);
			Fibonacci<Vec3> fib;
			for (auto it = beg; it != end; ++it)
			{
				*it = fib.next();
			}
			fib.reset();
			int idx = 0;
			for (auto it = beg; it != end; ++it, ++idx)
			{
				auto v = fib.next();
				Assert::IsTrue(idx < cnt);
				Assert::AreEqual(verts[idx], v);
				Assert::AreEqual(*it, v);
			}
			Assert::AreEqual(idx, cnt);
		}

		TEST_METHOD(vertex_buffer)
		{
			using namespace wyc;
			constexpr int cnt = 8;
			
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

			fibonacci<Vec3, CAttribArray::iterator>(pos.begin(), pos.end());
			fibonacci<Vec3, CAttribArray::iterator>(color.begin(), color.end());
			fibonacci<Vec3, CAttribArray::iterator>(normal.begin(), normal.end());
			int idx = 0;
			const CVertexBuffer &cvb = vb;
			auto fib = Fibonacci<Vec3>();
			for (auto it = cvb.begin(); it != cvb.end(); ++it, ++idx)
			{
				Assert::IsTrue(idx < cnt);
				const Vertex &v = *it;
				auto f = fib.next();
				Assert::AreEqual(v.pos, f);
				Assert::AreEqual(v.color, f);
				Assert::AreEqual(v.normal, f);
			}
			Assert::AreEqual(idx, cnt);

			idx = 0;
			auto cpos = cvb.get_attribute(ATTR_POSITION);
			Assert::IsTrue(cpos);
			auto it_pos = cpos.begin();
			auto ccolor = cvb.get_attribute(ATTR_COLOR);
			Assert::IsTrue(ccolor);
			auto it_color = ccolor.begin();
			auto cnormal = cvb.get_attribute(ATTR_NORMAL);
			Assert::IsTrue(cnormal);
			auto it_normal = cnormal.begin();
			for (auto it = vb.begin(); it != vb.end(); ++it, ++idx, ++it_pos, ++it_color, ++it_normal)
			{
				Assert::IsTrue(idx < cnt);
				Vertex &v = *it;
				v.pos = { 0, 0, 1 };
				Assert::IsTrue(it_pos != cpos.end());
				Assert::AreEqual(v.pos, (const Vec3&)(*it_pos));
				v.color = { 1, 0, 0 };
				Assert::IsTrue(it_color != ccolor.end());
				Assert::AreEqual(v.color, (const Vec3&)(*it_color));
				v.normal = { 0, 1, 0 };
				Assert::IsTrue(it_normal != cnormal.end());
				Assert::AreEqual(v.normal, (const Vec3&)(*it_normal));
			}
			Assert::AreEqual(idx, cnt);
			Assert::IsTrue(it_pos == cpos.end());
			Assert::IsTrue(it_color == ccolor.end());
			Assert::IsTrue(it_normal == cnormal.end());

			for (auto &it: pos)
			{
				Vec3 &v = it;
				Assert::AreEqual(v, Vec3{ 0, 0, 1 });
				it = Vec3{ 0, 0, 0 };
				Assert::AreEqual(v, Vec3{ 0, 0, 0 });
			}
			for (auto it : cpos)
			{
				Vec3 v = it;
				Assert::AreEqual(v, Vec3{ 0, 0, 0 });
			}

			vb.clear();
			Assert::IsTrue(vb.size() == 0);
			Assert::IsTrue(vb.vertex_size() == 0);
		}

		template<typename T>
		struct Fibonacci
		{
			float x, y, z;
			Fibonacci() : x(0), y(1), z(1) {}
			inline void reset()
			{
				x = 0;
				y = 1;
				z = 1;
			}
			inline T next()
			{
				T ret = { x, y, z };
				x = y;
				y = z;
				z = x + y;
				return ret;
			}
		};

		template<typename T, typename Iter>
		void fibonacci(Iter beg, Iter end)
		{
			auto fib = Fibonacci<T>();
			for (auto it = beg; it != end; ++it)
			{
				*it = fib.next();
			}
		}

	};
}

#define TO_STRING(type) template <> static std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<type>(const type& q) {return q.to_string();}

TO_STRING(ViewUnitTest::Vec3)
TO_STRING(ViewUnitTest::Vertex)
