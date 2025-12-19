#include <gtest/gtest.h>
#include "Exchange.hpp"
#include "BigWinBroker.hpp"
#include <thread>
#include <chrono>

// Вспомогательный брокер для тестов
class MockBroker : public Broker {
public:
    using Broker::Broker;
    void run() override {
        while (active_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

TEST(BrokerTest, CashAndItemOperations) {
    Exchange exchange(0.01);
    Item oil("Oil", 100.0);
    MockBroker broker(1, 500.0, 10.0, oil, &exchange);

    EXPECT_DOUBLE_EQ(broker.getCash(), 500.0);
    EXPECT_DOUBLE_EQ(broker.getItemQty(), 10.0);

    broker.addCash(100.0);
    EXPECT_DOUBLE_EQ(broker.getCash(), 600.0);

    broker.addItem(5.0);
    EXPECT_DOUBLE_EQ(broker.getItemQty(), 15.0);

    EXPECT_TRUE(broker.spendCash(200.0));
    EXPECT_DOUBLE_EQ(broker.getCash(), 400.0);
    EXPECT_FALSE(broker.spendCash(1000.0));

    EXPECT_TRUE(broker.sellItem(3.0));
    EXPECT_DOUBLE_EQ(broker.getItemQty(), 12.0);
    EXPECT_FALSE(broker.sellItem(20.0));
}

TEST(ExchangeTest, PlaceOrder) {
    Exchange exchange(0.01);
    Item oil("Oil", 100.0);
    MockBroker broker(1, 1000.0, 5.0, oil, &exchange);
    exchange.registerBroker(&broker);

    broker.placeBuyOrder(2.0, 95.0);   
    broker.placeSellOrder(1.0, 105.0);

    auto orders = exchange.getOrders();
    EXPECT_EQ(orders.size(), 2);
}

TEST(ExchangeTest, MatchOrders) {
    Exchange exchange(0.0);
    Item oil("Oil", 100.0);

    MockBroker buyer(1, 1000.0, 0.0, oil, &exchange);
    MockBroker seller(2, 0.0, 10.0, oil, &exchange);
    exchange.registerBroker(&buyer);
    exchange.registerBroker(&seller);

    buyer.placeBuyOrder(5.0, 102.0);
    seller.placeSellOrder(5.0, 98.0);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto orders = exchange.getOrders();
    EXPECT_EQ(orders.size(), 0);

    EXPECT_NEAR(buyer.getItemQty(), 5.0, 1e-5);
    EXPECT_NEAR(seller.getCash(), 500.0, 1e-5);
}

TEST(BigWinBrokerTest, BuysOnGoodDeal) {
    Exchange exchange(0.0);
    Item oil("Oil", 100.0);

    MockBroker seller(1, 0.0, 10.0, oil, &exchange);
    exchange.registerBroker(&seller);
    seller.placeSellOrder(3.0, 70.0);

    BigWinBroker buyer(2, 500.0, 0.0, oil, &exchange, 1.3);
    exchange.registerBroker(&buyer);
    buyer.start();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    buyer.stop();
    buyer.join();

    EXPECT_GT(buyer.getItemQty(), 0.0);
    EXPECT_LT(buyer.getCash(), 500.0);
}
