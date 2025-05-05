set(BUILD_SHARED_LIBS OFF CACHE BOOL \"Build all libraries static\")

cmake_minimum_required(VERSION 3.10)
project(ExternalTapeSort LANGUAGES CXX)

# Устанавливаем стандарт C++17 без расширений
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Находим внешние зависимости
find_package(yaml-cpp CONFIG REQUIRED)

# Собираем основное консольное приложение
add_executable(tape_sort
    src/main.cpp
    src/config.cpp
    src/file_tape.cpp
    src/counting_sort.cpp
    src/chunk_merge_sort.cpp
)

# Пути к вашим заголовкам
target_include_directories(tape_sort
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Линкуем библиотеки к основному бинарю
target_link_libraries(tape_sort
    PRIVATE 
        yaml-cpp::yaml-cpp
)

# Включаем поддержку тестов
enable_testing()

# Флаг для сборки тестов (по умолчанию — вкл.)
option(BUILD_TESTS "Build unit tests" ON)

# Если включены тесты — подключаем подкаталог tests/
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Опциональное правило установки
# install(TARGETS tape_sort RUNTIME DESTINATION bin)
