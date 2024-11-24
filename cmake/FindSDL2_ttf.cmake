# FindSDL2_ttf
# -------------
#
# Locate SDL2_ttf library
#
# This module defines:
#
# SDL2_TTF_LIBRARIES, the name of the library to link against
# SDL2_TTF_INCLUDE_DIRS, where to find the headers
# SDL2_TTF_FOUND, if false, do not try to link against
# SDL2_TTF_VERSION_STRING - human-readable string containing the
# version of SDL2_ttf
#
#
#
# For backward compatibility the following variables are also set:
#
# SDL2TTF_LIBRARY (same value as SDL2_TTF_LIBRARIES)
# SDL2TTF_INCLUDE_DIR (same value as SDL2_TTF_INCLUDE_DIRS)
# SDL2TTF_FOUND (same value as SDL2_TTF_FOUND)
#
#
#
# $SDLDIR is an environment variable that would correspond to the
# ./configure --prefix=$SDLDIR used in building SDL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake
# module, but with modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).

find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h
  HINTS
  $ENV{CMAKE_PREFIX_PATH}
  PATH_SUFFIXES SDL2_ttf/include
)

find_library(SDL2_TTF_LIBRARY
  NAMES SDL2_ttf
  HINTS
  $ENV{CMAKE_PREFIX_PATH}
  PATH_SUFFIXES SDL2_ttf/${VC_LIB_PATH_SUFFIX}
)

if(SDL2_TTF_INCLUDE_DIR AND EXISTS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h")
  file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_MAJOR "${SDL2_TTF_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_MINOR "${SDL2_TTF_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_PATCH "${SDL2_TTF_VERSION_PATCH_LINE}")
  set(SDL2_TTF_VERSION_STRING ${SDL2_TTF_VERSION_MAJOR}.${SDL2_TTF_VERSION_MINOR}.${SDL2_TTF_VERSION_PATCH})
  unset(SDL2_TTF_VERSION_MAJOR_LINE)
  unset(SDL2_TTF_VERSION_MINOR_LINE)
  unset(SDL2_TTF_VERSION_PATCH_LINE)
  unset(SDL2_TTF_VERSION_MAJOR)
  unset(SDL2_TTF_VERSION_MINOR)
  unset(SDL2_TTF_VERSION_PATCH)
endif()

set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_ttf
  REQUIRED_VARS SDL2_TTF_LIBRARIES SDL2_TTF_INCLUDE_DIRS
  VERSION_VAR SDL2_TTF_VERSION_STRING)

# for backward compatibility
set(SDL2TTF_LIBRARY ${SDL2_TTF_LIBRARIES})
set(SDL2TTF_INCLUDE_DIR ${SDL2_TTF_INCLUDE_DIRS})
set(SDL2TTF_FOUND ${SDL2_TTF_FOUND})

mark_as_advanced(SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)
