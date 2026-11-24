#include "GameFacade.h"
#include <iostream>
#include <vector>

GameFacade::GameFacade(GameField& field, std::shared_ptr<IObservable> bus, std::shared_ptr<Base> b)
    : base(std::move(b)) {
    mediator = std::make_shared<GameMediator>(field, bus, *base);
    bus->subscribe(base.get());   // База наблюдает за событиями юнитов (единый учёт)

    std::vector<std::shared_ptr<Handler>> handlers = {
        std::make_shared<CreateHandler>(*this),
        std::make_shared<MoveHandler>(*this),
        std::make_shared<AttackHandler>(*this),
        std::make_shared<CollectHandler>(*this),
        std::make_shared<HealHandler>(*this),
        std::make_shared<BuildHandler>(*this),
        std::make_shared<ResearchHandler>(*this),
        std::make_shared<ProduceHandler>(*this),
        std::make_shared<DistributeHandler>(*this),
        std::make_shared<StatsHandler>(*this),
        std::make_shared<InteractHandler>(*this),
        std::make_shared<TerrainHandler>(*this),
        std::make_shared<ObjectHandler>(*this),
        std::make_shared<MapHandler>(*this),
        std::make_shared<HelpHandler>(*this),
    };
    for (size_t i = 0; i + 1 < handlers.size(); ++i) handlers[i]->setNext(handlers[i + 1]);
    chain = handlers.front();
}

void GameFacade::processInput(const std::string& input) {
    if (input.find_first_not_of(" \t\r\n") == std::string::npos) return;   // пустая строка
    if (!chain->handle(input))
        std::cout << "[Ошибка] Неизвестная команда: " << input << " (введите help)\n";
}

bool GameFacade::executeCommand(const std::shared_ptr<Command>& cmd) {
    std::cout << "[Команда] " << cmd->describe() << "\n";
    bool ok = cmd->execute();
    std::cout << (ok ? "[Результат] выполнено\n" : "[Результат] отклонено, состояние не изменено\n");
    if (ok) history.push(cmd);
    return ok;
}

bool GameFacade::createUnit(const std::string& type, int x, int y) { return mediator->createUnit(type, x, y); }
bool GameFacade::moveUnit(int unitId, int x, int y) { return mediator->moveUnit(unitId, x, y); }
bool GameFacade::attack(int a, int t) { return mediator->attack(a, t); }
bool GameFacade::collectResource(int u, const std::string& r) { return mediator->collectResource(u, r); }
bool GameFacade::heal(int m, int t) { return mediator->healUnit(m, t); }
bool GameFacade::buildBuilding(const std::string& t, int x, int y) { return mediator->buildBuilding(t, x, y); }
bool GameFacade::research(const std::string& t) { return mediator->startResearch(t); }
bool GameFacade::distributeResources(const std::string& f, const std::string& t, int a) {
    return mediator->distributeResources(f, t, a);
}
bool GameFacade::produceUnit(const std::string& ut, int bid) { return mediator->produceUnit(ut, bid); }
bool GameFacade::getStats(const std::string& sub) { return mediator->getStats(sub); }
bool GameFacade::interact(int id, int x, int y) { return mediator->interactWithObject(id, x, y); }
bool GameFacade::placeTerrain(const std::string& t, int x, int y) { return mediator->setTerrain(t, x, y); }
bool GameFacade::placeObject(const std::string& t, int x, int y) { return mediator->setObject(t, x, y); }
bool GameFacade::showMap() { mediator->showMap(); return true; }

bool GameFacade::help() {
    std::cout <<
        "Команды (регистр не важен):\n"
        "  create unit <тип> at <x>,<y>         создать юнита (бесплатно); типы: infantry archer cavalry worker scout tank medic\n"
        "  move unit <id> to <x>,<y>            переместить\n"
        "  attack unit <id> target <id>         атаковать (урон = max(0, атака - броня))\n"
        "  collect unit <id> from <ресурс>      только worker, +50; ресурс: 1|wood 2|metal 3|energy\n"
        "  heal unit <id> target <id>           только medic, +30 HP, не выше максимума\n"
        "  build <тип> at <x>,<y>               barracks lab warehouse powerplant hospital\n"
        "  research <технология>                attack_upgrade (нужен barracks), armor_upgrade (нужна lab)\n"
        "  produce <тип юнита> in <id здания>   только barracks, тратит ресурсы\n"
        "  distribute <ресурс> to <здание> <n>  здание — тип или id\n"
        "  stats [units|buildings|resources|research]\n"
        "  interact unit <id> at <x>,<y>        юнит должен стоять на клетке с объектом\n"
        "  terrain <plains|forest|swamp|mountains> at <x>,<y>\n"
        "  object <chest|trap|altar> at <x>,<y>\n"
        "  map | help | exit\n";
    return true;
}
