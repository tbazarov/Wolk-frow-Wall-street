cmake_minimum_required(VERSION 3.14)
project(Wall_street)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(GTest REQUIRED)
add_executable(WallStreet
  Broker.cpp 
  Exchange.cpp 
  main.cpp 
  tests.cpp
  # BigWinBroker.cpp
)

target_link_libraries(WallStreet GTest::gtest GTest::gtest_main)