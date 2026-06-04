cmake_minimum_required(VERSION 3.21)
project(Ezcel VERSION 1.91.0 LANGUAGES CXX)

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
    src/model/NumberFormat.cpp src/model/NumberFormat.h
    src/model/SeriesFill.cpp src/model/SeriesFill.h
    src/model/FlashFill.cpp src/model/FlashFill.h
    src/model/AutoComplete.cpp src/model/AutoComplete.h
    src/model/DataTools.cpp src/model/DataTools.h
    src/model/RefCycle.cpp src/model/RefCycle.h
    src/model/NameValidate.cpp src/model/NameValidate.h
    src/model/RangeParse.cpp src/model/RangeParse.h
    src/model/SeriesGen.cpp src/model/SeriesGen.h
    src/model/TextCase.cpp src/model/TextCase.h
    src/model/CellShift.cpp src/model/CellShift.h
    src/model/Consolidate.cpp src/model/Consolidate.h
    src/model/Forecast.cpp src/model/Forecast.h
    src/model/Sparkline.cpp src/model/Sparkline.h
    src/model/GoalSeek.cpp src/model/GoalSeek.h)
target_include_directories(ezcel_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_model PUBLIC ezcel_formula ezcel_sort Qt6::Core Qt6::Gui)

# --- I/O file (CSV + XLSX qua QXlsx) -> static lib.
add_library(ezcel_io STATIC src/io/Csv.cpp src/io/Csv.h src/io/Xlsx.cpp src/io/Xlsx.h)
target_include_directories(ezcel_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_io PUBLIC Qt6::Core Qt6::Gui QXlsx::QXlsx)

# --- Tiện ích view (span ô gộp...) — cần Widgets -> static lib (test headless được).
add_library(ezcel_view STATIC src/view/MergeSpans.cpp src/view/MergeSpans.h
    src/view/CellBorderDelegate.cpp src/view/CellBorderDelegate.h
    src/view/Visibility.cpp src/view/Visibility.h
    src/view/HeaderMenu.cpp src/view/HeaderMenu.h
    src/view/Outline.cpp src/view/Outline.h
    src/view/CopyVisible.cpp src/view/CopyVisible.h
    src/view/FreezePanes.cpp src/view/FreezePanes.h
    src/view/FormulaHint.cpp src/view/FormulaHint.h
    src/ui/RibbonBar.cpp src/ui/RibbonBar.h)
target_include_directories(ezcel_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
# Delegate dùng hằng role + hàm thuần của model (sparkline...) -> phụ thuộc ezcel_model.
target_link_libraries(ezcel_view PUBLIC ezcel_model Qt6::Core Qt6::Gui Qt6::Widgets)

# --- Cập nhật (GitHub Releases) — cần Network + Widgets -> static lib.
add_library(ezcel_update STATIC
    src/update/VersionCompare.cpp src/update/VersionCompare.h
    src/update/Updater.cpp src/update/Updater.h)
target_include_directories(ezcel_update PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_update PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)

# --- App GUI ---
# app_icon.rc nhúng icon vào tệp .exe (hiện trên taskbar/Explorer/cửa sổ — Windows).
qt_add_executable(Ezcel WIN32 src/main.cpp src/MainWindow.cpp src/MainWindowFormat.cpp src/MainWindowFind.cpp src/ui/I18n.cpp src/ui/Shortcuts.cpp src/ui/Zoom.cpp src/ui/SheetNav.cpp src/ui/CellMode.cpp src/ui/WorkbookStats.cpp src/MainWindow.h resources.qrc app_icon.rc)
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

add_executable(test_seriesfill tests/test_seriesfill.cpp src/model/SeriesFill.cpp)
target_include_directories(test_seriesfill PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_seriesfill PRIVATE Qt6::Core)

add_executable(test_flashfill tests/test_flashfill.cpp src/model/FlashFill.cpp)
target_include_directories(test_flashfill PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_flashfill PRIVATE Qt6::Core)

add_executable(test_autocomplete tests/test_autocomplete.cpp src/model/AutoComplete.cpp)
target_include_directories(test_autocomplete PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_autocomplete PRIVATE Qt6::Core)

add_executable(test_datatools tests/test_datatools.cpp src/model/DataTools.cpp)
target_include_directories(test_datatools PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_datatools PRIVATE Qt6::Core)

add_executable(test_cellshift tests/test_cellshift.cpp src/model/CellShift.cpp)
target_include_directories(test_cellshift PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_cellshift PRIVATE Qt6::Core)

add_executable(test_consolidate tests/test_consolidate.cpp src/model/Consolidate.cpp src/model/DataTools.cpp)
target_include_directories(test_consolidate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_consolidate PRIVATE Qt6::Core)

add_executable(test_forecast tests/test_forecast.cpp src/model/Forecast.cpp)
target_include_directories(test_forecast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_forecast PRIVATE Qt6::Core)

add_executable(test_sparkline tests/test_sparkline.cpp src/model/Sparkline.cpp)
target_include_directories(test_sparkline PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_sparkline PRIVATE Qt6::Core)

add_executable(test_goalseek tests/test_goalseek.cpp src/model/GoalSeek.cpp)
target_include_directories(test_goalseek PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_goalseek PRIVATE Qt6::Core)

add_executable(test_headermenu tests/test_headermenu.cpp src/view/HeaderMenu.cpp)
target_include_directories(test_headermenu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_headermenu PRIVATE Qt6::Core)

add_executable(test_outline tests/test_outline.cpp src/view/Outline.cpp)
target_include_directories(test_outline PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_outline PRIVATE Qt6::Core)

add_executable(test_copyvisible tests/test_copyvisible.cpp src/view/CopyVisible.cpp)
target_include_directories(test_copyvisible PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_copyvisible PRIVATE Qt6::Core)

add_executable(test_refcycle tests/test_refcycle.cpp src/model/RefCycle.cpp)
target_include_directories(test_refcycle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_refcycle PRIVATE Qt6::Core)

add_executable(test_namevalidate tests/test_namevalidate.cpp src/model/NameValidate.cpp)
target_include_directories(test_namevalidate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_namevalidate PRIVATE Qt6::Core)

add_executable(test_rangeparse tests/test_rangeparse.cpp)
target_link_libraries(test_rangeparse PRIVATE ezcel_model Qt6::Core Qt6::Gui)

add_executable(test_seriesgen tests/test_seriesgen.cpp src/model/SeriesGen.cpp)
target_include_directories(test_seriesgen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_seriesgen PRIVATE Qt6::Core)

add_executable(test_textcase tests/test_textcase.cpp src/model/TextCase.cpp)
target_include_directories(test_textcase PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_textcase PRIVATE Qt6::Core)

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
add_test(NAME seriesfill COMMAND test_seriesfill)
add_test(NAME flashfill COMMAND test_flashfill)
add_test(NAME autocomplete COMMAND test_autocomplete)
add_test(NAME datatools COMMAND test_datatools)
add_test(NAME headermenu COMMAND test_headermenu)
add_test(NAME outline COMMAND test_outline)
add_test(NAME copyvisible COMMAND test_copyvisible)
add_test(NAME refcycle COMMAND test_refcycle)
add_test(NAME namevalidate COMMAND test_namevalidate)
add_test(NAME rangeparse COMMAND test_rangeparse)
add_test(NAME seriesgen COMMAND test_seriesgen)
add_test(NAME textcase COMMAND test_textcase)
add_test(NAME cellshift COMMAND test_cellshift)
add_test(NAME consolidate COMMAND test_consolidate)
add_test(NAME forecast COMMAND test_forecast)
add_test(NAME sparkline COMMAND test_sparkline)
add_test(NAME goalseek COMMAND test_goalseek)

# --- Sort (sắp xếp vùng dữ liệu) — module logic thuần (Qt Core) -> static lib + test.
add_library(ezcel_sort STATIC src/model/Sort.cpp src/model/Sort.h)
target_include_directories(ezcel_sort PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ezcel_sort PUBLIC Qt6::Core)

add_executable(test_sort tests/test_sort.cpp)
target_link_libraries(test_sort PRIVATE ezcel_sort Qt6::Core)
add_test(NAME sort COMMAND test_sort)
