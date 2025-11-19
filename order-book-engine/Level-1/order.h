// stores the order file structure 

#pragma once // tells the complier to load this file only ounce
#include<string>
#include<cstdint> 
/* 
provides fixed-width integer types accross various structure irrespective of the os exampe for example C++ only gurantee that
a certain about of bit would be provided for he specific data type but it could change with respect to os, address bus, processor
*/

// we will use an enum class so that orders can be match
enum class OrderSide{
    BUY,
    SELL
};

// stores the order structure

struct Order{
    uint64_t orderId;
    std::string SYM; // stock symbol name
    OrderSide side; // Buy or sell
    double price;   // price in ticks (e.g., cents or pips)
    uint64_t quantity;

    Order(uint64_t id,std::string name,OrderSide s,double p,uint64_t q):orderId(id),SYM(name),side(s),price(p),quantity(q)
    {}
    void print() const; // This function will not modify any member variables of the object so that accidently no one can update the value
};