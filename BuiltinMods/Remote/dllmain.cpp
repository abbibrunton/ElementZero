#include <exception>
#include <list>
#include <memory>

#include <dllentry.h>
#include <log.h>
#include <utility>
#include <yaml.h>
#include <base.h>
#include <remote.h>

#include <Core/Common.h>

#include "global.h"

DEF_LOGGER("Remote");

void dllenter() {}
void dllexit() {}

Settings settings;
std::unique_ptr<State> state;

DEFAULT_SETTINGS(settings);

std::map<std::string, void (*)()> &RegisterAPI::GetMap() {
  static std::map<std::string, void (*)()> temp;
  return temp;
}
std::list<std::pair<std::string, void (*)()>> &RegisterAPI::GetPreloadList() {
  static std::list<std::pair<std::string, void (*)()>> temp;
  return temp;
}

RegisterAPI::RegisterAPI(char const *name, bool check, void (*t)()) {
  if (check)
    GetMap().emplace(name, t);
  else
    GetPreloadList().emplace_back(name, t);
}

void PreInit() { state = std::make_unique<State>(); }

void PostInit() {
  for (auto [name, fn] : RegisterAPI::GetPreloadList()) {
    LOGV("Load builtin extension for %s") % name;
    fn();
  }
  for (auto [name, fn] : RegisterAPI::GetMap()) {
    if (GetLoadedMod(name.c_str())) {
      LOGV("Load builtin extension for %s") % name;
      fn();
    } else {
      LOGV("Skip builtin extension for %s: Target mod not loaded") % name;
    }
  }
}

void ServerStart() {
  LOGV("connecting to %s") % settings.endpoint;
  state->srv.Connect(settings.endpoint, {settings.name, "element-zero", Common::getServerVersionString()});
  state->srv.OnStop([](std::exception_ptr ep) {
    if (ep) try {
        std::rethrow_exception(ep);
      } catch (std::exception const &ex) { LOGE("Disconnected from gateway %s") % ex.what(); }
  });
  LOGI("Connected to gateway");
}

namespace Mod {

struct RemoteImpl : Remote {
  void AddMethod(std::string const &name, WsGw::Handler handler) override {
    if (!state) {
      LOGW("Skip register method: the Remote module not enabled.");
      return;
    }
    LOGV("Register method %s") % name;
    state->srv.RegisterHandler(name, handler);
  }
  void AddMethod(std::string const &name, WsGw::SyncHandler handler) override {
    if (!state) {
      LOGW("Skip register method: the Remote module not enabled.");
      return;
    }
    LOGV("Register method %s") % name;
    state->srv.RegisterHandler(name, handler);
  }

  void Broadcast(std::string_view name, WsGw::BufferView payload) override {
    if (!state) return;
    state->srv.Broadcast(name, payload);
  }
};

Remote &Remote::GetInstance() {
  static RemoteImpl impl;
  return impl;
}

} // namespace Mod