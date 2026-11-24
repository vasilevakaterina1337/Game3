#pragma once
#include <string>
#include <utility>

class GameFacade;

// Команда: объект с параметрами действия. Выполняется через прикладной вход (GameFacade),
// возвращает true при успехе. Бизнес-логики и разбора строк внутри команд нет.
class Command {
public:
    virtual ~Command() = default;
    virtual bool execute() = 0;
    virtual std::string describe() const = 0;   // для журнала действий пользователя
};

class CreateUnitCommand : public Command {
    GameFacade& facade; std::string type; int x, y;
public:
    CreateUnitCommand(GameFacade& f, std::string t, int x, int y) : facade(f), type(std::move(t)), x(x), y(y) {}
    bool execute() override; std::string describe() const override;
};

class MoveUnitCommand : public Command {
    GameFacade& facade; int unitId, x, y;
public:
    MoveUnitCommand(GameFacade& f, int id, int x, int y) : facade(f), unitId(id), x(x), y(y) {}
    bool execute() override; std::string describe() const override;
};

class AttackCommand : public Command {
    GameFacade& facade; int attackerId, targetId;
public:
    AttackCommand(GameFacade& f, int atk, int tgt) : facade(f), attackerId(atk), targetId(tgt) {}
    bool execute() override; std::string describe() const override;
};

class CollectResourceCommand : public Command {
    GameFacade& facade; int unitId; std::string resource;
public:
    CollectResourceCommand(GameFacade& f, int u, std::string r) : facade(f), unitId(u), resource(std::move(r)) {}
    bool execute() override; std::string describe() const override;
};

class HealCommand : public Command {
    GameFacade& facade; int medicId, targetId;
public:
    HealCommand(GameFacade& f, int m, int t) : facade(f), medicId(m), targetId(t) {}
    bool execute() override; std::string describe() const override;
};

class BuildBuildingCommand : public Command {
    GameFacade& facade; std::string type; int x, y;
public:
    BuildBuildingCommand(GameFacade& f, std::string t, int x, int y) : facade(f), type(std::move(t)), x(x), y(y) {}
    bool execute() override; std::string describe() const override;
};

class ResearchCommand : public Command {
    GameFacade& facade; std::string tech;
public:
    ResearchCommand(GameFacade& f, std::string t) : facade(f), tech(std::move(t)) {}
    bool execute() override; std::string describe() const override;
};

class DistributeResourcesCommand : public Command {
    GameFacade& facade; std::string from, to; int amount;
public:
    DistributeResourcesCommand(GameFacade& f, std::string from, std::string to, int a)
        : facade(f), from(std::move(from)), to(std::move(to)), amount(a) {}
    bool execute() override; std::string describe() const override;
};

class ProduceUnitCommand : public Command {
    GameFacade& facade; std::string unitType; int buildingId;
public:
    ProduceUnitCommand(GameFacade& f, std::string ut, int bid) : facade(f), unitType(std::move(ut)), buildingId(bid) {}
    bool execute() override; std::string describe() const override;
};

class StatsCommand : public Command {
    GameFacade& facade; std::string sub;
public:
    StatsCommand(GameFacade& f, std::string s = "") : facade(f), sub(std::move(s)) {}
    bool execute() override; std::string describe() const override;
};

class InteractCommand : public Command {
    GameFacade& facade; int unitId, x, y;
public:
    InteractCommand(GameFacade& f, int id, int x, int y) : facade(f), unitId(id), x(x), y(y) {}
    bool execute() override; std::string describe() const override;
};

class PlaceTerrainCommand : public Command {
    GameFacade& facade; std::string type; int x, y;
public:
    PlaceTerrainCommand(GameFacade& f, std::string t, int x, int y) : facade(f), type(std::move(t)), x(x), y(y) {}
    bool execute() override; std::string describe() const override;
};

class PlaceObjectCommand : public Command {
    GameFacade& facade; std::string type; int x, y;
public:
    PlaceObjectCommand(GameFacade& f, std::string t, int x, int y) : facade(f), type(std::move(t)), x(x), y(y) {}
    bool execute() override; std::string describe() const override;
};

class ShowMapCommand : public Command {
    GameFacade& facade;
public:
    explicit ShowMapCommand(GameFacade& f) : facade(f) {}
    bool execute() override; std::string describe() const override;
};

class HelpCommand : public Command {
    GameFacade& facade;
public:
    explicit HelpCommand(GameFacade& f) : facade(f) {}
    bool execute() override; std::string describe() const override;
};
