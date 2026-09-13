#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>
#include <queue>
#include <map>
#include <numeric>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

enum class OrderTypes {
    Good_Till_Cancel,
    Fill_and_Kill
};

enum class Side {
    BUY,
    SELL
};

using Price = std::uint32_t;
using Quantity = std::uint32_t;
using Order_ID = std::uint64_t;

struct Level_Info {
    Price price_;
    Quantity quantity_;
};

using Level_Infos = std::vector<Level_Info>;

class Order {
public:
    Order(OrderTypes order_type, Order_ID order_id, Side side, Price price, Quantity quantity);
    
    Order_ID Get_Order_ID() const;
    Side Get_Side() const;
    Price Get_Price() const;
    OrderTypes Get_OrderType() const;
    Quantity Get_Initial_Quantity() const;
    Quantity Get_remaining_Quantity() const;
    Quantity Get_filled_Quantity() const;
    void Fill(Quantity quantity);
    bool is_Filled() const;

private:
    OrderTypes order_type_;
    Order_ID order_id_;
    Side side_;
    Price price_;
    Quantity initial_quantity_, remaining_quntity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;

class Order_Modify {
public:
    Order_Modify(Order_ID order_id, Side side, Price price, Quantity quantity);
    
    Order_ID Get_Order_ID() const;
    Price Get_Price() const;
    Side Get_Side() const;
    Quantity Get_Quantity() const;
    OrderPointer To_Order_pointer(OrderTypes type) const;

private:
    Order_ID orderid_;
    Side side_;
    Price price_;
    Quantity quantity_;
};

struct Trade_Info {
    Order_ID orderid_;
    Price price_;
    Quantity quantity_;
};

class Trade {
public:
    Trade(const Trade_Info& bid_trade, const Trade_Info& ask_trade);
    
    const Trade_Info& Get_Bid_Trade() const;
    const Trade_Info& Get_Ask_Trade() const;

private:
    Trade_Info bidtrade_;
    Trade_Info asktrade_;
};

using Trades = std::vector<Trade>;

class Orderbook_level_Infos {
public:
    Orderbook_level_Infos(const Level_Infos& bids, const Level_Infos& asks);
    
    const Level_Infos& Get_Bids() const;
    const Level_Infos& Get_Asks() const;

private:
    Level_Infos bids_;
    Level_Infos asks_;
};

class Order_book {
public:
    Trades Add_orders(OrderPointer order);
    void Cancel_Order(Order_ID orderid);
    Trades ordermodify(Order_Modify order);
    std::size_t Size() const;
    Orderbook_level_Infos Get_Order_Infos() const;

private:
    struct order_entry {
        OrderPointer order_{nullptr};
        OrderPointers::iterator location_;
        order_entry(OrderPointer o = nullptr,
                   OrderPointers::iterator location = OrderPointers::iterator())
            : order_(o), location_(location) {}
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<Order_ID, order_entry> orders_;

    bool Can_Match(Side side, Price price) const;
    Trades Match_Orders();
};

#endif // ORDERBOOK_H