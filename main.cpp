#include "Exchange.hpp"
#include "BigWinBroker.hpp"
#include <memory>
#include <vector>
#include <iostream>

int main() {
    auto exchange = std::make_unique<Exchange>(0.03);
    Item oil("Oil", 100.0);

    std::vector<Exchange::BrokerPtr> brokers;
    brokers.push_back(std::make_shared<BigWinBroker>(1, 1000.0, 5.0, oil, exchange.get(), 1.3));
    brokers.push_back(std::make_shared<BigWinBroker>(2, 1200.0, 3.0, oil, exchange.get(), 1.2));
    brokers.push_back(std::make_shared<BigWinBroker>(3, 800.0, 7.0, oil, exchange.get(), 1.4));

    // запуск потоков брокеров
    std::vector<std::jthread> brokerThreads;
    for (auto& b : brokers) {
        exchange->registerBroker(b);
        brokerThreads.emplace_back([b](std::stop_token st) {
            b->run(st);
        });
    }

    std::cout << "Simulation started with 3 'BigWin' brokers.\n";
    std::cout << "Running for 30 seconds\n\n";

    std::this_thread::sleep_for(std::chrono::seconds(30));


    exchange->stop();

    std::cout << "\nFinal Results:\n";
    for (auto& b : brokers) {
        std::cout << "Broker " << b->getId() << ": $"
                  << b->getCash() << ", " << b->getItemQty() << " " << oil.name << "\n";
    }

    return 0;
}