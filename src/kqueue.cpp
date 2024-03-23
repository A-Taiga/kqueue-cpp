#include "kqueue.hpp"
#include "kqTypes.hpp"
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <format>

namespace KQ
{

    genericID_t::genericID_t(u64_t ident, Type_ID t) : v(ident), type(t) {}
    genericID_t::operator u64_t() const {return v;}
    Type_ID genericID_t::get_type() const {return type;}


    fd_t::fd_t(u64_t ident) : genericID_t(ident, Type_ID::KERNEL){}
    user_t::user_t(u64_t ident) : genericID_t(ident, Type_ID::KERNEL){}
    signal_t::signal_t(u64_t ident) : genericID_t(ident, Type_ID::KERNEL){}
    timer_t::timer_t(u64_t ident) : genericID_t(ident, Type_ID::KERNEL){}


    Kqueue::Kqueue (std::timespec timeout)
    : kq (::kqueue())
    , timeout (timeout)
    {
        if (kq == -1)
            throw std::runtime_error(strerror(errno));
    }

    bool Kqueue::add_clock_ms (timer_t id, i64_t time, const callable&& callback)
    {
        if (map.contains(id)) 
            return false;
        map[id] = {{},std::move(callback), Type_ID::TIMER};
        add_helper(id, EVFILT_TIMER, EV_ADD, 0, time, reinterpret_cast<uintptr_t>(&map[id]));
        return true;
    }

    bool Kqueue::add_clock_sec (timer_t id, i64_t time, const callable&& callback)
    {
        if (map.contains(id)) 
            return false;
        map[id] = {{},std::move(callback), Type_ID::TIMER};
        add_helper(id, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, time, reinterpret_cast<uintptr_t>(&map[id]));
        return true;
    }

    bool  Kqueue::add_timer_sec (timer_t id, i64_t time, const callable&& callback)
    {
        if (map.contains(id))
            return false;
        map[id] = {{}, std::move(callback), Type_ID::TIMER};
        add_helper(id, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, time, reinterpret_cast<uintptr_t>(&map[id]));
        return true;
    }

    void Kqueue::remove_timer(timer_t id)
    {
        if (!map.contains(id))
            throw Error("timer id not found");
        
        map[id].ev.flags = EV_DELETE;
        int ret = ::kevent64(kq, &map[id].ev, 1, nullptr, 0, 0, &timeout);
        if (ret == -1)
            throw Error(std::strerror(errno));

        map.erase(id);
    }

    void Kqueue::add_helper (const genericID_t& id, const i64_t& filter, const u16_t& flags, const u32_t& fflags, const i64_t& data, const u64_t& udata)
    {
        EV_SET64 (&map[id].ev, id, filter, flags, fflags, data, udata, 0, 0);
        int ret = ::kevent64(kq, &map[id].ev, 1, nullptr, 0, 0, &timeout);
        if (ret == -1)
            throw Error(std::strerror(errno));
    }


    void Kqueue::poll_helper (kevent64_s* eventList, std::size_t size)
    {
        std::size_t Nchanges;
        Nchanges = ::kevent64(kq, NULL, 0, eventList, size, 0,&timeout);
        if (Nchanges == -1)
            throw Error();

        if (Nchanges == -1)
            throw std::runtime_error(std::strerror(errno));
        for (std::size_t i = 0; i < Nchanges; i++)
        {
            struct kevent64_s* ev = &eventList[i];
            struct data* data = reinterpret_cast<struct data*>(ev->udata);

            if (ev->flags & EV_ERROR)
            {
                Error("EV_ERROR");
            }

            data->callback(ev);

            if (ev->flags & EV_ONESHOT)
            {
                switch (data->type)
                {
                    case Type_ID::KERNEL:   map.erase(fd_t(ev->ident)); break;
                    case Type_ID::USER:     map.erase(user_t(ev->ident)); break;
                    case Type_ID::SIGNAL:   map.erase(signal_t(ev->ident)); break;
                    case Type_ID::TIMER:    map.erase(timer_t(ev->ident)); break;
                }
            }
        }
    }


    Error::Error (std::string msg, std::source_location location)
    : message(std::format("{}:{} {}", location.file_name(), location.line(), msg))
    {}
    Error::Error (std::source_location location)
    : message(std::format("{}:{}", location.file_name(), location.line()))
    {}
    const char* Error::what() const noexcept
    {
        return message.c_str();
    }

}
