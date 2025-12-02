/*
    Data structure to keep different order of same price sorted by time
    so A fifo data struture would be better 
*/
#pragma once 
#include "Order.h"
#include<list>
#include<memory>
#include<algorithm>

class PriceLevel{
    public:
    double price;
    uint32_t totalvolume;// all order of same value
    std::list<std::shared_ptr<Order>>orders;// shared_prt links a pointer that die whenenver the scope ends to no memory leak

    // Now there might be case where someone just type price=13.13 so to avoid that explicit using which states only when called by naming this can the entry be made
    explicit PriceLevel(double p):price(p),totalvolume(0){}

    void addOrder(std::shared_ptr<Order>order)
    {
        orders.push_back(order);
        totalvolume+=order->remaining_order();// total-filled
    }

    void removeOrder(std::shared_ptr<Order>order)
    {
        auto it=std::find(orders.begin(),orders.end(),order);

        if(it!=orders.end())
        {
            totalvolume-=(*it)->remaining_order();
            orders.erase(it);
        }
    }
    // for any update or something happens we need to keep count of totalvolume
    void updatevolume()
    {
        totalvolume=0;
        for(auto it:orders)
        {
            totalvolume+=it->remaining_order();
        }
    }
    // indicators
    bool is_empty() const
    {
        return orders.empty();
    }

    std::shared_ptr<Order> getFirstOrder(){
        return orders.empty() ? nullptr : orders.front();
    }

    size_t getOrdercount()const{
        return orders.size();
    }
};
