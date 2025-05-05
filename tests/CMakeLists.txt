# tests/CMakeLists.txt
#
# Собираем и регистрируем unit-тесты GoogleTest

# Ищем GTest
find_package(GTest REQUIRED)

# Здесь перечисляем только тестовые файлы
set(TEST_SOURCES
    test_config.cpp
    test_file_tape.cpp
    test_counting_sort.cpp
    test_chunk_merge_sort.cpp
    test_main.cpp
)

# Чтобы не дублировать реализацию, подключаем исходники алгоритмов и FileTape
set(CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/config.cpp
    ${PROJECT_SOURCE_DIR}/src/file_tape.cpp
    ${PROJECT_SOURCE_DIR}/src/counting_sort.cpp
    ${PROJECT_SOURCE_DIR}/src/chunk_merge_sort.cpp
)

# Главный тестовый бинарник
add_executable(unit_tests
    ${TEST_SOURCES}
    ${CORE_SOURCES}
)

# Пути к заголовкам
target_include_directories(unit_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Линкуем с GTest, yaml-cpp
target_link_libraries(unit_tests
    PRIVATE
        GTest::gtest_main
        yaml-cpp
)

# Регистрируем тесты для CTest
include(GoogleTest)
gtest_discover_tests(unit_tests
    PROPERTIES
        LABELS "unit"
)

# При запуске `ctest --output-on-failure` все тесты из unit_tests будут выполнены
