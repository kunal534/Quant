#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "orderbook.h"

using namespace testing;

class orderbook_test : public :: testing :: Test{
    protected:
    void SetUp() override{

    }
    void TearDown() override{

    }
};

/*
test case 1 
check if orders are being added correctly
*/

TEST_F(orderbook_test,addorder)
{
    Order_book orderbook_;
    OrderPointer buy_order= std::make_shared<Order>(OrderTypes::Good_Till_Cancel,1001,Side::BUY,500,10);
    OrderPointer sell_order= std::make_shared<Order>(OrderTypes::Fill_and_Kill,1002,Side::SELL,600,8);

    orderbook_.Add_orders(buy_order);
    orderbook_.Add_orders(sell_order);

    auto bids=orderbook_.Get_Order_Infos().Get_Bids();
    auto asks=orderbook_.Get_Order_Infos().Get_Asks();

    ASSERT_EQ(1,bids.size())<<" Bids size mismatch - Expeceted 1"<<bids.size();
    ASSERT_EQ(1,asks.size())<<" Asks size mismatched - expected 1"<<asks.size();

    EXPECT_EQ(500,bids[0].quantity_)<<"Bids quantity mismatched";
    EXPECT_EQ(600,asks[0].quantity_)<<"Asks quantity mismatched";
}

/*
test case 2
Add orders with same price and check if they are merged
*/

TEST_F(orderbook_test,AddOrder_same_price)
{
    Order_book orderbook_;

    OrderPointer buy_order1= std::make_shared<Order>(OrderTypes::Good_Till_Cancel,1003,Side::BUY,400,5);
    OrderPointer buy_order2=std::make_shared<Order>(OrderTypes::Good_Till_Cancel,1004,Side::BUY,400,2);

    OrderPointer sell_order1= std::make_shared<Order>(OrderTypes::Fill_and_Kill,1005,Side::SELL,500,10);
    OrderPointer sell_order2= std::make_shared<Order>(OrderTypes::Fill_and_Kill,1006,Side::SELL,500,5);

    orderbook_.Add_orders(buy_order1);
    orderbook_.Add_orders(buy_order2);
    orderbook_.Add_orders(sell_order1);
    orderbook_.Add_orders(sell_order2);

    auto asks_=orderbook_.Get_Order_Infos().Get_Asks();
    auto bids_=orderbook_.Get_Order_Infos().Get_Bids();

    ASSERT_EQ(1,asks_.size())<<"Asks size mismatched";
    ASSERT_EQ(1,bids_.size())<<"BIds size mismatched";

}

// main input of the file
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
