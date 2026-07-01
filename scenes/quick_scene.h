#pragma once

#include <gui/scene_manager.h>

#define ADD_SCENE(prefix, name, id) QuickScene##id,
typedef enum {
#include "quick_scene_config.h"
    QuickSceneNum,
} QuickScene;
#undef ADD_SCENE

extern const SceneManagerHandlers quick_scene_handlers;

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "quick_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "quick_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "quick_scene_config.h"
#undef ADD_SCENE
