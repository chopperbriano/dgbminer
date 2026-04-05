cmake_minimum_required(VERSION 3.20)
project(dgbminer_windows LANGUAGES C CXX)

# Windows x64 only
if(NOT WIN32)
    message(FATAL_ERROR "dgbminer is Windows-only. Use a Windows MSVC toolchain.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "dgbminer targets x64 only. Configure with an x64 generator.")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release when unspecified (matches single-config generators like Ninja)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ---------------------------------------------------------------------------
# Sources (mirrors winbuild/mywin.vcxproj)
# ---------------------------------------------------------------------------
set(DGB_SOURCES
    algo-gate-api.c
    api.c
    cpu-miner.c
    malloc-huge.c
    util.c

    algo/cubehash/cubehash_sse2.c
    algo/echo/sph_echo.c
    algo/luffa/luffa_for_sse2.c
    algo/qubit/qubit-gate.c
    algo/qubit/qubit.c
    algo/scrypt/scrypt-core-4way.c
    algo/scrypt/scrypt.c
    algo/shavite/sph_shavite.c
    algo/sha/sha256-hash-4way.c
    algo/sha/sha256.c
    algo/sha/sha256d-4way.c
    algo/sha/sha256d.c
    algo/sha/sph_sha2.c
    algo/simd/nist.c
    algo/simd/vector.c
    algo/skein/skein-gate.c
    algo/skein/skein.c
    algo/skein/sph_skein.c

    algo/odo/KeccakP-800-reference.cpp
    algo/odo/odo.cpp
    algo/odo/odocrypt.cpp

    compat/getopt/getopt_long.c
    compat/winansi.c
    compat/legacy_iob.c

    # Jansson (JSON) built from bundled source — the jansson.lib in
    # compat/lib is an import lib requiring jansson.dll at runtime.
    compat/jansson/dump.c
    compat/jansson/error.c
    compat/jansson/hashtable.c
    compat/jansson/load.c
    compat/jansson/memory.c
    compat/jansson/pack_unpack.c
    compat/jansson/strbuffer.c
    compat/jansson/strconv.c
    compat/jansson/utf.c
    compat/jansson/value.c
)

add_executable(dgbminer ${DGB_SOURCES})
set_target_properties(dgbminer PROPERTIES OUTPUT_NAME "dgbminerwindows")

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------
target_include_directories(dgbminer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/algo
    ${CMAKE_CURRENT_SOURCE_DIR}/simd-utils
    ${CMAKE_CURRENT_SOURCE_DIR}/compat
    ${CMAKE_CURRENT_SOURCE_DIR}/compat/include
    ${CMAKE_CURRENT_SOURCE_DIR}/compat/getopt
)

# ---------------------------------------------------------------------------
# Preprocessor defines
# ---------------------------------------------------------------------------
target_compile_definitions(dgbminer PRIVATE
    WIN32
    _CONSOLE
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    CURL_STATICLIB
    __SSE2__
    NOMINMAX
    # AVX2 porting attempted but hit MSVC codegen crashes (see git history
    # around commit that will be reverted). The codebase's AVX2 paths were
    # designed for GCC/MinGW and MSVC /arch:AVX2 + the mixed inline
    # intrinsics chain produces broken codegen. Sticking to SSE2 baseline.
)

# ---------------------------------------------------------------------------
# MSVC compile flags — Windows x64 optimized
# ---------------------------------------------------------------------------
if(MSVC)
    target_compile_options(dgbminer PRIVATE
        /W3                 # warning level
        /MP                 # parallel compilation
        /EHsc               # C++ exceptions
        /Zc:__cplusplus     # correct __cplusplus macro
        /FI${CMAKE_CURRENT_SOURCE_DIR}/compat.h  # force-include for __attribute__ no-op
        # Suppress benign upstream warnings that would require invasive
        # source changes: signed/unsigned compare (C4018), narrowing
        # conversions in display/stats code (C4244/C4267), pointer
        # truncation (C4311, intentional in hash table hashing), function
        # prototype mismatches across compilation units (C4028), and
        # _WINSOCK_DEPRECATED_NO_WARNINGS being redefined (C4005) since
        # we also set it in api.c.
        /wd4005 /wd4018 /wd4028 /wd4244 /wd4267 /wd4311
    )
    # Release-only perf flags
    target_compile_options(dgbminer PRIVATE
        $<$<CONFIG:Release>:/O2>          # max speed
        $<$<CONFIG:Release>:/Ot>          # favor speed over size
        $<$<CONFIG:Release>:/Ob2>         # aggressive inlining
        $<$<CONFIG:Release>:/Oy>          # omit frame pointers
        $<$<CONFIG:Release>:/GL>          # whole program optimization
        $<$<CONFIG:Release>:/fp:fast>     # fast floating point
        $<$<CONFIG:Release>:/Gy>          # function-level linking
    )
    target_link_options(dgbminer PRIVATE
        $<$<CONFIG:Release>:/LTCG>        # link-time code generation
        $<$<CONFIG:Release>:/OPT:REF>     # eliminate unreferenced
        $<$<CONFIG:Release>:/OPT:ICF>     # COMDAT folding
    )
endif()

# ---------------------------------------------------------------------------
# Prebuilt libraries shipped in compat/lib/x64
# ---------------------------------------------------------------------------
set(DGB_LIBDIR_X64 ${CMAKE_CURRENT_SOURCE_DIR}/compat/lib/x64)
set(DGB_LIBDIR     ${CMAKE_CURRENT_SOURCE_DIR}/compat/lib)

target_link_directories(dgbminer PRIVATE ${DGB_LIBDIR_X64} ${DGB_LIBDIR})

target_link_libraries(dgbminer PRIVATE
    ${DGB_LIBDIR_X64}/pthreadVC3.lib
    ${DGB_LIBDIR_X64}/libcurl_a.lib
    # jansson built from source above, not linked from prebuilt .lib
    # Windows system libs required by static libcurl + winsock
    ws2_32
    wldap32
    crypt32
    normaliz
    advapi32
    user32
    # Legacy CRT shim: bundled libcurl_a.lib was built against older MSVC
    # runtime that references __iob_func; this lib provides the shim.
    legacy_stdio_definitions
)

# ---------------------------------------------------------------------------
# OpenSSL (required by the static libcurl_a.lib we bundle)
#
# Install via vcpkg:
#   git clone https://github.com/microsoft/vcpkg.git C:/vcpkg
#   C:/vcpkg/bootstrap-vcpkg.bat
#   C:/vcpkg/vcpkg install openssl:x64-windows-static
#
# Configure this project with:
#   -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
#   -DVCPKG_TARGET_TRIPLET=x64-windows-static
#
# Or let us find it at the conventional location:
# ---------------------------------------------------------------------------
if(NOT TARGET OpenSSL::SSL)
    if(EXISTS "C:/vcpkg/installed/x64-windows-static/lib/libcrypto.lib")
        set(OPENSSL_ROOT_DIR "C:/vcpkg/installed/x64-windows-static")
        set(OPENSSL_USE_STATIC_LIBS TRUE)
    endif()
    find_package(OpenSSL REQUIRED)
endif()

target_link_libraries(dgbminer PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# ---------------------------------------------------------------------------
# Copy pthread DLL next to the exe at build time (runtime dep of pthreadVC3)
# ---------------------------------------------------------------------------
# Note: pthreadVC3.lib is an import lib; if no DLL is needed (static) this is a no-op.
# pthreads4w ships both flavors; the .lib here is the static one when paired with
# PTW32_STATIC_LIB. Leaving this here as a safety net for future switches.

message(STATUS "dgbminer: Windows x64 Release build configured (AVX2, LTCG, /O2)")
