cmake_minimum_required(VERSION 3.0)
set(CMAKE_BUILD_TYPE Release)
project(HoistControl_app_project)
include_directories(libs)
add_executable(
	masterProcess
	masterProcess.c
	)
add_executable(
	watchdog
	watchdog.c
)
add_executable(
	userConsole
	userConsole.c
	)