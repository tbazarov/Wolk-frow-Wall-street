#pragma once
#include "Order.hpp"
#include <thread>
#include <atomic>

class Exchange;

class Broker {
public:
    Broker(size_t id, double cash, double itemQty, const Item& item, Exchange* ex);
    virtual ~Broker();

    void start();
    virtual void run() = 0;
    void stop() { active_ = false; }
    void join();

    size_t getId() const { return id_; }
    double getCash() const { return cash_; }
    double getItemQty() const { return itemQty_; }

    void addCash(double amount);
    void addItem(double amount);
    bool spendCash(double amount);
    bool sellItem(double amount);

protected:
    size_t id_;
    double cash_;
    double itemQty_;
    Item item_;
    Exchange* exchange_;
    std::thread thread_;
    std::atomic<bool> active_{true};

    void placeBuyOrder(double quantity, double pricePerUnit);
    void placeSellOrder(double quantity, double pricePerUnit);
};