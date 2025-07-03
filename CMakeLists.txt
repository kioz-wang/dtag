cmake_minimum_required(VERSION 3.20)
project(dtag VERSION 1.0)

set(CMAKE_C_STANDARD 11)
SET(CMAKE_c_FLAGS -Werror -Wall)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(${PROJECT_SOURCE_DIR}/chksum chksum_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/token token_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/logger logger_SOURCE)

add_library(${PROJECT_NAME} STATIC ${chksum_SOURCE} ${logger_SOURCE} dtag.c)
target_link_libraries(${PROJECT_NAME} md)
target_compile_definitions(${PROJECT_NAME} PUBLIC 
    __LOGGER_ENV__="log2stderr"
    __CHKSUM_MD5__
)

add_executable(${PROJECT_NAME}_cli dtag_cli.c ${token_SOURCE})
target_link_libraries(${PROJECT_NAME}_cli ${PROJECT_NAME})

add_executable(test_dtag test_dtag.c)
target_link_libraries(test_dtag ${PROJECT_NAME})

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)

install(TARGETS ${PROJECT_NAME})

set(CPACK_PACKAGE_VENDOR KiozWang)
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "${PROJECT_BINARY_DIR};/.git/;.gitignore;/.vscode/settings.json;/.cache/;/compile_commands.json")
include(CPack)
