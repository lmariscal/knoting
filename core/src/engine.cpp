#include <bx/math.h>
#include <knoting/engine.h>

namespace knot {

Engine::Engine() {
    m_windowModule = std::make_shared<knot::Window>(m_windowWidth, m_windowHeight, m_windowTitle, *this);
    m_forwardRenderModule = std::make_shared<knot::ForwardRenderer>(*this);
    m_physicsModule = std::make_shared<knot::Physics>(*this);
    m_assetManager = std::make_shared<knot::AssetManager>();

    // order dependent
    m_engineModules.emplace_back(m_windowModule);
    m_engineModules.emplace_back(m_assetManager);
    m_engineModules.emplace_back(m_forwardRenderModule);
    m_engineModules.emplace_back(m_physicsModule);

    for (auto& module : m_engineModules) {
        module->on_awake();
    }
    log::debug("init engine");
}

void Engine::update_modules() {
    for (auto& module : m_engineModules) {
        module->on_update(m_windowModule->get_delta_time());
        module->on_fixed_update();
    }

    // TODO move into functions when functionality exists

    // CORE SYSTEM ORDER

    // PHYSICS
    // 1) SYSTEM : Physics / update all dynamic transforms

    // SCRIPTS
    // 1) SYSTEM : Editor Camera movement

    // PBR RENDERING
    // 1) SYSTEM : Shadow pass
    // 2) SYSTEM : Update Active Camera (Runtime / Editor)
    // 3) SYSTEM : Depth Pass (No Transparent Objects)
    // 4) SYSTEM : PBR Render Pass
    // 5) SYSTEM : Skybox Render
    // 6) SYSTEM : Sorted Transparent Render Pass
    // 7) SYSTEM : Post Processing Stack

    m_forwardRenderModule->on_render();
    m_forwardRenderModule->on_post_render();

    for (auto& module : m_engineModules) {
        module->on_late_update();
    }
    swap_frame();
}

Engine::~Engine() {
    log::info("engine on destroy");
    for (auto& module : m_engineModules) {
        module->on_destroy();
    }
}

bool Engine::is_open() {
    return m_windowModule->is_open();
}
void Engine::add_subsystem(std::shared_ptr<Subsystem> subsystem) {
    m_engineModules.emplace_back(subsystem);
    subsystem->on_awake();
}
void Engine::swap_frame() {
    bgfx::frame();
}

std::optional<std::reference_wrapper<Engine>> Engine::get_active_engine() {
    return s_activeEngine;
}

void Engine::set_active_engine(std::optional<std::reference_wrapper<Engine>> engine) {
    s_activeEngine = std::ref(engine);
}
void Engine::reset_physics_module() {
    m_physicsModule.reset();
    m_physicsModule = std::make_shared<knot::Physics>(*this);
    m_physicsModule->on_awake();
}

}  // namespace knot
