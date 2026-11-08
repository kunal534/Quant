#include "Orderbook.h"

#include <benchmarksuite>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint64_t kBatchSize = 1000;

using BenchmarkConfig = benchmarksuite::stage_config_data;
using BenchmarkStage = benchmarksuite::benchmark_stage<
    "orderbook-v2",
    BenchmarkConfig{
        .clear_cpu_caches_before_iterations = false,
        .measured_iteration_count = 100,
        .max_iteration_count = 10000,
        .convergence_threshold = 1.0,
        .benchmark_type = benchmarksuite::benchmark_types::cpu,
        .max_time_in_s = 5,
        .rse_threshold = 2.5,
        .max_k = 10000,
        .min_k = 30,
    }>;

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

struct AddOrderNoMatch
{
    std::unique_ptr<Orderbook> book;
    std::vector<OrderPointer> orders;

    static void before(AddOrderNoMatch& data)
    {
        data.book = std::make_unique<Orderbook>();
        AddBaselineOrders(*data.book);
        data.orders.clear();
        data.orders.reserve(kBatchSize);
        for(std::uint64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            data.orders.push_back(MakeLimitOrder(id, Side::Buy, 99, 1));
        }
    }

    static std::uint64_t impl(AddOrderNoMatch& data)
    {
        for(auto& order : data.orders)
        {
            const auto trades = data.book->AddOrder(std::move(order));
            benchmarksuite::do_not_optimize_away(trades);
        }
        return kBatchSize;
    }
};

struct AddOrderWithMatch
{
    std::unique_ptr<Orderbook> book;
    std::vector<OrderPointer> orders;

    static void before(AddOrderWithMatch& data)
    {
        data.book = std::make_unique<Orderbook>();
        data.book->AddOrder(MakeLimitOrder(1, Side::Buy, 100, 1000));
        data.book->AddOrder(MakeLimitOrder(
            2, Side::Sell, 110, static_cast<Quantity>(kBatchSize + 1)));
        data.orders.clear();
        data.orders.reserve(kBatchSize);
        for(std::uint64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            data.orders.push_back(MakeLimitOrder(id, Side::Buy, 110, 1));
        }
    }

    static std::uint64_t impl(AddOrderWithMatch& data)
    {
        for(auto& order : data.orders)
        {
            const auto trades = data.book->AddOrder(std::move(order));
            benchmarksuite::do_not_optimize_away(trades);
        }
        return kBatchSize;
    }
};

struct CancelOrder
{
    std::unique_ptr<Orderbook> book;

    static void before(CancelOrder& data)
    {
        data.book = std::make_unique<Orderbook>();
        AddBaselineOrders(*data.book);
        for(std::uint64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            data.book->AddOrder(MakeLimitOrder(id, Side::Buy, 99, 1));
        }
    }

    static std::uint64_t impl(CancelOrder& data)
    {
        for(std::uint64_t index = 0; index < kBatchSize; ++index)
        {
            data.book->CancelOrder(static_cast<OrderId>(index + 3));
        }
        return kBatchSize;
    }
};

struct ModifyOrder
{
    std::unique_ptr<Orderbook> book;
    std::vector<OrderModify> modifications;

    static void before(ModifyOrder& data)
    {
        data.book = std::make_unique<Orderbook>();
        AddBaselineOrders(*data.book);
        data.modifications.clear();
        data.modifications.reserve(kBatchSize);
        for(std::uint64_t index = 0; index < kBatchSize; ++index)
        {
            const auto id = static_cast<OrderId>(index + 3);
            data.book->AddOrder(MakeLimitOrder(id, Side::Buy, 99, 1));
            data.modifications.emplace_back(id, 98, 1, Side::Buy);
        }
    }

    static std::uint64_t impl(ModifyOrder& data)
    {
        for(const auto& modification : data.modifications)
        {
            const auto trades = data.book->ModifyOrder(modification);
            benchmarksuite::do_not_optimize_away(trades);
        }
        return kBatchSize;
    }
};

void ReplaceAll(std::string& text, const std::string& from, const std::string& to)
{
    std::size_t position = 0;
    while((position = text.find(from, position)) != std::string::npos)
    {
        text.replace(position, from.size(), to);
        position += to.size();
    }
}

void RemoveLastCsvColumn(std::string& text)
{
    std::size_t lineStart = 0;
    while(lineStart < text.size())
    {
        const auto lineEnd = text.find('\n', lineStart);
        const auto end = lineEnd == std::string::npos ? text.size() : lineEnd;
        const auto comma = text.rfind(',', end);
        auto nextLineStart = lineEnd == std::string::npos ? text.size() : lineEnd + 1;
        if(comma != std::string::npos && comma >= lineStart)
        {
            text.erase(comma, end - comma);
            if(lineEnd != std::string::npos)
            {
                nextLineStart = comma + 1;
            }
        }
        if(lineEnd == std::string::npos)
        {
            break;
        }
        lineStart = nextLineStart;
    }
}

void PrintTestResults(std::string_view testName, bool& hasCycleCounters)
{
    const auto results = BenchmarkStage::get_test_results(testName);
    auto report = results.to_csv(false);
    hasCycleCounters |= report.find("Cycles/Byte") != std::string::npos;
    ReplaceAll(report, "Throughput (MB/s)", "Throughput (Miops/s)");
    ReplaceAll(report, "File Size (Bytes)", "Operations per batch");
    ReplaceAll(report, "Latency / Run (ns)", "Batch time (ns)");
    ReplaceAll(report, "/Byte", "/Operation");
    RemoveLastCsvColumn(report);
    std::cout << report << '\n';
}

void PrintResults()
{
    bool hasCycleCounters = false;
    PrintTestResults("add-no-match", hasCycleCounters);
    PrintTestResults("add-with-match", hasCycleCounters);
    PrintTestResults("cancel", hasCycleCounters);
    PrintTestResults("modify", hasCycleCounters);
    if(!hasCycleCounters)
    {
        std::cerr << "Hardware cycle counters were unavailable. On macOS, "
                     "BenchmarkSuite requires elevated privileges to access "
                     "Apple's kperf/kpc counters.\n";
    }
}
}

int main()
{
    AddOrderNoMatch addOrderNoMatch;
    AddOrderWithMatch addOrderWithMatch;
    CancelOrder cancelOrder;
    ModifyOrder modifyOrder;

    BenchmarkStage::run_benchmark<
        "add-no-match", "orderbook", AddOrderNoMatch>(addOrderNoMatch);
    BenchmarkStage::run_benchmark<
        "add-with-match", "orderbook", AddOrderWithMatch>(addOrderWithMatch);
    BenchmarkStage::run_benchmark<
        "cancel", "orderbook", CancelOrder>(cancelOrder);
    BenchmarkStage::run_benchmark<
        "modify", "orderbook", ModifyOrder>(modifyOrder);

    PrintResults();
}
