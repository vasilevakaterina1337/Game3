#pragma once
#include <memory>
#include <vector>
#include "Commands.h"

// Журнал УСПЕШНО выполненных команд. Undo в этой работе не реализован (необязательное расширение):
// прежний undo() только выбрасывал запись и ничего не откатывал, поэтому он удалён.
class CommandHistory {
    std::vector<std::shared_ptr<Command>> history;
public:
    void push(std::shared_ptr<Command> cmd) { history.push_back(std::move(cmd)); }
    std::shared_ptr<Command> getLastCommand() const { return history.empty() ? nullptr : history.back(); }
    size_t size() const { return history.size(); }
};
