set(SDL2_INCLUDE_DIRS "$ENV{SDL2DIR}/include")

# Support both 32 and 64 bit builds
if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(SDL2_LIBRARIES "$ENV{SDL2DIR}/lib/x64/SDL2.lib;$ENV{SDL2DIR}/lib/x64/SDL2main.lib")
else()
  set(SDL2_LIBRARIES "$ENV{SDL2DIR}/lib/x86/SDL2.lib;$ENV{SDL2DIR}/lib/x86/SDL2main.lib")
endif()

string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)