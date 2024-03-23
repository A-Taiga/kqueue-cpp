#include "kqTypes.hpp"
#include "kqueue.hpp"
static KQ::Kqueue kq({5,0});




static void t1(struct kevent64_s* ev)
{
	static int i = 1;
	printf("sec: %d\n", i++);
	if (i > 5)
		kq.remove_timer(ev->ident);

}

static void t2(struct kevent64_s* ev)
{
	static int i = 1;
	printf("ms: %d\n", i++);
	if (i > 10)
		kq.remove_timer(ev->ident);
}

int main()
{

	kq.add_clock_ms(0, 500, t2);
	kq.add_clock_sec(1, 1, t1);







	while (true)
	{
		kq.poll<10000>();
	}

	return 0;
}