cmake_minimum_required(VERSION 3.20)
project(czc VERSION 0.0.1 LANGUAGES CXX)

# C++23 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 生成 compile_commands.json（用于 clang-tidy）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# 覆盖率选项
# ============================================================================
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang 使用 source-based coverage
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # GCC 使用 gcov
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()

# macOS: 确保 clang-tidy 能找到系统头文件
if(APPLE)
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(MACOS_SDK_PATH)
        add_compile_options(-isysroot ${MACOS_SDK_PATH})
    endif()
endif()

# ============================================================================
# 第三方依赖
# ============================================================================
include(FetchContent)

# CLI11 - 命令行解析
FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v2.6.1
)

# glaze - JSON 序列化库
FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG        v6.1.0
)

# tomlplusplus - TOML 配置文件解析
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)

# GoogleTest - 单元测试框架
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cli11 glaze tomlplusplus googletest)

# ============================================================================
# 包含目录
# ============================================================================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# Lexer 库
# ============================================================================
set(LEXER_SOURCES
    src/lexer/source_manager.cpp
    src/lexer/source_reader.cpp
    src/lexer/token.cpp
    src/lexer/utf8.cpp
    src/lexer/scanner.cpp
    src/lexer/ident_scanner.cpp
    src/lexer/number_scanner.cpp
    src/lexer/string_scanner.cpp
    src/lexer/comment_scanner.cpp
    src/lexer/char_scanner.cpp
    src/lexer/lexer_error.cpp
    src/lexer/lexer.cpp
)

# 查找 ICU 库（用于 Unicode 支持）
# macOS Homebrew ICU 路径提示
if(APPLE)
    set(ICU_ROOT "/opt/homebrew/opt/icu4c")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/icu4c")
endif()
find_package(ICU COMPONENTS uc REQUIRED)

add_library(czc_lexer STATIC ${LEXER_SOURCES})
target_include_directories(czc_lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(czc_lexer PUBLIC ICU::uc)

# ============================================================================
# CLI 库
# ============================================================================
set(CLI_SOURCES
    src/cli/cli.cpp
    src/cli/options.cpp
    src/cli/output/text_formatter.cpp
    src/cli/output/json_formatter.cpp
    src/cli/commands/lex_command.cpp
    src/cli/commands/version_command.cpp
)

add_library(czc_cli STATIC ${CLI_SOURCES})
target_link_libraries(czc_cli 
    PUBLIC czc_lexer 
    PUBLIC CLI11::CLI11
    PUBLIC glaze::glaze
    PUBLIC tomlplusplus::tomlplusplus
)
target_include_directories(czc_cli PUBLIC ${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# 可执行文件
# ============================================================================
add_executable(czc apps/czc/main.cpp)
target_link_libraries(czc PRIVATE czc_cli)

# ============================================================================
# 编译器警告选项
# ============================================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(czc_lexer PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(czc_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(czc PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(czc_lexer PRIVATE /W4)
    target_compile_options(czc_cli PRIVATE /W4)
    target_compile_options(czc PRIVATE /W4)
endif()

# ============================================================================
# 测试
# ============================================================================
enable_testing()

# Lexer 测试
set(LEXER_TEST_SOURCES
    test/lexer/source_manager_test.cpp
    test/lexer/source_reader_test.cpp
    test/lexer/token_test.cpp
    test/lexer/lexer_test.cpp
    test/lexer/ident_scanner_test.cpp
    test/lexer/number_scanner_test.cpp
    test/lexer/string_scanner_test.cpp
    test/lexer/comment_scanner_test.cpp
    test/lexer/char_scanner_test.cpp
    test/lexer/utf8_test.cpp
    test/lexer/lexer_error_test.cpp
    test/lexer/scanner_test.cpp
)

# 覆盖率模式下直接编译源文件到测试中
if(ENABLE_COVERAGE)
    add_executable(lexer_tests ${LEXER_TEST_SOURCES} ${LEXER_SOURCES})
    target_include_directories(lexer_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(lexer_tests 
        PRIVATE GTest::gtest_main
        PRIVATE ICU::uc
    )
else()
    add_executable(lexer_tests ${LEXER_TEST_SOURCES})
    target_link_libraries(lexer_tests 
        PRIVATE czc_lexer
        PRIVATE GTest::gtest_main
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(lexer_tests PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(lexer_tests PRIVATE /W4)
endif()

include(GoogleTest)
gtest_discover_tests(lexer_tests)