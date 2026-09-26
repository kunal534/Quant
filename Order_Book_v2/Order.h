#pragma once
#include<format>
#include "OrderType.h"
#include "Using.h"
#include "Side.h"
#include "Nan.h"

#include<vector>
#include<list>

class Order{
    public:

    Order(OrderType ordertype,OrderId orderid, Price price, Quantity quantity, Side side ):ordertype_{ordertype},price_{price},initialQuantity_{quantity},side_{side},remainingQuantity_{quantity},orderid_{orderid}
    {}

    // For atmaket price we don't care the price so a non value should be 
    Order(OrderId orderid,Side side,Quantity quantity):Order(OrderType::Market,orderid,nan::InvalidPrice,quantity,side)
    {}

    // dummy api

    OrderId GetOrderId() const {return orderid_;}
    Quantity GetRemainingQuantity() const {return remainingQuantity_;}
    Price GetPrice() const {return price_;}
    Side GetSide() const {return side_;}
    OrderType GetOrderType() const {return ordertype_;}
    bool isFilled() const { return remainingQuantity_==0?1:0; }
    
    void Fill(Quantity quantity)
    {
        if(quantity > GetRemainingQuantity())
        {
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than it's remaining quantity",GetOrderId()));
        }
    }

    void ToGoodTillCancel(Price price ){
        if(GetOrderType()!=OrderType::Market)
        {
            throw std::logic_error(std::format("Order ({}) cannot have it's price adjusted only Market type can",GetOrderId()));

        }
        price_=price;
        ordertype_=OrderType::Good_Till_Cancel;
    }
    private:
    OrderType ordertype_;
    Price price_;
    OrderId orderid_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
    Side side_;
};
using OrderPointer=std::shared_ptr<Order>;
using OrderPointers=std::list<OrderPointer>;