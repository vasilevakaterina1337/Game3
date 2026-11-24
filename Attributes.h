#pragma once
#include <algorithm>

// Атрибуты юнита (работа №1). Инварианты: значения неотрицательны,
// здоровье не превышает максимум, мёртвого лечением не воскресить.

class Health {
    int value;
    int maxValue;
public:
    explicit Health(int maxV)
        : value(std::max(0, maxV)), maxValue(std::max(0, maxV)) {}

    // Отрицательное значение игнорируется (раньше takeDamage(-20) работал как лечение сверх максимума).
    void decrease(int amount) {
        if (amount <= 0) return;
        value = std::max(0, value - amount);
    }
    void increase(int amount) {
        if (amount <= 0 || value == 0) return;
        value = std::min(maxValue, value + amount);
    }
    bool isZero() const { return value <= 0; }
    int getValue() const { return value; }
    int getMax() const { return maxValue; }
};

class Armor {
    int value;
public:
    explicit Armor(int v) : value(std::max(0, v)) {}

    // ЕДИНСТВЕННОЕ место, где броня вычитается из урона:
    // урон = max(0, атака - (броня + бонус брони)).
    int reduceDamage(int damage, int armorBonus = 0) const {
        return std::max(0, damage - (value + armorBonus));
    }
    int getValue() const { return value; }
};

class Attack {
    int value;
public:
    explicit Attack(int v) : value(std::max(0, v)) {}

    int calculateDamage(const Armor& targetArmor, int attackBonus = 0, int armorBonus = 0) const {
        return targetArmor.reduceDamage(value + attackBonus, armorBonus);
    }
    int getValue() const { return value; }
};
