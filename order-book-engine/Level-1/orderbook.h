#include "Order.h"
#include<iostream>

/*
    A order book is the storing the data and would store the basic structure 
*/

class OrderBook{
    private:
    std::string symbol;
    std::vector<Order>buyorder;
    std::vector<Order>sellorder;

    public:
    /*
    To prevent privite slient / implicent conversion by the complier if a future function calls like orderbook()
    */
    //1. constuctor for a specific security
    explicit OrderBook(const std::string &sym);

    //2. add the order
    void add_order(const Order& order);
    
    //3. display the order
    void display_book() const;

    //4. number of order
    size_t get_total_order() const;
};
