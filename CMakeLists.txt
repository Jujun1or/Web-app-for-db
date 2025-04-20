cmake_minimum_required(VERSION 3.15)
project(library_app)

add_definitions(-D_WIN32_WINNT=0x0601)

# Явное указание путей vcpkg
set(VCPKG_ROOT "C:/Users/rmedv/Data_base/Web-app-for-db/vcpkg" CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

# Настройка генератора
set(CMAKE_GENERATOR "Visual Studio 17 2022" CACHE STRING "" FORCE)
set(CMAKE_GENERATOR_PLATFORM x64 CACHE STRING "" FORCE)

# Настройки компилятора
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Поиск зависимостей
find_package(restinio REQUIRED)
find_package(libpqxx CONFIG REQUIRED) 
find_package(nlohmann_json REQUIRED)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/db/*.cpp"
    "src/server/*.cpp"
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    restinio::restinio   
    libpqxx::pqxx            
    nlohmann_json::nlohmann_json
    ws2_32
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/frontend/static
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/frontend/static
)