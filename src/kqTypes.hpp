#ifndef KQTYPES_HPP
#define KQTYPES_HPP
#include <functional>
#include <sys/event.h>
#include <iostream>
namespace KQ {


	using u64_t = unsigned long long;
	using i64_t = long long;
	using i16_t = short;
	using u16_t = unsigned short;
	using u32_t = unsigned int;

    enum class Type_ID: short
    {
        KERNEL,
        USER,
        SIGNAL,
        TIMER,
    };

    struct data
    {
        struct kevent64_s ev;
        std::function<void(struct kevent64_s*)> callback;
        Type_ID type;

    };

    struct genericID_t
    {
        u64_t v;
        genericID_t (u64_t, Type_ID);
        genericID_t () {}
        operator u64_t() const;
        Type_ID get_type() const;
        private:
        Type_ID type;
    };

    struct fd_t : public genericID_t {fd_t (); fd_t (u64_t);};
    struct user_t : public genericID_t { user_t (); user_t (u64_t); };
    struct signal_t : public genericID_t { signal_t (); signal_t (u64_t); };
    struct timer_t : public genericID_t { timer_t (); timer_t (u64_t); };

}

template<>
struct std::hash<KQ::genericID_t>
{
    std::size_t operator() (const KQ::genericID_t& generic) const
    {
        return std::hash<int>{}(generic) ^ std::hash<short>{}(static_cast<short>(generic.get_type()));
    }
};



#endif