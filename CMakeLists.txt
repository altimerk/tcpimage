cmake_minimum_required(VERSION 3.13)
project(TCP)

set(CMAKE_CXX_STANDARD 14)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system  thread)

if(Boost_FOUND)

    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS})

endif()

add_executable(server server.cpp)
target_link_libraries(server PRIVATE ${OpenCV_LIBS}   pthread  ${Boost_LIBRARIES})

add_executable(client client.cpp utils.cpp)
target_link_libraries(client PRIVATE ${OpenCV_LIBS}   pthread  ${Boost_LIBRARIES} )

add_executable(perf_test perfTestClient.cpp utils.cpp)
target_link_libraries(perf_test PRIVATE ${OpenCV_LIBS}   pthread  ${Boost_LIBRARIES} )











