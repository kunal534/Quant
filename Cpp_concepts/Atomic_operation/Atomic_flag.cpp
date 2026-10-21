#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

using namespace std::chrono;

class Spinlock
{
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

public:
    // purpose of memory_order_acquire is to make sure that complier doesn;t change the order of execution for optimization
    void Lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire))
        {
        }
    }

    bool is_Lock()
    {
        return flag_.test_and_set(std::memory_order_acquire);// return previous value and update flag value
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
        lock.Lock();// set's the flag_ to 1 
        std::cout << "[T1] lock acquired. holding for 2 seconds\n";

        std::this_thread::sleep_for(seconds(2));

        std::cout << "[T1] releasing lock\n";
        lock.Unlock();
    });


    std::thread t2([&]
    {
        std::cout << "[T2] trying to acquire lock\n";
        int attempts = 0;

        while (lock.is_Lock())
        {
            ++attempts;
            std::cout << "[T2] lock is busy, retrying... (attempt "
                      << attempts << ")\n";
        }

        std::cout << "[T2] lock acquired after " << attempts
                  << " attempts\n";
    });

    t1.join();
    t2.join();

    return 0;
}