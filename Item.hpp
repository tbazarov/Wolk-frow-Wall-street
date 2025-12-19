#pragma once
#include <string>

struct Item {
    std::string name;
    double basePrice;

    Item(std::string n = "Asset", double p = 100.0) : name(std::move(n)), basePrice(p) {}
};