#pragma once

#include <SDL3/SDL_log.h>

// Thin wrappers over SDL's logging so the engine doesn't need to pull in <iostream>/<format>.
#define ENGINE_LOG_INFO(...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define ENGINE_LOG_WARN(...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define ENGINE_LOG_ERROR(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
