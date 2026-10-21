#include<iostream>
#include<atomic>

int main()
{
    
    int arr[20];
    for(int i=0;i<20;i++)
    {
        arr[i]=i+1;
    }
    std::atomic<int*>temp(arr);
    std::cout<<"Is Atomic integer pointer lock free ?"<<(temp.is_lock_free()?"Yes":"No")<<std::endl;

    int *temp_p=arr+3;

    std::cout<<" value reference by pointer "<<*(temp.load())<<"\n";

    // since here it's lock free weak can be used
    temp.store(temp_p);
    bool new_value=temp.compare_exchange_weak(temp_p,arr+10);
    std::cout<<"store operation successfull "<<(new_value?"Yes":"No")<<"\n";
    std::cout<<" new value by pointer "<<*(temp.load())<<std::endl;

    std::atomic<int*> temp1(arr);
    temp1.fetch_add(10);
    std::cout<<"increased value of arr[0] by 10 via fetch_add "<<*temp1.load()<<std::endl;
    temp1.fetch_sub(10);
    std::cout<<"decreased value of arr[0] by 10 via fetch_add "<<*temp1.load()<<std::endl;

    return 0;
}