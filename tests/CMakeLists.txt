add_executable(smoke smoke.c)
target_link_libraries(smoke PRIVATE libpewter)

add_test(NAME smoke COMMAND smoke ${CMAKE_SOURCE_DIR}/../sample.sav)
