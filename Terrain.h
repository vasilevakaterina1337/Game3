#pragma once
#include <memory>
#include <iostream>
#include <string>
#include "Unit.h"

// Ландшафт (работа №2). Нейтральный множитель скорости = 1.0, непроходимость — отдельный bool.
class Terrain {
public:
    virtual ~Terrain() = default;
    virtual bool isPassable(const Unit& unit) const = 0;
    // Проверка без побочных эффектов (без вывода) — для поиска свободной клетки.
    virtual bool canEnter(const Unit& unit) const { return isPassable(unit); }
    virtual bool isBuildable() const { return true; }
    virtual double getSpeedModifier(const Unit& unit) const = 0;
    virtual double getDefenseBonus(const Unit& unit) const = 0;
    virtual char getSymbol() const = 0;
};

class Plains : public Terrain {
public:
    bool isPassable(const Unit&) const override { return true; }
    double getSpeedModifier(const Unit&) const override { return 1.0; }
    double getDefenseBonus(const Unit&) const override { return 0.0; }
    char getSymbol() const override { return '.'; }
};

class Forest : public Terrain {
public:
    bool isPassable(const Unit& unit) const override { return unit.getType() != UnitType::Cavalry; }
    double getSpeedModifier(const Unit&) const override { return 0.5; }
    double getDefenseBonus(const Unit&) const override { return 5.0; }
    char getSymbol() const override { return '*'; }
};

class Swamp : public Terrain {
public:
    bool isPassable(const Unit&) const override { return true; }
    double getSpeedModifier(const Unit&) const override { return 0.3; }
    double getDefenseBonus(const Unit&) const override { return 0.0; }
    char getSymbol() const override { return '~'; }
};

class Mountains : public Terrain {
public:
    bool isPassable(const Unit& unit) const override { return unit.getIsFlying(); }
    bool isBuildable() const override { return false; }
    double getSpeedModifier(const Unit&) const override { return 0.5; }
    double getDefenseBonus(const Unit&) const override { return 10.0; }
    char getSymbol() const override { return '^'; }
};

// Заместитель: контролирует доступ к реальному ландшафту и сообщает об отказе.
class TerrainProxy : public Terrain {
    std::shared_ptr<Terrain> realTerrain;
public:
    explicit TerrainProxy(std::shared_ptr<Terrain> t) : realTerrain(std::move(t)) {}
    bool isPassable(const Unit& unit) const override {
        bool passable = realTerrain->isPassable(unit);
        if (!passable) std::cout << "[Ландшафт] " << unit.getName() << " не может пройти на эту клетку\n";
        return passable;
    }
    bool canEnter(const Unit& unit) const override { return realTerrain->isPassable(unit); }
    bool isBuildable() const override { return realTerrain->isBuildable(); }
    double getSpeedModifier(const Unit& unit) const override { return realTerrain->getSpeedModifier(unit); }
    double getDefenseBonus(const Unit& unit) const override { return realTerrain->getDefenseBonus(unit); }
    char getSymbol() const override { return realTerrain->getSymbol(); }
};
