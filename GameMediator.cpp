#include "GameMediator.h"
#include "Factories.h"
#include <iostream>
#include <vector>

namespace {
    constexpr int kCollectAmount = 50;
    constexpr int kHealAmount = 30;
    constexpr int kAttackUpgradeBonus = 5;
    constexpr int kArmorUpgradeBonus = 3;

    void err(const std::string& m) { std::cout << "[Ошибка] " << m << "\n"; }
    void info(const std::string& m) { std::cout << "[Медиатор] " << m << "\n"; }
    std::string at(int x, int y) { return "(" + std::to_string(x) + "," + std::to_string(y) + ")"; }

    std::string join(const std::vector<std::string>& v) {
        std::string s;
        for (const auto& item : v) { if (!s.empty()) s += ", "; s += item; }
        return s;
    }

    std::string costText(const Cost& c) {
        std::string s;
        auto part = [&](const char* n, int v) {
            if (v > 0) { if (!s.empty()) s += " "; s += std::string(n) + "=" + std::to_string(v); }
        };
        part("wood", c.wood); part("metal", c.metal); part("energy", c.energy);
        return s.empty() ? "бесплатно" : s;
    }

    std::string placementError(PlaceResult r) {
        switch (r) {
        case PlaceResult::OutOfBounds: return "координаты вне карты";
        case PlaceResult::Occupied:    return "клетка занята";
        case PlaceResult::Impassable:  return "клетка непроходима для этого юнита";
        case PlaceResult::NotOnField:  return "юнит не найден на поле";
        default: return "";
        }
    }

    struct TechInfo {
        std::string name;
        Cost cost;
        std::string building;      // здание, в котором ведётся исследование
        std::string description;
    };

    const std::vector<TechInfo>& techTable() {
        static const std::vector<TechInfo> table = {
            { "attack_upgrade", Cost{ 0, 100, 0 }, "barracks", "+5 к атаке всех юнитов" },
            { "armor_upgrade",  Cost{ 50, 100, 0 }, "lab",     "+3 к броне всех юнитов" },
        };
        return table;
    }

    const TechInfo* findTech(const std::string& name) {
        for (const auto& t : techTable()) if (t.name == name) return &t;
        return nullptr;
    }
}

GameMediator::GameMediator(GameField& f, std::shared_ptr<IObservable> bus, Base& b)
    : field(f), base(b), eventBus(std::move(bus)), resources(b) {
}

// Бонусы вычисляются из множества изученных технологий, а не накапливаются: повторный показ
// статистики или повторное исследование ничего не усиливают.
int GameMediator::attackBonus() const { return researched.count("attack_upgrade") ? kAttackUpgradeBonus : 0; }
int GameMediator::armorBonus() const { return researched.count("armor_upgrade") ? kArmorUpgradeBonus : 0; }

// ---------------------------------------------------------------- юниты

bool GameMediator::createUnit(const std::string& type, int x, int y) {
    auto factory = createFactory(type);
    if (!factory) {
        err("Неизвестный тип юнита '" + type + "'. Доступно: " + join(unitTypeNames()));
        return false;
    }
    if (!base.canAddUnit()) {
        err("Лимит юнитов базы исчерпан (" + std::to_string(base.getUnitCount()) + "/"
            + std::to_string(base.getMaxUnits()) + ")");
        return false;
    }
    std::shared_ptr<Unit> u(factory->createUnit(nextUnitId).release());
    PlaceResult r = field.checkPlacement(*u, x, y);
    if (r != PlaceResult::Ok) {
        err("Невозможно создать юнита на " + at(x, y) + ": " + placementError(r));
        return false;
    }
    field.addUnit(u, x, y);
    base.addUnit(u);
    ++nextUnitId;
    eventBus->notify({ UnitEvent::Created, u->getId() });
    info("Создан " + u->getName() + " id=" + std::to_string(u->getId()) + " на " + at(x, y));
    return true;
}

bool GameMediator::moveUnit(int unitId, int x, int y) {
    auto u = base.findUnit(unitId);
    if (!u) { err("Юнит с id=" + std::to_string(unitId) + " не найден"); return false; }
    if (u->getX() == x && u->getY() == y) {
        err("Юнит " + std::to_string(unitId) + " уже находится на " + at(x, y));
        return false;
    }
    PlaceResult r = field.moveUnit(u, x, y);   // атомарно: при отказе ничего не меняется
    if (r != PlaceResult::Ok) {
        err("Невозможно переместить юнита " + std::to_string(unitId) + " на " + at(x, y) + ": " + placementError(r));
        return false;
    }
    u->setOrder("move to " + std::to_string(x) + "," + std::to_string(y));
    info("Юнит " + std::to_string(unitId) + " перемещён на " + at(x, y));
    return true;
}

bool GameMediator::attack(int attackerId, int targetId) {
    if (attackerId == targetId) { err("Юнит не может атаковать самого себя"); return false; }
    auto atk = base.findUnit(attackerId);
    auto tgt = base.findUnit(targetId);
    if (!atk) { err("Атакующий с id=" + std::to_string(attackerId) + " не найден"); return false; }
    if (!tgt) { err("Цель с id=" + std::to_string(targetId) + " не найдена"); return false; }

    int dmg = combat.calculateDamage(*atk, *tgt, attackBonus(), armorBonus());
    int hpBefore = tgt->getHealth();
    atk->setOrder("attack unit " + std::to_string(targetId));
    tgt->takeDamage(dmg);
    eventBus->notify({ UnitEvent::Damaged, targetId });
    info(atk->getName() + " #" + std::to_string(attackerId) + " атакует " + tgt->getName() + " #"
        + std::to_string(targetId) + ": урон " + std::to_string(dmg) + " (HP "
        + std::to_string(hpBefore) + " -> " + std::to_string(tgt->getHealth()) + ")");

    if (!tgt->isAlive()) {
        field.removeUnit(tgt);
        eventBus->notify({ UnitEvent::Destroyed, targetId });   // База освобождает лимит
        info(tgt->getName() + " #" + std::to_string(targetId) + " уничтожен");
    }
    return true;
}

bool GameMediator::collectResource(int unitId, const std::string& resource) {
    std::string res = ResourceSystem::normalize(resource);
    if (res.empty()) {
        err("Неизвестный ресурс '" + resource + "'. Доступно: 1|wood, 2|metal, 3|energy");
        return false;
    }
    auto u = base.findUnit(unitId);
    if (!u) { err("Юнит с id=" + std::to_string(unitId) + " не найден"); return false; }
    if (u->getType() != UnitType::Worker) {
        err("Собирать ресурсы может только Рабочий (юнит " + std::to_string(unitId) + " — " + u->getName() + ")");
        return false;
    }
    resources.add(res, kCollectAmount);
    u->setOrder("collect " + res);
    info("Рабочий #" + std::to_string(unitId) + " собрал " + std::to_string(kCollectAmount) + " " + res
        + " (всего " + res + ": " + std::to_string(resources.get(res)) + ")");
    return true;
}

bool GameMediator::healUnit(int medicId, int targetId) {
    auto m = base.findUnit(medicId);
    if (!m) { err("Юнит с id=" + std::to_string(medicId) + " не найден"); return false; }
    if (m->getType() != UnitType::Medic) {
        err("Лечить может только Медик (юнит " + std::to_string(medicId) + " — " + m->getName() + ")");
        return false;
    }
    auto t = base.findUnit(targetId);
    if (!t) { err("Цель с id=" + std::to_string(targetId) + " не найдена"); return false; }
    if (t->getHealth() >= t->getMaxHealth()) {
        err(t->getName() + " #" + std::to_string(targetId) + " уже имеет полное здоровье ("
            + std::to_string(t->getMaxHealth()) + ")");
        return false;
    }
    int before = t->getHealth();
    t->heal(kHealAmount);   // лечение ограничено максимумом внутри Health
    m->setOrder("heal unit " + std::to_string(targetId));
    info("Медик #" + std::to_string(medicId) + " вылечил " + t->getName() + " #" + std::to_string(targetId)
        + ": +" + std::to_string(t->getHealth() - before) + " HP (" + std::to_string(t->getHealth())
        + "/" + std::to_string(t->getMaxHealth()) + ")");
    return true;
}

bool GameMediator::interactWithObject(int unitId, int x, int y) {
    auto u = base.findUnit(unitId);
    if (!u) { err("Юнит с id=" + std::to_string(unitId) + " не найден"); return false; }
    auto obj = field.getObject(x, y);
    if (!obj) { err("На клетке " + at(x, y) + " нет нейтрального объекта"); return false; }
    if (u->getX() != x || u->getY() != y) {
        err("Юнит " + std::to_string(unitId) + " стоит на " + at(u->getX(), u->getY())
            + ", а объект на " + at(x, y) + ": подойдите к объекту (move)");
        return false;
    }
    *obj + *u;
    return true;
}

// ---------------------------------------------------------------- база

bool GameMediator::buildBuilding(const std::string& type, int x, int y) {
    auto b = createBuilding(type, x, y);
    if (!b) {
        err("Неизвестный тип здания '" + type + "'. Доступно: " + join(buildingTypeNames()));
        return false;
    }
    std::string why = field.canBuildAt(x, y);
    if (!why.empty()) { err("Нельзя строить на " + at(x, y) + ": " + why); return false; }

    Cost cost = buildingCost(type);
    if (!resources.canAfford(cost)) {
        err("Недостаточно ресурсов для " + type + " (" + costText(cost) + "): " + resources.describeMissing(cost));
        return false;
    }

    // Все проверки пройдены — теперь согласованное изменение.
    resources.spend(cost);
    b->setId(nextBuildingId++);
    base.addBuilding(b);
    field.placeBuilding(b, x, y);
    info("Построено " + b->displayName() + " (" + type + ") id=" + std::to_string(b->getId())
        + " на " + at(x, y) + ", стоимость: " + costText(cost));
    return true;
}

bool GameMediator::startResearch(const std::string& tech) {
    const TechInfo* t = findTech(tech);
    if (!t) {
        std::vector<std::string> names;
        for (const auto& item : techTable()) names.push_back(item.name);
        err("Неизвестная технология '" + tech + "'. Доступно: " + join(names));
        return false;
    }
    if (researched.count(t->name)) { err("Технология " + t->name + " уже изучена"); return false; }
    if (!base.findBuildingByType(t->building)) {
        err("Для " + t->name + " нужно здание " + t->building + " (постройте его командой build)");
        return false;
    }
    if (!resources.canAfford(t->cost)) {
        err("Недостаточно ресурсов для " + t->name + " (" + costText(t->cost) + "): " + resources.describeMissing(t->cost));
        return false;
    }
    resources.spend(t->cost);
    researched.insert(t->name);
    info("Исследовано " + t->name + ": " + t->description + ", стоимость: " + costText(t->cost));
    return true;
}

bool GameMediator::distributeResources(const std::string& from, const std::string& to, int amount) {
    std::string res = ResourceSystem::normalize(from);
    if (res.empty()) { err("Неизвестный ресурс '" + from + "'. Доступно: wood, metal, energy"); return false; }
    if (amount <= 0) { err("Количество ресурса должно быть положительным (получено " + std::to_string(amount) + ")"); return false; }

    // получатель — id здания или тип здания (берётся первое построенное)
    std::shared_ptr<Building> target;
    bool numeric = !to.empty() && to.find_first_not_of("0123456789") == std::string::npos && to.size() < 9;
    if (numeric) target = base.findBuilding(std::stoi(to));
    else target = base.findBuildingByType(to);
    if (!target) { err("Здание '" + to + "' не найдено (укажите id или тип построенного здания)"); return false; }

    Cost cost;
    if (res == "wood") cost.wood = amount;
    else if (res == "metal") cost.metal = amount;
    else cost.energy = amount;
    if (!resources.canAfford(cost)) {
        err("Недостаточно ресурсов: " + resources.describeMissing(cost));
        return false;
    }
    resources.spend(cost);
    target->receive(res, amount);   // ресурс не исчезает, а попадает на склад здания
    info("Передано " + std::to_string(amount) + " " + res + " -> " + target->getType() + " #"
        + std::to_string(target->getId()) + " (на складе здания: " + std::to_string(target->getStock().at(res)) + ")");
    return true;
}

bool GameMediator::produceUnit(const std::string& unitType, int buildingId) {
    auto factory = createFactory(unitType);
    if (!factory) {
        err("Неизвестный тип юнита '" + unitType + "'. Доступно: " + join(unitTypeNames()));
        return false;
    }
    auto b = base.findBuilding(buildingId);
    if (!b) { err("Здание с id=" + std::to_string(buildingId) + " не найдено (список: stats buildings)"); return false; }
    if (!b->canProduce()) {
        err("Здание " + b->getType() + " #" + std::to_string(buildingId) + " не производит юнитов (нужен barracks)");
        return false;
    }
    if (!base.canAddUnit()) {
        err("Лимит юнитов базы исчерпан (" + std::to_string(base.getUnitCount()) + "/"
            + std::to_string(base.getMaxUnits()) + ")");
        return false;
    }
    Cost cost = unitCost(unitType);
    if (!resources.canAfford(cost)) {
        err("Недостаточно ресурсов для " + unitType + " (" + costText(cost) + "): " + resources.describeMissing(cost));
        return false;
    }
    std::shared_ptr<Unit> u(factory->createUnit(nextUnitId).release());
    int sx = 0, sy = 0;
    if (!field.findFreeNear(*u, b->getX(), b->getY(), sx, sy)) {
        err("Нет свободной клетки рядом со зданием #" + std::to_string(buildingId));
        return false;
    }

    resources.spend(cost);
    field.addUnit(u, sx, sy);
    base.addUnit(u);
    ++nextUnitId;
    eventBus->notify({ UnitEvent::Created, u->getId() });
    info(b->displayName() + " #" + std::to_string(buildingId) + " произвёл " + u->getName() + " id="
        + std::to_string(u->getId()) + " на " + at(sx, sy) + ", стоимость: " + costText(cost));
    return true;
}

// ---------------------------------------------------------------- окружение

bool GameMediator::setTerrain(const std::string& type, int x, int y) {
    std::shared_ptr<Terrain> t;
    if (type == "plains") t = std::make_shared<Plains>();
    else if (type == "forest") t = std::make_shared<Forest>();
    else if (type == "swamp") t = std::make_shared<Swamp>();
    else if (type == "mountains") t = std::make_shared<Mountains>();
    else { err("Неизвестный ландшафт '" + type + "'. Доступно: plains, forest, swamp, mountains"); return false; }
    std::string why = field.setTerrain(x, y, t);
    if (!why.empty()) { err("Ландшафт не установлен на " + at(x, y) + ": " + why); return false; }
    info("Ландшафт " + type + " установлен на " + at(x, y));
    return true;
}

bool GameMediator::setObject(const std::string& type, int x, int y) {
    std::shared_ptr<NeutralObject> o;
    if (type == "chest") o = std::make_shared<TreasureChest>();
    else if (type == "trap") o = std::make_shared<Trap>();
    else if (type == "altar") o = std::make_shared<Altar>();
    else { err("Неизвестный объект '" + type + "'. Доступно: chest, trap, altar"); return false; }
    std::string why = field.setObject(x, y, o);
    if (!why.empty()) { err("Объект не установлен на " + at(x, y) + ": " + why); return false; }
    info("Объект " + type + " установлен на " + at(x, y));
    return true;
}

// ---------------------------------------------------------------- просмотр

void GameMediator::printUnits() {
    std::cout << "Юниты (" << base.getUnitCount() << "/" << base.getMaxUnits() << "):\n";
    int ab = attackBonus(), rb = armorBonus();
    for (auto u : field) {   // обход поля итератором: поле — источник истины о позициях
        std::cout << "  #" << u->getId() << " " << u->getName()
            << " HP=" << u->getHealth() << "/" << u->getMaxHealth()
            << " ATK=" << u->getAttack() << (ab ? "(+" + std::to_string(ab) + ")" : "")
            << " ARM=" << u->getArmor() << (rb ? "(+" + std::to_string(rb) + ")" : "")
            << " pos=" << at(u->getX(), u->getY())
            << " order=" << u->getOrder() << "\n";
    }
}

void GameMediator::printBuildings() const {
    std::cout << "Здания (" << base.getBuildings().size() << "):\n";
    for (const auto& b : base.getBuildings()) {
        std::cout << "  #" << b->getId() << " " << b->getType() << " pos=" << at(b->getX(), b->getY())
            << " готовность=" << b->getProgress() << "%";
        if (!b->getStock().empty()) {
            std::cout << " склад:";
            for (const auto& kv : b->getStock()) std::cout << " " << kv.first << "=" << kv.second;
        }
        std::cout << "\n";
    }
}

void GameMediator::printResources() const {
    std::cout << "Ресурсы: wood=" << resources.get("wood") << " metal=" << resources.get("metal")
        << " energy=" << resources.get("energy") << "\n";
}

void GameMediator::printResearch() const {
    std::cout << "Исследования:\n";
    for (const auto& t : techTable()) {
        std::cout << "  " << t.name << " (" << t.description << ") — ";
        if (researched.count(t.name)) std::cout << "изучено\n";
        else std::cout << "доступно в " << t.building << ", стоимость: " << costText(t.cost) << "\n";
    }
}

bool GameMediator::getStats(const std::string& sub) {
    if (sub.empty() || sub == "all") {
        printResources(); printUnits(); printBuildings(); printResearch();
    }
    else if (sub == "units") printUnits();
    else if (sub == "buildings") printBuildings();
    else if (sub == "resources") printResources();
    else if (sub == "research") printResearch();
    else {
        err("Неизвестный раздел статистики '" + sub + "'. Доступно: units, buildings, resources, research");
        return false;
    }
    return true;
}

void GameMediator::showMap() { field.printMap(); }
