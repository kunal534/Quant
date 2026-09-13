#include "orderbook.h"
#include <iostream>
#include <numeric>
#include <algorithm>

// Order implementations
Order::Order(OrderTypes order_type, Order_ID order_id, Side side, Price price, Quantity quantity)
    : order_type_(order_type), order_id_(order_id), side_(side),
      price_(price), initial_quantity_(quantity), remaining_quntity_(quantity) {}

Order_ID Order::Get_Order_ID() const { return order_id_; }
Side Order::Get_Side() const { return side_; }
Price Order::Get_Price() const { return price_; }
OrderTypes Order::Get_OrderType() const { return order_type_; }
Quantity Order::Get_Initial_Quantity() const { return initial_quantity_; }
Quantity Order::Get_remaining_Quantity() const { return remaining_quntity_; }
Quantity Order::Get_filled_Quantity() const { return Get_Initial_Quantity() - Get_remaining_Quantity(); }

void Order::Fill(Quantity quantity) {
    if (quantity > Get_remaining_Quantity()) {
        throw std::runtime_error("Order (" + std::to_string(Get_Order_ID()) +
                               ") cannot be filled more than its remaining quantity");
    }
    remaining_quntity_ -= quantity;
}

bool Order::is_Filled() const { return Get_remaining_Quantity() == 0; }

// Order_Modify implementations
Order_Modify::Order_Modify(Order_ID order_id, Side side, Price price, Quantity quantity)
    : orderid_(order_id), side_(side), price_(price), quantity_(quantity) {}

Order_ID Order_Modify::Get_Order_ID() const { return orderid_; }
Price Order_Modify::Get_Price() const { return price_; }
Side Order_Modify::Get_Side() const { return side_; }
Quantity Order_Modify::Get_Quantity() const { return quantity_; }

OrderPointer Order_Modify::To_Order_pointer(OrderTypes type) const {
    return std::make_shared<Order>(type, Get_Order_ID(), Get_Side(), Get_Price(), Get_Quantity());
}

// Trade implementations
Trade::Trade(const Trade_Info& bid_trade, const Trade_Info& ask_trade)
    : bidtrade_(bid_trade), asktrade_(ask_trade) {}

const Trade_Info& Trade::Get_Bid_Trade() const { return bidtrade_; }
const Trade_Info& Trade::Get_Ask_Trade() const { return asktrade_; }

// Orderbook_level_Infos implementations
Orderbook_level_Infos::Orderbook_level_Infos(const Level_Infos& bids, const Level_Infos& asks)
    : bids_(bids), asks_(asks) {}

const Level_Infos& Orderbook_level_Infos::Get_Bids() const { return bids_; }
const Level_Infos& Orderbook_level_Infos::Get_Asks() const { return asks_; }

// Order_book implementations
bool Order_book::Can_Match(Side side, Price price) const {
    if (side == Side::BUY) {
        if (asks_.empty()) return false;
        const auto& [bestAsk_, _] = *asks_.begin();
        return price >= bestAsk_;
    } else {
        if (bids_.empty()) return false;
        const auto& [bestBid_, _] = *bids_.begin();
        return price <= bestBid_;
    }
}

Trades Order_book::Match_Orders() {
    Trades trades;
    
    while (!bids_.empty() && !asks_.empty()) {
        auto bid_it = bids_.begin();
        auto ask_it = asks_.begin();
        
        if (bid_it->first < ask_it->first)
            break;
        
        auto& bid_orders = bid_it->second;
        auto& ask_orders = ask_it->second;
        
        while (!bid_orders.empty() && !ask_orders.empty()) {
            auto& bid = bid_orders.front();
            auto& ask = ask_orders.front();
            
            Quantity qty = std::min(bid->Get_remaining_Quantity(), ask->Get_remaining_Quantity());
            bid->Fill(qty);
            ask->Fill(qty);
            
            trades.push_back(Trade{
                Trade_Info{bid->Get_Order_ID(), bid->Get_Price(), qty},
                Trade_Info{ask->Get_Order_ID(), ask->Get_Price(), qty}
            });
            
            // Remove filled orders – store IDs before popping
            if (bid->is_Filled()) {
                Order_ID bid_id = bid->Get_Order_ID();
                bid_orders.pop_front();
                orders_.erase(bid_id);
            }
            if (ask->is_Filled()) {
                Order_ID ask_id = ask->Get_Order_ID();
                ask_orders.pop_front();
                orders_.erase(ask_id);
            }
        }
        
        if (bid_orders.empty())
            bids_.erase(bid_it);
        if (ask_orders.empty())
            asks_.erase(ask_it);
    }
    
    // Handle remaining Fill-and-Kill orders
    if (!bids_.empty()) {
        auto& orders = bids_.begin()->second;
        if (!orders.empty()) {
            auto& order = orders.front();
            if (order->Get_OrderType() == OrderTypes::Fill_and_Kill)
                Cancel_Order(order->Get_Order_ID());
        }
    }
    if (!asks_.empty()) {
        auto& orders = asks_.begin()->second;
        if (!orders.empty()) {
            auto order = orders.front();
            if (order->Get_OrderType() == OrderTypes::Fill_and_Kill)
                Cancel_Order(order->Get_Order_ID());
        }
    }
    
    return trades;
}

Trades Order_book::Add_orders(OrderPointer order) {
    if (!order) throw std::runtime_error("Null order");
    if (order->Get_Initial_Quantity() == 0) {
        return Trades{};  // reject zero‑quantity orders
    }

    auto it = orders_.find(order->Get_Order_ID());
    if (it != orders_.end()) {
        OrderPointer existing_order = it->second.order_;
        std::string error_msg = "Order ID " + std::to_string(order->Get_Order_ID()) +
                              " already exists [Side: " + (existing_order->Get_Side() == Side::BUY ? "BUY" : "SELL") +
                              " Price: " + std::to_string(existing_order->Get_Price()) +
                              ", Remaining: " + std::to_string(existing_order->Get_remaining_Quantity()) + "]";
        throw std::runtime_error(error_msg);
    }

    if (order->Get_OrderType() == OrderTypes::Fill_and_Kill && !Can_Match(order->Get_Side(), order->Get_Price())) {
        throw std::runtime_error("Can't fulfill this order");
    }

    OrderPointers::iterator iterator;
    if (order->Get_Side() == Side::BUY) {
        auto& orders = bids_[order->Get_Price()];
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size() - 1);
    } else {
        auto& orders = asks_[order->Get_Price()];
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size() - 1);
    }
    orders_.insert({order->Get_Order_ID(), order_entry(order, iterator)});
    return Match_Orders();
}

void Order_book::Cancel_Order(Order_ID orderid) {
    auto it = orders_.find(orderid);
    if (it == orders_.end()) {
        std::cout << "Order " << orderid << " not found\n";
        return;
    }

    // Copy pointer and iterator before erasing
    OrderPointer order = it->second.order_;
    OrderPointers::iterator orderiterator = it->second.location_;
    orders_.erase(it);

    if (order->Get_Side() == Side::BUY) {
        auto price = order->Get_Price();
        auto& orders = bids_.at(price);
        orders.erase(orderiterator);
        if (orders.empty()) bids_.erase(price);
    } else {
        auto price = order->Get_Price();
        auto& orders = asks_.at(price);
        orders.erase(orderiterator);
        if (orders.empty()) asks_.erase(price);
    }
}

Trades Order_book::ordermodify(Order_Modify order) {
    auto it = orders_.find(order.Get_Order_ID());
    if (it == orders_.end()) {
        throw std::runtime_error("Order not found");
    }
    // Store order type before cancellation
    OrderTypes existing_type = it->second.order_->Get_OrderType();
    Cancel_Order(order.Get_Order_ID());
    return Add_orders(order.To_Order_pointer(existing_type));
}

std::size_t Order_book::Size() const { return orders_.size(); }

Orderbook_level_Infos Order_book::Get_Order_Infos() const {
    Level_Infos ask_Infos, bid_Infos;
    ask_Infos.reserve(orders_.size());
    bid_Infos.reserve(orders_.size());

    auto Create_Level_Infos = [](Price price, const OrderPointers& orders) {
        Quantity total_quantity = std::accumulate(
            orders.begin(),
            orders.end(),
            (Quantity)0,
            [](Quantity running_sum, const OrderPointer& order) {
                return running_sum + order->Get_remaining_Quantity();
            }
        );
        return Level_Info{price, total_quantity};
    };

    for (const auto& [price, order_pointer] : bids_) {
        bid_Infos.push_back(Create_Level_Infos(price, order_pointer));
    }

    for (const auto& [price, order_pointer] : asks_) {
        ask_Infos.push_back(Create_Level_Infos(price, order_pointer));
    }
    return Orderbook_level_Infos{bid_Infos, ask_Infos};
}