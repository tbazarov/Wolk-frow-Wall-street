#include "Exchange.hpp"
#include "Broker.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

Exchange::Exchange(double feePerMinute) : feePerMinute_(feePerMinute) {
    matchingThread_ = std::jthread([this](std::stop_token st) {
        while (!st.stop_requested()) {
            chargeFees();
            std::vector<Order> snapshot;
            {
                std::lock_guard<std::mutex> lock(mtx_);
                snapshot = orders_;
            }

            bool matched = false;
            for (size_t i = 0; i < snapshot.size() && !matched; ++i) {
                if (snapshot[i].type != OrderType::BUY) continue;
                for (size_t j = 0; j < snapshot.size(); ++j) {
                    if (snapshot[j].type != OrderType::SELL) continue;
                    if (snapshot[i].item.name != snapshot[j].item.name) continue;
                    if (snapshot[i].pricePerUnit < snapshot[j].pricePerUnit) continue;

                    double tradePrice = (snapshot[i].pricePerUnit + snapshot[j].pricePerUnit) / 2.0;
                    double qty = std::min(snapshot[i].quantity, snapshot[j].quantity);

                    // обновление брокеров
                    {
                        std::lock_guard<std::mutex> lock(mtx_);
                        for (auto& b : brokers_) {
                            if (b->getId() == snapshot[i].brokerId) b->addItem(qty);
                            if (b->getId() == snapshot[j].brokerId) b->addCash(qty * tradePrice);
                        }
                    }

                    // удаление заявок
                    {
                        std::lock_guard<std::mutex> lock(mtx_);
                        orders_.erase(
                            std::remove_if(orders_.begin(), orders_.end(),
                                [&](const Order& o) {
                                    return (o.brokerId == snapshot[i].brokerId && o.type == OrderType::BUY) ||
                                           (o.brokerId == snapshot[j].brokerId && o.type == OrderType::SELL);
                                }),
                            orders_.end()
                        );
                    }

                    std::cout << "Trade: " << qty << " " << snapshot[i].item.name
                              << " @ " << tradePrice << " (B" << snapshot[i].brokerId
                              << " ↔ B" << snapshot[j].brokerId << ")\n";
                    matched = true;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}

Exchange::~Exchange() {
    stop();
}

void Exchange::registerBroker(const BrokerPtr& broker) {
    std::lock_guard<std::mutex> lock(mtx_);
    brokers_.push_back(broker);
}

void Exchange::placeOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mtx_);
    orders_.push_back(order);
}

std::vector<Order> Exchange::getOrders() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return orders_;
}

void Exchange::chargeFees() {
    std::lock_guard<std::mutex> lock(mtx_);
    auto now = std::chrono::steady_clock::now();
    for (auto& order : orders_) {
        double elapsedMinutes = std::chrono::duration<double>(now - order.timestamp).count() / 60.0;
        double totalFee = feePerMinute_ * elapsedMinutes;
        if (totalFee > order.feeAccrued) {
            double newFee = totalFee - order.feeAccrued;
            order.feeAccrued = totalFee;

            for (auto& broker : brokers_) {
                if (broker->getId() == order.brokerId) {
                    broker->spendCash(newFee);
                    break;
                }
            }
        }
    }
}

void Exchange::stop() {
    if (running_.exchange(false)) {
    }
}