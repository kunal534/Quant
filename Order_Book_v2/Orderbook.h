#pragma once

#include "Using.h"
#include "Order.h"
#include "Order_Modify.h"
#include "LevelInfo.h"
#include "Trade.h"

#include<map>
#include<unordered_map>
class Orderbook{
    private:

    // for deletion/ updation orderid is associated with it's location
    struct OrderEntry{
        OrderPointer order_{nullptr};
        OrderPointers::iterator location_;
    };
    // for every accumulated ask/bids at a certain price
    struct LevelData{
        Quantity quantity_{0};// total number of order at a price
        Quantity OrderCount_{};// total count of order
        enum class action{
            Add,
            Remove,
            Match
        };
    };
    
    // data at per price
    std::unordered_map<Price,OrderEntry>data_;
    // bids
    std::map<Price,OrderPointers,std::greater<Price>>bids_;

    // asks
    std::map<Price,OrderPointers,std::less<Price>>asks_;

    // to get order location quickly
    std::unordered_map<OrderId,OrderEntry>order_;

    mutable std::mutex ordersMutex;
    public:

    Orderbook();
    Orderbook(const Orderbook&) = delete;
    void operator=(const Orderbook& ) = delete;
    Orderbook(const Orderbook&& )=delete;
    void operator= (const Orderbook&& )=delete;
    ~Orderbook();

    Trades AddOrder(OrderPointer order);
    void CancelOrder(OrderId orderid);
    Trades ModifyOrder(OrderPointer order);
};