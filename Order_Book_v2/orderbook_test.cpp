#include "Orderbook.h"

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
struct SubmittedOrder
{
    OrderPointer order;
    Side side;
    OrderType type;
    Price limitPrice;
    Quantity initialQuantity;
    Quantity tradedQuantity{0};
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

void Check(bool condition, const char* message)
{
    if(!condition)
    {
        throw std::runtime_error(message);
    }
}

void RunOneSidedOrderFlow()
{
    Orderbook book;
    auto buy = MakeOrder(OrderType::Good_Till_Cancel, 1, Side::Buy, 100, 10);
    auto sell = MakeOrder(OrderType::Good_Till_Cancel, 2, Side::Sell, 110, 10);

    Check(book.AddOrder(buy).empty(), "an unmatched buy must not produce trades");
    Check(buy->GetRemainingQuantity()==10, "an unmatched buy must remain open");
    Check(book.AddOrder(sell).empty(), "non-crossing orders must not produce trades");
    Check(sell->GetRemainingQuantity()==10, "an unmatched sell must remain open");
}

void RunCancelOrderFlow()
{
    Orderbook book;
    Check(book.AddOrder(MakeOrder(
        OrderType::Good_Till_Cancel, 1000, Side::Buy, 100, 1000)).empty(),
        "baseline buy must not produce trades");
    Check(book.AddOrder(MakeOrder(
        OrderType::Good_Till_Cancel, 1001, Side::Sell, 110, 1000)).empty(),
        "non-crossing baseline sell must not produce trades");

    for(OrderId id = 1; id <= 256; ++id)
    {
        Check(book.AddOrder(MakeOrder(
            OrderType::Good_Till_Cancel, id, Side::Buy, 99, 1)).empty(),
            "an unmatched buy must not produce trades");
    }

    for(OrderId id = 1; id <= 256; ++id)
    {
        book.CancelOrder(id);
    }
}

void RunRandomOrderFlow()
{
    constexpr unsigned seed = 0x51A7u;
    constexpr int operationCount = 500;
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> operation(0, 99);
    std::uniform_int_distribution<int> typeChoice(0, 4);
    std::uniform_int_distribution<int> sideChoice(0, 1);
    std::uniform_int_distribution<int> priceChoice(95, 115);
    std::uniform_int_distribution<int> quantityChoice(1, 10);

    const std::array<OrderType, 5> types{
        OrderType::Good_Till_Cancel,
        OrderType::Fill_and_Kill,
        OrderType::Fill_or_Kill,
        OrderType::Good_for_day,
        OrderType::Market
    };

    Orderbook book;
    std::map<OrderId, SubmittedOrder> submitted;
    std::vector<OrderId> cancellableOrders;
    OrderId nextId = 1;
    std::size_t addCount = 0;
    std::size_t cancelCount = 0;
    std::size_t tradeCount = 0;
    Quantity executedQuantity = 0;

    const auto addOrder = [&](OrderType type, Side side, Price price, Quantity quantity,
                              bool canCancel) {
        const OrderId id = nextId++;
        auto order = MakeOrder(type, id, side, price, quantity);
        submitted.emplace(id, SubmittedOrder{
            order, side, type, price, quantity, 0
        });

        const auto trades = book.AddOrder(order);
        ++addCount;
        if(canCancel)
        {
            cancellableOrders.push_back(id);
        }

        for(const auto& trade : trades)
        {
            const auto& bid = trade.GetBidTrade();
            const auto& ask = trade.GetAskTrade();
            Check(submitted.contains(bid.orderid_) && submitted.contains(ask.orderid_),
                "trade refers to an unknown order id");
            auto& buy = submitted.at(bid.orderid_);
            auto& sell = submitted.at(ask.orderid_);
            Check(buy.side==Side::Buy && sell.side==Side::Sell,
                "trade has incorrect buy/sell order sides");
            Check(bid.quantity_>0 && bid.quantity_==ask.quantity_,
                "trade quantities must be positive and equal");
            Check(bid.price_>=ask.price_, "trade prices do not cross");
            Check(buy.type==OrderType::Market || bid.price_<=buy.limitPrice,
                "buy order traded above its limit");
            Check(sell.type==OrderType::Market || ask.price_>=sell.limitPrice,
                "sell order traded below its limit");

            buy.tradedQuantity += bid.quantity_;
            sell.tradedQuantity += ask.quantity_;
            executedQuantity += bid.quantity_;
            ++tradeCount;
        }
    };

    addOrder(OrderType::Good_Till_Cancel, Side::Buy, 100, 10000, false);
    addOrder(OrderType::Good_Till_Cancel, Side::Sell, 110, 10000, false);

    for(int index = 0; index < operationCount; ++index)
    {
        const bool shouldCancel = !cancellableOrders.empty() && operation(random) < 25;
        if(shouldCancel)
        {
            std::uniform_int_distribution<std::size_t> chooseOrder(
                0, cancellableOrders.size() - 1);
            const auto position = chooseOrder(random);
            const auto id = cancellableOrders[position];
            book.CancelOrder(id);
            cancellableOrders.erase(cancellableOrders.begin() + position);
            ++cancelCount;
            continue;
        }

        const auto type = types[typeChoice(random)];
        const auto side = sideChoice(random)==0 ? Side::Buy : Side::Sell;
        auto quantity = static_cast<Quantity>(quantityChoice(random));
        if(type==OrderType::Fill_and_Kill || type==OrderType::Market)
        {
            quantity = 1;
        }
        addOrder(type, side, static_cast<Price>(priceChoice(random)), quantity, true);
    }

    for(const auto& [id, record] : submitted)
    {
        (void)id;
        Check(record.tradedQuantity<=record.initialQuantity,
            "an order was filled beyond its submitted quantity");
        Check(record.tradedQuantity + record.order->GetRemainingQuantity()
                == record.initialQuantity,
            "traded and remaining quantities do not add up to the submitted quantity");
    }

    std::cout << "Seed: " << seed << '\n'
              << "Random operations: " << operationCount << '\n'
              << "Orders added: " << addCount << '\n'
              << "Orders cancelled: " << cancelCount << '\n'
              << "Trades: " << tradeCount << '\n'
              << "Quantity executed: " << executedQuantity << '\n'
              << "Random order flow passed\n";
}
}

int main()
{
    try
    {
        RunOneSidedOrderFlow();
        RunCancelOrderFlow();
        RunRandomOrderFlow();
    }
    catch(const std::exception& error)
    {
        std::cerr << "Random order flow failed: " << error.what() << '\n';
        return 1;
    }
}
