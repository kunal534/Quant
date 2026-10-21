#include<iostream>
#include<thread>
#include<atomic>
#include<cassert>
/*
    1. View of the threads does not need to be consisten to each other, here view means what a specific thread would see at the memory location either at cache or ram 
    2. No restruction on instruction reordering 

    NOTE: 
    In case of relaxed it remove the ordering constraint from complier which gives us the maximum performance but could result in unpredict behavior
*/

int payload=0;

std::atomic<bool>ready{false};

void producer(){
    payload=42;
    /*
        With relaxed type there could be a chance that complier may move the line 22 operation above the 18 and if that's done consumer spinnig loop would be broken which would result in payload data showing 0 instead of 42
    */
    ready.store(true,std::memory_order_relaxed);
}

void consumer(){
    while(!ready.load(std::memory_order_relaxed))
    {
        //spinning
    }
    std::cout<<"payload value "<<payload<<std::endl;
}

int main()
{
    std::thread a(producer);
    std::thread b(consumer);
    a.join();
    b.join();
    return 0;
}