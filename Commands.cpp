#include "Commands.h"
#include "GameFacade.h"

static std::string xy(int x, int y) { return std::to_string(x) + "," + std::to_string(y); }

bool CreateUnitCommand::execute() { return facade.createUnit(type, x, y); }
std::string CreateUnitCommand::describe() const { return "create unit " + type + " at " + xy(x, y); }

bool MoveUnitCommand::execute() { return facade.moveUnit(unitId, x, y); }
std::string MoveUnitCommand::describe() const { return "move unit " + std::to_string(unitId) + " to " + xy(x, y); }

bool AttackCommand::execute() { return facade.attack(attackerId, targetId); }
std::string AttackCommand::describe() const {
    return "attack unit " + std::to_string(attackerId) + " target " + std::to_string(targetId);
}

bool CollectResourceCommand::execute() { return facade.collectResource(unitId, resource); }
std::string CollectResourceCommand::describe() const {
    return "collect unit " + std::to_string(unitId) + " from " + resource;
}

bool HealCommand::execute() { return facade.heal(medicId, targetId); }
std::string HealCommand::describe() const {
    return "heal unit " + std::to_string(medicId) + " target " + std::to_string(targetId);
}

bool BuildBuildingCommand::execute() { return facade.buildBuilding(type, x, y); }
std::string BuildBuildingCommand::describe() const { return "build " + type + " at " + xy(x, y); }

bool ResearchCommand::execute() { return facade.research(tech); }
std::string ResearchCommand::describe() const { return "research " + tech; }

bool DistributeResourcesCommand::execute() { return facade.distributeResources(from, to, amount); }
std::string DistributeResourcesCommand::describe() const {
    return "distribute " + from + " to " + to + " " + std::to_string(amount);
}

bool ProduceUnitCommand::execute() { return facade.produceUnit(unitType, buildingId); }
std::string ProduceUnitCommand::describe() const {
    return "produce " + unitType + " in " + std::to_string(buildingId);
}

bool StatsCommand::execute() { return facade.getStats(sub); }
std::string StatsCommand::describe() const { return sub.empty() ? "stats" : "stats " + sub; }

bool InteractCommand::execute() { return facade.interact(unitId, x, y); }
std::string InteractCommand::describe() const {
    return "interact unit " + std::to_string(unitId) + " at " + xy(x, y);
}

bool PlaceTerrainCommand::execute() { return facade.placeTerrain(type, x, y); }
std::string PlaceTerrainCommand::describe() const { return "terrain " + type + " at " + xy(x, y); }

bool PlaceObjectCommand::execute() { return facade.placeObject(type, x, y); }
std::string PlaceObjectCommand::describe() const { return "object " + type + " at " + xy(x, y); }

bool ShowMapCommand::execute() { return facade.showMap(); }
std::string ShowMapCommand::describe() const { return "map"; }

bool HelpCommand::execute() { return facade.help(); }
std::string HelpCommand::describe() const { return "help"; }
