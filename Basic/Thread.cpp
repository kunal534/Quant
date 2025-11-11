#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include<semaphore>
#include <vector>
using namespace std::chrono_literals;

// ---------------- Shared state ----------------
std::mutex mtx;                      // protects data_ready, price, and buffers
std::condition_variable cv;          // used for producer-consumer signaling
bool data_ready = false;             // simple flag for a one-shot event
double price = 100.0;                // shared price updated by producers

std::vector<double> tick_buffer;     // a simple queue/buffer (not lock-free)
const size_t batch_target = 4;       // when we hit 4, wake all consumers

std::atomic<int> ticks_count{0};     // lock-free stat counter

// Limit concurrent pricing workers (e.g., external API quota/CPU pool)
// Fallback semaphore implementation for environments that lack std::counting_semaphore
class Semaphore {
    std::mutex m;
    std::condition_variable cv;
    int count;
public:
    explicit Semaphore(int n) : count(n) {}
    void acquire() {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&]{ return count > 0; });
        --count;
    }
    void release() {
        std::lock_guard<std::mutex> lock(m);
        ++count;
        cv.notify_one();
    }
};
Semaphore worker_slots(3); // allow up to 3 concurrent workers

// ---------------- Mutex + lock_guard ----------------
// Safe price update: protects critical section with a mutex.
void update_price(double delta) {
    std::lock_guard<std::mutex> guard(mtx);    // RAII lock
    price += delta;
    // No manual unlock; guard releases on scope exit.
    std::cout << "[update_price] price=" << price << "\n";
}

// ---------------- Condition variable: notify_one ----------------
// Producer: pushes a tick and notifies one consumer to process it.
void producer_one_shot(double new_tick) {
    {
        std::lock_guard<std::mutex> guard(mtx);
        tick_buffer.push_back(new_tick);
        data_ready = true;                      // signal condition
        ++ticks_count;                          // lock-free stat
        std::cout << "[producer_one_shot] pushed " << new_tick
                  << ", ticks_count=" << ticks_count.load() << "\n";
    }
    // Notify a single waiter; which thread wakes is unspecified.
    cv.notify_one();
}

// ---------------- Condition variable: notify_all (batch) ----------------
// Producer: accumulates into a batch; when size reaches target, wake everyone.
void producer_batched(double new_tick) {
    bool reached_batch = false;
    {
        std::lock_guard<std::mutex> guard(mtx);
        tick_buffer.push_back(new_tick);
        ++ticks_count;
        reached_batch = (tick_buffer.size() >= batch_target);
        if (reached_batch) {
            std::cout << "[producer_batched] batch_ready size="
                      << tick_buffer.size() << "\n";
        } else {
            std::cout << "[producer_batched] queued size="
                      << tick_buffer.size() << "\n";
        }
    }
    if (reached_batch) {
        // Wake all consumers; each will re-check its predicate.
        cv.notify_all();
    }
}

// ---------------- Consumer using unique_lock + predicate ----------------
// Consumer waits efficiently until there is something to do.
void consumer_process() {
    for (;;) {
        std::vector<double> local;   // drain some work
        {
            std::unique_lock<std::mutex> lock(mtx);
            // Predicate handles spurious wakeups correctly.
            cv.wait(lock, [] {
                return !tick_buffer.empty() || data_ready; // example predicates
            });

            // Move some work out under the lock (keep critical section short)
            if (!tick_buffer.empty()) {
                // Take at most two ticks this round to share work fairly
                size_t take = std::min<size_t>(2, tick_buffer.size());
                local.insert(local.end(),
                             tick_buffer.begin(),
                             tick_buffer.begin() + take);
                tick_buffer.erase(tick_buffer.begin(),
                                  tick_buffer.begin() + take);
            }
            // Reset one-shot flag if used
            data_ready = !tick_buffer.empty();
        }

        // Process outside the lock
        for (double t : local) {
            // Simulate pricing workload guarded by a worker pool
            worker_slots.acquire(); // semaphore limits concurrent work
            std::thread([t] {
                std::this_thread::sleep_for(50ms); // pretend compute
                std::cout << "[consumer] priced tick=" << t << "\n";
                worker_slots.release();
            }).detach();
        }

        // Exit condition for demo (not typical in services)
        if (ticks_count.load() >= 12 && tick_buffer.empty()) {
            break;
        }
    }
}

// ---------------- Demo main ----------------
int main() {
    std::thread c1(consumer_process);
    std::thread c2(consumer_process);

    // Demonstrate mutex-protected update (would race without the mutex)
    std::thread u1([] { update_price(+0.25); });
    std::thread u2([] { update_price(-0.10); });

    // One-shot notifications (notify_one)
    std::thread p1([] { producer_one_shot(100.5); });
    std::thread p2([] { producer_one_shot(100.7); });

    // Batched notifications (notify_all when batch threshold reached)
    std::thread p3([] { producer_batched(100.9); });
    std::thread p4([] { producer_batched(101.1); });
    std::thread p5([] { producer_batched(101.3); });
    std::thread p6([] { producer_batched(101.5); });

    u1.join(); u2.join();
    p1.join(); p2.join(); p3.join(); p4.join(); p5.join(); p6.join();
    c1.join(); c2.join();

    std::cout << "[main] final price=" << price
              << ", ticks_count=" << ticks_count.load() << "\n";
}
