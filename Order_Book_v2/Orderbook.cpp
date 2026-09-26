#include "Orderbook.h"

#include<numeric>
#include<chrono>

void Orderbook::OnOrderAdded(OrderPointer order)
{
    UpdateLevelInfo(order->GetPrice(),order->GetRemainingQuantity(),LevelActions::Action::Add);
}
void Orderbook::UpdateLevelInfo(Price price,Quantity quantity,LevelActions::Action action)
{
    auto& Price_data=data_[price];
    Price_data.OrderCount_ += action == LevelActions::Action::Remove?-1 : action == LevelActions::Action::Add?1:0;
    if(action == LevelActions::Action::Remove || action==LevelActions::Action::Match)
    {
        Price_data.quantity_-=1;        
    }
    else{
        Price_data.quantity_+=1;
    }
    if(Price_data.OrderCount_==0)
    {
        data_.erase(price);
    }
}

Trades Orderbook::MatchOrders()
{
    Trades trades;
    trades.reserve(orders_.size());
    while(true)
    {
        auto&[BidPrice,bids]=*bids_.begin();
        auto&[AskPrice,asks]=*asks_.begin();

        if(BidPrice<AskPrice)break;
        
        while(!bids.empty() && !asks.empty())
        {
            // from the list getting first entry
            auto bid=bids.front();
            auto ask=asks.front();

            Quantity common_q=std::min(bid->GetRemainingQuantity(),ask->GetRemainingQuantity());

            bid->Fill(common_q);
            ask->Fill(common_q);

            if(bid->isFilled())
            {
                bids.pop_front();
                orders_.erase(bid->GetOrderId());
            }

            if(ask->isFilled())
            {
                asks.pop_front();
                orders_.erase(ask->GetOrderId());
            }

            trades.push_back(Trade{
                Trade_Info(bid->GetOrderId(),bid->GetPrice(),common_q),
                Trade_Info(ask->GetOrderId(),ask->GetPrice(),common_q)
            });
        }
        if(bids.empty())
        {
            bids_.erase(BidPrice);
            data_.erase(BidPrice);
        }
        if(asks.empty())
        {
            asks_.erase(AskPrice);
            data_.erase(AskPrice);
        }
        // now if there are any order lefts of fill and kill they are need to be removed
        if(!asks_.empty())
        {
            auto&[_,asks]=*asks_.begin();
            auto& best_order=asks.front();
            if(best_order->GetOrderType()==OrderType::Fill_and_Kill)
            {
                CancelOrder(best_order->GetOrderId());
            }
        }

        if(!bids_.empty())
        {
            auto&[_,bids]=*bids_.begin();
            auto& best_order=bids.front();
            if(best_order->GetOrderType()==OrderType::Fill_and_Kill)
            {
                CancelOrder(best_order->GetOrderId());
            }
        }
    }
    return trades;
}
Trades Orderbook::AddOrder(OrderPointer order)
{
    std::scoped_lock orderlock{ ordersMutex_ };
    if(orders_.contains(order->GetOrderId()))
    {
        return {};
    }
    // if market type meaning it can't sit in orderbook as it's price is nan
    // here aggresice rest is used which means if all quantity is not filled rest would be kept 
    if(order->GetOrderType()==OrderType::Market)
    {
        if(order->GetSide()==Side::Buy && !asks_.empty())
        {
            // used deference as rbegin provides a pointer 
            const auto&[worstAsk,_]=*asks_.rbegin();
            order->ToGoodTillCancel(worstAsk);
        }
        else if(order->GetSide()==Side::Sell && !bids_.empty())
        {
            const auto&[worstBid,_]=*bids_.rbegin();
            order->ToGoodTillCancel(worstBid);
        }
        else
        {
            return {};
        }
    }    
    else if(order->GetOrderType()==OrderType::Fill_and_Kill && !CanMatch(order->GetSide(),order->GetPrice()))
    {
        return {};
    }
    else if(order->GetOrderType()==OrderType::Fill_or_Kill && !CanFullyFill(order->GetSide(),order->GetPrice(),order->GetRemainingQuantity()))
    {
        return {};
    }
    // if no issue till now an iterator can be created safetly

    OrderPointers::iterator iterator;

    if(order->GetSide()==Side::Buy)
    {
        auto& orders=bids_[order->GetPrice()];// return orderpointer
        orders.push_back(order);
        iterator=std::prev(orders.end());
    }

    if(order->GetSide()==Side::Sell)
    {
        auto& orders=asks_[order->GetPrice()];
        orders.push_back(order);
        iterator=std::prev(orders.end());
    }
    orders_.insert({order->GetOrderId(),OrderEntry({order,iterator})});
    OnOrderAdded(order);
    return MatchOrders();
}