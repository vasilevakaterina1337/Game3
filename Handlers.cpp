#include "Handlers.h"
#include "GameFacade.h"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <iostream>
#include <sstream>

namespace {
    std::vector<std::string> tokenize(const std::string& line) {
        std::istringstream ss(line);
        std::vector<std::string> out;
        std::string w;
        while (ss >> w) {
            std::transform(w.begin(), w.end(), w.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            out.push_back(w);
        }
        return out;
    }

    // Строгий разбор целого: вся строка должна быть числом ("2x" и "" не принимаются).
    bool toInt(const std::string& s, int& out) {
        if (s.empty()) return false;
        auto res = std::from_chars(s.data(), s.data() + s.size(), out);
        return res.ec == std::errc() && res.ptr == s.data() + s.size();
    }

    // "x,y" -> два целых
    bool toPos(const std::string& s, int& x, int& y) {
        size_t c = s.find(',');
        if (c == std::string::npos || s.find(',', c + 1) != std::string::npos) return false;
        return toInt(s.substr(0, c), x) && toInt(s.substr(c + 1), y);
    }

    const char* kBadPos = "координаты должны быть вида x,y (целые числа, без пробелов)";
    const char* kBadId = "id должен быть целым числом";
}

bool CommandHandler::handle(const std::string& cmd) {
    auto tokens = tokenize(cmd);
    if (tokens.empty() || tokens[0] != keyword_) return passOn(cmd);
    tokens.erase(tokens.begin());
    process(tokens);
    return true;
}

void CommandHandler::badArgs(const std::string& format, const std::string& detail) const {
    std::cout << "[Ошибка] Неверные аргументы команды '" << keyword_ << "'. Формат: " << format;
    if (!detail.empty()) std::cout << " (" << detail << ")";
    std::cout << "\n";
}

void CreateHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "create unit <тип> at <x>,<y>";
    int x, y;
    if (a.size() != 4 || a[0] != "unit" || a[2] != "at") return badArgs(fmt);
    if (!toPos(a[3], x, y)) return badArgs(fmt, kBadPos);
    facade.executeCommand(std::make_shared<CreateUnitCommand>(facade, a[1], x, y));
}

void MoveHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "move unit <id> to <x>,<y>";
    int id, x, y;
    if (a.size() != 4 || a[0] != "unit" || a[2] != "to") return badArgs(fmt);
    if (!toInt(a[1], id)) return badArgs(fmt, kBadId);
    if (!toPos(a[3], x, y)) return badArgs(fmt, kBadPos);
    facade.executeCommand(std::make_shared<MoveUnitCommand>(facade, id, x, y));
}

void AttackHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "attack unit <id> target <id>";
    int atk, tgt;
    if (a.size() != 4 || a[0] != "unit" || a[2] != "target") return badArgs(fmt);
    if (!toInt(a[1], atk) || !toInt(a[3], tgt)) return badArgs(fmt, kBadId);
    facade.executeCommand(std::make_shared<AttackCommand>(facade, atk, tgt));
}

void CollectHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "collect unit <id> from <ресурс: 1|wood, 2|metal, 3|energy>";
    int id;
    if (a.size() != 4 || a[0] != "unit" || a[2] != "from") return badArgs(fmt);
    if (!toInt(a[1], id)) return badArgs(fmt, kBadId);
    facade.executeCommand(std::make_shared<CollectResourceCommand>(facade, id, a[3]));
}

void HealHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "heal unit <id медика> target <id цели>";
    int medic, tgt;
    if (a.size() != 4 || a[0] != "unit" || a[2] != "target") return badArgs(fmt);
    if (!toInt(a[1], medic) || !toInt(a[3], tgt)) return badArgs(fmt, kBadId);
    facade.executeCommand(std::make_shared<HealCommand>(facade, medic, tgt));
}

void BuildHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "build <тип здания> at <x>,<y>";
    int x, y;
    if (a.size() != 3 || a[1] != "at") return badArgs(fmt);
    if (!toPos(a[2], x, y)) return badArgs(fmt, kBadPos);
    facade.executeCommand(std::make_shared<BuildBuildingCommand>(facade, a[0], x, y));
}

void ResearchHandler::process(const std::vector<std::string>& a) {
    if (a.size() != 1) return badArgs("research <технология>");
    facade.executeCommand(std::make_shared<ResearchCommand>(facade, a[0]));
}

void ProduceHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "produce <тип юнита> in <id здания>";
    int bid;
    if (a.size() != 3 || a[1] != "in") return badArgs(fmt);
    if (!toInt(a[2], bid)) return badArgs(fmt, "id здания должен быть целым числом");
    facade.executeCommand(std::make_shared<ProduceUnitCommand>(facade, a[0], bid));
}

void DistributeHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "distribute <ресурс> to <тип или id здания> <количество>";
    int amount;
    if (a.size() != 4 || a[1] != "to") return badArgs(fmt);
    if (!toInt(a[3], amount)) return badArgs(fmt, "количество должно быть целым числом");
    facade.executeCommand(std::make_shared<DistributeResourcesCommand>(facade, a[0], a[2], amount));
}

void StatsHandler::process(const std::vector<std::string>& a) {
    if (a.size() > 1) return badArgs("stats [units|buildings|resources|research]");
    facade.executeCommand(std::make_shared<StatsCommand>(facade, a.empty() ? "" : a[0]));
}

void InteractHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "interact unit <id> at <x>,<y>";
    int id, x, y;
    if (a.size() != 4 || a[0] != "unit" || a[2] != "at") return badArgs(fmt);
    if (!toInt(a[1], id)) return badArgs(fmt, kBadId);
    if (!toPos(a[3], x, y)) return badArgs(fmt, kBadPos);
    facade.executeCommand(std::make_shared<InteractCommand>(facade, id, x, y));
}

void TerrainHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "terrain <plains|forest|swamp|mountains> at <x>,<y>";
    int x, y;
    if (a.size() != 3 || a[1] != "at") return badArgs(fmt);
    if (!toPos(a[2], x, y)) return badArgs(fmt, kBadPos);
    facade.executeCommand(std::make_shared<PlaceTerrainCommand>(facade, a[0], x, y));
}

void ObjectHandler::process(const std::vector<std::string>& a) {
    const char* fmt = "object <chest|trap|altar> at <x>,<y>";
    int x, y;
    if (a.size() != 3 || a[1] != "at") return badArgs(fmt);
    if (!toPos(a[2], x, y)) return badArgs(fmt, kBadPos);
    facade.executeCommand(std::make_shared<PlaceObjectCommand>(facade, a[0], x, y));
}

void MapHandler::process(const std::vector<std::string>& a) {
    if (!a.empty()) return badArgs("map");
    facade.executeCommand(std::make_shared<ShowMapCommand>(facade));
}

void HelpHandler::process(const std::vector<std::string>& a) {
    if (!a.empty()) return badArgs("help");
    facade.executeCommand(std::make_shared<HelpCommand>(facade));
}
