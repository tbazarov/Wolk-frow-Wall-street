#include <gtest/gtest.h>
#include "Exchange.hpp"
#include "BigWinBroker.hpp"
#include <thread>
#include <chrono>
#include <memory>

// Вспомогательный MockBroker
class MockBroker : public Broker {
public:
    using Broker::Broker;

    void run(std::stop_token st) override {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

// Тест: Broker корректно управляет деньгами и товарами
TEST(BrokerTest, CashAndItemOperations) {
    auto exchange = std::make_unique<Exchange>(0.01);
    Item oil("Oil", 100.0);
    MockBroker broker(1, 500.0, 10.0, oil, exchange.get());

    EXPECT_DOUBLE_EQ(broker.getCash(), 500.0);
    EXPECT_DOUBLE_EQ(broker.getItemQty(), 10.0);

    broker.addCash(100.0);
    EXPECT_DOUBLE_EQ(broker.getCash(), 600.0);

    broker.addItem(5.0);
    EXPECT_DOUBLE_EQ(broker.getItemQty(), 15.0);

    EXPECT_TRUE(broker.spendCash(200.0));
    EXPECT_DOUBLE_EQ(broker.getCash(), 400.0);
    EXPECT_FALSE(broker.spendCash(1000.0)); // недостаточно средств

    EXPECT_TRUE(broker.sellItem(3.0));
    EXPECT_DOUBLE_EQ(broker.getItemQty(), 12.0);
    EXPECT_FALSE(broker.sellItem(20.0)); // недостаточно товара
}

// Тест: Exchange корректно размещает заявки
TEST(ExchangeTest, PlaceOrder) {
    auto exchange = std::make_unique<Exchange>(0.01);
    Item oil("Oil", 100.0);
    auto broker = std::make_shared<MockBroker>(1, 1000.0, 5.0, oil, exchange.get());
    exchange->registerBroker(broker);

    broker->placeBuyOrder(2.0, 95.0);
    broker->placeSellOrder(1.0, 105.0);

    auto orders = exchange->getOrders();
    EXPECT_EQ(orders.size(), 2);

    bool hasBuy = false, hasSell = false;
    for (const auto& order : orders) {
        if (order.type == OrderType::BUY && order.quantity == 2.0 && order.pricePerUnit == 95.0)
            hasBuy = true;
        if (order.type == OrderType::SELL && order.quantity == 1.0 && order.pricePerUnit == 105.0)
            hasSell = true;
    }
    EXPECT_TRUE(hasBuy);
    EXPECT_TRUE(hasSell);
}

// Тест: Exchange матчинг заявок BUY/SELL
TEST(ExchangeTest, MatchOrders) {
    auto exchange = std::make_unique<Exchange>(0.0);
    Item oil("Oil", 100.0);

    auto buyer = std::make_shared<MockBroker>(1, 1000.0, 0.0, oil, exchange.get());
    auto seller = std::make_shared<MockBroker>(2, 0.0, 10.0, oil, exchange.get());

    exchange->registerBroker(buyer);
    exchange->registerBroker(seller);

    // Размещаем совместимые заявки
    buyer->placeBuyOrder(5.0, 102.0);   // готов купить по 102
    seller->placeSellOrder(5.0, 98.0);  // готов продать по 98

    // Даём время на матчинг (1-2 сек)
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // После матчинга заявок не должно остаться
    auto orders = exchange->getOrders();
    EXPECT_EQ(orders.size(), 0);

    // Проверяем балансы
    EXPECT_NEAR(buyer->getItemQty(), 5.0, 1e-5);
    EXPECT_NEAR(seller->getCash(), 5.0 * 100.0, 1e-5); // (102+98)/2 = 100
}

// Тест: BigWinBroker размещает заявку при выгодной цене
TEST(BigWinBrokerTest, PlacesOrderOnGoodDeal) {
    auto exchange = std::make_unique<Exchange>(0.0);
    Item oil("Oil", 100.0);

    // Создаём "агрессивного" продавца
    auto seller = std::make_shared<MockBroker>(1, 0.0, 10.0, oil, exchange.get());
    exchange->registerBroker(seller);
    seller->placeSellOrder(3.0, 70.0); // продаёт сильно дешевле базовой цены

    // BigWinBroker с порогом 1.3 (покупает, если цена <= 100/1.3 ≈ 76.9)
    auto buyer = std::make_shared<BigWinBroker>(2, 500.0, 0.0, oil, exchange.get(), 1.3);
    exchange->registerBroker(buyer);

    // Запускаем брокера в отдельном jthread
    std::jthread brokerThread([buyer](std::stop_token st) {
        buyer->run(st);
    });

    // ожидание реакции брокера
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // остановка потока

    // Должна была произойти покупка
    EXPECT_GT(buyer->getItemQty(), 0.0);
    EXPECT_LT(buyer->getCash(), 500.0);
}