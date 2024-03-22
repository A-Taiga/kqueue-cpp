#include "kqueue.hpp"
#include <iostream>


static Kqueue kq({5, 0});

static void user_event_callback(struct kevent* ev)
{
	std::cout << "Hello world!" << std::endl;
}

static const auto seconds = [i = 1](struct kevent* ev) mutable
{
	printf("seconds: %d\n", i++);
	if (i > 10)
	{
		kq.remove_timer(ev->ident);
		kq.register_uEvent(0, EV_ONESHOT, NOTE_TRIGGER, {user_event_callback});
	}
};


int main()
{
	kq.register_timer_seconds(0, 1, {seconds});
	while (true)
	{
		kq.handle_events();
	}
	return 0;
}