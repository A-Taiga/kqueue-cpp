#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <cstddef>
#include <cstring>
#include <ctime>
#include <exception>
#include <source_location>
#include <unordered_map>
#include "kqTypes.hpp"

#define CAST_OBJ(X) ((uintptr_t)(X))

namespace KQ 
{
    class Error : public std::exception
    {
        private:
            std::string message;
        public:
            Error (std::string msg, std::source_location = std::source_location());
            Error (std::source_location = std::source_location());
            virtual const char* what() const noexcept;
    };
    
    class Kqueue
    {
        private:
            using callable = std::function<void(struct kevent64_s*)>;
            int kq;
            std::timespec timeout;
            std::unordered_map<genericID_t, data, std::hash<genericID_t>> map;
            void remove_from_list(const genericID_t& id);
            void add_helper (const genericID_t& id, const i64_t& filter, const u16_t& flags, const u32_t& fflags, const i64_t& data, const u64_t& udata);
            void poll_helper(kevent64_s* eventList, std::size_t size);
        public:
            Kqueue (std::timespec timeout);
            bool add_clock_ms (timer_t id, i64_t time, const callable&& callback);
            bool add_clock_sec (timer_t id, i64_t time, const callable&& callback);
            bool add_timer_sec (timer_t id, i64_t time, const callable&& callback);
            void remove_timer(timer_t id);

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


