add_library(Components
    Arrow.cpp
    Arrow.h
    Button.h
    FunctionalButton.h
    ToggleButton.h
    ToggleTextButton.h
    Input.h
)

target_include_directories(Components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Components PUBLIC Clipboard raylib)