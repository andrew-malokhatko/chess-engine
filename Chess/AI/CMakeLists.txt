add_library( AI
	AI.cpp
	Book.cpp
	BookParser.cpp
	Evaluation.cpp
	Search.cpp

	AI.h
	Book.h
	BookParser.h
	Evaluation.h
	Search.h

	PieceSquareTables.h
	Sort.h
)

target_include_directories(AI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(AI PUBLIC Core)
