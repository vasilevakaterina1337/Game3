#pragma once
#include <string>
#include <memory>
#include "Attributes.h"

enum class UnitType { Infantry, Archer, Cavalry, Worker, Scout, Tank, Medic };

class Unit {
protected:
    int id;
    UnitType type;
    Health health;
    Armor armor;
    Attack attack;
    int x, y;
    int speed;
    bool isFlying;
    std::string currentOrder;

public:
    Unit(int id, UnitType type, int hp, int arm, int atk, int spd, bool fly = false)
        : id(id), type(type), health(hp), armor(arm), attack(atk), x(0), y(0),
        speed(spd), isFlying(fly), currentOrder("none") {
    }

    virtual ~Unit() = default;

    virtual void move(int newX, int newY) { x = newX; y = newY; }
    virtual void takeDamage(int damage) { health.decrease(damage); }
    virtual void heal(int amount) { health.increase(amount); }

    // Урон по цели по единой формуле (см. Armor::reduceDamage).
    int damageAgainst(const Unit& target, int attackBonus = 0, int armorBonus = 0) const {
        return attack.calculateDamage(target.armor, attackBonus, armorBonus);
    }

    void setOrder(const std::string& order) { currentOrder = order; }
    std::string getOrder() const { return currentOrder; }

    int getId() const { return id; }
    UnitType getType() const { return type; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getHealth() const { return health.getValue(); }
    int getMaxHealth() const { return health.getMax(); }
    int getArmor() const { return armor.getValue(); }
    int getAttack() const { return attack.getValue(); }
    bool getIsFlying() const { return isFlying; }
    bool isAlive() const { return !health.isZero(); }
    void setPosition(int nx, int ny) { x = nx; y = ny; }

    virtual std::string getName() const = 0;
    virtual std::unique_ptr<Unit> clone() const = 0;
};

class Infantry : public Unit {
public:
    Infantry(int id, int hp, int arm, int atk) : Unit(id, UnitType::Infantry, hp, arm, atk, 1) {}
    std::string getName() const override { return "Пехотинец"; }
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Infantry>(*this); }
};

class Archer : public Unit {
public:
    Archer(int id, int hp, int arm, int atk) : Unit(id, UnitType::Archer, hp, arm, atk, 1) {}
    std::string getName() const override { return "Лучник"; }
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Archer>(*this); }
};

class Cavalry : public Unit {
public:
    Cavalry(int id, int hp, int arm, int atk) : Unit(id, UnitType::Cavalry, hp, arm, atk, 3) {}
    std::string getName() const override { return "Кавалерия"; }
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Cavalry>(*this); }
};

class Worker : public Unit {
public:
    Worker(int id, int hp, int arm, int atk) : Unit(id, UnitType::Worker, hp, arm, atk, 1) {}
    std::string getName() const override { return "Рабочий"; }
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Worker>(*this); }
};

class Scout : public Unit {
public:
    Scout(int id, int hp, int arm, int atk) : Unit(id, UnitType::Scout, hp, arm, atk, 4) {}
    std::string getName() const override { return "Разведчик"; }
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Scout>(*this); }
};

class Tank : public Unit {
public:
    Tank(int id, int hp, int arm, int atk) : Unit(id, UnitType::Tank, hp, arm, atk, 1) {}
    std::string getName() const override { return "Танк"; }
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Tank>(*this); }
};

class Medic : public Unit {
public:
    Medic(int id, int hp, int arm, int atk) : Unit(id, UnitType::Medic, hp, arm, atk, 2) {}
    std::string getName() const override { return "Медик"; }
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Medic>(*this); }
};
