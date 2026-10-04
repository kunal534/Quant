#include "Orderbook.h"

#include<numeric>
#include<chrono>
#include<ctime>

void Orderbook::CancelOrderInternals(OrderId orderid)
{
    if(!orders_.contains(orderid))return;

    const auto [order,iterator]=orders_.at(orderid);
    orders_.erase(orderid);
    if(order->GetSide()==Side::Buy)
    {
        auto price=order->GetPrice();
        auto& orders_at_price=bids_.at(price);// orderpointer list at a specific bids
        orders_at_price.erase(iterator);
        if(orders_at_price.empty())
        {
            bids_.erase(price);
        }
    }
    else{
        auto price=order->GetPrice();
        auto& orders_at_price=asks_.at(price);
        orders_at_price.erase(iterator);
        if(orders_at_price.empty())
        {
            asks_.erase(price);
        }
    }
}

void Orderbook::CancelOrders(OrderIds orderids)
{
    std::scoped_lock orderbook_lock{ordersMutex_};
    for(const auto& orderid:orderids)
    {
        CancelOrderInternals(orderid);
    }
}
void Orderbook::CancelOrder(OrderId orderid)
{
    std::scoped_lock orderbook_lock{ ordersMutex_};
    CancelOrderInternals(orderid);
}


void Orderbook::OnOrderCancel(OrderPointer order)
{
    UpdateLevelInfo(order->GetPrice(),order->GetRemainingQuantity(),LevelActions::Action::Remove);
}
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
            order->GoodTillCancel(worstAsk);
        }
        else if(order->GetSide()==Side::Sell && !bids_.empty())
        {
            const auto&[worstBid,_]=*bids_.rbegin();
            order->GoodTillCancel(worstBid);
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
bool Orderbook::CanMatch(Side side, Price price)const{
    if(side==Side::Buy)
    {
        if(asks_.empty())
        {
            return false;
        }
        const auto& [best_asks,_]=*asks_.begin();
        return price>=best_asks;
    }
    else
    {
        if(bids_.empty())
        {
            return false;
        }
        const auto& [best_bids,_]=*bids_.begin();
        return price<=best_bids;
    }
}
bool Orderbook::CanFullyFill(Side side, Price price, Quantity quantity)const
{
    if(!CanMatch(side,price))
    {
        return false;
    }
    std::optional<Price>threshold;
    if(side==Side::Buy)
    {
        const auto& [best_ask,_]=*asks_.begin();
        threshold=best_ask;
    }
    else
    {
        const auto& [best_bid,_]=*bids_.begin();
        threshold=best_bid;
    }
    for(const auto&[levelPrice,levelData]:data_)
    {
        // since data stores both ask and bid we need to filter out values which are purely bid or purely ask
        if(threshold.has_value()&&(
            (side==Side::Buy && threshold.value()<levelPrice)||
            (side==Side::Sell && threshold.value()>levelPrice)
        )){
            continue;
        }

        if((side==Side::Buy && price<levelPrice)||
    (side==Side::Sell && price>levelPrice))
    {
        continue;
    }
        if(quantity <=levelData.quantity_)
        {
            return true;
        }
        quantity-=levelData.quantity_;
    }
    return false;
}
Trades Orderbook::ModifyOrder(OrderModify order)
    {   
        OrderType ordertype;
        {
            std::scoped_lock ordersLock { ordersMutex_ };
            if(!orders_.contains(order.GetOrderId()))
            {
                return {};
            }
            const auto& [existing_Order,_]=orders_.at(order.GetOrderId());
            ordertype=existing_Order->GetOrderType();// deference via pointer hence -> and no . 
        }   
        CancelOrder(order.GetOrderId());
        return AddOrder(order.ToOrderPointer(ordertype));
    }

void Orderbook::PruneGoodForDayOrders()
{
    using namespace std::chrono;
    const auto end=hours(16);
    while(true){
        const auto now=system_clock::now();// opaque value -> means that it's like a black box
        const auto now_=system_clock::to_time_t(now);// in numeric value from 19070
        std::tm now_in_parts{};
        localtime_r(&now_,&now_in_parts);// for windows the definition is in reverse order
        
        // current time is passed then make a new thread for tommorow 
        if(now_in_parts.tm_hour>=end.count())
        {
            now_in_parts.tm_mday+=1;
        }
        now_in_parts.tm_hour=end.count();
        now_in_parts.tm_min=0;
        now_in_parts.tm_sec=0;

        auto next=system_clock::from_time_t(mktime(&now_in_parts));// converts back C style time_t to modern time_point
        // tm_year,tm_mday... -> mktime(calender-> epoch seconds) -> time_t (3131451)
        auto sleep_till=next-now + milliseconds(100);// added 100 millisecond extra for safety margin
        // the case where orderbook is getting destroyed sleeping prunethread would also needed to be killed
        {
            std::unique_lock prune_lock{ ordersMutex_};
            
            
        }
    }
}