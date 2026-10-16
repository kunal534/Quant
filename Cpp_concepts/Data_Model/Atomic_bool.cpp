#include<iostream>
#include<atomic>

int main(){
    std::atomic<bool>t1;
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
    std::cout<<"now t2 is turned false as "<<t2.load()<<" hence lock is free "<<t2.is_lock_free()<<"\n";
    return 0;
}