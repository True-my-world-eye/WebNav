# tests/CMakeLists.txt
# 单元测试
find_package(Qt6 REQUIRED COMPONENTS Test)

function(add_webnav_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
        WebNav_core
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# 单元测试
add_webnav_test(test_database_manager   unit/test_database_manager.cpp)
add_webnav_test(test_link_repository    unit/test_link_repository.cpp)
add_webnav_test(test_folder_repository  unit/test_folder_repository.cpp)
add_webnav_test(test_tag_repository     unit/test_tag_repository.cpp)
add_webnav_test(test_crypto_service     unit/test_crypto_service.cpp)
add_webnav_test(test_bookmark_importer  unit/test_bookmark_importer.cpp)
