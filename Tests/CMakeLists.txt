add_library (Tests
	Tests.cpp
	Tests.h
)

target_include_directories(Tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Tests PUBLIC Core)