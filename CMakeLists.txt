cmake_minimum_required(VERSION 3.16)

# Only x64 is supported/tested for now; override with -DARCH=... if a 32-bit build is ever needed.
if(NOT ARCH)
    set(ARCH "x64")
endif()

# When building with Clang, CMake's Windows-Clang platform module (Modules/Platform/Windows-Clang.cmake)
# calls enable_language(RC) as part of setting up the Clang toolchain during project()/enable_language(CXX)
# itself, so this must run and take effect BEFORE project() is called, the same way a compiler override
# would. LLVM's own RC compiler (llvm-rc, what CMake would otherwise auto-detect here) fails on some
# vendored third-party .rc files (e.g. FreeType's copyright symbol trips a "non-ASCII codepoint" bug), so
# prefer the real Windows SDK rc.exe, discovered generically rather than pinned to one SDK version.
if(CMAKE_HOST_WIN32 AND NOT CMAKE_RC_COMPILER)
    # Windows SDKs always install under this fixed location, so unlike the LLVM path (which
    # varies by user choice at install time) this is safe to reference directly.
    file(GLOB WINDOWS_SDK_RC_CANDIDATES
        "C:/Program Files (x86)/Windows Kits/10/bin/*/${ARCH}/rc.exe"
    )
    if(WINDOWS_SDK_RC_CANDIDATES)
        list(SORT WINDOWS_SDK_RC_CANDIDATES)
        list(POP_BACK WINDOWS_SDK_RC_CANDIDATES WINDOWS_SDK_RC_COMPILER)
        set(CMAKE_RC_COMPILER "${WINDOWS_SDK_RC_COMPILER}" CACHE FILEPATH "RC Compiler" FORCE)
    endif()
endif()

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(sdl-boilerplate)

# CMake's default Windows-Clang RC rule (set up during project(), see above) always pipes .rc files
# through `clang -E` first and hands the result to the RC compiler in a format tailored to llvm-rc,
# including GCC/Clang-style depfile flags (-MD -MF). The real Windows SDK rc.exe understands neither
# that preprocessed format nor those flags, so when we've swapped in rc.exe: invoke it directly (it has
# its own built-in preprocessor) and drop depfile generation for RC sources (a minor loss: accurate
# dependency tracking for the rare .rc file edit, not a correctness issue).
if(CMAKE_RC_COMPILER MATCHES "/rc\\.exe$")
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <DEFINES> <INCLUDES> <FLAGS> /fo <OBJECT> <SOURCE>")
    set(CMAKE_DEPFILE_FLAGS_RC "")

    # rc.exe also has no compiler-driver auto-discovery of the SDK headers (windows.h etc.) the way
    # Clang/MSVC do when invoked normally. It relies on the INCLUDE env var from a VS Developer Prompt,
    # which we don't have here. Derive the SDK's um/shared include dirs from the same SDK install
    # rc.exe itself came from (.../Windows Kits/10/bin/<version>/<arch>/rc.exe) and pass them explicitly.
    get_filename_component(_rc_arch_dir "${CMAKE_RC_COMPILER}" DIRECTORY)   # .../bin/<version>/<arch>
    get_filename_component(_rc_version_dir "${_rc_arch_dir}" DIRECTORY)     # .../bin/<version>
    get_filename_component(_rc_sdk_version "${_rc_version_dir}" NAME)       # <version>
    get_filename_component(_rc_bin_dir "${_rc_version_dir}" DIRECTORY)      # .../bin
    get_filename_component(_rc_sdk_root "${_rc_bin_dir}" DIRECTORY)         # .../Windows Kits/10
    set(CMAKE_RC_FLAGS "-I\"${_rc_sdk_root}/include/${_rc_sdk_version}/um\" -I\"${_rc_sdk_root}/include/${_rc_sdk_version}/shared\"")
endif()

# Default build type if none was given on the command line (-DCMAKE_BUILD_TYPE=...).
# Note this must NOT be a plain set() without CACHE, otherwise it silently overrides
# whatever the caller passed in.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Built binaries land directly in the configured build directory (CMAKE_BINARY_DIR), i.e.
# wherever `-B <dir>` or the active CMakePresets.json preset points. Debug and Release
# configures of the same architecture naturally stay isolated as long as each uses its own
# binary directory (the provided presets already do this).
set(BUILD_DIR "${CMAKE_BINARY_DIR}" CACHE STRING "BuildDir" FORCE)
set(INSTALL_DIR "${CMAKE_SOURCE_DIR}/install/${ARCH}" CACHE STRING "InstallDir" FORCE)

# `cmake --install <build-dir>` with no extra flags lands here by default: a clean, player-ready
# folder (just the .exe, the DLLs it actually needs, and assets/ — no .pdb/.lib/build artifacts).
# `CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT` is NOT a reliable guard for "did the caller
# override this" here — it's only true on a build directory's very first configure, so it goes
# stale (false) on every reconfigure after that. `cmake --install <dir> --prefix <other>` already
# overrides this per-invocation without touching the cache, so a plain FORCE default is enough.
set(CMAKE_INSTALL_PREFIX "${INSTALL_DIR}" CACHE PATH "Install path" FORCE)

# Add subdirectories for engine and game projects
add_subdirectory(Engine)
add_subdirectory(Game)

# Engine's and Game's own file(COPY ... assets ...) calls (see their CMakeLists.txt) both target
# this same ${CMAKE_BINARY_DIR}/assets folder, merging into one at configure time — install that
# merged result rather than either source assets/ alone.
install(DIRECTORY ${CMAKE_BINARY_DIR}/assets DESTINATION .)
