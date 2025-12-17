cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(nrf_pcnt)

target_sources(app PRIVATE src/example.c)
target_sources(app PRIVATE src/nrf52_pcnt.c)