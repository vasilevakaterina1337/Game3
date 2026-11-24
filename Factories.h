#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Unit.h"
#include "Cost.h"

// Фабрики юнитов (работа №1) — без изменений в логике создания.
class UnitFactory {
public:
    virtual std::unique_ptr<Unit> createUnit(int id) const = 0;
    virtual ~UnitFactory() = default;
};

class InfantryFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(int id) const override { return std::make_unique<Infantry>(id, 100, 10, 15); }
};
class ArcherFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(int id) const override { return std::make_unique<Archer>(id, 70, 5, 25); }
};
class CavalryFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(int id) const override { return std::make_unique<Cavalry>(id, 120, 15, 20); }
};
class WorkerFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(int id) const override { return std::make_unique<Worker>(id, 50, 0, 5); }
};
class ScoutFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(int id) const override { return std::make_unique<Scout>(id, 60, 5, 10); }
};
class TankFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(int id) const override { return std::make_unique<Tank>(id, 300, 50, 40); }
};
class MedicFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(int id) const override { return std::make_unique<Medic>(id, 80, 5, 5); }
};

// Единственная таблица «имя типа -> фабрика». Раньше соответствие имя->число
// дублировалось в обработчике ввода и в посреднике.
inline std::unique_ptr<UnitFactory> createFactory(const std::string& type) {
    if (type == "infantry") return std::make_unique<InfantryFactory>();
    if (type == "archer")   return std::make_unique<ArcherFactory>();
    if (type == "cavalry")  return std::make_unique<CavalryFactory>();
    if (type == "worker")   return std::make_unique<WorkerFactory>();
    if (type == "scout")    return std::make_unique<ScoutFactory>();
    if (type == "tank")     return std::make_unique<TankFactory>();
    if (type == "medic")    return std::make_unique<MedicFactory>();
    return nullptr;
}

inline const std::vector<std::string>& unitTypeNames() {
    static const std::vector<std::string> names = {
        "infantry", "archer", "cavalry", "worker", "scout", "tank", "medic" };
    return names;
}

// Стоимость производства юнита в бараке (команда create — отладочная, бесплатная).
inline Cost unitCost(const std::string& type) {
    if (type == "infantry") return Cost{ 50, 0, 0 };
    if (type == "archer")   return Cost{ 60, 10, 0 };
    if (type == "cavalry")  return Cost{ 80, 30, 0 };
    if (type == "worker")   return Cost{ 30, 0, 0 };
    if (type == "scout")    return Cost{ 40, 0, 0 };
    if (type == "tank")     return Cost{ 100, 80, 0 };
    if (type == "medic")    return Cost{ 50, 10, 0 };
    return Cost{};
}
