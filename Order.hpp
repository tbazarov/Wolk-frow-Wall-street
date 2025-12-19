#pragma once
#include "Item.hpp"
#include <chrono>

enum class OrderType { BUY, SELL };

struct Order {
    size_t brokerId;
    OrderType type;
    Item item;
    double quantity;
    double pricePerUnit;
    mutable double feeAccrued = 0.0;
    std::chrono::steady_clock::time_point timestamp;

    Order(size_t id, OrderType t, const Item& it, double q, double p)
        : brokerId(id), type(t), item(it), quantity(q), pricePerUnit(p),
          timestamp(std::chrono::steady_clock::now()) {}
};