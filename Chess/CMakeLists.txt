add_library (Chess
	GameManager.cpp
	GameManager.h
)

target_include_directories(Chess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Chess PUBLIC Core AI Tests)
