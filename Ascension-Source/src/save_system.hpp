#pragma once

// internal
#include "common.hpp"
#include "components.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SaveData
{
public:
	const std::string data_path = save_path("save.json");
	const std::string respawn_path = save_path("respawn.json");
	SaveData() {
		init();
	}
	void init();
	void saveGame();
	void saveLevel();
	void savePlayer();

	void setSpawn();
	void saveSpawn();

	void loadGame(); // load from file into gameData

	json getGameData() {
		return gameData;
	}
	void setGameData(json data) {
		gameData = data;
	}

	json getRespawnData() {
		return lastSavedGameState;
	}
private:
	json gameData;
	json lastSavedGameState;
};

extern SaveData saveData;