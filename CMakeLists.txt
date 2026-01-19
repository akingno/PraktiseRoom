cmake_minimum_required(VERSION 3.27)
project(room_temp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

add_subdirectory("third_party/SDL" "${CMAKE_BINARY_DIR}/_deps/SDL-build" EXCLUDE_FROM_ALL)

set(SDL3IMAGE_PNG ON CACHE BOOL "" FORCE)
set(SDL3IMAGE_VENDORED ON CACHE BOOL "" FORCE)

add_subdirectory("third_party/SDL_image" "${CMAKE_BINARY_DIR}/_deps/SDL_image-build" EXCLUDE_FROM_ALL)

set(SDL3TTF_VENDORED ON CACHE BOOL "" FORCE)

add_subdirectory("third_party/SDL_ttf" "${CMAKE_BINARY_DIR}/_deps/SDL_ttf-build" EXCLUDE_FROM_ALL)

add_executable(room_temp main.cpp
        Room.h
        Room.cpp
        tools/Random.cpp
        tools/Utils.h
        renders/IRender.h
        renders/ASCIIRender.cpp
        renders/ASCIIRender.h
        Blackboard.h
        tools/IPathfinder.h
        tools/AStarPathfinder.h
        ActionExecutor.h
        ActionExecutor.cpp
        tools/AStarPathfinder.cpp
        renders/SDL3Render.cpp
        renders/SDL3Render.h
        Config.h
        Item.h
        ItemBuilder.cpp
        ItemBuilder.h
        ItemRegistry.h
        GameContentInit.h
        ItemLayer.cpp
        ItemLayer.h
        ShortMemory.h
        actions/Action.h
        actions/MoveToAction.h
        actions/InteractAction.h
        actions/WaitAction.h
        actions/SequenceAction.h
        actions/MoveToAction.cpp
        actions/InteractAction.cpp
        Agent.h
        actions/SignalChatAction.h
        actions/TransferMemoryAction.h
        actions/WaitForChatAction.h
        actions/SelectAgentAction.h
        actions/ActionFactory.cpp
        actions/ActionFactory.h
        Brain.cpp
        Brain.h
)

set_target_properties(room_temp PROPERTIES WIN32_EXECUTABLE OFF)

target_include_directories(room_temp PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(room_temp PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
)
target_compile_definitions(room_temp PRIVATE _WIN32_WINNT=0x0A00)