#include "kqueue.hpp"
#include "kqTypes.hpp"
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <format>
#include <utility>

#define CAST(X) (uintptr_t) X

namespace KQ
{

    data::data (callable&& cb, Type_ID type)
    : callback(std::move(cb))
    , type (type)
    { puts("callback&&");}

    data::data (const callable&& cb, Type_ID type)
    : callback(std::move(cb))
    , type (type)
    {
        puts("const callback&&");
    }

    genericID_t::genericID_t(u64_t ident, Type_ID t) : v(ident), type(t) {}
    genericID_t::operator u64_t() const {return v;}
    Type_ID genericID_t::get_type() const {return type;}
    fd_t::fd_t(u64_t ident) : genericID_t(ident, Type_ID::KERNEL){}
    user_t::user_t(u64_t ident) : genericID_t(ident, Type_ID::USER){}
    signal_t::signal_t(u64_t ident) : genericID_t(ident, Type_ID::SIGNAL){}
    timer_t::timer_t(u64_t ident) : genericID_t(ident, Type_ID::TIMER){}
    tick_t::tick_t(u64_t ident) : genericID_t(ident, Type_ID::TICK){}


    Kqueue::Kqueue (std::timespec timeout)
    : kq (::kqueue())
    , timeout (timeout)
    {
        if (kq == -1)
            throw std::runtime_error(strerror(errno));
    }

    bool Kqueue::add_vnode_ev (fd_t id, EVF flags, VFF fflags, const callable&& callback)
    {
        if (map.contains(id)) return false;
        map[id] = std::make_pair<struct kevent64_s, data> ({}, {std::move(callback), Type_ID::KERNEL});
        auto& src = map[id];
        src.first = {id, EVFILT_VNODE, (u16_t)(EV_ADD | (u16_t)flags), (u32_t) fflags, 0, CAST(&src.second)};
        add (src.first);
        return true;
    }


    bool Kqueue::add_custom_ev(user_t id, EVF flags, UFF fflags, const callable&& callback)
    {
        if (map.contains(id)) return false;
        map[id] = std::make_pair<struct kevent64_s, data> ({}, {std::move(callback), Type_ID::USER});
        auto& src = map[id];
        src.first = {id, EVFILT_USER, (u16_t)(EV_ADD | (u16_t)flags), (u32_t) fflags, 0, CAST(&src.second)};
        add (src.first);
        return true;
    }

    bool Kqueue::tirgger_custom_ev (user_t id, EVF flags, UFF fflags)
    {
        if (!map.contains(id)) return false;
        auto& src = map[id];
        src.first.flags = (EV_ADD | (u16_t)flags);
        src.first.fflags = (u32_t)UFF::TRIGGER | (u32_t)fflags;
        add (src.first);
        return true;
    }

    bool Kqueue::update_custom_ev (user_t id, EVF flags, UFF fflags)
    {
        if (!map.contains(id)) return false;
        auto &src = map[id];
        src.first.flags = (EV_ADD | (u16_t)flags);
        src.first.fflags = (u32_t)fflags;
        add (src.first);
        return true;
    }

    bool Kqueue::add_custom_fflags (user_t id, UFF option, const int flags)
    {
        if (!map.contains(id))return false;
        auto &src = map[id];
        src.first.fflags = NOTE_FFCOPY | flags;
        add (src.first);
        return true;
    }


    bool Kqueue::add_clock_ms (tick_t id, i64_t time, const callable&& callback)
    {
        if (map.contains(id)) return false;
        map[id] = std::make_pair<struct kevent64_s, KQ::data> ({}, {std::move(callback), Type_ID::TICK});
        auto& src = map[id];
        src.first = {id, EVFILT_TIMER, EV_ADD, 0, time, CAST(&src.second)};
        add (src.first);
        return true;
    }

    bool Kqueue::add_timer_ms (timer_t id, i64_t time, const callable&& callback)
    {
        if (map.contains(id)) 
            return false;

        auto &src = map[id];
        src.second = {std::move(callback), Type_ID::TIMER};
        src.first = {id, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, time, CAST(&src.second)};
        add (src.first);
        return true;
    }

    bool Kqueue::add_clock_sec (tick_t id, i64_t time, const callable&& callback)
    {
        if (map.contains(id)) 
            return false;
        // map.try_emplace(id, std::make_pair<struct kevent64_s, data>({}, {std::move(callback), Type_ID::TIMER}));
        map[id] = std::make_pair<struct kevent64_s, data>({}, {std::move(callback), Type_ID::TIMER});
        // auto &src = map[id];
        // src.first = {id, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, time, CAST(&src.second)};
        // add (src.first);
        return true;
    }

    bool Kqueue::add_timer_sec (timer_t id, i64_t time, const callable&& callback)
    {
        // if (map.contains(id))
        //     throw Error("id already in list");
        // auto src = map.try_emplace(id, MAKE_MAP_PAIR({}, callback, Type_ID::TIMER)).first->second;
        // src.first = {id, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, time, CAST(&src.second)};
        // add (src.first);
        return true;
    }

    void Kqueue::remove_timer(timer_t id)
    {
        if (!map.contains(id))
            throw Error("timer id not found");
        
        map[id].first.flags = EV_DELETE;
        add(map[id].first);
        map.erase(id);
    }

    void Kqueue::remove_clock(tick_t id)
    {
        if (!map.contains(id))
            throw Error("clock id not found");
        
        map[id].first.flags = EV_DELETE;
        add(map[id].first);
        map.erase(id);
    }

    void Kqueue::add (const struct kevent64_s& ev)
    {
        int ret = ::kevent64(kq, &ev, 1, nullptr, 0, 0, &timeout);
        if (ret == -1)
            throw Error(std::strerror(errno));
    }

    void Kqueue::poll_helper (kevent64_s* eventList, std::size_t size)
    {
        std::size_t Nchanges;
        struct kevent64_s* ev;
        struct data* data;

        Nchanges = ::kevent64(kq, NULL, 0, eventList, size, 0,&timeout);
        if (Nchanges == -1)
            throw std::runtime_error(std::strerror(errno));

        for (std::size_t i = 0; i < Nchanges; i++)
        {
            ev = &eventList[i];
            data = reinterpret_cast<struct data*>(ev->udata);

            if (ev->flags & EV_ERROR)
                Error("EV_ERROR");

            data->callback(ev);

            if (ev->flags & EV_ONESHOT)
            {
                switch (data->type)
                {
                    case Type_ID::KERNEL:   map.erase(fd_t(ev->ident)); break;
                    case Type_ID::USER:     map.erase(user_t(ev->ident)); break;
                    case Type_ID::SIGNAL:   map.erase(signal_t(ev->ident)); break;
                    case Type_ID::TIMER:    map.erase(timer_t(ev->ident)); break;
                    case Type_ID::TICK:    map.erase(tick_t(ev->ident)); break;
                    
                }
            }
        }
    }

    Error_::Error_ (const char* msg, int line, const char* file)
    : message(std::format("{}:{} {}", file, line, msg))
    {}
    Error_::Error_ (int line, const char* file)
    : message(std::format("{}:{}", line, file))
    {}
    const char* Error_::what() const noexcept
    {
        return message.c_str();
    }

}
