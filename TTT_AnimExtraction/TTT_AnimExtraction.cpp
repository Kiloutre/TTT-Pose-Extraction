#include <fstream>

#include "TTT_AnimExtraction.hpp"

void DeleteIfPartialAnimExists(const std::string& folder, unsigned int anim_id)
{
	std::string prefix = std::to_string(anim_id) + "_";
	for (const auto& entry : std::filesystem::directory_iterator(folder))
	{
		std::string filename = entry.path().string();

		if (!endsWith<std::string>(filename, ".tttposes_partial")) {
			continue;
		}

		filename.erase(0, filename.find_last_of('/') + 1);
		unsigned int file_anim_id = std::atoi(filename.c_str());

		if (file_anim_id == anim_id) {
			try {
				std::filesystem::remove(entry.path());
			}
			catch (const std::exception&) {
				printf("Failed to remove %s\n", entry.path().string().c_str());
			}
			return;
		}
	}
}

Game::Game()
{
	CreateDirectoryA(EXTRACTION_FOLDER, nullptr);

	for (unsigned int i = 0; i < PLAYER_COUNT; ++i)
	{
		Player& player = m_players[i];
		player.id = i;

		std::string player_folder = EXTRACTION_FOLDER;
		player_folder += "P" + std::to_string(i + 1) + "/";

		CreateDirectoryA(player_folder.c_str(), nullptr);

		for (const auto& entry : std::filesystem::directory_iterator(player_folder))
		{
			std::string filename = entry.path().string();

			if (!endsWith<std::string>(filename, ".tttposes")) {
				//printf("%s\n", filename.c_str());
				continue;
			}

			filename.erase(0, filename.find_last_of('/') + 1);
			unsigned int animation_id = std::atoi(filename.c_str());
			player.m_animations.extracted.insert(animation_id);

			printf("(P%u) Found existing animation [%u], will be ignored during extraction.\n", player.id, animation_id);
		}
	}

}

DWORD Game::GetGamePID(const char* processName)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32{ 0 };
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	DWORD pid = (DWORD)-1;

	if (GetLastError() != ERROR_ACCESS_DENIED)
	{
		pe32.dwSize = sizeof(PROCESSENTRY32W);
		if (Process32First(hProcessSnap, &pe32)) {
			if (strcmp(pe32.szExeFile, processName) == 0) {
				pid = pe32.th32ProcessID;
			}
			else {
				while (Process32Next(hProcessSnap, &pe32)) {
					if (strcmp(pe32.szExeFile, processName) == 0) {
						pid = pe32.th32ProcessID;
						break;
					}
				}
			}
		}
		CloseHandle(hProcessSnap);
	}

	return pid;
}


uint32_t Game::readUInt32(gameAddr addr) const
{
	uint32_t value{ (uint32_t)-1 };
	ReadProcessMemory(m_processHandle, (LPCVOID)addr, (LPVOID)&value, 4, nullptr);
	return value;
}


uint16_t Game::readUInt16(gameAddr addr) const
{
	uint16_t value{ (uint16_t)-1 };
	ReadProcessMemory(m_processHandle, (LPCVOID)addr, (LPVOID)&value, 2, nullptr);
	return value;
}


float Game::readFloat(gameAddr addr) const
{
	float value{ (float)-1 };
	ReadProcessMemory(m_processHandle, (LPCVOID)addr, (LPVOID)&value, 4, nullptr);
	return value;
}

void Game::LoadProcess()
{
	DWORD pid = GetGamePID("pcsx2.exe");
	if (pid != (DWORD)-1) {
		m_processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (m_processHandle == nullptr) {
			printf("-- FAILED TO OPEN PROCESS (lacking perms?) --\n");
			throw;
		}
	}
	else {
		printf("-- FAILED TO FIND PID --\n");
		throw;
	}
}

void Game::UpdateCurrentMove(Player& player)
{
	player.m_currentAnimationId = readUInt16(BASE_ADDRESS + 0x87974C + PLAYER_OFFSET * player.id);
	player.m_currentFrame = readUInt32(BASE_ADDRESS + 0x8796FC + PLAYER_OFFSET * player.id);
}

void Game::OnNewMove(Player& player)
{
	// New animation
	player.m_currentMove.addr = readUInt32(BASE_ADDRESS + 0x884C28 + PLAYER_OFFSET * player.id);
	player.m_currentMove.max_frame = readUInt16(BASE_ADDRESS + player.m_currentMove.addr + 0x18);
	player.m_prevAnimationId = player.m_currentAnimationId;

	if (player.m_animations.extracted.contains(player.m_currentAnimationId)) {
		printf("(P%u) %u: Already fully extracted, not extracting.\n", player.id, player.m_currentAnimationId);
		player.m_mustExtract = false;
		return;
	}


	if (!player.m_animations.pool.contains(player.m_currentAnimationId))
	{
		printf("(P%u) - New anim: - %u, addr %llx, max frame %u\n", player.id, player.m_currentAnimationId, player.m_currentMove.addr, player.m_currentMove.max_frame);
		player.m_animations.pool[player.m_currentAnimationId] = {
			.max_frame = player.m_currentMove.max_frame,
			.obtained_frames = std::vector<bool>(player.m_currentMove.max_frame, false),
			.keyframes = {}
		};
	}
	else {
		printf("(P%u) - Curr anim: - %u, addr %llx, max frame %u\n", player.id, player.m_currentAnimationId, player.m_currentMove.addr, player.m_currentMove.max_frame);
	}
	player.m_animations.current = &player.m_animations.pool[player.m_currentAnimationId];
	player.m_mustExtract = true;

}


void Game::SaveAnim(Player& player, unsigned int max_frame, bool partial_anim)
{
	if (!partial_anim) {
		player.m_animations.extracted.insert(player.m_currentAnimationId);
	}


	// write to file
	{
		CreateDirectoryA(EXTRACTION_FOLDER, nullptr);

		std::string filename = EXTRACTION_FOLDER;
		filename += "P" + std::to_string(player.id + 1) + "/" + std::to_string(player.m_currentAnimationId) + (partial_anim ? ".tttposes_partial" : ".tttposes");

		std::ofstream anim_file(filename, std::ios::binary);
		anim_file << "TTTP";
		anim_file.write((const char*)&max_frame, 4);

		for (unsigned int frame = 1; frame <= max_frame; ++frame)
		{
			for (auto value : player.m_animations.current->keyframes[frame])
			{
				anim_file.write((const char*)&value, 4);
			}
		}
	}


	if (!partial_anim)
	{
		printf("(P%u) %u: Finished!\n", player.id, player.m_currentAnimationId);
		std::string anim_folder = EXTRACTION_FOLDER;
		anim_folder += "P" + std::to_string(player.id + 1) + "/";
		DeleteIfPartialAnimExists(anim_folder, player.m_currentAnimationId);
	}
	else {
		printf("(P%u) %u: Saving partial anim (%u frames)\n", player.id, player.m_currentAnimationId, max_frame);
	}
}

void Game::SaveCurrentKeyframe(Player& player)
{
	std::vector<float> frame_bones;

	for (gameAddr relative_bone_addr : BONE_OFFSETS)
	{
		gameAddr absolute_bone_addr = BASE_ADDRESS + relative_bone_addr;

		for (unsigned int i = 0; i < 3; ++i)
		{
			frame_bones.push_back(readFloat(absolute_bone_addr));
			absolute_bone_addr += sizeof(float);
		}
	}

	{
		// Securities
		unsigned int currentFrame = readUInt32(BASE_ADDRESS + 0x8796FC + PLAYER_OFFSET * player.id);
		if (player.m_currentFrame != currentFrame) {
			printf("(P%u) !!!! m_currentFrame != currentFrame !!!! (%u vs %u)", player.id, player.m_currentFrame, currentFrame);
			// Ensure that the current frame hasn't changed to validate the data we obtained (if the game happens to advance its frame counter during our reading)
			return;
		}

		unsigned int currentAnimationId = readUInt16(BASE_ADDRESS + 0x87974C + PLAYER_OFFSET * player.id);
		if (player.m_currentAnimationId != currentAnimationId) {
			printf("(P%u) !!!! m_currentAnimationId != currentAnimationId !!!! (%u vs %u)", player.id, player.m_currentAnimationId, currentAnimationId);
			// Ensure that the current move hasn't changed to validate the data we obtained (if the game happens to change move during our reading)
			return;
		}
	}

	auto& obtained_frames = player.m_animations.current->obtained_frames;

	obtained_frames[player.m_currentFrame - 1] = true;
	player.m_animations.current->keyframes[player.m_currentFrame] = frame_bones;

	// Check if animation is complete
	std::vector<unsigned int> missing_frames;
	auto iter = obtained_frames.begin();
	for (size_t i = 0; i < obtained_frames.size(); ++i)
	{
		auto curr_iter = iter + i;
		if (!*curr_iter) {
			missing_frames.push_back(i + 1);
		}
	}

	if (missing_frames.size() == 0) {
		SaveAnim(player, player.m_animations.current->max_frame);
	}
	else if (missing_frames[0] != 1 && player.m_currentFrame == (player.m_animations.current->last_saved_max_frame + 1)) {
		SaveAnim(player, missing_frames[0], true);
		player.m_animations.current->last_saved_max_frame = player.m_currentFrame;
	}
	else {
		if (missing_frames.size() < 10) {
			std::string missing_text;
			for (auto f : missing_frames) {
				missing_text += std::to_string(f) + " ";
			}

			printf("(P%u) %u: Obtained frame %u. Remaining: [ %s]\n", player.id, player.m_currentAnimationId, player.m_currentFrame, missing_text.c_str());
		}
		else {
			printf("(P%u) %u: Obtained frame %u. Remaining: %llu frames\n", player.id, player.m_currentAnimationId, player.m_currentFrame, missing_frames.size());
		}
	}
}


void Game::ProcessPlayer(Player& player)
{
	player.m_prevFrame = player.m_currentFrame;
	UpdateCurrentMove(player);

	if (player.m_currentAnimationId != player.m_prevAnimationId) {
		OnNewMove(player);
	}
	else if (player.m_prevFrame == player.m_currentFrame) {
		// Same anim & frame as last loop run, skip
		return;
	}

	if (!player.m_mustExtract) return;

	if ((player.m_currentFrame - 1) >= player.m_animations.current->obtained_frames.size() || player.m_animations.current->obtained_frames[player.m_currentFrame - 1]) {
		// This keyframe was already saved, ignore it
		return;
	}

	SaveCurrentKeyframe(player);
}

void Game::Mainloop()
{
	while (true)
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));

		for (unsigned int i = 0; i < PLAYER_COUNT; ++i) {
			ProcessPlayer(m_players[i]);
		}
	}
}

int main()
{
	Game game;
	try {
		game.LoadProcess();
		printf("Successfully opened process\n");
	}
	catch (const std::exception&) {
		printf("Failed to open process\n");
		return 1;
	}

	game.Mainloop();

	return 0;
}
