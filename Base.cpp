#include "Base.h"

void Base::onUnitEvent(const UnitEvent& event) {
    if (event.type == UnitEvent::Destroyed) {
        auto before = units.size();
        units.erase(std::remove_if(units.begin(), units.end(),
            [&](const std::shared_ptr<Unit>& u) { return u->getId() == event.unitId; }), units.end());
        if (units.size() != before)   // повторное событие ничего не освобождает второй раз
            std::cout << "[База " << id << "] Юнит " << event.unitId
            << " уничтожен, лимит освобождён (" << units.size() << "/" << maxUnits << ")\n";
    }
    else if (event.type == UnitEvent::Created) {
        std::cout << "[База " << id << "] Юнит " << event.unitId << " принят на учёт ("
            << units.size() << "/" << maxUnits << ")\n";
    }
}

bool Base::addUnit(std::shared_ptr<Unit> unit) {
    if (!unit || !canAddUnit()) return false;
    units.push_back(std::move(unit));
    return true;
}

std::shared_ptr<Unit> Base::findUnit(int unitId) const {
    for (const auto& u : units)
        if (u->getId() == unitId) return u;
    return nullptr;
}

std::shared_ptr<Building> Base::findBuilding(int buildingId) const {
    for (const auto& b : buildings)
        if (b->getId() == buildingId) return b;
    return nullptr;
}

std::shared_ptr<Building> Base::findBuildingByType(const std::string& type) const {
    for (const auto& b : buildings)
        if (b->getType() == type) return b;
    return nullptr;
}
