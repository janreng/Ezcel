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

# Danh sách nguồn — bổ sung dần theo từng phase port.
set(EZCEL_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/formula/Formula.cpp
    src/model/SpreadsheetModel.cpp
)

set(EZCEL_HEADERS
    src/MainWindow.h
    src/formula/Formula.h
    src/model/SpreadsheetModel.h
)

qt_add_executable(Ezcel WIN32 ${EZCEL_SOURCES} ${EZCEL_HEADERS})

target_include_directories(Ezcel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Ezcel PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg)

# Tổ chức: app bảng tính, sau này thêm QXlsx (P4) qua FetchContent.
