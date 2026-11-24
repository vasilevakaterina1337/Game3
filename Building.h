#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Cost.h"

// Минимальная модель построек. Строительство мгновенное (таймер реального времени не требуется ТЗ),
// поэтому progress сразу равен 100.
class Building {
protected:
    std::string type;
    int id = 0;
    int hp = 100;
    int progress = 100;
    int x, y;
    std::map<std::string, int> stock;   // ресурсы, распределённые в здание (distribute)
public:
    Building(std::string t, int x, int y) : type(std::move(t)), x(x), y(y) {}
    virtual ~Building() = default;

    virtual bool canProduce() const { return false; }    // производит юнитов
    virtual std::string displayName() const { return type; }

    void setId(int newId) { id = newId; }
    int getId() const { return id; }
    std::string getType() const { return type; }
    int getProgress() const { return progress; }
    int getX() const { return x; }
    int getY() const { return y; }

    void receive(const std::string& resource, int amount) { stock[resource] += amount; }
    const std::map<std::string, int>& getStock() const { return stock; }
};

class Barracks : public Building {
public:
    Barracks(int x, int y) : Building("barracks", x, y) {}
    bool canProduce() const override { return true; }
    std::string displayName() const override { return "Барак"; }
};

class Laboratory : public Building {
public:
    Laboratory(int x, int y) : Building("lab", x, y) {}
    std::string displayName() const override { return "Лаборатория"; }
};

class Warehouse : public Building {
public:
    Warehouse(int x, int y) : Building("warehouse", x, y) {}
    std::string displayName() const override { return "Склад"; }
};

class PowerPlant : public Building {
public:
    PowerPlant(int x, int y) : Building("powerplant", x, y) {}
    std::string displayName() const override { return "Электростанция"; }
};

class Hospital : public Building {
public:
    Hospital(int x, int y) : Building("hospital", x, y) {}
    std::string displayName() const override { return "Госпиталь"; }
};

// Единая точка создания зданий и их стоимости.
inline std::shared_ptr<Building> createBuilding(const std::string& type, int x, int y) {
    if (type == "barracks")   return std::make_shared<Barracks>(x, y);
    if (type == "lab")        return std::make_shared<Laboratory>(x, y);
    if (type == "warehouse")  return std::make_shared<Warehouse>(x, y);
    if (type == "powerplant") return std::make_shared<PowerPlant>(x, y);
    if (type == "hospital")   return std::make_shared<Hospital>(x, y);
    return nullptr;
}

inline const std::vector<std::string>& buildingTypeNames() {
    static const std::vector<std::string> names = { "barracks", "lab", "warehouse", "powerplant", "hospital" };
    return names;
}

inline Cost buildingCost(const std::string& type) {
    if (type == "barracks")   return Cost{ 100, 0, 0 };
    if (type == "lab")        return Cost{ 100, 50, 0 };
    if (type == "warehouse")  return Cost{ 80, 0, 0 };
    if (type == "powerplant") return Cost{ 100, 50, 0 };
    if (type == "hospital")   return Cost{ 120, 30, 0 };
    return Cost{};
}
