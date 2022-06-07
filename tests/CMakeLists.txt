cmake_minimum_required(VERSION 3.5)
project(oti_examples)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/platform-dependent)

link_directories(/home/igor/src/trovemat_kiosk/Externals/libs)

add_executable(test_commands
    ${CMAKE_CURRENT_SOURCE_DIR}/test_commands/test_commands.c
)

add_executable(test_config
    ${CMAKE_CURRENT_SOURCE_DIR}/test_config/test_config.c
)

add_executable(test_emv
    ${CMAKE_CURRENT_SOURCE_DIR}/test_emv/test_emv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test_emv/polling_manager.c
)

add_executable(test_load_fw
    ${CMAKE_CURRENT_SOURCE_DIR}/test_load_fw/test_load_fw.c
)

add_executable(test_transparent
    ${CMAKE_CURRENT_SOURCE_DIR}/test_transparent/test_transparent.c
)

add_executable(test_nxp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_nxp_jcop/test_nxp_jcop.c
)

target_link_libraries(test_config oti pthread)
target_link_libraries(test_commands oti pthread)
target_link_libraries(test_emv oti pthread)
target_link_libraries(test_load_fw oti pthread)
target_link_libraries(test_transparent oti pthread)
target_link_libraries(test_nxp oti pthread)
