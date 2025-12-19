#pragma once
#include "Order.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

class Broker;

class Exchange {
public:
    explicit Exchange(double feePerMinute = 0.1);
    ~Exchange();

    void registerBroker(Broker* broker);
    void placeOrder(const Order& order);
    std::vector<Order> getOrders() const;
    void stop();
    double getFeePerMinute() const { return feePerMinute_; }

private:
    void runMatchingLoop();
    void chargeFees();

    mutable std::mutex mtx_;
    std::vector<Order> orders_;
    std::vector<Broker*> brokers_;
    std::atomic<bool> running_{true};
    std::thread matchingThread_;
    double feePerMinute_;
};