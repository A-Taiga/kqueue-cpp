#ifndef KQTYPES_HPP
#define KQTYPES_HPP
#include <functional>
#include <sys/event.h>
namespace KQ {


	using u64_t = std::uint64_t;
	using i64_t = std::int64_t;
	using i16_t = std::int16_t;
	using u16_t = std::uint16_t;
	using u32_t = std::uint32_t;
    using callable = std::function<void(struct kevent64_s*)>;


    // enum class KFILT : i16_t
    // {
    //     READ    = EVFILT_READ,
    //     WRITE   = EVFILT_WRITE,
    //     AIO     = EVFILT_AIO,
    //     VNODE   = EVFILT_VNODE,
    //     PROC    = EVFILT_PROC,
    // };
    enum class VFF : u32_t
    {
        DELETE = NOTE_DELETE,     
        WRITE = NOTE_WRITE,      
        EXTEND = NOTE_EXTEND,     
        ATTRIB = NOTE_ATTRIB,     
        LINK = NOTE_LINK,       
        RENAME = NOTE_RENAME,     
        REVOKE = NOTE_REVOKE,     
        NONE = NOTE_NONE,       
        FUNLOCK = NOTE_FUNLOCK,
    };

    enum class EVA : u16_t
    {
        ADD     = EV_ADD,
        DELETE  = EV_DELETE,
        ENABLE  = EV_ENABLE,
        DISABLE = EV_DISABLE,
    };

    enum class EVF : u16_t
    {
        ONESHOT   = EV_ONESHOT,
        CLEAR     = EV_CLEAR,
        RECEIPT   = EV_RECEIPT,
        DISPATCH  = EV_DISPATCH,
        NONE      = 0,
    };
    // using _EVF = std::underlying_type_t<EVF>;

    enum class UFF : u32_t
    {
        TRIGGER     = NOTE_TRIGGER,
        NOP         = NOTE_FFNOP,
        AND         = NOTE_FFAND,
        OR          = NOTE_FFOR,
        COPY        = NOTE_FFCOPY,
        CTRLMASK    = NOTE_FFCTRLMASK,
        FLAGKMASK   = NOTE_FFLAGSMASK,
    };

    enum class Type_ID: short
    {
        KERNEL,
        USER,
        SIGNAL,
        TIMER,
        TICK,
    };

    struct data
    {
        callable callback;
        Type_ID type;
        data () {puts("data()");}
        data (callable&& cb, Type_ID type);
        data (const callable&& cb, Type_ID type);
    };

    struct genericID_t
    {
        u64_t v;
        genericID_t (u64_t, Type_ID);
        operator u64_t() const;
        Type_ID get_type() const;
        private:
        Type_ID type;
    };

    struct fd_t : public genericID_t {fd_t (); fd_t (u64_t);};
    struct user_t : public genericID_t { user_t (); user_t (u64_t); };
    struct signal_t : public genericID_t { signal_t (); signal_t (u64_t); };
    struct tick_t : public genericID_t { tick_t (); tick_t (u64_t); };
    struct timer_t : public genericID_t { timer_t (); timer_t (u64_t); };

}

template<>
struct std::hash<KQ::genericID_t>
{
    std::size_t operator () (const KQ::genericID_t& generic) const
    {
        return std::hash<int>{}(generic.v) ^ std::hash<short>{}(static_cast<int>(generic.get_type()));
    }
};



#endif