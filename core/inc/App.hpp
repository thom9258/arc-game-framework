#pragma once

#include "Defs.hpp"
#include "Logger.hpp"
#include "SceneManager.hpp"

namespace arc {

class App;

class BaseScene : public IScene {
    App* m_app = nullptr;

  protected:
    virtual App* app(void) final {
        assert(m_app != nullptr);
        return m_app;
    };

  public:
    void set_app(App* _e) { m_app = _e; };
};

class App {
    Logger* m_logger{nullptr};
    SceneManager m_scene_manager{};

  public:
    App(void);

    /*Logging Methods*/
    void add_logger(Logger* _logger);
    bool INFO(const std::string& _msg);
    bool DEBUG(const std::string& _msg);
    bool WARNING(const std::string& _msg);
    bool ERROR(const std::string& _msg);

    /*Scene Management Methods*/
    [[nodiscard]] SceneKey scene_add(const std::string& _name,
                                     std::shared_ptr<IScene> _scene);
    bool scene_init_async(SceneKey _scene);
    bool scene_destroy_async(SceneKey _scene);
    [[nodiscard]] bool scene_is_inited(SceneKey _scene);
    [[nodiscard]] SceneKey scene_active_get(void);
    std::shared_ptr<IScene> scene_active_get_ptr(void);
    SceneKey scene_active_set(SceneKey _scene);
    [[nodiscard]] std::string scene_name(SceneKey _scene);

};

}; /*namespace arc*/
