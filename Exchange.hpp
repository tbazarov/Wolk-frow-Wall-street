#pragma once
#include "Order.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <atomic>

class Broker;

class Exchange {
public:
    using BrokerPtr = std::shared_ptr<Broker>;

    explicit Exchange(double feePerMinute = 0.1);
    ~Exchange();

    void registerBroker(const BrokerPtr& broker);
    void placeOrder(const Order& order);
    std::vector<Order> getOrders() const;
    void stop();
    double getFeePerMinute() const { return feePerMinute_; }

private:
    void runMatchingLoop();
    void chargeFees();

    mutable std::mutex mtx_;
    std::vector<Order> orders_;
    std::vector<BrokerPtr> brokers_;
    std::atomic<bool> running_{true};
    std::jthread matchingThread_;
    double feePerMinute_;
};