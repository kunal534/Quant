#include<iostream>
#include<atomic>
#include<thread>

int main(){
    std::atomic<int>temp(20);
    
    int expected_value=20;
    int incorrect_value=5;
    std::cout<<"previous value assocaited with atomic bool "<<temp.load()<<std::endl;
    // template -> atomic varible.compare_exchange_weak(expected, desired) 
    /*
        would update value only if expected == atomic variable.load else produce false with no change
        used when we want to check before proceeding 
    */
    bool value_change_possible=temp.compare_exchange_weak(expected_value,10);
    // bool value_change_possible=temp.compare_exchange_weak(incorrect_value,10);


    /*
        Now there'a catch that in case of ARM architecture compare_exchange_weak can give false negative as there's no direct compare in them and they use :
        LL( Load Linked ): Reads a value from memory and marks it
        SC (Store conditional): Attempts to write a new value, but write would successed only if marked memory is not disturbed via any other interferance 

        For such case compare_exchange_strong is use which gurantee atomicity for both x86 and arm architecture it does via a loop along with weak exchange


    */
    std::atomic<int>temp2(10);

    bool value_change_via_strong=temp2.compare_exchange_strong(incorrect_value,100);
    std::cout<<"operation successfull "<<(value_change_possible?"Yes":"No")<<std::endl;
    std::cout<<"value after CAS(Compare and swap instruction) "<<temp.load()<<std::endl;


    return 0;
}