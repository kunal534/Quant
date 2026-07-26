#include<iostream>
#include<vector>
#include<unordered_map>
#include<list>
#include<queue>
#include<map>
#include<numeric>
#include<algorithm>
#include<memory>
#include<stdexcept>// for std::runtime_error

//---- Enums----
enum class OrderTypes {
    Good_Till_Cancel,
    Fill_and_Kill
};

enum class Side{
    BUY,
    SELL
};

// --- Type Aliases----
using Price=std::uint32_t;
using Quantity=std::uint32_t;
using Order_ID=std::uint64_t;


// ----Level_Info-----
struct Level_Info{
    Price price_;
    Quantity quantity_;
};

using Level_Infos=std::vector<Level_Info>;

// --- Orderebook Level Snapshot ---
class Orderbook_level_Infos
{
    public:
    // initialization for a direction copy into the bids_, ask_
    Orderbook_level_Infos(const Level_Infos &bids,const Level_Infos &asks):bids_{bids},asks_{asks}
    {
    }

    const Level_Infos& Get_Bids() const {return bids_; };
    const Level_Infos& Get_Asks() const { return asks_;};

    private:
    Level_Infos bids_;
    Level_Infos asks_;

};

// ---- Order ----
class Order{
    public:
    Order(OrderTypes order_type,Order_ID order_id, Side side,Price price, Quantity quantity)
    : order_type_ {order_type},
    order_id_ {order_id},
    side_ {side},
    price_ {price},
    initial_quantity_ {quantity},
    remaining_quntity_ {quantity}
    {}
    // API Calling
    Order_ID Get_Order_ID() const{ return order_id_;}
    Side Get_Side() const{return side_;}
    Price Get_Price() const { return price_;}
    OrderTypes Get_OrderType() const { return order_type_;}
    Quantity Get_Initial_Quantity() const {return initial_quantity_;}
    Quantity Get_remaining_Quantity() const {return remaining_quntity_;}
    Quantity Get_filled_Quantity() const{ return Get_Initial_Quantity() - Get_remaining_Quantity();} 
    void Fill(Quantity quantity)
    {
        if(quantity > Get_remaining_Quantity())
        {
            throw std::runtime_error( "Order (" + std::to_string(Get_Order_ID()) + ") cannot be filled more than its remaining quantity");
        }
        remaining_quntity_ -=quantity;
    }
    bool is_Filled() const
    {
        return Get_remaining_Quantity()==0;
    }
    private:
    OrderTypes order_type_ ;
    Order_ID order_id_ ;
    Side side_ ;
    Price price_ ;
    Quantity initial_quantity_, remaining_quntity_ ;
};

using OrderPointer =std::shared_ptr<Order>;
using OrderPointers= std::list<OrderPointer>;

// ---- Order Modify ----
class Order_Modify{
    public:
    Order_Modify(Order_ID order_id,Side side,Price price,Quantity quantity):
    orderid_ {order_id},
    side_ {side},
    price_ {price},
    quantity_ {quantity}
    {}
    // API calling
    Order_ID Get_Order_ID() const{return orderid_;}  
    Price Get_Price() const{return price_;}
    Side Get_Side() const {return side_;}
    Quantity Get_Quantity() const {return quantity_;}



    // to modify the order with the current order which is stored in the orderbook

    OrderPointer To_Order_pointer(OrderTypes type ) const{
        return std::make_shared<Order>(type,Get_Order_ID(),Get_Side(),Get_Price(),Get_Quantity());
    }

    private:
    Order_ID orderid_;
    Side side_;
    Price price_;
    Quantity quantity_;
};

// ---- Trade Info ----
struct Trade_Info{
    Order_ID orderid_;
    Price price_;
    Quantity quantity_;
};

// for a single trade
class Trade{
    public:
    Trade(const Trade_Info&bid_trade, const Trade_Info&ask_trade):
    bidtrade_ {bid_trade},
    asktrade_ {ask_trade}
    {
    }
    // API
    const Trade_Info& Get_Bid_Trade() const{return bidtrade_;}
    const Trade_Info& Get_Ask_Trade() const{return asktrade_;}

    private:
    Trade_Info bidtrade_;
    Trade_Info asktrade_;
};

// There can be a whole sweep of order

using Trades= std::vector<Trade>;

// ---- Order Book ----
class Order_book{
    private:
    struct order_entry{
        OrderPointer order_{nullptr}; // actual order
        OrderPointers::iterator location_; // location of the order via reference of it

        order_entry(OrderPointer o=nullptr,
            OrderPointers::iterator location=OrderPointers::iterator()):
            order_(o),location_(location){}
    };


    // for bids
    std::map<Price,OrderPointers,std::greater<Price>>bids_;
    // for asks
    std::map<Price,OrderPointers,std::less<Price>>asks_;
    // for per orders
    std::unordered_map<Order_ID,order_entry>orders_;

    bool Can_Match(Side side,Price price)const{
        if(side == Side::BUY)
        {
            if(asks_.empty())
            {
                return false;
            }
            // in case of Buy the best ask would be the one which would be shown at the top which would be the lowest due to desperate issue
            const auto&[bestAsk_,_]=*asks_.begin();// the _ means ignore the other value just get the first value which would be the Price
            return price>=bestAsk_;
        }
        else
        {
            if(bids_.empty())
            {
                return false;
            }
            // in case of sell the market would show the highest bids for the price 
            const auto&[bestBid_,_]=*bids_.begin();
            return price<=bestBid_;
        }
    }

    // 
    Trades Match_Orders(){
        Trades trades;
        trades.reserve(orders_.size());
        while(true)
        {
            if(bids_.empty()|| asks_.empty())
            {
                break;
            }

            auto& [bidPrice,bids] = *bids_.begin();
            auto& [askPrice,asks] = *asks_.begin();
            
            // is ask is more than bid then no trade is possible
            if(bidPrice< askPrice)break;

            while(bids.size() && asks.size())
            {
                auto& bid=bids.front();
                auto& ask=asks.front();

                Quantity quanity = std::min(bid->Get_remaining_Quantity(), ask->Get_remaining_Quantity());
                bid->Fill(quanity);
                ask->Fill(quanity);

                if(bid->is_Filled())
                {
                    bids.pop_front();
                    orders_.erase(bid->Get_Order_ID());
                }
                if(ask->is_Filled())
                {
                    asks.pop_front();
                    orders_.erase(ask->Get_Order_ID());
                }
                // what if all the bids at a certain bidprice becames filled so remove tha bidprice
                if(bids.empty())
                {
                    bids_.erase(bidPrice);
                }
                // what if all the asks at a certain askprice becames filled so remove tha askprice
                if(asks.empty())
                {
                    asks_.erase(askPrice);
                }

                // Transaction are stored in trades
                trades.push_back(Trade{
                    Trade_Info{bid->Get_Order_ID(),bid->Get_Price(),quanity},
                    Trade_Info{ask->Get_Order_ID(),ask->Get_Price(),quanity}
            });
            }
        }
        // for case of fill and kill if 
        if(!bids_.empty())
        {
            auto&[_,bids]=*bids_.begin();
            auto& order=bids.front();
            if(order->Get_OrderType()==OrderTypes::Fill_and_Kill)
            {
                Cancel_Order(order->Get_Order_ID());
            }
        }
        if(!asks_.empty())
        {
            auto&[_,asks]=*asks_.begin();
            auto order=asks.front();
            if(order->Get_OrderType()==OrderTypes::Fill_and_Kill)
            {
                Cancel_Order(order->Get_Order_ID());
            }
        }
        return trades;
    }
    public:
    Trades Add_orders(OrderPointer order)
    {
        if(!order) throw std::runtime_error("Null order");

        auto it=orders_.find(order->Get_Order_ID());

        if(it!=orders_.end())
        {
            OrderPointer existing_order=it->second.order_;
            std::string error_msg="Order ID "+ std::to_string(order->Get_Order_ID()) + " already exists [Side: " + (existing_order->Get_Side() == Side::BUY?"BUY":"SELL") +
        "Price: " + std::to_string(existing_order->Get_Price()) + ", Remaining: " +std::to_string(existing_order->Get_remaining_Quantity()) +"]";

        throw std::runtime_error(error_msg);
        } 

        if(order->Get_OrderType()==OrderTypes::Fill_and_Kill && !Can_Match(order->Get_Side(),order->Get_Price()))
        {
            throw std::runtime_error("Can't fullfil this order");
        }
        
        OrderPointers::iterator iterator;

        if(order->Get_Side()==Side::BUY)
        {
            // get reference to the price level
            auto& orders=bids_[order->Get_Price()];

            orders.push_back(order);
            iterator=std::next(orders.begin(),orders.size()-1);
            /*
            orders.begin() → iterator pointing to the first element
            orders.size() - 1 → index of the last element (newly added)
            std::next() → advances the iterator by that many positions
            Returns an iterator pointing to the newly added order
            */
        }
        else
        {
            auto& orders=asks_[order->Get_Price()];

            orders.push_back(order);
            iterator=std::next(orders.begin(),orders.size()-1);
        }
        orders_.insert({order->Get_Order_ID(),order_entry(order,iterator)});
        return Match_Orders();
    }
    void Cancel_Order(Order_ID orderid)
    {
        auto it=orders_.find(orderid);
        if(it==orders_.end())
        {
            std::cout<<"Order "<<orderid<<" not found\n";
            return;
        }

        // removed from orders_ now need to remove from asks_,bids_ where the top level are being stored
        const auto& [order,orderiterator]=orders_.at(orderid);
        
        orders_.erase(orderid);

        if(order->Get_Side()==Side::BUY)
        {
            // the flow:
            /*
            1. Get the side to which orderid belongs
            2. erase the iterator of the given specific orderby
            3. If there's no more entry left at that price then remove that price
            */
            auto price=order->Get_Price();
            auto& orders=bids_.at(price);// orders shows all the order at the certain price
            orders.erase(orderiterator);
            if(orders.empty())
            {
                bids_.erase(price);
            }
        }
        else
        {
            auto price=order->Get_Price();
            auto& orders=asks_.at(price);
            orders.erase(orderiterator);
            if(orders.empty())
            {
                asks_.erase(price);
            }
        }           
    }
    Trades ordermodify(Order_Modify order)
    {
        if(orders_.find(order.Get_Order_ID())==orders_.end())
        {
            throw std::runtime_error("Order not found");
        }
        const auto&[existingorder,_]=orders_.at(order.Get_Order_ID());
        Cancel_Order(order.Get_Order_ID());
        return Add_orders(order.To_Order_pointer(existingorder->Get_OrderType()));
    }
    std::size_t Size() const{ return orders_.size();}


    Orderbook_level_Infos Get_Order_Infos()const{
        Level_Infos ask_Infos,bid_Infos;

        ask_Infos.reserve(orders_.size());
        bid_Infos.reserve(orders_.size());
        
        // lambda function 
        auto Create_Level_Infos=[](Price price, const OrderPointers& orders){
            Quantity total_quantity=std::accumulate(
                orders.begin(),
                orders.end(),
                (Quantity)0,
                [](Quantity running_sum,const OrderPointer& order){
                    return running_sum + order->Get_remaining_Quantity();
                }
            );
            return Level_Info{price,total_quantity};
        };
        
        for(const auto&[price,order_pointer]:bids_)
        {
            bid_Infos.push_back(Create_Level_Infos(price,order_pointer));
        }

        for(const auto&[price,order_pointer]:asks_)
        {
            ask_Infos.push_back(Create_Level_Infos(price,order_pointer));
        }
        return Orderbook_level_Infos{bid_Infos,ask_Infos};
    }
};

int main()
{
    Order_book orderbook;
    const Order_ID order_id=1;
    orderbook.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel,order_id,Side::BUY,10000,10));
    std::cout<<orderbook.Size()<<std::endl;
    return 0;
}