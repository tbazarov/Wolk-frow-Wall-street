#pragma once
#include "Order.hpp"
#include <thread>
#include <memory>

class Exchange;

class Broker {
public:
    using Ptr = std::shared_ptr<Broker>;

    Broker(size_t id, double cash, double itemQty, const Item& item, Exchange* ex);
    virtual ~Broker() = default;

    virtual void run(std::stop_token st) = 0; // stop_token

    size_t getId() const { return id_; }
    double getCash() const { return cash_; }
    double getItemQty() const { return itemQty_; }

    void addCash(double amount);
    void addItem(double amount);
    bool spendCash(double amount);
    bool sellItem(double amount);

    void placeBuyOrder(double quantity, double pricePerUnit);
    void placeSellOrder(double quantity, double pricePerUnit);

protected:
    size_t id_;
    double cash_;
    double itemQty_;
    Item item_;
    Exchange* exchange_;
};