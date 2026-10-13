#include<iostream>
#include<chrono>
#include<thread>
#include<condition_variable>
#include<mutex>

using namespace std::chrono;

int main()
{
    std::mutex mtx;
    std::condition_variable cv;
    bool ready=false;
    /*
        The logic is that since ready would be false and t2 is checking with respect to ready so it will go too sleep 
        t1 will make it true and notify t2 after 2 sec
    */
    std::thread t1([&]{
        
        std::cout<<"[T1] goes to sleep for 2 sec\n";   
        std::this_thread::sleep_for(seconds(2));
        {
            std::unique_lock lock{mtx};// protect (ready )write against read by t2
            ready=true;
            // after this scope the lock would be released as it's used via unique_lock 
        }
        std::cout<<"2 sec done updating variable from [T1]\n";
        cv.notify_one();
    });    

    std::thread t2([&]{

        std::unique_lock lock{mtx};

        // even if t2 runs first and comes to wait condition the lock would itself unlock it or any further thread use
        cv.wait(lock,[&]{return ready;});

        std::cout<<"[T2] acuired the lock\n";
    });
    t1.join();
    t2.join();

    return 0;
}