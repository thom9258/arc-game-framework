#pragma once

#include "Defs.hpp"
#include "App.hpp"

namespace arc {
    
class Engine;
class GameScene;

using ECSSystem = std::function<void(GameScene*)>;
//using ECSSystem = std::function<void(Engine*, entt::registry*)>;

class GameScene : public IScene {
    entt::registry m_ECS = entt::registry();
    Engine* m_engine = nullptr;

public:
    entt::registry* ECS(void)
    {
        return &m_ECS;
    };
    Engine* engine(void) 
    {
        assert(m_engine != nullptr);
        return m_engine;
    };

    void set_engine(Engine* _e) {m_engine = _e;};

    void ECS_tick(std::vector<ECSSystem>& _systems)
    {
        for (auto system: _systems)
            system(this);
    };
};

struct WindowConfig {
    size_t width = 800;
    size_t height = 400;
    std::string name = "ARC Engine";
};

class Engine : public App {
    arc::Logger m_engine_log;
    arc::Logger m_user_log;
    /*renderer;
      AssetManager;
     */
public:
    void start_window(WindowConfig* _config);
    Logger* logger(void);
};

}; /*namespace arc*/
