#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "orderbook.h"

using namespace testing;

class orderbook_test : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(orderbook_test, addorder) {
    Order_book orderbook_;
    OrderPointer buy_order = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1001, Side::BUY, 500, 10);
    OrderPointer sell_order = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1002, Side::SELL, 600, 8);

    orderbook_.Add_orders(buy_order);
    orderbook_.Add_orders(sell_order);

    auto bids = orderbook_.Get_Order_Infos().Get_Bids();
    auto asks = orderbook_.Get_Order_Infos().Get_Asks();

    ASSERT_EQ(1, bids.size());
    ASSERT_EQ(1, asks.size());

    EXPECT_EQ(500, bids[0].price_);
    EXPECT_EQ(10, bids[0].quantity_);
    EXPECT_EQ(600, asks[0].price_);
    EXPECT_EQ(8, asks[0].quantity_);
}

TEST_F(orderbook_test, AddOrder_same_price) {
    Order_book orderbook_;

    OrderPointer buy_order1 = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1003, Side::BUY, 400, 5);
    OrderPointer buy_order2 = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1004, Side::BUY, 400, 2);
    OrderPointer sell_order1 = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1005, Side::SELL, 500, 10);
    OrderPointer sell_order2 = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1006, Side::SELL, 500, 5);

    orderbook_.Add_orders(buy_order1);
    orderbook_.Add_orders(buy_order2);
    orderbook_.Add_orders(sell_order1);
    orderbook_.Add_orders(sell_order2);

    auto asks_ = orderbook_.Get_Order_Infos().Get_Asks();
    auto bids_ = orderbook_.Get_Order_Infos().Get_Bids();

    ASSERT_EQ(1, asks_.size());
    ASSERT_EQ(1, bids_.size());

    EXPECT_EQ(7, bids_[0].quantity_);
    EXPECT_EQ(15, asks_[0].quantity_);
    EXPECT_EQ(400, bids_[0].price_);
    EXPECT_EQ(500, asks_[0].price_);
}

TEST_F(orderbook_test, matching_same_price) {
    Order_book orderbook_;
    OrderPointer buy_order = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1007, Side::BUY, 100, 10);
    OrderPointer sell_order = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1008, Side::SELL, 100, 5);

    orderbook_.Add_orders(buy_order);
    auto trades = orderbook_.Add_orders(sell_order);

    ASSERT_EQ(1, trades.size());
    EXPECT_EQ(5, trades[0].Get_Bid_Trade().quantity_);
    EXPECT_EQ(100, trades[0].Get_Bid_Trade().price_);

    auto bids = orderbook_.Get_Order_Infos().Get_Bids();
    auto asks = orderbook_.Get_Order_Infos().Get_Asks();

    ASSERT_EQ(1, bids.size());
    EXPECT_EQ(5, bids[0].quantity_);
    EXPECT_TRUE(asks.empty());
}

TEST_F(orderbook_test, matching_cross_spread) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1009, Side::SELL, 100, 5));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1010, Side::SELL, 105, 10));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1011, Side::SELL, 110, 7));

    auto trades = orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1012, Side::BUY, 110, 12));

    ASSERT_EQ(2, trades.size());
    EXPECT_EQ(5, trades[0].Get_Bid_Trade().quantity_);
    EXPECT_EQ(7, trades[1].Get_Bid_Trade().quantity_);

    auto asks = orderbook_.Get_Order_Infos().Get_Asks();
    ASSERT_EQ(2, asks.size());
    EXPECT_EQ(105, asks[0].price_);
    EXPECT_EQ(3, asks[0].quantity_);
    EXPECT_EQ(110, asks[1].price_);
    EXPECT_EQ(7, asks[1].quantity_);
    EXPECT_TRUE(orderbook_.Get_Order_Infos().Get_Bids().empty());
}

TEST_F(orderbook_test, fill_and_kill_success) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1013, Side::SELL, 100, 10));
    auto trades = orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Fill_and_Kill, 1014, Side::BUY, 100, 5));

    ASSERT_EQ(1, trades.size());
    EXPECT_EQ(5, trades[0].Get_Bid_Trade().quantity_);

    auto asks = orderbook_.Get_Order_Infos().Get_Asks();
    auto bids = orderbook_.Get_Order_Infos().Get_Bids();
    EXPECT_EQ(1, asks.size());
    EXPECT_TRUE(bids.empty());
}

TEST_F(orderbook_test, fill_and_kill_fail) {
    Order_book orderbook_;
    EXPECT_THROW(
        orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Fill_and_Kill, 1015, Side::BUY, 50, 5)),
        std::runtime_error
    );
    EXPECT_TRUE(orderbook_.Get_Order_Infos().Get_Bids().empty());
    EXPECT_TRUE(orderbook_.Get_Order_Infos().Get_Asks().empty());
}

TEST_F(orderbook_test, fill_and_kill_partial) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1016, Side::SELL, 100, 10));
    auto trades = orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Fill_and_Kill, 1017, Side::BUY, 100, 15));

    ASSERT_EQ(1, trades.size());
    EXPECT_EQ(10, trades[0].Get_Bid_Trade().quantity_);

    EXPECT_TRUE(orderbook_.Get_Order_Infos().Get_Bids().empty());
    EXPECT_TRUE(orderbook_.Get_Order_Infos().Get_Asks().empty());
}

TEST_F(orderbook_test, cancel_order) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1018, Side::BUY, 100, 10));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1019, Side::BUY, 100, 5));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1020, Side::SELL, 200, 8));

    auto snapshot = orderbook_.Get_Order_Infos();
    EXPECT_EQ(2, snapshot.Get_Bids().size() + snapshot.Get_Asks().size());

    orderbook_.Cancel_Order(1018);
    auto bids = orderbook_.Get_Order_Infos().Get_Bids();
    ASSERT_EQ(1, bids.size());
    EXPECT_EQ(5, bids[0].quantity_);

    orderbook_.Cancel_Order(1020);
    EXPECT_TRUE(orderbook_.Get_Order_Infos().Get_Asks().empty());
}

TEST_F(orderbook_test, cancel_nonexistent_order) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1021, Side::BUY, 100, 10));

    try {
        orderbook_.Cancel_Order(9999);
        SUCCEED() << "Cancel_Order did not throw an exception";
    } catch (...) {
        FAIL() << "Cancel_Order should not throw an exception";
    }

    auto bids = orderbook_.Get_Order_Infos().Get_Bids();
    ASSERT_EQ(1, bids.size());
    EXPECT_EQ(10, bids[0].quantity_);
}

TEST_F(orderbook_test, modify_order) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1022, Side::BUY, 100, 10));

    Order_Modify modify(1022, Side::BUY, 150, 10);
    auto trades = orderbook_.ordermodify(modify);

    EXPECT_TRUE(trades.empty());

    auto bids = orderbook_.Get_Order_Infos().Get_Bids();
    ASSERT_EQ(1, bids.size());
    EXPECT_EQ(150, bids[0].price_);
    EXPECT_EQ(10, bids[0].quantity_);
}

TEST_F(orderbook_test, modify_nonexistent_order) {
    Order_book orderbook_;
    Order_Modify modify(9999, Side::BUY, 100, 10);
    EXPECT_THROW(orderbook_.ordermodify(modify), std::runtime_error);
}

TEST_F(orderbook_test, duplicate_order_id) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1023, Side::BUY, 100, 10));
    EXPECT_THROW(
        orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1023, Side::SELL, 200, 5)),
        std::runtime_error
    );
}

TEST_F(orderbook_test, multiple_price_levels) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1024, Side::BUY, 100, 10));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1025, Side::BUY, 100, 5));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1026, Side::BUY, 95, 8));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1027, Side::SELL, 105, 7));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1028, Side::SELL, 110, 12));

    auto snapshot = orderbook_.Get_Order_Infos();
    auto bids = snapshot.Get_Bids();
    auto asks = snapshot.Get_Asks();

    ASSERT_EQ(2, bids.size());
    EXPECT_EQ(100, bids[0].price_);
    EXPECT_EQ(15, bids[0].quantity_);
    EXPECT_EQ(95, bids[1].price_);
    EXPECT_EQ(8, bids[1].quantity_);

    ASSERT_EQ(2, asks.size());
    EXPECT_EQ(105, asks[0].price_);
    EXPECT_EQ(7, asks[0].quantity_);
    EXPECT_EQ(110, asks[1].price_);
    EXPECT_EQ(12, asks[1].quantity_);
}

TEST_F(orderbook_test, complex_matching) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1029, Side::BUY, 100, 10));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1030, Side::BUY, 95, 8));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1031, Side::SELL, 98, 5));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1032, Side::SELL, 100, 6));

    auto trades = orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1033, Side::BUY, 100, 15));

    ASSERT_GT(trades.size(), 0);

    auto snapshot = orderbook_.Get_Order_Infos();
    auto bids = snapshot.Get_Bids();
    auto asks = snapshot.Get_Asks();

    ASSERT_EQ(2, bids.size());
    EXPECT_EQ(100, bids[0].price_);
    EXPECT_EQ(14, bids[0].quantity_);  // 10 existing + 4 new
    EXPECT_EQ(95, bids[1].price_);
    EXPECT_EQ(8, bids[1].quantity_);
    EXPECT_TRUE(asks.empty());
}

TEST_F(orderbook_test, large_quantity_matching) {
    Order_book orderbook_;
    const Quantity large_qty = 1000000;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1034, Side::SELL, 100, large_qty));

    auto trades = orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1035, Side::BUY, 100, large_qty / 2));

    ASSERT_EQ(1, trades.size());
    EXPECT_EQ(large_qty / 2, trades[0].Get_Bid_Trade().quantity_);

    auto asks = orderbook_.Get_Order_Infos().Get_Asks();
    ASSERT_EQ(1, asks.size());
    EXPECT_EQ(large_qty / 2, asks[0].quantity_);
}

TEST_F(orderbook_test, add_null_order) {
    Order_book orderbook_;
    EXPECT_THROW(orderbook_.Add_orders(nullptr), std::runtime_error);
}

TEST_F(orderbook_test, modify_order_same_values) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1036, Side::BUY, 100, 10));

    Order_Modify modify(1036, Side::BUY, 100, 10);
    auto trades = orderbook_.ordermodify(modify);

    EXPECT_TRUE(trades.empty());

    auto bids = orderbook_.Get_Order_Infos().Get_Bids();
    ASSERT_EQ(1, bids.size());
    EXPECT_EQ(100, bids[0].price_);
    EXPECT_EQ(10, bids[0].quantity_);
}

TEST_F(orderbook_test, modify_order_triggers_matching) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1037, Side::SELL, 100, 10));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1038, Side::BUY, 90, 5));

    auto snapshot_before = orderbook_.Get_Order_Infos();
    EXPECT_EQ(1, snapshot_before.Get_Bids().size());
    EXPECT_EQ(1, snapshot_before.Get_Asks().size());

    Order_Modify modify(1038, Side::BUY, 100, 5);
    auto trades = orderbook_.ordermodify(modify);

    ASSERT_EQ(1, trades.size());
    EXPECT_EQ(5, trades[0].Get_Bid_Trade().quantity_);

    auto snapshot_after = orderbook_.Get_Order_Infos();
    EXPECT_TRUE(snapshot_after.Get_Bids().empty());
    auto asks = snapshot_after.Get_Asks();
    ASSERT_EQ(1, asks.size());
    EXPECT_EQ(5, asks[0].quantity_);
}

TEST_F(orderbook_test, fifo_order_matching) {
    Order_book orderbook_;
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1039, Side::SELL, 100, 10));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1040, Side::SELL, 100, 5));
    orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1041, Side::SELL, 100, 8));

    auto asks_before = orderbook_.Get_Order_Infos().Get_Asks();
    EXPECT_EQ(1, asks_before.size());
    EXPECT_EQ(23, asks_before[0].quantity_);

    auto trades = orderbook_.Add_orders(std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1042, Side::BUY, 100, 12));

    ASSERT_EQ(2, trades.size());
    EXPECT_EQ(10, trades[0].Get_Bid_Trade().quantity_);
    EXPECT_EQ(2, trades[1].Get_Bid_Trade().quantity_);

    auto asks_after = orderbook_.Get_Order_Infos().Get_Asks();
    ASSERT_EQ(1, asks_after.size());
    EXPECT_EQ(11, asks_after[0].quantity_);
}

TEST_F(orderbook_test, zero_quantity_orders) {
    Order_book orderbook_;
    auto zero_order = std::make_shared<Order>(OrderTypes::Good_Till_Cancel, 1043, Side::BUY, 100, 0);
    auto trades = orderbook_.Add_orders(zero_order);

    EXPECT_TRUE(trades.empty());

    auto bids = orderbook_.Get_Order_Infos().Get_Bids();
    EXPECT_TRUE(bids.empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}