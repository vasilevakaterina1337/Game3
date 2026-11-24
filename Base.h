#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <string>
#include "Unit.h"
#include "Building.h"

struct UnitEvent {
    enum Type { Created, Damaged, Destroyed } type;
    int unitId;
};

class IObserver {
public:
    virtual void onUnitEvent(const UnitEvent& event) = 0;
    virtual ~IObserver() = default;
};

class IObservable {
    std::vector<IObserver*> observers;
public:
    void subscribe(IObserver* obs) {
        if (std::find(observers.begin(), observers.end(), obs) == observers.end())
            observers.push_back(obs);
    }
    void notify(const UnitEvent& event) {
        for (auto obs : observers) obs->onUnitEvent(event);
    }
};

class ResourceSystem;

// База (работа №2) — ЕДИНСТВЕННЫЙ владелец учёта: живые юниты, здания, ресурсы.
// Лимит юнитов проверяется до добавления и освобождается при событии Destroyed (ровно один раз).
class Base : public IObserver {
    int id;
    int health = 1000;
    size_t maxUnits = 10;
    std::vector<std::shared_ptr<Unit>> units;
    std::vector<std::shared_ptr<Building>> buildings;

    int wood = 1000;
    int metal = 500;
    int energy = 200;
    friend class ResourceSystem;   // изменяет ресурсы только адаптер ResourceSystem

public:
    explicit Base(int id) : id(id) {}

    void onUnitEvent(const UnitEvent& event) override;

    int getId() const { return id; }

    // юниты
    bool canAddUnit() const { return units.size() < maxUnits; }
    bool addUnit(std::shared_ptr<Unit> unit);   // false, если лимит исчерпан (состояние не меняется)
    size_t getUnitCount() const { return units.size(); }
    size_t getMaxUnits() const { return maxUnits; }
    const std::vector<std::shared_ptr<Unit>>& getUnits() const { return units; }
    std::shared_ptr<Unit> findUnit(int unitId) const;

    // здания
    void addBuilding(std::shared_ptr<Building> b) { buildings.push_back(std::move(b)); }
    const std::vector<std::shared_ptr<Building>>& getBuildings() const { return buildings; }
    std::shared_ptr<Building> findBuilding(int buildingId) const;
    std::shared_ptr<Building> findBuildingByType(const std::string& type) const;

    // только чтение ресурсов
    int getWood() const { return wood; }
    int getMetal() const { return metal; }
    int getEnergy() const { return energy; }
};
