#include "quest_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

QuestSystem::QuestSystem() {
}

void QuestSystem::init(Entity& manager) {
	manager = manager;
	quests = QuestRegistry();
}

void QuestSystem::step(float elapsed_ms) {

}

void QuestSystem::checkQuests() {
	QuestManager& qm = registry.questmanagers.get(manager);
	// // 
	// for (int q : qm.openQuests) {
	// 	bool questComplete = true;
	// 	Quest tq = *quests.quest_list[q];
	// 	/*
	// 	map<ITEM, int>::iterator it;
	// 	for (it = tq.itemRequirements.begin(); it != tq.itemRequirements.end(); it++) {
	// 		if (qm.inventory[it->first] <= it->second) {
	// 			questComplete = false;
	// 		}
	// 	}
	// 	*/
	// 	// iterate through open items
	// 	// if requirements satisfied
	// 		// remove items from inventory
	// 		// pop quest
	// 		// emplace child quests
	// }
}