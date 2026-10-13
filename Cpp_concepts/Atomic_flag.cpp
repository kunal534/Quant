#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

using namespace std::chrono;

class Spinlock
{
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

public:
    void Lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire))
        {
        }
    }

    bool TryLock()
    {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    void Unlock()
    {
        flag_.clear(std::memory_order_release);
    }
};

int main()
{
    Spinlock lock;

    std::thread t1([&]
    {
        std::cout << "[T1] acquiring lock\n";
        lock.Lock();
        std::cout << "[T1] lock acquired. holding for 2 seconds\n";

        std::this_thread::sleep_for(seconds(2));

        std::cout << "[T1] releasing lock\n";
        lock.Unlock();
    });

    std::this_thread::sleep_for(milliseconds(100));

    std::thread t2([&]
    {
        std::cout << "[T2] trying to acquire lock\n";
        int attempts = 0;

        while (!lock.TryLock())
        {
            ++attempts;
            std::cout << "[T2] lock is busy, retrying... (attempt "
                      << attempts << ")\n";
            std::this_thread::sleep_for(milliseconds(300));
        }

        std::cout << "[T2] lock acquired after " << attempts
                  << " attempts\n";
        lock.Unlock();
    });

    t1.join();
    t2.join();

    return 0;
}