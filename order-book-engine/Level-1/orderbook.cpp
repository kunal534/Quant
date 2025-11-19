#include "Orderbook.h"
#include<iomanip>// input output manipulator
#include<iostream>

OrderBook::OrderBook(const std::string& sym):symbol(sym){
    std::cout<<"Created Order book for: "<<sym<<"\n";
}

void OrderBook::add_order(const Order& order)
{
    // now ther are 2 cases either a orderbook existing for this or else not then create
    if(order.SYM!= symbol)
    {
        std::cout<<"Error in order symbol matching"<<"\n";
        return;
    }
    if(order.side==OrderSide::BUY)
    {
        buyorder.push_back(order);
        std::cout<<"Buy order updated"<<order.orderId<<"\n";
    }
    else{
        sellorder.push_back(order);
        std::cout<<"Sell order updated"<<order.orderId<<"\n";
    }
}
void OrderBook:: display_book() const
{
    std::cout << "\n========== ORDER BOOK: " << symbol << " ==========\n\n";
    std::cout << "SELL ORDERS (Asks):\n";
    std::cout << std::setw(10) << "Order ID" 
              << std::setw(10) << "Price" 
              << std::setw(10) << "Quantity" << "\n";
    std::cout << "----------------------------------------\n";

    if(sellorder.empty())
    {
        std::cout<<"no sell order"<<"\n";
    }else{
        for(const auto&order:sellorder)
        {
            // setw to show number of character
            std::cout<<std::setw(10)<<order.orderId
                     <<std::setw(10)<<std::fixed<<std::setprecision(2)<<order.price
                     <<std::setw(10)<<order.quantity<<"\n";
        }
    }
    std::cout<<"\n";
    // Display BUY orders (bids)
    std::cout << "BUY ORDERS (Bids):\n";
    std::cout << std::setw(10) << "Order ID" 
              << std::setw(10) << "Price" 
              << std::setw(10) << "Quantity" << "\n";
    std::cout << "----------------------------------------\n";
    
    if (buyorder.empty()) {
        std::cout << "  (no buy orders)\n";
    } else {
        for (const auto& order : buyorder) {
            std::cout << std::setw(10) << order.orderId
                      << std::setw(10) << std::fixed << std::setprecision(2) << order.price
                      << std::setw(10) << order.quantity << "\n";
        }
    }
    
    std::cout << "\n==========================================\n\n";
}

size_t OrderBook:: get_total_order()const{
    return buyorder.size()+sellorder.size();
}
// a log entry to get us a one liner 
void Order::print() const
{
    std::cout<<"Order: "<<orderId<<(side==OrderSide::BUY?"BUY":"SELL")<<
    " "<<quantity<<" Share of "<<SYM<<" @ $"<<std::fixed<<std::setprecision(2)<<price<<"\n";
}   
