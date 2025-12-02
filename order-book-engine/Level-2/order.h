// stores the basic structure and upgraded with time for the pricelevel using chrono

#pragma once
#include<cstdint>
#include<chrono>
#include<string>

enum class OrderSide{
    BUY,
    SELL
};

enum class OrderType{
    MARKET,// execute immediately at any price
    LIMIT// executre greater than the aligned price
};

struct Order{
    uint64_t orderId;
    uint64_t timestamp;
    std::string Symbol;
    OrderSide Side;
    OrderType Type;
    double price;
    uint64_t quantity;
    uint64_t filledquantity;

    Order(uint64_t id,std::string sym,double p,OrderSide s,OrderType t,uint64_t q):orderId(id),Symbol(sym),price(p),Side(s), Type(t),quantity(q),filledquantity(0),timestamp(getCurrentTimestamp()){};

    static uint64_t getCurrentTimestamp(){
        // steady_clock=> a monotonic clock that moves only in forward ,now as in Captures the current time point according to the steady clock.
        // time_since_epoch => since the time system has started
        return std::chrono::steady_clock::now().time_since_epoch().count();
    }
    uint64_t remaining_order(){
        return quantity-filledquantity;
    }
    void print() const;
    
    bool isFilled()
    {
        return filledquantity>=quantity;
    }
    void addfilled(uint64_t amount)
    {
        quantity+=amount;
    }
};

