// TTT_AnimExtraction.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets.

#pragma once

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>

#include "Utils.hpp"
#include "constants.hpp"

// Structures and data types

typedef uint64_t gameAddr;

typedef std::vector<float> AnimationKeyframe;

struct Animation
{
	unsigned int last_saved_max_frame = 1;
	unsigned int max_frame;
	std::vector<bool> obtained_frames;
	std::map<unsigned int, AnimationKeyframe> keyframes;
};

struct Player {
	unsigned int m_prevAnimationId = 1;
	unsigned int m_currentAnimationId = 0;
	unsigned int m_prevFrame = 0;
	unsigned int m_currentFrame = 1;
	bool m_mustExtract = false;

	struct {
		gameAddr addr;
		unsigned int max_frame;
	} m_currentMove;

	struct {
		std::map<unsigned int, Animation> pool;
		std::set<unsigned int> extracted;
		Animation* current = nullptr;
	} m_animations;

	unsigned int id;
};

class Game
{
private:
	HANDLE m_processHandle;

	Player  m_players[PLAYER_COUNT];

	void SaveAnim(Player& player, unsigned int max_frame, bool partial_anim = false);
	void SaveCurrentKeyframe(Player& player);
	void OnNewMove(Player& player);
	void UpdateCurrentMove(Player& player);

	DWORD GetGamePID(const char* processName);
	float readFloat(gameAddr addr) const;
	uint32_t readUInt32(gameAddr addr) const;
	uint16_t readUInt16(gameAddr addr) const;
public:
	Game();
	void ProcessPlayer(Player& player);
	void Mainloop();
	void LoadProcess();
};

// TODO: Référencez ici les en-têtes supplémentaires nécessaires à votre programme.
