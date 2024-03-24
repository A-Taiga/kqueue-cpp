#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <cstddef>
#include <cstring>
#include <ctime>
#include <exception>
#include <unordered_map>
#include "kqTypes.hpp"


#define Error(...) Error_(__VA_ARGS__ __VA_OPT__(,) __LINE__, __FILE__)

namespace KQ 
{
    class Error_ : public std::exception
    {
        private:
            std::string message;
        public:
            Error_(const char* msg, int line, const char* file);
            Error_(int line, const char* file);
            virtual const char* what() const noexcept;
    };
    
    class Kqueue
    {
        private:
            int kq;
            std::timespec timeout;
            std::unordered_map <genericID_t, std::pair <struct kevent64_s, data>, std::hash<genericID_t>> map;
            void add (const struct kevent64_s& ev);
            void poll_helper(kevent64_s* eventList, std::size_t size);
        public:
            Kqueue (std::timespec timeout);
            /* kernel events */
            bool add_vnode_ev (fd_t id, EVF flags, VFF fflags, const callable&& callback);
            /* userspace events */
            bool add_custom_ev (user_t id, EVF flags, UFF fflags, const callable&& callback);
            bool tirgger_custom_ev (user_t id, EVF flags = EVF::NONE, UFF fflags = UFF::NOP);
            bool update_custom_ev (user_t id, EVF flags, UFF fflags);
            bool add_custom_fflags (user_t id, UFF option, int flags);
            bool remove_custom_ev (user_t id);
            /* clocks and timers */
            bool add_clock_ms (tick_t id, i64_t time, const callable&& callback);
            bool add_timer_ms (timer_t id, i64_t time, const callable&& callback);
            bool add_clock_sec (tick_t id, i64_t time, const callable&& callback);
            bool add_timer_sec (timer_t id, i64_t time, const callable&& callback);
            void remove_timer(timer_t id);
            void remove_clock(tick_t id);
            
            template <std::size_t MAX_EVENTS>
            void poll();
    };

    template <std::size_t MAX_EVENTS>
    inline void Kqueue::poll()
    {
        struct kevent64_s eventList[MAX_EVENTS];
        poll_helper(eventList, MAX_EVENTS);
    }
}

#endif


