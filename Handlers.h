#pragma once
#include <memory>
#include <string>
#include <vector>

class GameFacade;

// Цепочка обязанностей. Каждый обработчик отвечает за свою группу команд (по первому слову):
//  - чужая команда -> передаётся дальше по цепочке;
//  - своя команда с неверными аргументами -> понятная ошибка, действие НЕ выполняется, команда НЕ
//    выдаётся за неизвестную;
//  - своя корректная команда -> создаётся объект команды и выполняется через фасад.
// handle() возвращает true, если команда распознана (успешно выполнена или отклонена с объяснением).
class Handler {
protected:
    std::shared_ptr<Handler> next_;
    bool passOn(const std::string& cmd) { return next_ ? next_->handle(cmd) : false; }
public:
    virtual ~Handler() = default;
    void setNext(std::shared_ptr<Handler> n) { next_ = std::move(n); }
    virtual bool handle(const std::string& cmd) = 0;
};

class CommandHandler : public Handler {
protected:
    GameFacade& facade;
    std::string keyword_;
    CommandHandler(GameFacade& f, std::string keyword) : facade(f), keyword_(std::move(keyword)) {}

    virtual void process(const std::vector<std::string>& args) = 0;   // args — слова после ключевого
    void badArgs(const std::string& format, const std::string& detail = "") const;
public:
    bool handle(const std::string& cmd) override;
};

#define DECLARE_HANDLER(Name, Keyword)                                       \
    class Name : public CommandHandler {                                     \
    public:                                                                  \
        explicit Name(GameFacade& f) : CommandHandler(f, Keyword) {}         \
    protected:                                                               \
        void process(const std::vector<std::string>& args) override;         \
    };

DECLARE_HANDLER(CreateHandler, "create")
DECLARE_HANDLER(MoveHandler, "move")
DECLARE_HANDLER(AttackHandler, "attack")
DECLARE_HANDLER(CollectHandler, "collect")
DECLARE_HANDLER(HealHandler, "heal")
DECLARE_HANDLER(BuildHandler, "build")
DECLARE_HANDLER(ResearchHandler, "research")
DECLARE_HANDLER(ProduceHandler, "produce")
DECLARE_HANDLER(DistributeHandler, "distribute")
DECLARE_HANDLER(StatsHandler, "stats")
DECLARE_HANDLER(InteractHandler, "interact")
DECLARE_HANDLER(TerrainHandler, "terrain")
DECLARE_HANDLER(ObjectHandler, "object")
DECLARE_HANDLER(MapHandler, "map")
DECLARE_HANDLER(HelpHandler, "help")

#undef DECLARE_HANDLER
