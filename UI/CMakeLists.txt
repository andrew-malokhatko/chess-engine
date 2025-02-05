add_library (UI
	Game.cpp
	GameDraw.cpp
	Utils.cpp

	Game.h
	Utils.h
	AssetManager.h
)

target_include_directories(UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(UI PUBLIC Chess Components)