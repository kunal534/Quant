#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#define ORDERBOOK_STANDALONE
#include "orderbook.cpp"

static int tests_run = 0;
static int tests_failed = 0;

void expect(bool condition, const std::string &description)
{
    ++tests_run;
    if (!condition)
    {
        std::cerr << "FAIL: " << description << "\n";
        ++tests_failed;
    }
}

template <typename T>
void expect_eq(const T &actual, const T &expected, const std::string &description)
{
    ++tests_run;
    if (!(actual == expected))
    {
        std::cerr << "FAIL: " << description << "\n"
                  << "  expected: " << expected << "\n"
                  << "  actual:   " << actual << "\n";
        ++tests_failed;
    }
}

void test_add_single_buy_order()
{
    Order_book book;
    book.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1, Side::BUY, 10000, 10));

    expect_eq(book.Size(), static_cast<std::size_t>(1), "Single buy order should increase size to 1");
    const auto &levels = book.Get_Order_Infos().Get_Bids();
    expect_eq(levels.size(), static_cast<std::size_t>(1), "Book should expose one bid price level");
    expect_eq(levels[0].price_, static_cast<Price>(10000), "Bid level price should equal order price");
    expect_eq(levels[0].quantity_, static_cast<Quantity>(10), "Bid level quantity should equal remaining quantity");
}

void test_trade_execution_for_matching_orders()
{
    Order_book book;
    book.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1, Side::BUY, 10000, 10));
    Trades trades = book.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 2, Side::SELL, 10000, 10));

    expect_eq(trades.size(), static_cast<std::size_t>(1), "Matching buy and sell orders should produce one trade");
    expect_eq(book.Size(), static_cast<std::size_t>(0), "Filled orders should be removed from the book");
    expect_eq(trades[0].Get_Bid_Trade().quantity_, static_cast<Quantity>(10), "Trade quantity should be equal to min quantity");
    expect_eq(trades[0].Get_Ask_Trade().price_, static_cast<Price>(10000), "Trade ask price should preserve original ask price");
}

void test_cancel_removes_order_from_book()
{
    Order_book book;
    book.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1, Side::BUY, 10000, 10));
    book.Cancel_Order(1);

    expect_eq(book.Size(), static_cast<std::size_t>(0), "Cancel_Order should remove the order from the book");
    expect_eq(book.Get_Order_Infos().Get_Bids().size(), static_cast<std::size_t>(0), "Cancelled bid should remove the price level");
}

void test_fill_and_kill_rejects_unmatched_order()
{
    Order_book book;
    bool thrown = false;
    try
    {
        book.Add_orders(std::make_shared<Order>(OrderTypes::Fill_and_Kill, 1, Side::BUY, 10000, 10));
    }
    catch (const std::runtime_error &)
    {
        thrown = true;
    }

    expect(thrown, "Fill_and_Kill order must throw when no matching liquidity exists");
    expect_eq(book.Size(), static_cast<std::size_t>(0), "Unmatched Fill_and_Kill orders should not be added to the book");
}

void test_modify_order_updates_price_and_quantity()
{
    Order_book book;
    book.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1, Side::BUY, 10000, 10));
    Orders trades = book.ordermodify(Order_Modify(1, Side::BUY, 10050, 10));

    expect_eq(book.Size(), static_cast<std::size_t>(1), "Modified order should remain in the book");
    const auto &levels = book.Get_Order_Infos().Get_Bids();
    expect_eq(levels.size(), static_cast<std::size_t>(1), "Modified order should still create one bid level");
    expect_eq(levels[0].price_, static_cast<Price>(10050), "Order modify should update the bid price");
    expect_eq(trades.size(), static_cast<std::size_t>(0), "Modifying an order without matching should not generate trades");
}

int main()
{
    test_add_single_buy_order();
    test_trade_execution_for_matching_orders();
    test_cancel_removes_order_from_book();
    test_fill_and_kill_rejects_unmatched_order();
    test_modify_order_updates_price_and_quantity();

    std::cout << "Tests run: " << tests_run << "\n";
    if (tests_failed == 0)
    {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cout << "Tests failed: " << tests_failed << "\n";
    return 1;
}
