include(AMXConfig)

add_subdirectory(amx)

file(GLOB_RECURSE OMPLOGGER_FILES
    *.hpp
    *.cpp
)

add_library(${PROJECT_NAME} SHARED ${OMPLOGGER_FILES})

include_directories(
    ../deps
    ../deps/pawn/source
    ../deps/pawn/source/linux
)

GroupSourcesByFolder(${PROJECT_NAME})

if (UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/..:$ORIGIN/")

    set(CMAKE_CXX_FLAGS "-Wno-int-to-pointer-cast -Wno-attributes ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "-Wno-int-to-pointer-cast -Wno-attributes ${CMAKE_C_FLAGS}")

    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
endif ()

target_link_libraries(${PROJECT_NAME}
    ${LIBRARIES}
    OMP-SDK
    fmt
    amx
    omp-logger-sdk
)