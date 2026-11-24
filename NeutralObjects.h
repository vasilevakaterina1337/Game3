#pragma once
#include <memory>
#include <iostream>
#include "Unit.h"

// Нейтральные объекты и стратегии (работа №2).
class InteractionStrategy {
public:
    virtual ~InteractionStrategy() = default;
    virtual void apply(Unit& unit) = 0;
};

class AttackBoostStrategy : public InteractionStrategy {
public:
    void apply(Unit& unit) override {
        std::cout << "[Объект] Бонус атаки для " << unit.getName() << " (информационный эффект)\n";
    }
};

class HealthRestoreStrategy : public InteractionStrategy {
public:
    void apply(Unit& unit) override {
        int before = unit.getHealth();
        unit.heal(20);   // раньше takeDamage(-20): здоровье могло превысить максимум
        std::cout << "[Объект] Здоровье восстановлено для " << unit.getName()
            << ": +" << (unit.getHealth() - before) << " HP\n";
    }
};

class NeutralObject {
protected:
    std::shared_ptr<InteractionStrategy> strategy;
public:
    explicit NeutralObject(std::shared_ptr<InteractionStrategy> strat) : strategy(std::move(strat)) {}
    virtual ~NeutralObject() = default;

    virtual void interact(Unit& unit) { strategy->apply(unit); }

    virtual NeutralObject& operator+(Unit& unit) {
        interact(unit);
        return *this;
    }
    virtual NeutralObject& operator-(Unit& unit) {
        std::cout << "[Ловушка] " << unit.getName() << " теряет 20 здоровья\n";
        unit.takeDamage(20);
        return *this;
    }
    virtual NeutralObject& operator*(Unit& unit) {
        std::cout << "[Объект] " << unit.getName() << " взаимодействует с объектом\n";
        interact(unit);
        return *this;
    }
};

class TreasureChest : public NeutralObject {
public:
    TreasureChest() : NeutralObject(std::make_shared<HealthRestoreStrategy>()) {}
};

class Trap : public NeutralObject {
public:
    Trap() : NeutralObject(std::make_shared<AttackBoostStrategy>()) {}
};

class Altar : public NeutralObject {
public:
    Altar() : NeutralObject(std::make_shared<AttackBoostStrategy>()) {}
};
