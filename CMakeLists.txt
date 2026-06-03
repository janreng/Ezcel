cmake_minimum_required(VERSION 3.21)
project(Ezcel VERSION 0.20.0 LANGUAGES CXX)

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

# --- QXlsx (đọc/ghi XLSX) — tải qua FetchContent, ghim tag v1.5.0.
# Tải về build/_deps (đã gitignore), không nhúng nguồn vào repo.
include(FetchContent)
FetchContent_Declare(
    QXlsx
    GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
    GIT_TAG        v1.5.0
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  QXlsx
)
FetchContent_MakeAvailable(QXlsx)

# --- Engine công thức (không phụ thuộc GUI) -> static lib dùng chung.
add_library(ezcel_formula STATIC
    src/formula/Formula.cpp
    src/formula/Functions.cpp
    src/formula/Value.cpp
)
target_include_directories(ezcel_formula PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_formula PUBLIC Qt6::Core)

# --- Model (cần Qt Gui cho QColor/QFont; không cần Widgets) -> static lib.
add_library(ezcel_model STATIC src/model/SpreadsheetModel.cpp src/model/SpreadsheetModel.h
    src/model/TextSearch.cpp src/model/TextSearch.h)
target_include_directories(ezcel_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_model PUBLIC ezcel_formula Qt6::Core Qt6::Gui)

# --- I/O file (CSV + XLSX qua QXlsx) -> static lib.
add_library(ezcel_io STATIC src/io/Csv.cpp src/io/Csv.h src/io/Xlsx.cpp src/io/Xlsx.h)
target_include_directories(ezcel_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_io PUBLIC Qt6::Core Qt6::Gui QXlsx::QXlsx)

# --- Tiện ích view (span ô gộp...) — cần Widgets -> static lib (test headless được).
add_library(ezcel_view STATIC src/view/MergeSpans.cpp src/view/MergeSpans.h)
target_include_directories(ezcel_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_view PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

# --- App GUI ---
qt_add_executable(Ezcel WIN32 src/main.cpp src/MainWindow.cpp src/MainWindowFormat.cpp src/MainWindowFind.cpp src/MainWindow.h)
target_include_directories(Ezcel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Ezcel PRIVATE ezcel_model ezcel_io ezcel_view ezcel_formula Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg)

# --- Tests (console, headless) ---
add_executable(test_formula tests/test_formula.cpp)
target_link_libraries(test_formula PRIVATE ezcel_formula Qt6::Core)

add_executable(test_model tests/test_model.cpp)
target_link_libraries(test_model PRIVATE ezcel_model Qt6::Core Qt6::Gui)

add_executable(test_io tests/test_io.cpp)
target_link_libraries(test_io PRIVATE ezcel_io Qt6::Core)

add_executable(test_view tests/test_view.cpp)
target_link_libraries(test_view PRIVATE ezcel_view ezcel_model Qt6::Core Qt6::Gui Qt6::Widgets)

enable_testing()
add_test(NAME formula COMMAND test_formula)
add_test(NAME model COMMAND test_model)
add_test(NAME io COMMAND test_io)
add_test(NAME view COMMAND test_view)
