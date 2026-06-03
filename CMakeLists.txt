cmake_minimum_required(VERSION 3.21)
project(Ezcel VERSION 1.16.0 LANGUAGES CXX)

# Bản port C++ của Ezcel (gốc Python/PySide6). Dùng Qt6 Widgets + MinGW.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg Network)

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
    src/model/TextSearch.cpp src/model/TextSearch.h
    src/model/CondFormat.cpp src/model/CondFormat.h
    src/model/Filter.cpp src/model/Filter.h
    src/model/PasteOps.cpp src/model/PasteOps.h
    src/model/Stats.cpp src/model/Stats.h
    src/model/AutoSum.cpp src/model/AutoSum.h
    src/model/Validation.cpp src/model/Validation.h
    src/model/CellStyles.cpp src/model/CellStyles.h
    src/model/GotoSpecial.cpp src/model/GotoSpecial.h
    src/model/NumberFormat.cpp src/model/NumberFormat.h)
target_include_directories(ezcel_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_model PUBLIC ezcel_formula ezcel_sort Qt6::Core Qt6::Gui)

# --- I/O file (CSV + XLSX qua QXlsx) -> static lib.
add_library(ezcel_io STATIC src/io/Csv.cpp src/io/Csv.h src/io/Xlsx.cpp src/io/Xlsx.h)
target_include_directories(ezcel_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_io PUBLIC Qt6::Core Qt6::Gui QXlsx::QXlsx)

# --- Tiện ích view (span ô gộp...) — cần Widgets -> static lib (test headless được).
add_library(ezcel_view STATIC src/view/MergeSpans.cpp src/view/MergeSpans.h
    src/view/CellBorderDelegate.cpp src/view/CellBorderDelegate.h
    src/view/Visibility.cpp src/view/Visibility.h)
target_include_directories(ezcel_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_view PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

# --- Cập nhật (GitHub Releases) — cần Network + Widgets -> static lib.
add_library(ezcel_update STATIC
    src/update/VersionCompare.cpp src/update/VersionCompare.h
    src/update/Updater.cpp src/update/Updater.h)
target_include_directories(ezcel_update PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_update PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)

# --- App GUI ---
qt_add_executable(Ezcel WIN32 src/main.cpp src/MainWindow.cpp src/MainWindowFormat.cpp src/MainWindowFind.cpp src/ui/I18n.cpp src/ui/Shortcuts.cpp src/ui/Zoom.cpp src/ui/SheetNav.cpp src/ui/CellMode.cpp src/ui/WorkbookStats.cpp src/MainWindow.h resources.qrc)
target_include_directories(Ezcel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(Ezcel PRIVATE EZCEL_VERSION="${PROJECT_VERSION}")
target_link_libraries(Ezcel PRIVATE ezcel_model ezcel_io ezcel_view ezcel_update ezcel_formula Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::Network)

# --- Tests (console, headless) ---
add_executable(test_formula tests/test_formula.cpp)
target_link_libraries(test_formula PRIVATE ezcel_formula Qt6::Core)

add_executable(test_model tests/test_model.cpp)
target_link_libraries(test_model PRIVATE ezcel_model Qt6::Core Qt6::Gui)

add_executable(test_io tests/test_io.cpp)
target_link_libraries(test_io PRIVATE ezcel_io Qt6::Core)

add_executable(test_view tests/test_view.cpp)
target_link_libraries(test_view PRIVATE ezcel_view ezcel_model Qt6::Core Qt6::Gui Qt6::Widgets)

# So sánh phiên bản — test thuần Core (không kéo Network).
add_executable(test_version tests/test_version.cpp src/update/VersionCompare.cpp)
target_include_directories(test_version PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_version PRIVATE Qt6::Core)

add_executable(test_i18n tests/test_i18n.cpp src/ui/I18n.cpp)
target_include_directories(test_i18n PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_i18n PRIVATE Qt6::Core)

add_executable(test_numfmt tests/test_numfmt.cpp src/model/NumberFormat.cpp)
target_include_directories(test_numfmt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_numfmt PRIVATE Qt6::Core Qt6::Gui)

add_executable(test_shortcuts tests/test_shortcuts.cpp src/ui/Shortcuts.cpp)
target_include_directories(test_shortcuts PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_shortcuts PRIVATE Qt6::Core)

add_executable(test_zoom tests/test_zoom.cpp src/ui/Zoom.cpp)
target_include_directories(test_zoom PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_zoom PRIVATE Qt6::Core)

add_executable(test_sheetnav tests/test_sheetnav.cpp src/ui/SheetNav.cpp)
target_include_directories(test_sheetnav PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_sheetnav PRIVATE Qt6::Core)

add_executable(test_cellmode tests/test_cellmode.cpp src/ui/CellMode.cpp)
target_include_directories(test_cellmode PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_cellmode PRIVATE Qt6::Core)

add_executable(test_wbstats tests/test_wbstats.cpp src/ui/WorkbookStats.cpp)
target_include_directories(test_wbstats PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_wbstats PRIVATE Qt6::Core)

enable_testing()
add_test(NAME formula COMMAND test_formula)
add_test(NAME model COMMAND test_model)
add_test(NAME io COMMAND test_io)
add_test(NAME view COMMAND test_view)
add_test(NAME version COMMAND test_version)
add_test(NAME i18n COMMAND test_i18n)
add_test(NAME numfmt COMMAND test_numfmt)
add_test(NAME shortcuts COMMAND test_shortcuts)
add_test(NAME zoom COMMAND test_zoom)
add_test(NAME sheetnav COMMAND test_sheetnav)
add_test(NAME cellmode COMMAND test_cellmode)
add_test(NAME wbstats COMMAND test_wbstats)

# --- Sort (sắp xếp vùng dữ liệu) — module logic thuần (Qt Core) -> static lib + test.
add_library(ezcel_sort STATIC src/model/Sort.cpp src/model/Sort.h)
target_include_directories(ezcel_sort PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_sort PUBLIC Qt6::Core)

add_executable(test_sort tests/test_sort.cpp)
target_link_libraries(test_sort PRIVATE ezcel_sort Qt6::Core)
add_test(NAME sort COMMAND test_sort)
