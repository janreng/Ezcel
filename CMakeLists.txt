cmake_minimum_required(VERSION 3.21)
project(Ezcel VERSION 0.1.0 LANGUAGES CXX)

# Bản port C++ của Ezcel (gốc Python/PySide6). Dùng Qt6 Widgets + MinGW.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)

# --- Engine công thức (không phụ thuộc GUI) -> static lib dùng chung app + test.
add_library(ezcel_formula STATIC
    src/formula/Formula.cpp
    src/formula/Functions.cpp
    src/formula/Value.cpp
)
target_include_directories(ezcel_formula PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_formula PUBLIC Qt6::Core)

# --- App GUI ---
qt_add_executable(Ezcel WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/model/SpreadsheetModel.cpp
    src/model/SpreadsheetModel.h
)
target_include_directories(Ezcel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Ezcel PRIVATE ezcel_formula Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg)

# --- Test engine (console, headless) ---
add_executable(test_formula tests/test_formula.cpp)
target_link_libraries(test_formula PRIVATE ezcel_formula Qt6::Core)

enable_testing()
add_test(NAME formula COMMAND test_formula)
