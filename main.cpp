#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include "GameFacade.h"

namespace {
    std::string trim(const std::string& s) {
        size_t b = s.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) return "";
        size_t e = s.find_last_not_of(" \t\r\n");
        return s.substr(b, e - b + 1);
    }

    struct Input {
        bool eof = false;
        std::string ask(const std::string& prompt) {
            std::cout << prompt;
            std::string s;
            if (!std::getline(std::cin, s)) { eof = true; return ""; }
            return trim(s);
        }
    };

    const char* kMenu =
        "\nМЕНЮ (или вводите команды текстом, список — help)\n"
        " 1 создать юнита        2 переместить         3 атаковать\n"
        " 4 собрать ресурс       5 лечить              6 построить здание\n"
        " 7 исследовать          8 распределить        9 произвести юнита\n"
        "10 статистика          11 карта              12 ландшафт\n"
        "13 нейтральный объект  14 взаимодействие     15 help\n"
        " 0 выход\n";

    // Меню лишь собирает строку команды: разбор и проверка — в цепочке обработчиков.
    std::string composeFromMenu(int n, Input& in) {
        std::string a, b, c;
        switch (n) {
        case 1:  a = in.ask("Тип (infantry archer cavalry worker scout tank medic): "); b = in.ask("X: "); c = in.ask("Y: ");
            return "create unit " + a + " at " + b + "," + c;
        case 2:  a = in.ask("ID юнита: "); b = in.ask("X: "); c = in.ask("Y: ");
            return "move unit " + a + " to " + b + "," + c;
        case 3:  a = in.ask("ID атакующего: "); b = in.ask("ID цели: ");
            return "attack unit " + a + " target " + b;
        case 4:  a = in.ask("ID рабочего: "); b = in.ask("Ресурс (1|wood 2|metal 3|energy): ");
            return "collect unit " + a + " from " + b;
        case 5:  a = in.ask("ID медика: "); b = in.ask("ID цели: ");
            return "heal unit " + a + " target " + b;
        case 6:  a = in.ask("Здание (barracks lab warehouse powerplant hospital): "); b = in.ask("X: "); c = in.ask("Y: ");
            return "build " + a + " at " + b + "," + c;
        case 7:  a = in.ask("Технология (attack_upgrade armor_upgrade): ");
            return "research " + a;
        case 8:  a = in.ask("Ресурс (wood metal energy): "); b = in.ask("Здание (тип или id): "); c = in.ask("Количество: ");
            return "distribute " + a + " to " + b + " " + c;
        case 9:  a = in.ask("Тип юнита: "); b = in.ask("ID здания: ");
            return "produce " + a + " in " + b;
        case 10: a = in.ask("Раздел (Enter — всё; units buildings resources research): ");
            return "stats " + a;
        case 11: return "map";
        case 12: a = in.ask("Ландшафт (plains forest swamp mountains): "); b = in.ask("X: "); c = in.ask("Y: ");
            return "terrain " + a + " at " + b + "," + c;
        case 13: a = in.ask("Объект (chest trap altar): "); b = in.ask("X: "); c = in.ask("Y: ");
            return "object " + a + " at " + b + "," + c;
        case 14: a = in.ask("ID юнита: "); b = in.ask("X: "); c = in.ask("Y: ");
            return "interact unit " + a + " at " + b + "," + c;
        case 15: return "help";
        default: return "";
        }
    }

    bool isNumber(const std::string& s) {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
    }

    int runScript(GameFacade& game, const std::string& path) {
        std::ifstream f(path);
        if (!f) { std::cerr << "Не удалось открыть сценарий: " << path << "\n"; return 1; }
        std::string raw;
        while (std::getline(f, raw)) {
            std::string line = trim(raw);
            if (line.empty()) continue;
            if (line[0] == '#') { std::cout << "\n" << line << "\n"; continue; }
            std::cout << "\n> " << line << "\n";
            game.processInput(line);
        }
        return 0;
    }
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   // все исходники в UTF-8
    SetConsoleCP(CP_UTF8);
#endif

    auto eventBus = std::make_shared<IObservable>();
    auto base = std::make_shared<Base>(1);
    GameField field(10, 10);
    GameFacade game(field, eventBus, base);

    if (argc >= 3 && std::string(argv[1]) == "--script") return runScript(game, argv[2]);

    std::cout << "ММО РПГ — управление игрой. Введите help, menu или команду.\n";
    Input in;
    while (!in.eof) {
        std::cout << "\n> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;

        std::string lower = line;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lower == "0" || lower == "exit" || lower == "quit") break;
        if (lower == "menu") { std::cout << kMenu; continue; }

        if (isNumber(lower) && lower.size() <= 2) {
            std::string composed = composeFromMenu(std::stoi(lower), in);
            if (in.eof) break;
            if (composed.empty()) std::cout << "[Ошибка] Нет такого пункта меню (введите menu)\n";
            else game.processInput(composed);
            continue;
        }
        game.processInput(line);
    }
    return 0;
}
