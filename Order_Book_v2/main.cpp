#include "Orderbook.h"

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <vector>

namespace
{
struct LiveOrder
{
    OrderPointer order;
    bool cancelled{false};
};

OrderPointer MakeOrder(
    OrderType type, OrderId id, Side side, Price price, Quantity quantity)
{
    if(type==OrderType::Market)
    {
        return std::make_shared<Order>(id, side, quantity);
    }
    return std::make_shared<Order>(type, id, price, quantity, side);
}

const char* OrderTypeName(OrderType type)
{
    switch(type)
    {
        case OrderType::Good_Till_Cancel: return "GTC";
        case OrderType::Fill_and_Kill: return "FAK";
        case OrderType::Fill_or_Kill: return "FOK";
        case OrderType::Good_for_day: return "GFD";
        case OrderType::Market: return "MARKET";
    }
    return "UNKNOWN";
}
}

int main()
{
    constexpr unsigned seed = 0x51A7u;
    constexpr int operationCount = 200;
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> operation(0, 99);
    std::uniform_int_distribution<int> typeChoice(0, 4);
    std::uniform_int_distribution<int> sideChoice(0, 1);
    std::uniform_int_distribution<int> priceChoice(95, 115);
    std::uniform_int_distribution<int> quantityChoice(1, 5);
    const std::array<OrderType, 5> types{
        OrderType::Good_Till_Cancel,
        OrderType::Fill_and_Kill,
        OrderType::Fill_or_Kill,
        OrderType::Good_for_day,
        OrderType::Market
    };

    Orderbook orderbook;
    std::map<OrderId, LiveOrder> orders;
    std::vector<OrderId> cancellableOrders;
    OrderId nextId = 1;
    std::size_t tradeCount = 0;
    Quantity executedQuantity = 0;
    std::size_t cancelCount = 0;

    const auto submit = [&](OrderType type, Side side, Price price, Quantity quantity,
                            bool allowCancel) {
        const auto id = nextId++;
        auto order = MakeOrder(type, id, side, price, quantity);
        orders.emplace(id, LiveOrder{order});
        const auto trades = orderbook.AddOrder(order);

        std::cout << "ADD id=" << id << " type=" << OrderTypeName(type)
                  << " side=" << (side==Side::Buy ? "BUY" : "SELL")
                  << " price=" << price << " qty=" << quantity << '\n';
        for(const auto& trade : trades)
        {
            const auto& bid = trade.GetBidTrade();
            const auto& ask = trade.GetAskTrade();
            std::cout << "  TRADE buy=" << bid.orderid_ << " sell=" << ask.orderid_
                      << " qty=" << bid.quantity_ << '\n';
            ++tradeCount;
            executedQuantity += bid.quantity_;
        }

        if(allowCancel && (type==OrderType::Good_Till_Cancel
                || type==OrderType::Good_for_day))
        {
            cancellableOrders.push_back(id);
        }
    };

    std::cout << "Seed: " << seed << "; random order-book activity\n";

    // Keep both sides populated so the existing matcher always has both sides to inspect.
    submit(OrderType::Good_Till_Cancel, Side::Buy, 100, 1000, false);
    submit(OrderType::Good_Till_Cancel, Side::Sell, 110, 1000, false);

    for(int index = 0; index < operationCount; ++index)
    {
        if(!cancellableOrders.empty() && operation(random) < 25)
        {
            std::uniform_int_distribution<std::size_t> chooseOrder(
                0, cancellableOrders.size() - 1);
            const auto position = chooseOrder(random);
            const auto id = cancellableOrders[position];
            auto& record = orders.at(id);
            if(!record.cancelled && record.order->GetRemainingQuantity()>0)
            {
                orderbook.CancelOrder(id);
                record.cancelled = true;
                ++cancelCount;
                std::cout << "CANCEL id=" << id
                          << " remaining=" << record.order->GetRemainingQuantity() << '\n';
            }
            cancellableOrders.erase(cancellableOrders.begin() + position);
            continue;
        }

        const auto type = types[typeChoice(random)];
        const auto side = sideChoice(random)==0 ? Side::Buy : Side::Sell;
        auto quantity = static_cast<Quantity>(quantityChoice(random));
        if(type==OrderType::Fill_and_Kill || type==OrderType::Market)
        {
            quantity = 1;
        }
        submit(type, side, static_cast<Price>(priceChoice(random)), quantity, true);
    }

    std::size_t openOrders = 0;
    for(const auto& [id, record] : orders)
    {
        (void)id;
        if(!record.cancelled && record.order->GetRemainingQuantity()>0)
        {
            ++openOrders;
        }
    }

    std::cout << "Summary: trades=" << tradeCount
              << " executed_quantity=" << executedQuantity
              << " cancelled_orders=" << cancelCount
              << " orders_with_remaining_quantity=" << openOrders << '\n';
}
