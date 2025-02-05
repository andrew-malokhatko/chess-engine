add_library( Core

	ChessBoardCore.cpp
    ChessBoardMasks.cpp
    ChessBoardMoves.cpp
    Zobrist.cpp
    
    ChessBoard.h
    Zobrist.h

    ChessBoardConsts.h
    GameState.h
    Masks.h
    Move.h
    Pieces.h
    Debug.h
)


target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})