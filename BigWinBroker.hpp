#pragma once
#include "Broker.hpp"

class BigWinBroker : public Broker {
public:
    BigWinBroker(size_t id, double cash, double itemQty, const Item& item, Exchange* ex, double threshold = 1.25);
    void run() override;

private:
    double profitThreshold_;
    int idleTicks_ = 0;
    static constexpr int MAX_IDLE = 12;
};