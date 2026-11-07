#include "Orderbook.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace
{
constexpr std::int64_t kBatchSize = 256;
constexpr std::int64_t kBenchmarkIterations = 1000;

using Clock = std::chrono::steady_clock;
using Latencies = std::vector<std::int64_t>;

template<typename Operation>
void MeasureLatency(Latencies& latencies, Operation&& operation)
{
    const auto start = Clock::now();
    operation();
    const auto elapsed = Clock::now() - start;
    latencies.push_back(
        std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count());
}

void ReportLatencyPercentiles(benchmark::State& state, Latencies& latencies)
{
    std::sort(latencies.begin(), latencies.end());
    const auto percentile = [&latencies](std::int64_t percent) {
        const auto rank = (percent * static_cast<std::int64_t>(latencies.size()) + 99) / 100;
        return static_cast<double>(latencies[static_cast<std::size_t>(rank - 1)]);
    };

    state.counters["p50_ns"] = percentile(50);
    state.counters["p99_ns"] = percentile(99);
}

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
    Latencies latencies;
    latencies.reserve(kBenchmarkIterations * kBatchSize);

    for(auto _ : state)
    {
        (void)_;
        Orderbook book;
        AddBaselineOrders(book);

        const auto start = Clock::now();
        for(std::int64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            auto order = MakeLimitOrder(id, Side::Buy, 99, 1);
            MeasureLatency(latencies, [&] {
                auto trades = book.AddOrder(std::move(order));
                benchmark::DoNotOptimize(trades);
            });
        }
        const auto elapsed = Clock::now() - start;
        state.SetIterationTime(std::chrono::duration<double>(elapsed).count());
    }

    state.SetItemsProcessed(state.iterations() * kBatchSize);
    ReportLatencyPercentiles(state, latencies);
}

void BM_AddOrder_WithMatch(benchmark::State& state)
{
    Latencies latencies;
    latencies.reserve(kBenchmarkIterations * kBatchSize);

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
            auto order = MakeLimitOrder(id, Side::Buy, 110, 1);
            MeasureLatency(latencies, [&] {
                auto trades = book.AddOrder(std::move(order));
                benchmark::DoNotOptimize(trades);
            });
        }
        const auto elapsed = Clock::now() - start;
        state.SetIterationTime(std::chrono::duration<double>(elapsed).count());
    }

    state.SetItemsProcessed(state.iterations() * kBatchSize);
    ReportLatencyPercentiles(state, latencies);
}

void BM_CancelOrder(benchmark::State& state)
{
    Latencies latencies;
    latencies.reserve(kBenchmarkIterations * kBatchSize);

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
            MeasureLatency(latencies, [&] {
                book.CancelOrder(id);
            });
        }
        const auto elapsed = Clock::now() - start;
        state.SetIterationTime(std::chrono::duration<double>(elapsed).count());
    }

    state.SetItemsProcessed(state.iterations() * kBatchSize);
    ReportLatencyPercentiles(state, latencies);
}

void BM_ModifyOrder(benchmark::State& state)
{
    Latencies latencies;
    latencies.reserve(kBenchmarkIterations * kBatchSize);

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
            const OrderModify modifyOrder(id, 98, 1, Side::Buy);
            MeasureLatency(latencies, [&] {
                auto trades = book.ModifyOrder(modifyOrder);
                benchmark::DoNotOptimize(trades);
            });
        }
        const auto elapsed = Clock::now() - start;
        state.SetIterationTime(std::chrono::duration<double>(elapsed).count());
    }

    state.SetItemsProcessed(state.iterations() * kBatchSize);
    ReportLatencyPercentiles(state, latencies);
}
}

BENCHMARK(BM_AddOrder_NoMatch)->Iterations(kBenchmarkIterations)->UseManualTime();
BENCHMARK(BM_AddOrder_WithMatch)->Iterations(kBenchmarkIterations)->UseManualTime();
BENCHMARK(BM_CancelOrder)->Iterations(kBenchmarkIterations)->UseManualTime();
BENCHMARK(BM_ModifyOrder)->Iterations(kBenchmarkIterations)->UseManualTime();

BENCHMARK_MAIN();
