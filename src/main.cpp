#include "kqTypes.hpp"
#include "kqueue.hpp"
#include <fcntl.h>
static KQ::Kqueue kq({5,0});

// enum flags 
// {
// 	F1 = 10,
// 	F2 = 20,
// 	F3 = 30,
// };

// [[maybe_unused]] static void t (struct kevent64_s* ev)
// {
// 	puts("FIRE");
// 	if (ev->fflags & F1 && ev->fflags & F3)
// 	{
// 		puts("F1 && F3");
// 	}
// }

// static auto l = [i = 1] (auto&& arg) mutable
// {
// 	printf("| = %d\n", i++);
// 	if (i >= 5)
// 	{
// 		kq.remove_timer(arg->ident);
// 		kq.update_custom_ev(0, KQ::EVF::NONE, KQ::UFF::TRIGGER);
// 	}
// };

int main()
{
	kq.add_vnode_ev(::open("aa.txt", O_RDONLY), KQ::EVF::CLEAR, KQ::VFF::WRITE, [](auto&&)
	{
		puts("WRITTEN");
	});

	// kq.add_clock_ms(0, 100, [i = 1] (auto&& arg) mutable
	// {
	// 	printf("%d\n", i++);
	// 	if (i >= 50)
	// 	{
	// 		kq.remove_clock(arg->ident);
	// 		kq.add_custom_fflags(0, KQ::UFF::COPY, F1 | F3);
	// 		kq.add_clock_ms(0, 500, l);
	// 	}
	// });

	// kq.add_custom_ev(0, KQ::EVF::ONESHOT, KQ::UFF::COPY, t);



	while (true)
	{
		kq.poll<10000>();
	}

	return 0;
}