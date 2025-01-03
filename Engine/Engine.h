#pragma once

#include "Core.h"
#include "GameEngine.h"
#include "EntryPoint.h"
#include "src/GameEvent.h"
#include "src/AssetManager.h"
#include "src/InputManager.h"
#include "src/Camera.h"

#include "src/sdl/SDLEventDispatcher.h"
#include "src/sdl/SDLEventHandler.h"

#include "src/ecs/EntityManager.h"
#include "src/ecs/component/AnimationComponent.h"
#include "src/ecs/component/SpriteComponent.h"
#include "src/ecs/component/TransformComponent.h"
#include "src/ecs/component/VelocityComponent.h"
#include "src/ecs/system/AnimationSystem.h"
