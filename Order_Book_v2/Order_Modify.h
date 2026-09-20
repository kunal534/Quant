#pragma once

#include "Order.h"

class OrderModify{
    public:
    OrderModify(OrderId orderid,Price price,Quantity quantity, Side side):orderid_{orderid},price_{price},quantity_{quantity},side_{side}
    {}

    // dummy kpi

    OrderId GetOrderId() const {return orderid_;}
    Price GetPrice() const {return price_;}
    Quantity GetQuantity() const {return orderid_;}
    Side GetSide() const {return side_;}

    // a shared poiner to store new order which are modified

    OrderPointer ToOrderModify(OrderType type)const
    {
        return std::make_shared<Order>(type,GetOrderId(),GetPrice(),GetQuantity());
    }
    
    private:
    OrderId orderid_;
    Price price_;
    Quantity quantity_;
    Side side_;
};