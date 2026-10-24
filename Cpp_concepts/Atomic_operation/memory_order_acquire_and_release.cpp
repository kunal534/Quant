#include <atomic>
#include <thread>
#include <iostream>
#include <string>

// Shared data 
std::string document = ""; 
std::atomic<bool> ready{false}; 

void producer() {
    std::cout << "Producer: Writing the document...\n";
    
    // Step 1: Write to a normal, non-atomic variable.
    // This goes into Thread 1's private cache.
    document = "Hello, World!"; 

    // Step 2: RELEASE. 
    // The barrier forces 'document' to be flushed to shared memory 
    // BEFORE 'ready' is set to true.
    ready.store(true, std::memory_order_release); 
    
    std::cout << "Producer: Document published.\n";
}

// ---------------- THREAD 2 (Consumer) ----------------
void consumer() {
    std::cout << "Consumer: Waiting for document...\n";
    
    // Step 3: ACQUIRE.
    // The barrier forces the CPU to wait here and validate the cache.
    // It will NOT let the 'std::cout' below jump up before this check.
    while (!ready.load(std::memory_order_acquire)) {
        // Spin: keep checking until 'ready' is true.
    }

    // Step 4: Read the data.
    // Because of the Acquire, we are GUARANTEED to see "Hello, World!"
    std::cout << "Consumer: Read document -> " << document << "\n";
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
    return 0;
}