#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include "Unit.h"
#include "Terrain.h"
#include "NeutralObjects.h"
#include "Building.h"

enum class PlaceResult { Ok, OutOfBounds, Occupied, Impassable, NotOnField };

// Поле — единственный источник истины о расположении юнитов, зданий, объектов и ландшафта.
class GameField {
    int width, height;
    std::vector<std::vector<std::shared_ptr<Unit>>> grid;
    std::vector<std::vector<std::shared_ptr<Terrain>>> terrains;
    std::vector<std::vector<std::shared_ptr<NeutralObject>>> objects;
    std::vector<std::vector<std::shared_ptr<Building>>> buildingsGrid;

public:
    GameField(int w, int h);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool inBounds(int x, int y) const { return x >= 0 && x < width && y >= 0 && y < height; }

    // Итератор по занятым юнитами клеткам (построчно). begin() != end(), если на поле есть юниты.
    class Iterator {
        GameField& field;
        int curX = 0, curY = 0;
        void skipEmpty();
    public:
        Iterator(GameField& f, bool atEnd);
        bool operator!=(const Iterator& other) const { return curX != other.curX || curY != other.curY; }
        Iterator& operator++();
        std::shared_ptr<Unit> operator*() const { return field.grid[curY][curX]; }
    };
    Iterator begin() { return Iterator(*this, false); }
    Iterator end() { return Iterator(*this, true); }

    // --- юниты: проверки выполняются ДО изменения состояния ---
    PlaceResult checkPlacement(const Unit& unit, int x, int y, bool verbose = true) const;
    bool addUnit(std::shared_ptr<Unit> unit, int x, int y);
    PlaceResult moveUnit(const std::shared_ptr<Unit>& unit, int x, int y);   // атомарно
    void removeUnit(const std::shared_ptr<Unit>& unit);
    std::shared_ptr<Unit> getUnitAt(int x, int y) const;
    // Первая свободная клетка рядом (сначала сама клетка, затем 8 соседей).
    bool findFreeNear(const Unit& unit, int cx, int cy, int& outX, int& outY) const;

    // --- ландшафт, объекты, здания: возвращают текст причины отказа ("" = успех) ---
    std::string setTerrain(int x, int y, std::shared_ptr<Terrain> t);
    std::string setObject(int x, int y, std::shared_ptr<NeutralObject> obj);
    std::shared_ptr<NeutralObject> getObject(int x, int y) const;
    std::string canBuildAt(int x, int y) const;
    void placeBuilding(std::shared_ptr<Building> b, int x, int y);   // после успешного canBuildAt

    void printMap() const;
};

inline GameField::GameField(int w, int h) : width(w), height(h) {
    grid.assign(height, std::vector<std::shared_ptr<Unit>>(width, nullptr));
    terrains.assign(height, std::vector<std::shared_ptr<Terrain>>(width, std::make_shared<Plains>()));
    objects.assign(height, std::vector<std::shared_ptr<NeutralObject>>(width, nullptr));
    buildingsGrid.assign(height, std::vector<std::shared_ptr<Building>>(width, nullptr));
}

inline GameField::Iterator::Iterator(GameField& f, bool atEnd) : field(f) {
    if (atEnd) { curX = 0; curY = field.height; }
    else skipEmpty();
}

inline void GameField::Iterator::skipEmpty() {
    while (curY < field.height && !field.grid[curY][curX]) {
        if (++curX >= field.width) { curX = 0; ++curY; }
    }
}

inline GameField::Iterator& GameField::Iterator::operator++() {
    if (curY >= field.height) return *this;
    if (++curX >= field.width) { curX = 0; ++curY; }
    skipEmpty();
    return *this;
}

inline PlaceResult GameField::checkPlacement(const Unit& unit, int x, int y, bool verbose) const {
    if (!inBounds(x, y)) return PlaceResult::OutOfBounds;
    if (grid[y][x]) return PlaceResult::Occupied;
    const auto& t = terrains[y][x];
    bool ok = verbose ? t->isPassable(unit) : t->canEnter(unit);
    return ok ? PlaceResult::Ok : PlaceResult::Impassable;
}

inline bool GameField::addUnit(std::shared_ptr<Unit> unit, int x, int y) {
    if (!unit || checkPlacement(*unit, x, y) != PlaceResult::Ok) return false;
    grid[y][x] = unit;
    unit->setPosition(x, y);
    return true;
}

inline PlaceResult GameField::moveUnit(const std::shared_ptr<Unit>& unit, int x, int y) {
    if (!unit || !inBounds(unit->getX(), unit->getY()) || grid[unit->getY()][unit->getX()] != unit)
        return PlaceResult::NotOnField;
    PlaceResult r = checkPlacement(*unit, x, y);   // проверка до любых изменений
    if (r != PlaceResult::Ok) return r;
    grid[unit->getY()][unit->getX()] = nullptr;
    grid[y][x] = unit;
    unit->setPosition(x, y);
    return PlaceResult::Ok;
}

inline void GameField::removeUnit(const std::shared_ptr<Unit>& unit) {
    if (unit && inBounds(unit->getX(), unit->getY()) && grid[unit->getY()][unit->getX()] == unit)
        grid[unit->getY()][unit->getX()] = nullptr;
}

inline std::shared_ptr<Unit> GameField::getUnitAt(int x, int y) const {
    return inBounds(x, y) ? grid[y][x] : nullptr;
}

inline bool GameField::findFreeNear(const Unit& unit, int cx, int cy, int& outX, int& outY) const {
    if (checkPlacement(unit, cx, cy, false) == PlaceResult::Ok) { outX = cx; outY = cy; return true; }
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            if (checkPlacement(unit, cx + dx, cy + dy, false) == PlaceResult::Ok) {
                outX = cx + dx; outY = cy + dy;
                return true;
            }
        }
    }
    return false;
}

inline std::string GameField::setTerrain(int x, int y, std::shared_ptr<Terrain> t) {
    if (!inBounds(x, y)) return "координаты вне карты";
    if (!t) return "ландшафт не задан";
    if (grid[y][x] && !t->isPassable(*grid[y][x]))
        return "на клетке стоит юнит, которому такой ландшафт непроходим";
    if (buildingsGrid[y][x] && !t->isBuildable())
        return "на клетке стоит здание, а такой ландшафт не позволяет строить";
    terrains[y][x] = std::make_shared<TerrainProxy>(t);
    return "";
}

inline std::string GameField::setObject(int x, int y, std::shared_ptr<NeutralObject> obj) {
    if (!inBounds(x, y)) return "координаты вне карты";
    if (buildingsGrid[y][x]) return "на клетке здание";
    objects[y][x] = std::move(obj);
    return "";
}

inline std::shared_ptr<NeutralObject> GameField::getObject(int x, int y) const {
    return inBounds(x, y) ? objects[y][x] : nullptr;
}

inline std::string GameField::canBuildAt(int x, int y) const {
    if (!inBounds(x, y)) return "координаты вне карты (допустимо x: 0.." + std::to_string(width - 1)
        + ", y: 0.." + std::to_string(height - 1) + ")";
    if (buildingsGrid[y][x]) return "клетка уже занята зданием";
    if (grid[y][x]) return "на клетке стоит юнит";
    if (objects[y][x]) return "на клетке нейтральный объект";
    if (!terrains[y][x]->isBuildable()) return "ландшафт не позволяет строить";
    return "";
}

inline void GameField::placeBuilding(std::shared_ptr<Building> b, int x, int y) {
    if (inBounds(x, y)) buildingsGrid[y][x] = std::move(b);
}

inline void GameField::printMap() const {
    std::cout << "\n   ";
    for (int i = 0; i < width; ++i) std::cout << i << " ";
    std::cout << "\n";
    for (int y = 0; y < height; ++y) {
        std::cout << y << "  ";
        for (int x = 0; x < width; ++x) {
            char c = terrains[y][x]->getSymbol();
            if (buildingsGrid[y][x]) c = 'B';
            if (objects[y][x]) c = 'O';
            if (grid[y][x]) {
                switch (grid[y][x]->getType()) {
                case UnitType::Infantry: c = 'I'; break;
                case UnitType::Archer:   c = 'A'; break;
                case UnitType::Cavalry:  c = 'C'; break;
                case UnitType::Worker:   c = 'W'; break;
                case UnitType::Scout:    c = 'S'; break;
                case UnitType::Tank:     c = 'T'; break;
                case UnitType::Medic:    c = 'M'; break;
                }
            }
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\nЛегенда: . равнина, * лес, ~ болото, ^ горы, O объект, B здание\n"
        << "         I пехотинец, A лучник, C кавалерия, W рабочий, S разведчик, T танк, M медик\n";
}
