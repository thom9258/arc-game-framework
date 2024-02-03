#pragma once

#include <memory>
#include <atomic>

namespace arc {
namespace core {

class IScene {
  public:
    virtual bool init(void) = 0;
    virtual bool update(void) = 0;
    virtual bool destroy(void) = 0;

    /*Optional*/
    virtual bool draw(void) { return true; };
    /*TODO: call these when swapping scenes!*/
    virtual bool on_activate(void) { return true; };
    virtual bool on_deactivate(void) { return true; };
};

class SceneHandle {
public:
    template <typename Scene, class... Args>
    void make(Args&&... _args) {
        m_scene = std::make_shared<Scene>((_args)...);
    }
    IScene* scene();
    bool async_init();
    bool async_destroy();
    bool can_update();

private:
    std::shared_ptr<IScene> m_scene{nullptr};
    std::atomic<bool> m_ready{false};
};

}; /*ns*/
}; /*ns*/
