#include<iostream>
#include<atomic>

int main(){
    std::atomic<bool>t1;
    // load=> gives us linkage path to tell complier to perform a paticular particular read, with this particular ordering memory barrier
    std::cout<<"default value of atomic variable "<<(t1.load())<<"\n";

    t1.store(true);
    std::atomic<bool>t2;
    
    // store operation
    t2.store(t1);

    //load operation -> get's value
    std::cout<<"value in t2 stored via store operation "<<t2.load()<<"\n";

    // exchange pop value and replace it with provided
    bool temp=t2.exchange(false);
    std::cout<<"final value stored in temp after exchange with t2 "<<temp<<"\n";

    // For is_lock_free()=> tells about the hardware implementation of the type itself wheather t1 can be written/read atomically by the CPU without using lock or a mutex
    

    struct LargeData { int a, b, c, d, e, f, g, h; };

    std::atomic<LargeData>big_atomic;
    if(!big_atomic.is_lock_free())
    {
        std::cout<<"LargeData is not lock free. The complier hid a mutex inside it"<<"\n";
    }
    return 0;
}