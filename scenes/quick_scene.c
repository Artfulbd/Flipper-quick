#include "quick_scene.h"

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const quick_on_enter_handlers[])(void*) = {
#include "quick_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const quick_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "quick_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const quick_on_exit_handlers[])(void* context) = {
#include "quick_scene_config.h"
};
#undef ADD_SCENE

const SceneManagerHandlers quick_scene_handlers = {
    .on_enter_handlers = quick_on_enter_handlers,
    .on_event_handlers = quick_on_event_handlers,
    .on_exit_handlers  = quick_on_exit_handlers,
    .scene_num         = QuickSceneNum,
};
