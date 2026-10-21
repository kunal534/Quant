#include<iostream>
#include<thread>
#include<atomic>
#include<cassert>
// by default complier uses memory order sequential 
// all value data type is atomic so no data race and all would be lock free 
std::atomic<bool>x{false},y{false};
std::atomic<int>z{0};

void write_x()
{
    x.store(true,std::memory_order_seq_cst);
}

void write_y()
{
    y.store(true,std::memory_order_seq_cst);
}

void read_x_then_y()
{
    // looping still x is true
    while(!x.load(std::memory_order_seq_cst))
    {}

    if(y.load(std::memory_order_seq_cst))
    {
        z++;
    }
}

void read_y_then_x()
{
    while(!y.load(std::memory_order_seq_cst))
    {}

    if(x.load(std::memory_order_seq_cst))
    {
        z++;
    }
}
int main()
{

    std::thread thread_a(write_x);
    std::thread thread_b(write_y);
    std::thread thread_c(read_x_then_y);
    std::thread thread_d(read_y_then_x);

    thread_a.join();
    thread_b.join();
    thread_c.join();
    thread_d.join();

    // if this statement is false the complier would stop the program then and there
    assert(z.load()!=0);

    std::cout << "Final value of z: " << z.load() << "\n";
    return 0;
}