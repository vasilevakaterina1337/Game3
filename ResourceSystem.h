#pragma once
#include <string>
#include "Base.h"
#include "Cost.h"

// Адаптер над ресурсами Base: второго хранилища нет, все изменения идут в Base.
class ResourceSystem {
    Base& base;

    int* slot(const std::string& r) {
        if (r == "wood") return &base.wood;
        if (r == "metal") return &base.metal;
        if (r == "energy") return &base.energy;
        return nullptr;
    }
    const int* slot(const std::string& r) const {
        if (r == "wood") return &base.wood;
        if (r == "metal") return &base.metal;
        if (r == "energy") return &base.energy;
        return nullptr;
    }

public:
    explicit ResourceSystem(Base& b) : base(b) {}

    // "1"/"wood" -> "wood", "2"/"metal" -> "metal", "3"/"energy" -> "energy"; иначе "".
    static std::string normalize(const std::string& token) {
        if (token == "1" || token == "wood") return "wood";
        if (token == "2" || token == "metal") return "metal";
        if (token == "3" || token == "energy") return "energy";
        return "";
    }

    int get(const std::string& r) const {
        const int* p = slot(r);
        return p ? *p : 0;
    }

    void add(const std::string& r, int amount) {
        int* p = slot(r);
        if (p && amount > 0) *p += amount;
    }

    bool canAfford(const Cost& c) const {
        return c.wood >= 0 && c.metal >= 0 && c.energy >= 0
            && base.wood >= c.wood && base.metal >= c.metal && base.energy >= c.energy;
    }

    // Атомарно: либо списывается всё, либо ничего.
    bool spend(const Cost& c) {
        if (!canAfford(c)) return false;
        base.wood -= c.wood;
        base.metal -= c.metal;
        base.energy -= c.energy;
        return true;
    }

    std::string describeMissing(const Cost& c) const {
        std::string s;
        auto part = [&](const char* name, int need, int have) {
            if (need > have) {
                if (!s.empty()) s += ", ";
                s += std::string(name) + ": нужно " + std::to_string(need) + ", есть " + std::to_string(have);
            }
        };
        part("wood", c.wood, base.wood);
        part("metal", c.metal, base.metal);
        part("energy", c.energy, base.energy);
        return s;
    }
};
