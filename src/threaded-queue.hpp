// Taken from pawn-websockets & samp mysql

#pragma once

#include "singleton.hpp"

#include <queue>
#include <mutex>
#include <functional>

class ThreadedQueue : public Singleton<ThreadedQueue>
{
    typedef std::function<void()> Callback;
    typedef std::queue<Callback> CallbackQueue;

    friend class Singleton<ThreadedQueue>;

public:
    void Dispatch(Callback callback);
    void Process();

private:
    CallbackQueue m_Queue;
    std::mutex m_Mutex;

    ThreadedQueue() = default;
    ~ThreadedQueue() = default;
};