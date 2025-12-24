#include "BigWinBroker.hpp"
#include "Exchange.hpp"
#include "Item.hpp"
#include <thread>

BigWinBroker::BigWinBroker(size_t id, double cash, double itemQty, const Item& item, Exchange* ex, double threshold)
    : Broker(id, cash, itemQty, item, ex), profitThreshold_(threshold) {}

void BigWinBroker::run(std::stop_token st) {
    while (!st.stop_requested()) {
        auto orders = exchange_->getOrders();
        bool traded = false;

        for (const auto& order : orders) {
            if (order.type == OrderType::SELL && order.pricePerUnit <= item_.basePrice / profitThreshold_) {
                double qty = std::min(3.0, cash_ / order.pricePerUnit);
                if (qty > 0.1) {
                    placeBuyOrder(qty, order.pricePerUnit);
                    traded = true;
                    break;
                }
            }
        }

        if (!traded) {
            for (const auto& order : orders) {
                if (order.type == OrderType::BUY && order.pricePerUnit >= item_.basePrice * profitThreshold_) {
                    double qty = std::min(3.0, itemQty_);
                    if (qty > 0.1) {
                        placeSellOrder(qty, order.pricePerUnit);
                        traded = true;
                        break;
                    }
                }
            }
        }

        if (traded) {
            idleTicks_ = 0;
        } else {
            idleTicks_++;
        }

        if (idleTicks_ > MAX_IDLE && itemQty_ > 0) {
            double fireSalePrice = item_.basePrice * 0.8;
            double qty = std::min(2.0, itemQty_);
            placeSellOrder(qty, fireSalePrice);
            idleTicks_ = 0;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}