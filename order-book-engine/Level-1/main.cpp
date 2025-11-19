#include "Orderbook.h"
#include<iostream>

int main(){
    std::cout<<"Starting with the basic"<<"\n";

    OrderBook stock("APPLE");

    // adding the stock
    std::cout<<"Adding sample values "<<"\n";

    uint64_t index=1;// we have kept this a global one 
    /*
        Now to add a order with the specified data strucuture we need to make
        objects of order then add them in order book
    */

    Order b1(index++,"APPLE",OrderSide::BUY,150.00, 100);
    Order b2(index++,"APPLE",OrderSide::BUY,149.50, 200);
    Order b3(index++,"APPLE",OrderSide::BUY,149.00, 150);

    // made the Order objects now lets add it to the orderbook
    stock.add_order(b1);
    stock.add_order(b2);
    stock.add_order(b3);

    std::cout<<"Adding selling order"<<"\n";

    Order s1(index++, "APPLE", OrderSide::SELL, 151.00, 100);
    Order s2(index++, "APPLE", OrderSide::SELL, 151.50, 200);
    Order s3(index++, "APPLE", OrderSide::SELL, 152.00, 150);

    stock.add_order(s1);
    stock.add_order(s2);
    stock.add_order(s3);

    stock.display_book();

    std::cout<<"Total book in order"<<stock.get_total_order()<<"\n";

    std::cout<<"Showcasing a specific entry"<<"\n";
    b1.print();
    s1.print();
    return 0;
}