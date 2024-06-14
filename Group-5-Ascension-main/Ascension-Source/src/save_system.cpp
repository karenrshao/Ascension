#include "save_system.hpp"
#include <fstream>
#include <tiny_ecs_registry.hpp>
#include "components.hpp"

SaveData saveData;

void SaveData::init() {
	// try loading game file
	std::fstream gameFile(data_path);

	if (gameFile.is_open()) {
		gameFile >> gameData;

		gameFile.close();
	}
	
	// try loading respawn file
	std::fstream respawnFile(respawn_path);

	if (respawnFile.is_open()) {
		respawnFile >> lastSavedGameState;

		respawnFile.close();
	}

}

void SaveData::saveLevel() {
	GameManager& gm = registry.gameManagers.components[0];

	// LEVELS
	json levelData;

	json enemy_array = json::array();
	json item_array = json::array();
	json breakables_array = json::array();
	for (auto& entity : registry.levelElements.entities) {
		vec2 ePos;
		vec2 eScale;

		if (registry.motions.has(entity)) {
			ePos = registry.motions.get(entity).position;
			eScale = registry.motions.get(entity).scale;
		}

		if (registry.deadlys.has(entity)) {
			Deadly& deadly = registry.deadlys.get(entity);
			json enemyData = {
				{ "position", {
						{"x", ePos.x},
						{"y", ePos.y}
				}},
				{
					"type", (int)deadly.type
				}
			};
			enemy_array.push_back(enemyData);
		}

		if (registry.items.has(entity)) {
			json itemData = {
				{ "type", (int) registry.items.get(entity).item_id },
				{ "sprite", (int)registry.renderRequests.get(entity).used_texture },
				{ "position", {
					{"x", ePos.x},
					{"y", ePos.y}
				}},
			};
			item_array.push_back(itemData);
		}

		// breakables
		if (registry.breakables.has(entity) && !registry.summonables.has(entity)) {
			Breakable& b = registry.breakables.get(entity);
			json breakableData = {
				{ "hp", b.hp },
				{ "sprite", (int) registry.renderRequests.get(entity).used_texture },
				{ "scale", {
					{"x", eScale.x},
					{"y", eScale.y}
				}},
				{ "position", {
					{"x", ePos.x},
					{"y", ePos.y}
				}}
			};
			breakables_array.push_back(breakableData);
		}
	}
	levelData["enemy"] = enemy_array;
	levelData["items"] = item_array;
	levelData["breakables"] = breakables_array;

	gameData["levels"][(int) gm.currentLevel] = levelData;
}

void SaveData::savePlayer() {
	GameManager& gm = registry.gameManagers.components[0];
	gameData["current_level"] = (int) gm.currentLevel;

	QuestManager& qm = registry.questmanagers.components[0];

	json quest_array = json::array();
	for (int questIndex : qm.openQuests) {
		int stage = (int) questRegistry.quest_list[questIndex]->stage;
		quest_array.push_back(
			{{ std::to_string(questIndex), stage }}
		);
	}
	json questData = {
		{"open_quests", quest_array},
		{"closed_quests", qm.completedQuests},
		{"conditions", qm.conditions}
	};
	gameData["quests"] = questData;

	Entity player = registry.players.entities[0];
	vec2 playerPosition = registry.motions.get(player).position;
	
	// PLAYER DATA
	json playerData = {
		{
			"health", registry.players.get(player).health
		},
		{
			"position", 
				{
					{"x", playerPosition.x},
					{"y", playerPosition.y}
				}
		}
	};

	gameData["player"] = playerData;

}

void SaveData::saveGame() {
	std::ofstream saveFile(data_path);
	saveFile << std::setw(4) << gameData << std::endl;
	saveFile.close();

}

void SaveData::setSpawn() {
	savePlayer();
	saveLevel();
	lastSavedGameState = gameData;
	saveSpawn();
}

void SaveData::saveSpawn() {
	std::ofstream saveFile(respawn_path);
	saveFile << std::setw(4) << lastSavedGameState << std::endl;
	saveFile.close();
}