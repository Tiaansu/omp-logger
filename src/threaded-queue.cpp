#include "threaded-queue.hpp"


void ThreadedQueue::Dispatch(ThreadedQueue::Callback callback)
{
    std::lock_guard<std::mutex> lock_guard(m_Mutex);
    m_Queue.push(callback);
}

void ThreadedQueue::Process()
{
    std::lock_guard<std::mutex> lock_guard(m_Mutex);
    while (!m_Queue.empty())
    {
        auto action = std::move(m_Queue.front());
        m_Queue.pop();

        action();
    }
}