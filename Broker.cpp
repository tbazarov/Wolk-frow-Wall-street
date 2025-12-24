#include "Broker.hpp"
#include "Exchange.hpp"
#include <iostream>
#include <mutex>

static std::mutex ioMutex;

Broker::Broker(size_t id, double cash, double itemQty, const Item& item, Exchange* ex)
    : id_(id), cash_(cash), itemQty_(itemQty), item_(item), exchange_(ex) {}

void Broker::addCash(double amount) { cash_ += amount; }
void Broker::addItem(double amount) { itemQty_ += amount; }

bool Broker::spendCash(double amount) {
    if (cash_ >= amount) { cash_ -= amount; return true; }
    return false;
}

bool Broker::sellItem(double amount) {
    if (itemQty_ >= amount) { itemQty_ -= amount; return true; }
    return false;
}

void Broker::placeBuyOrder(double quantity, double pricePerUnit) {
    if (spendCash(quantity * pricePerUnit)) {
        Order order{id_, OrderType::BUY, item_, quantity, pricePerUnit};
        exchange_->placeOrder(order);
        {
            std::lock_guard<std::mutex> lock(ioMutex);
            std::cout << "B" << id_ << " BUY " << quantity << "" << pricePerUnit << "\n";
        }
    }
}

void Broker::placeSellOrder(double quantity, double pricePerUnit) {
    if (sellItem(quantity)) {
        Order order{id_, OrderType::SELL, item_, quantity, pricePerUnit};
        exchange_->placeOrder(order);
        {
            std::lock_guard<std::mutex> lock(ioMutex);
            std::cout << "B" << id_ << " SELL " << quantity << "" << pricePerUnit << "\n";
        }
    }
}