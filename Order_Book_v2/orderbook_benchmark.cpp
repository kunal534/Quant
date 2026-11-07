#include "Orderbook.h"

#include <benchmark/benchmark.h>

#include <chrono>
#include <cstdint>
#include <memory>

namespace
{
constexpr std::int64_t kBatchSize = 256;

using Clock = std::chrono::steady_clock;

OrderPointer MakeLimitOrder(OrderId id, Side side, Price price, Quantity quantity)
{
    return std::make_shared<Order>(
        OrderType::Good_Till_Cancel, id, price, quantity, side);
}

void AddBaselineOrders(Orderbook& book)
{
    book.AddOrder(MakeLimitOrder(1, Side::Buy, 100, 1000));
    book.AddOrder(MakeLimitOrder(2, Side::Sell, 110, 1000));
}

void BM_AddOrder_NoMatch(benchmark::State& state)
{
    for(auto _ : state)
    {
        (void)_;
        Orderbook book;
        AddBaselineOrders(book);

        const auto start = Clock::now();
        for(std::int64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            auto trades = book.AddOrder(
                MakeLimitOrder(id, Side::Buy, 99, 1));
            benchmark::DoNotOptimize(trades);
        }
        const auto elapsed = Clock::now() - start;
        state.SetIterationTime(std::chrono::duration<double>(elapsed).count());
    }

    state.SetItemsProcessed(state.iterations() * kBatchSize);
}

void BM_AddOrder_WithMatch(benchmark::State& state)
{
    for(auto _ : state)
    {
        (void)_;
        Orderbook book;
        book.AddOrder(MakeLimitOrder(1, Side::Buy, 100, 1000));
        book.AddOrder(MakeLimitOrder(2, Side::Sell, 110,
            static_cast<Quantity>(kBatchSize + 1)));

        const auto start = Clock::now();
        for(std::int64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            auto trades = book.AddOrder(
                MakeLimitOrder(id, Side::Buy, 110, 1));
            benchmark::DoNotOptimize(trades);
        }
        const auto elapsed = Clock::now() - start;
        state.SetIterationTime(std::chrono::duration<double>(elapsed).count());
    }

    state.SetItemsProcessed(state.iterations() * kBatchSize);
}

void BM_CancelOrder(benchmark::State& state)
{
    for(auto _ : state)
    {
        (void)_;
        Orderbook book;
        AddBaselineOrders(book);
        for(std::int64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            book.AddOrder(MakeLimitOrder(id, Side::Buy, 99, 1));
        }

        const auto start = Clock::now();
        for(std::int64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            book.CancelOrder(id);
        }
        const auto elapsed = Clock::now() - start;
        state.SetIterationTime(std::chrono::duration<double>(elapsed).count());
    }

    state.SetItemsProcessed(state.iterations() * kBatchSize);
}

void BM_ModifyOrder(benchmark::State& state)
{
    for(auto _ : state)
    {
        (void)_;
        Orderbook book;
        AddBaselineOrders(book);
        for(std::int64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            book.AddOrder(MakeLimitOrder(id, Side::Buy, 99, 1));
        }

        const auto start = Clock::now();
        for(std::int64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            auto trades = book.ModifyOrder(OrderModify(id, 98, 1, Side::Buy));
            benchmark::DoNotOptimize(trades);
        }
        const auto elapsed = Clock::now() - start;
        state.SetIterationTime(std::chrono::duration<double>(elapsed).count());
    }

    state.SetItemsProcessed(state.iterations() * kBatchSize);
}
}

BENCHMARK(BM_AddOrder_NoMatch)->Iterations(10)->UseManualTime();
BENCHMARK(BM_AddOrder_WithMatch)->Iterations(10)->UseManualTime();
BENCHMARK(BM_CancelOrder)->Iterations(10)->UseManualTime();
BENCHMARK(BM_ModifyOrder)->Iterations(10)->UseManualTime();

BENCHMARK_MAIN();
