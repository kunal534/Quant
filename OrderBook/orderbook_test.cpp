#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "orderbook.h"

using namespace testing;

class Orderbook_book_test:public::testing::Test{
    // as private would result in making it unaccesble for child class
    protected:
    std::unique_ptr<Order_book>orderbook;
    void SetUp() override {
        orderbook = std::make_unique<Order_book>();
    }
    
};