#include <iostream>
#include <atomic>

using namespace std;

class CTest
{
public:
	CTest(int count)
	{
		if(count > 0) {
			m_ptr = new int[count];
			memset(m_ptr, 0, sizeof(int)*count);
			m_count = count;
		}
		else {
			m_ptr = 0;
			m_count = 0;
		}
	}
	CTest(CTest &&t)
	{
		if(m_ptr)
			delete [] m_ptr;
		m_ptr = t.m_ptr;
		m_count = t.m_count;
		t.m_ptr = 0;
		t.m_count = 0;
		printf("CTest move constructor\n");
	}
	CTest(const CTest &t)
	{
		if(m_ptr)
			delete [] m_ptr;
		m_count = t.m_count;
		if(m_count > 0)
		{
			m_ptr = new int[m_count];
			memcpy(m_ptr, t.m_ptr, sizeof(int)*m_count);
		}
		printf("CTest copy constructor\n");
	}
	~CTest()
	{
		if(m_ptr) {
			delete [] m_ptr;
			m_ptr = 0;
		}
	}

private:
	int *m_ptr;
	unsigned m_count;
};

int main()
{
	cout << "Hello world" << endl;
	return 0;
}

