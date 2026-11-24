#pragma once
#include <memory>
#include <string>
#include "GameMediator.h"
#include "CommandHistory.h"
#include "Commands.h"
#include "Handlers.h"

// Фасад — единый прикладной вход. Все пользовательские действия идут по пути:
//   строка -> цепочка обработчиков -> объект команды -> executeCommand -> метод фасада -> посредник.
// Внутренние объекты (посредник, поле, база) наружу не выдаются.
class GameFacade {
    std::shared_ptr<Base> base;                 // держит базу живой для подписки на события
    std::shared_ptr<GameMediator> mediator;
    std::shared_ptr<Handler> chain;
    CommandHistory history;

public:
    GameFacade(GameField& field, std::shared_ptr<IObservable> bus, std::shared_ptr<Base> b);
    GameFacade(const GameFacade&) = delete;               // обработчики хранят ссылку на фасад
    GameFacade& operator=(const GameFacade&) = delete;

    // Разбор строки: цепочка обработчиков. Неизвестная команда — сообщение, состояние не меняется.
    void processInput(const std::string& input);

    // Выполнение команды: журнал действия и результата; в историю попадают только успешные команды.
    bool executeCommand(const std::shared_ptr<Command>& cmd);

    // Прикладные операции (получатели команд)
    bool createUnit(const std::string& type, int x, int y);
    bool moveUnit(int unitId, int x, int y);
    bool attack(int attackerId, int targetId);
    bool collectResource(int unitId, const std::string& resource);
    bool heal(int medicId, int targetId);
    bool buildBuilding(const std::string& type, int x, int y);
    bool research(const std::string& tech);
    bool distributeResources(const std::string& from, const std::string& to, int amount);
    bool produceUnit(const std::string& unitType, int buildingId);
    bool getStats(const std::string& sub = "");
    bool interact(int unitId, int x, int y);
    bool placeTerrain(const std::string& type, int x, int y);
    bool placeObject(const std::string& type, int x, int y);
    bool showMap();
    bool help();

    size_t historySize() const { return history.size(); }
};
