#pragma once
#include <memory>
#include <set>
#include <string>
#include "GameField.h"
#include "Base.h"
#include "ResourceSystem.h"
#include "CombatSystem.h"
#include "Building.h"

// Посредник: координирует поле, базу, ресурсы и бой. Сами объекты друг о друге не знают.
// Правило для каждой операции: СНАЧАЛА проверка всех условий, ПОТОМ согласованное изменение.
// Все операции возвращают true при успехе и false при отказе (отказ не оставляет побочных эффектов).
class GameMediator {
    GameField& field;
    Base& base;
    std::shared_ptr<IObservable> eventBus;

    ResourceSystem resources;   // адаптер над ресурсами Base
    CombatSystem combat;
    std::set<std::string> researched;

    int nextUnitId = 1;         // увеличиваются только после успешного создания
    int nextBuildingId = 1;

    int attackBonus() const;
    int armorBonus() const;
    void printUnits();
    void printBuildings() const;
    void printResources() const;
    void printResearch() const;

public:
    GameMediator(GameField& f, std::shared_ptr<IObservable> bus, Base& b);

    // юниты
    bool createUnit(const std::string& type, int x, int y);
    bool moveUnit(int unitId, int x, int y);
    bool attack(int attackerId, int targetId);
    bool collectResource(int unitId, const std::string& resource);
    bool healUnit(int medicId, int targetId);
    bool interactWithObject(int unitId, int x, int y);

    // база
    bool buildBuilding(const std::string& type, int x, int y);
    bool startResearch(const std::string& tech);
    bool distributeResources(const std::string& from, const std::string& to, int amount);
    bool produceUnit(const std::string& unitType, int buildingId);

    // окружение и просмотр
    bool setTerrain(const std::string& type, int x, int y);
    bool setObject(const std::string& type, int x, int y);
    bool getStats(const std::string& sub);
    void showMap();
};
