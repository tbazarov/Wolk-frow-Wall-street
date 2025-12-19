#include "Exchange.hpp"
#include "BigWinBroker.hpp"
#include <memory>
#include <vector>
#include <iostream>

int main() {
    Exchange exchange(0.03);
    Item oil("Oil", 100.0);

    std::vector<std::unique_ptr<Broker>> brokers;
    brokers.push_back(std::make_unique<BigWinBroker>(1, 1000.0, 5.0, oil, &exchange, 1.3));
    brokers.push_back(std::make_unique<BigWinBroker>(2, 1200.0, 3.0, oil, &exchange, 1.2));
    brokers.push_back(std::make_unique<BigWinBroker>(3, 800.0, 7.0, oil, &exchange, 1.4));

    for (auto& b : brokers) {
        exchange.registerBroker(b.get());
        b->start();
    }

    std::cout << "🚀 Simulation started with 3 'BigWin' brokers.\n";
    std::cout << "Running for 30 seconds...\n\n";

    std::this_thread::sleep_for(std::chrono::seconds(30));

    for (auto& b : brokers) {
        b->stop();
        b->join();
    }
    exchange.stop();

    std::cout << "\n🏁 Final Results:\n";
    for (auto& b : brokers) {
        std::cout << "Broker " << b->getId() << ": $"
                  << b->getCash() << ", " << b->getItemQty() << " " << oil.name << "\n";
    }

    return 0;
}