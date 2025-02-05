#pragma once

#include <raylib.h>
#include <string>
#include <iostream>

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

// if pathes are not defined we are running it via VS, so load resources normally
#if !defined(SOUNDS_PATH) || !defined(IMAGES_PATH) || !defined(GAMES_PATH)

#define SOUNDS_PATH ./Resources/Sounds/
#define IMAGES_PATH ./Resources/Images/
#define GAMES_PATH ./Resources/Games/

#endif

class AssetManager
{
public:

	const std::string audioPath = EXPAND(SOUNDS_PATH);
	const std::string imagePath = EXPAND(IMAGES_PATH);
	const std::string gamePath = EXPAND(GAMES_PATH);

	Image piecesImage;

	Sound startSound;
	Sound endSound;
	Sound moveSound;
	Sound captureSound;
	Sound checkSound;
	Sound promotionSound;
	Sound castlingSound;

	AssetManager()
	{
		std::cout << audioPath << " " << imagePath << " " << gamePath << "\n";

		piecesImage = LoadImage((imagePath + "chessPieces.png").c_str());

		InitAudioDevice();

		startSound = LoadSound((audioPath + "game-start.wav").c_str());
		endSound = LoadSound((audioPath + "game-end.wav").c_str());
		moveSound = LoadSound((audioPath + "move-self.wav").c_str());
		checkSound = LoadSound((audioPath + "move-check.wav").c_str());
		captureSound = LoadSound((audioPath + "capture.wav").c_str());
		promotionSound = LoadSound((audioPath + "promote.wav").c_str());
		castlingSound = LoadSound((audioPath + "castle.wav").c_str());
	}

	~AssetManager()
	{
		UnloadImage(piecesImage);

		UnloadSound(startSound);
		UnloadSound(endSound);
		UnloadSound(moveSound);
		UnloadSound(captureSound);
		UnloadSound(checkSound);
		UnloadSound(promotionSound);
		UnloadSound(castlingSound);

	}
};