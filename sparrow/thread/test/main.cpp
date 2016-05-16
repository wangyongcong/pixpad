#include <stdlib.h>

#include "lockfree_rb_q.h"
#include "mpmc_queue.h"
#include "spsc_queue.h"
#include "ring_queue.h"
#include "folly_queue.h"

int main()
{
	//RUN_TEST(lockfree_rb_q);
	//RUN_TEST(mpmc_queue);
	RUN_TEST(spsc_queue);
	RUN_TEST(folly_queue);
	RUN_TEST(ring_queue);

	::system("pause");
}