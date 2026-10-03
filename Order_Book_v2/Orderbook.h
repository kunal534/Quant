#pragma once

#include "Using.h"
#include "Order.h"
#include "Order_Modify.h"
#include "LevelInfo.h"
#include "Trade.h"

#include<map>
#include<thread>
#include<unordered_map>
class Orderbook{
    private:

    // for deletion/ updation orderid is associated with it's location
    struct OrderEntry{
        OrderPointer order_{nullptr};
        OrderPointers::iterator location_;
    };
    // for every accumulated ask/bids at a certain price
    struct LevelActions{
        Quantity quantity_{0};// total number of order at a price
        Quantity OrderCount_{};// total count of order
        enum class Action{
            Add,
            Remove,
            Match
        };
    };
    
    // data at per price
    std::unordered_map<Price,LevelActions>data_;
    // bids
    std::map<Price,OrderPointers,std::greater<Price>>bids_;

    // asks
    std::map<Price,OrderPointers,std::less<Price>>asks_;

    // to get order location quickly
    std::unordered_map<OrderId,OrderEntry>orders_;

    // mutex is termed as mutable as function are const and mutex state would need to be change from lock to unlock 
    mutable std::mutex ordersMutex_;
    std::thread orderPruneThread_;

    bool CanFullyFill(Side side,Price price,Quantity quantity)const;
    bool CanMatch(Side side, Price price)const;
    void OnOrderAdded(OrderPointer);
    void OnOrderCancel(OrderPointer);
    Trades MatchOrders();
    void UpdateLevelInfo(Price price,Quantity quantity,LevelActions::Action action);
    public:

    Orderbook();
    Orderbook(const Orderbook&) = delete; // copy constructor
    void operator=(const Orderbook& ) = delete; // copy assignment
    Orderbook(const Orderbook&& )=delete; // move constructor
    void operator= (const Orderbook&& )=delete; // move assignment
    ~Orderbook();

    Trades AddOrder(OrderPointer order);
    void CancelOrder(OrderId orderid);
    void CancelOrders(OrderIds orderids);
    void CancelOrderInternals(OrderId orderid);
    Trades ModifyOrder(OrderModify order);
};