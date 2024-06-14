#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "render_system.hpp"

class AISystem
{
private:
	std::vector<vec2> summonPosReq;
	std::vector<ENEMY_TYPE_ID> summonTypeReq;
public:
	void step(float elapsed_ms, RenderSystem* renderer);
	void handleTreeAI(Entity& entity);
	void handleBirdAI(Entity& entity, RenderSystem* renderer);
	void handleBirdmanAI(Entity& entity, RenderSystem* renderer);
	void handleDiveBirdAI(Entity& entity);
	void handleGuardAI(Entity& entity);
	void handleGuardCaptainAI(Entity& entity, RenderSystem* renderer);
	void handleGuardSummonAI(Entity& entity);
	void handleBossAI(Entity& entity, RenderSystem* renderer);

	void templateMobAI(Entity& entity, RenderSystem* renderer);

	void doPatrol(Motion& motion, Deadly& deadly, Mob& mob, Physics& physics, Patrol& patrol);
};


/* ENEMY NOTES:

CURRENT ENEMY TYPES:
 - TREE: uses basic handleTreeAI. Stays in spawn position, will chase player if detected.
 - BIRD: Flies. Patrols around spawn position. 
 - GUARD: Patrols around spawn position.
 - GUARDCAPTAIN: Similar to guard, except will summon two GUARDSUMMONS to chase the player.
 - GUARDSUMMONS: Strictly chases the player. Does not have a detection mechanic, has to be defeated.
 - BIRDMAN: Flies. Stays in position. Throws feathers at the player.
 - RAT: Use TreeAI.


*/