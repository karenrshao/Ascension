// internal
#include "ai_system.hpp"
#include <iostream>
#include <world_init.hpp>



void AISystem::doPatrol(Motion& motion, Deadly& deadly, Mob& mob, Physics& physics, Patrol& patrol) {
	if (motion.scale.x > 0) {
		if (motion.position.x > deadly.patrolPosition.x - 300.f) {
			physics.targetVelocity.x = mob.moveSpeed / 2 * -1.0f;
		}
		else {
			if (patrol.reachEnd == false) {
				patrol.reachEnd = true;
				patrol.patrolTimer = 2500;
			}
			else {
				if (patrol.patrolTimer <= 0) {
					physics.targetVelocity.x = mob.moveSpeed / 2 * 1.0f;
					patrol.reachEnd = false;
				}
				else {
					physics.targetVelocity.x = 0;
				}
			}
		}
	}
	else {
		if (motion.position.x < deadly.patrolPosition.x + 300.f) {
			physics.targetVelocity.x = mob.moveSpeed / 2 * 1.0f;
		}
		else {
			if (patrol.reachEnd == false) {
				patrol.reachEnd = true;
				patrol.patrolTimer = 2500;
			}
			else {
				if (patrol.patrolTimer <= 0) {
					physics.targetVelocity.x = mob.moveSpeed / 2 * -1.0f;
					patrol.reachEnd = false;
				}
				else {
					physics.targetVelocity.x = 0;
				}
			}
		}
	}
}


void AISystem::step(float elapsed_ms, RenderSystem* renderer)
{
	for (Entity e : registry.deadlys.entities) {
		Deadly& ai = registry.deadlys.get(e);
		if (ai.type == ENEMY_TYPE_ID::TREE) {
			handleTreeAI(e);
		}
		else if (ai.type == ENEMY_TYPE_ID::BIRDMAN) {
			handleBirdmanAI(e, renderer);
		}
		else if (ai.type == ENEMY_TYPE_ID::RAT) {
			handleTreeAI(e);
		}
		else if (ai.type == ENEMY_TYPE_ID::BIRDO) {
			handleBirdAI(e, renderer);
		}
		else if (ai.type == ENEMY_TYPE_ID::DIVE_BIRDO) {
			handleDiveBirdAI(e);
		}
		else if (ai.type == ENEMY_TYPE_ID::GUARD) {
			handleGuardAI(e);
		} 
		else if (ai.type == ENEMY_TYPE_ID::GUARDCAPTAIN) {
			handleGuardCaptainAI(e, renderer);
		}
		else if (ai.type == ENEMY_TYPE_ID::GUARDSUMMON) {
			handleGuardSummonAI(e);
		} 
		else if (ai.type == ENEMY_TYPE_ID::BOSS) {
			handleBossAI(e, renderer);
		}
	}

	while (!summonPosReq.empty())
	{
		createEnemy(renderer, summonPosReq.back(), summonTypeReq.back());
		summonPosReq.pop_back();
		summonTypeReq.pop_back();
	}

}



void AISystem::handleTreeAI(Entity& entity) {
	Player& playerCom = registry.players.components[0];
	Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];
	GameManager& gm = registry.gameManagers.components[0];
	bool stealth = registry.stealth.has(player);


	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = playerPos - motion.position;
	float dist = glm::length(distVector);
	vec2 dir = distVector / dist;
	vec2 enemyDir = glm::vec2(motion.scale.x > 0 ? -1.0f : 1.0f, 0.0f);
	float dotProduct = glm::dot(enemyDir, dir);
	float adjustedAggroDistance = deadly.aggroDistance * glm::abs(dotProduct);

	
	if (deadly.state == ENEMY_STATE_ID::IDLE) {
		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (adjustedAggroDistance > dist && !gm.stealth && dotProduct > 0 && !gm.stealth) { // CHASE
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
			if (gm.enemiesInBattle.size() == 0 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.insert(entity);
		}
		else { // RETURN TO POSITION
			if (abs(motion.position.x - deadly.patrolPosition.x) > 30 || abs(motion.position.y - deadly.patrolPosition.y) > 30) {
				vec2 patrolVector = deadly.patrolPosition - motion.position;
				vec2 dir2 = glm::normalize(patrolVector);
				int xdir = (patrolVector.x > 0.f) - (patrolVector.x < 0.f);
				physics.targetVelocity.x = xdir * mob.moveSpeed;
				deadly.state = ENEMY_STATE_ID::IDLE;
			}
			else {
				physics.velocity.x = 0;
				physics.velocity.y = 0;
			}
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::ALERT) {
		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (deadly.aggroDistance > dist && !stealth) {
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
		}
		else if (deadly.alertTimer <= 0) {
			deadly.state = ENEMY_STATE_ID::IDLE;
			if (gm.enemiesInBattle.size() == 1 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.erase(entity);
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::CHASE) {
		if (deadly.aggroDistance < dist || gm.stealth) {
			deadly.state = ENEMY_STATE_ID::ALERT;
			deadly.alertTimer = ENEMY_ALERT_TIMER;
		}
	}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}
	
	return;
}


void AISystem::handleDiveBirdAI(Entity& entity) {
	/*Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];
	bool stealth = registry.stealth.has(player);
	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = motion.position - playerPos;
	float dist = sqrt(distVector.x * distVector.x + distVector.y * distVector.y);
	vec2 dir = glm::normalize(distVector);
	std::cout << "attack? " << deadly.attacking << std::endl;
	std::cout << "attack timer: " << deadly.attackTimer << std::endl;

	// beginning
	// deadly.attacking = false;
	// deadly.attackTimer = 0;

	if (deadly.aggroDistance > dist && !stealth && deadly.attackTimer <= 0) {
		deadly.attacking = false;
		deadly.attackTimer = 5000;
		deadly.originalPosition = motion.position;
	}

	if (deadly.attackTimer <= 0) {
		deadly.originalPosition = motion.position;
		vec2 distVector = motion.position - playerPos;
		float dist = sqrt(distVector.x * distVector.x + distVector.y * distVector.y);
		vec2 dir = glm::normalize(distVector);
		physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
		physics.targetVelocity.y = -dir.y * (mob.moveSpeed + 300);
		deadly.attackTimer = 5000;
	}
	if (physics.velocity.y == 0) {
		vec2 distVector = deadly.originalPosition - playerPos;
		float dist = sqrt(distVector.x * distVector.x + distVector.y * distVector.y);
		vec2 dir = glm::normalize(distVector);
		physics.targetVelocity.x = -dir.x * (mob.moveSpeed + 100);
		physics.targetVelocity.y = dir.y * (mob.moveSpeed + 300);
	}*/
	/*
	// detected player
	if (deadly.aggroDistance > dist && !stealth && deadly.attackTimer <= 0 && deadly.state != ENEMY_STATE_ID::CHASE) {
		deadly.attacking = false;
		deadly.attackTimer = 5000;
		deadly.state = ENEMY_STATE_ID::CHASE;
	}

	// timer over
	if (deadly.attackTimer <= 0 && motion.position.y <= playerPos.y && !deadly.attacking) {
		deadly.attacking = true;
		deadly.targetPosition = playerPos;
		deadly.originalPosition = motion.position;
		physics.velocity.y = 500;	
	}

	// reaches the bottom of the dive
	// deadly.attacking = true;
	// deadly.attackTimer = 0;
	std::cout << "velocity" << physics.velocity.y << std::endl;
	if (physics.velocity.y == 0 && deadly.attacking && deadly.attackTimer <= 0) {
		physics.velocity.y = -500;
	}

	// rises back to original Y position
	if (motion.position.y == deadly.originalPosition.y && deadly.attacking && deadly.attackTimer <= 0) {
		deadly.attacking = false;
		deadly.attackTimer = 5000;
	}

	
	//if (deadly.state == ENEMY_STATE_ID::IDLE) {
	//	std::cout << "Idling" << std::endl;
	//	physics.targetVelocity.x = 0;
	//	if (deadly.aggroDistance > dist && !stealth) { // CHASE
	//		physics.targetVelocity.x = -dir.x * (mob.moveSpeed + 100);
	//		deadly.state = ENEMY_STATE_ID::CHASE;
	//		deadly.attacking = true;
	//	}
	//	else { // RETURN TO POSITION
	//		vec2 patrolVector = deadly.patrolPosition - motion.position;
	//		vec2 dir2 = glm::normalize(patrolVector);
	//		int xdir = (patrolVector.x > 0.f) - (patrolVector.x < 0.f);
	//		physics.targetVelocity.x = xdir * mob.moveSpeed;
	//		deadly.state = ENEMY_STATE_ID::IDLE;
	//	}
	//}
	//else if (deadly.state == ENEMY_STATE_ID::ALERT) {
	//	physics.targetVelocity.x = 0;
	//	physics.targetVelocity.y = 0;
	//	if (deadly.aggroDistance > dist && !stealth) {
	//		physics.targetVelocity.x = -dir.x * (mob.moveSpeed + 100);
	//		deadly.state = ENEMY_STATE_ID::CHASE;
	//		deadly.attacking = true;
	//	} else if (deadly.alertTimer <= 0) {
	//		deadly.state = ENEMY_STATE_ID::IDLE;
	//		
	//		// RETURN TO ORIGINAL PATROL;
	//	}
	//		
	//	
	//}
	//else if (deadly.state == ENEMY_STATE_ID::CHASE) {
	//	if (deadly.attacking) { // DIVING
	//		if (abs(motion.position.y - deadly.targetPosition.y) < 50) {
	//			physics.targetVelocity.y = -500;
	//			deadly.attacking = false;
	//			deadly.attackTimer = 5000;
	//		} else {
	//			vec2 distVector2 = motion.position - deadly.targetPosition;
	//			float dist2 = sqrt(distVector.x * distVector.x + distVector.y * distVector.y);
	//			vec2 dir2 = glm::normalize(distVector);
	//			physics.targetVelocity.x = -dir2.x * (mob.moveSpeed + 100);
	//			physics.targetVelocity.y = -dir2.y * (mob.moveSpeed + 300);
	//		}
	//	} else { //  PATROLLING
	//		if (deadly.aggroDistance < dist || stealth) {
	//		deadly.state = ENEMY_STATE_ID::ALERT;
	//		deadly.alertTimer = ENEMY_ALERT_TIMER;
	//		}

	//		if (motion.position.y + 300 > playerPos.y) {
	//		physics.targetVelocity.y = -250;
	//		} else if (motion.position.y - 300 < playerPos.y){
	//			physics.targetVelocity.y = 250;
	//		} else {
	//			physics.targetVelocity.y = 0;
	//		}

	//		if (deadly.chaseTimer <= 0) {
	//			physics.targetVelocity.x = -dir.x * (mob.moveSpeed + 100);
	//			deadly.chaseTimer = 2000;
	//		}

	//		if (deadly.attackTimer <= 0) {
	//			physics.targetVelocity.x = -dir.x * (mob.moveSpeed + 100);
	//			physics.targetVelocity.y = -dir.y * (mob.moveSpeed + 300);
	//			deadly.targetPosition = playerPos;
	//			deadly.attacking = true;
	//		}	
	//	}
	//		
	//}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}
	*/
}

void AISystem::handleBirdAI(Entity& entity, RenderSystem* renderer) {
	Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];
	Player& playerCom = registry.players.get(player);
	GameManager& gm = registry.gameManagers.components[0];
	Patrol& patrol = registry.patrols.components[0];
	bool stealth = registry.stealth.has(player);


	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = playerPos - motion.position;
	float dist = glm::length(distVector);
	vec2 dir = distVector / dist;
	vec2 enemyDir = glm::vec2(motion.scale.x > 0 ? -1.0f : 1.0f, 0.0f);
	float dotProduct = glm::dot(enemyDir, dir);
	float adjustedAggroDistance = deadly.aggroDistance * glm::abs(dotProduct);

	
	if (deadly.state == ENEMY_STATE_ID::IDLE) {
		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (deadly.aggroDistance/1.3 > dist && !gm.stealth) { // CHASE
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
			if (gm.enemiesInBattle.size() == 0 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.insert(entity);
		}
		else { // RETURN TO POSITION
			if (abs(motion.position.x - deadly.patrolPosition.x) > 30 || abs(motion.position.y - deadly.patrolPosition.y) > 30) {
				vec2 patrolVector = deadly.patrolPosition - motion.position;
				vec2 dir2 = glm::normalize(patrolVector);
				int xdir = (patrolVector.x > 0.f) - (patrolVector.x < 0.f);
				physics.targetVelocity.x = xdir * mob.moveSpeed;
				int ydir = (patrolVector.y > 0.f) - (patrolVector.y < 0.f);
				physics.targetVelocity.y = ydir * mob.moveSpeed;
				deadly.state = ENEMY_STATE_ID::IDLE;
			}
			else {
				doPatrol(motion, deadly, mob, physics, patrol);
			}
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::ALERT) {

		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (deadly.aggroDistance > dist && !stealth) {
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
			gm.enemiesInBattle.insert(entity);
		}
		else if (deadly.alertTimer <= 0) {
			deadly.state = ENEMY_STATE_ID::IDLE;
			if (gm.enemiesInBattle.size() == 1 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.erase(entity);
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::CHASE) {
		if (deadly.aggroDistance < dist || gm.stealth) {
			deadly.state = ENEMY_STATE_ID::ALERT;
			deadly.alertTimer = ENEMY_ALERT_TIMER;
		}

		if (motion.position.y + 200 > playerPos.y) {
			physics.targetVelocity.y = -150;
		} else if (motion.position.y - 200 < playerPos.y){
			physics.targetVelocity.y = 150;
		} else {
			physics.targetVelocity.y = 0;
		}
		if (deadly.chaseTimer <= 0) {
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.chaseTimer = 1500;
		}
		if (deadly.attackTimer <= 0 && abs(motion.position.x - playerPos.x) < 30) {
			createProjectile(renderer, motion.position, motion.position, 0.f, PROJECTILE_TYPE_ID::ENEMY_EGG);
			deadly.attackTimer = 4000; 
		}		
	}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}
	
	return;
}

void AISystem::handleGuardAI(Entity& entity) {
	Player& playerCom = registry.players.components[0];
	Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];
	GameManager& gm = registry.gameManagers.components[0];
	Patrol& patrol = registry.patrols.components[0];
	bool stealth = registry.stealth.has(player);


	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = playerPos - motion.position;
	float dist = glm::length(distVector);
	vec2 dir = distVector / dist;
	vec2 enemyDir = glm::vec2(motion.scale.x > 0 ? -1.0f : 1.0f, 0.0f);
	float dotProduct = glm::dot(enemyDir, dir);
	float adjustedAggroDistance = deadly.aggroDistance * glm::abs(dotProduct);

	// Start of cone
	float diagonalConeLength = sqrt(deadly.aggroDistance * deadly.aggroDistance + motion.scale.y / 2 * motion.scale.y / 2);

	Entity topLine = createLine({ motion.position.x + (deadly.aggroDistance * enemyDir.x / 2), motion.position.y - motion.scale.y / 4 }, { diagonalConeLength, 2 });
	registry.motions.get(topLine).angle = -tan(motion.scale.y / 2 / deadly.aggroDistance) * enemyDir.x;
	Entity bottomLine = createLine({ motion.position.x + (deadly.aggroDistance * enemyDir.x / 2), motion.position.y + motion.scale.y / 4 }, { diagonalConeLength, 2 });
	registry.motions.get(bottomLine).angle = tan(motion.scale.y / 2 / deadly.aggroDistance) * enemyDir.x;
	createLine({ motion.position.x + deadly.aggroDistance * enemyDir.x, motion.position.y }, { 2, motion.scale.y });
	// End of cone

	vec2 decoyDistVector = motion.position - gm.decoyPosition;
	float decoyDist = sqrt(decoyDistVector.x * decoyDistVector.x + decoyDistVector.y * decoyDistVector.y);
	vec2 decoyDir = glm::normalize(decoyDistVector);


	if (deadly.state == ENEMY_STATE_ID::IDLE) {
		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (adjustedAggroDistance > dist && !gm.stealth && dotProduct > 0 && !gm.stealth) { // CHASE
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
			if (gm.enemiesInBattle.size() == 0 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.insert(entity);
		}
		else if (deadly.alertDistance > decoyDist && gm.decoyTimer > 0) {
			physics.targetVelocity.x = -decoyDir.x * (mob.moveSpeed + 200);
			deadly.state = ENEMY_STATE_ID::ALERT;
			deadly.alertTimer = ENEMY_ALERT_TIMER;
		}
		else { // RETURN TO POSITION
			doPatrol(motion, deadly, mob, physics, patrol);
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::ALERT) {
		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (adjustedAggroDistance > dist && !gm.stealth && dotProduct > 0 && !stealth) {
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
			if (gm.enemiesInBattle.size() == 0 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.insert(entity);
		}
		else if (gm.decoyTimer > 0) {
			if (abs(motion.position.x - gm.decoyPosition.x) > 50) {
				physics.targetVelocity.x = -decoyDir.x * (mob.moveSpeed + 200);
			}
		}
		else if (deadly.alertTimer <= 0) {
			deadly.state = ENEMY_STATE_ID::IDLE;
			if (gm.enemiesInBattle.size() == 1 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.erase(entity);
		}
		 
	}
	else if (deadly.state == ENEMY_STATE_ID::CHASE) {
		if (deadly.aggroDistance*1.3 < dist || gm.stealth) {
			deadly.state = ENEMY_STATE_ID::ALERT;
			deadly.alertTimer = ENEMY_ALERT_TIMER;
		}
		if (deadly.chaseTimer <= 0) {
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.chaseTimer = 500;
		}
	}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}

	return;
}

void AISystem::handleBirdmanAI(Entity& entity, RenderSystem* renderer) {
	Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];
	Player& playerCom = registry.players.get(player);
	GameManager& gm = registry.gameManagers.components[0];
	bool stealth = registry.stealth.has(player);


	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = playerPos - motion.position;
	float dist = glm::length(distVector);
	vec2 dir = distVector / dist;
	vec2 enemyDir = glm::vec2(motion.scale.x > 0 ? -1.0f : 1.0f, 0.0f);
	float dotProduct = glm::dot(enemyDir, dir);
	float adjustedAggroDistance = deadly.aggroDistance * glm::abs(dotProduct);


	if (deadly.state == ENEMY_STATE_ID::IDLE) {
		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (deadly.aggroDistance / 1.3 > dist && !gm.stealth) { // CHASE
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
			if (gm.enemiesInBattle.size() == 0 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.insert(entity);
		}
		else { // RETURN TO POSITION
			if (abs(motion.position.x - deadly.patrolPosition.x) > 30 || abs(motion.position.y - deadly.patrolPosition.y) > 30) {
				vec2 patrolVector = deadly.patrolPosition - motion.position;
				vec2 dir2 = glm::normalize(patrolVector);
				int xdir = (patrolVector.x > 0.f) - (patrolVector.x < 0.f);
				physics.targetVelocity.x = xdir * mob.moveSpeed;
				int ydir = (patrolVector.y > 0.f) - (patrolVector.y < 0.f);
				physics.targetVelocity.y = ydir * mob.moveSpeed;
				deadly.state = ENEMY_STATE_ID::IDLE;
			}
			else {
				physics.velocity.x = 0;
				physics.velocity.y = 0;
			}
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::ALERT) {

		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (deadly.aggroDistance > dist && !stealth) {
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
			gm.enemiesInBattle.insert(entity);
		}
		else if (deadly.alertTimer <= 0) {
			deadly.state = ENEMY_STATE_ID::IDLE;
			if (gm.enemiesInBattle.size() == 1 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.erase(entity);
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::CHASE) {
		if (deadly.aggroDistance < dist || gm.stealth) {
			deadly.state = ENEMY_STATE_ID::ALERT;
			deadly.alertTimer = ENEMY_ALERT_TIMER;
		}

		if (motion.position.y + 200 > playerPos.y) {
			physics.targetVelocity.y = -150;
		}
		else if (motion.position.y - 200 < playerPos.y) {
			physics.targetVelocity.y = 150;
		}
		else {
			physics.targetVelocity.y = 0;
		}
		if (deadly.chaseTimer <= 0) {
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.chaseTimer = 1500;
		}
		if (deadly.attackTimer <= 0) {
			createProjectile(renderer, motion.position, dir, 0.f, PROJECTILE_TYPE_ID::FEATHER);
			deadly.attackTimer = 4000;
		}
	}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}

	return;
}

void AISystem::handleGuardCaptainAI(Entity& entity, RenderSystem* renderer) {
	Player& playerCom = registry.players.components[0];
	Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];
	GameManager& gm = registry.gameManagers.components[0];
	Patrol& patrol = registry.patrols.components[0];
	bool stealth = registry.stealth.has(player);


	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = playerPos - motion.position;
	float dist = glm::length(distVector);
	vec2 dir = distVector / dist;
	vec2 enemyDir = glm::vec2(motion.scale.x > 0 ? -1.0f : 1.0f, 0.0f);
	float dotProduct = glm::dot(enemyDir, dir);
	float adjustedAggroDistance = deadly.aggroDistance * glm::abs(dotProduct);

	vec2 decoyDistVector = motion.position - gm.decoyPosition;
	float decoyDist = sqrt(decoyDistVector.x * decoyDistVector.x + decoyDistVector.y * decoyDistVector.y);
	vec2 decoyDir = glm::normalize(decoyDistVector);


	if (deadly.state == ENEMY_STATE_ID::IDLE) {
		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (adjustedAggroDistance > dist && !gm.stealth && dotProduct > 0 && !gm.stealth) { // CHASE
			deadly.attackTimer = 1500;
			deadly.attacking = true;
			deadly.state = ENEMY_STATE_ID::CHASE;
			if (gm.enemiesInBattle.size() == 0 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.insert(entity);
		}
		else if (deadly.alertDistance > decoyDist && gm.decoyTimer > 0) {
			physics.targetVelocity.x = -decoyDir.x * (mob.moveSpeed + 200);
			deadly.state = ENEMY_STATE_ID::ALERT;
			deadly.alertTimer = ENEMY_ALERT_TIMER;
		}
		else { // RETURN TO POSITION
			doPatrol(motion, deadly, mob, physics, patrol);
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::ALERT) {
		physics.targetVelocity.x = 0;
		physics.targetVelocity.y = 0;
		if (adjustedAggroDistance > dist && !gm.stealth && dotProduct > 0 && !stealth) {
			physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
			deadly.state = ENEMY_STATE_ID::CHASE;
			if (gm.enemiesInBattle.size() == 0 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.insert(entity);
		}
		else if (gm.decoyTimer > 0) {
			if (abs(motion.position.x - gm.decoyPosition.x) > 50) {
				physics.targetVelocity.x = -decoyDir.x * (mob.moveSpeed + 200);
			}
		}
		else if (deadly.alertTimer <= 0) {
			deadly.state = ENEMY_STATE_ID::IDLE;
			if (gm.enemiesInBattle.size() == 1 && gm.musicTimer <= 0) {
				gm.musicTimer = 2000;
			}
			gm.enemiesInBattle.erase(entity);
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::CHASE) {
		if (deadly.attackTimer > 0) {
			physics.targetVelocity.x = 0;
		}
		else {
			if (deadly.attacking) {
				vec2 summonPos = { motion.position.x - 100, motion.position.y };
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x += 200;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				deadly.attacking = false;
			}
			if (deadly.aggroDistance * 1.3 < dist || gm.stealth) {
				deadly.state = ENEMY_STATE_ID::ALERT;
				deadly.alertTimer = ENEMY_ALERT_TIMER;
			}
			if (deadly.chaseTimer <= 0) {
				physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
				deadly.chaseTimer = 500;
			}
		}
		
	}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}

	return;
}

void AISystem::handleGuardSummonAI(Entity& entity) {
	Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];

	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = playerPos - motion.position;
	float dist = glm::length(distVector);
	vec2 dir = distVector / dist;

	if (deadly.chaseTimer <= 0) {
		physics.targetVelocity.x = dir.x * (mob.moveSpeed + 100);
		deadly.chaseTimer = 500;
	}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}

	return;
}

void AISystem::handleBossAI(Entity& entity, RenderSystem* renderer) {
	Player& playerCom = registry.players.components[0];
	Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];
	GameManager& gm = registry.gameManagers.components[0];
	Boss& boss = registry.boss.components[0];
	bool stealth = registry.stealth.has(player);


	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = playerPos - motion.position;
	float dist = glm::length(distVector);
	vec2 dir = distVector / dist;

	/*
		BOSS FIGHT:
		4 Waves of Guards. Will constantly shoot projectiles at the player that will also hurt guards.
		Wave 1: IDLE
			- 4 Guards spawn. Boss will not shoot anything yet.
		Wave 2: ALERT
			- 6 Guards spawn. Boss will start shooting fireballs.
		Wave 3: CHASE
			- 4 Guards spawn. Boss will also start raining icicles.
		Wave 4: EXTRA
			- 2 Guards spawn. Boss will speed up magic.
	*/

	if (deadly.state == ENEMY_STATE_ID::IDLE) {
		if (boss.startTimer > 0) {
			// some kind of mana chargin animation?
		}
		else {
			if (boss.summonCount < 0) {
				vec2 summonPos = {motion.position.x, motion.position.y };
				summonPos.x = motion.position.x - 325.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x = motion.position.x - 250.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x = motion.position.x + 250.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x += 125.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				boss.summonCount = 4;
			}
			if (boss.summonCount == 0) {
				boss.summonCount = 6;
				deadly.state = ENEMY_STATE_ID::ALERT;
				deadly.alertTimer = 4000;

				vec2 summonPos = { motion.position.x, motion.position.y };
				summonPos.x = motion.position.x - 400.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x = motion.position.x - 325.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x = motion.position.x - 250.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x = motion.position.x + 250.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x = motion.position.x + 325.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
				summonPos.x = motion.position.x + 400.f;
				summonPosReq.push_back(summonPos);
				summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
			}
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::ALERT) {
		if (boss.summonCount == 0) {
			boss.summonCount = 4;
			deadly.state = ENEMY_STATE_ID::CHASE;
			deadly.alertTimer = 4000;
			deadly.chaseTimer = 7000;
			vec2 summonPos = { motion.position.x, motion.position.y };
			summonPos.x = motion.position.x - 325.f;
			summonPosReq.push_back(summonPos);
			summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
			summonPos.x = motion.position.x - 250.f;
			summonPosReq.push_back(summonPos);
			summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
			summonPos.x = motion.position.x + 250.f;
			summonPosReq.push_back(summonPos);
			summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
			summonPos.x = motion.position.x + 325.f;
			summonPosReq.push_back(summonPos);
			summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
		}
		else {
			if (deadly.alertTimer <= 0) {
				vec2 summonPos = { registry.motions.get(player).position.x,  0};
				createProjectile(renderer, summonPos, summonPos, -1.f, PROJECTILE_TYPE_ID::FIREBALL);
				deadly.alertTimer = 2000;
			}
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::CHASE) {
		if (boss.summonCount == 0) {
			boss.summonCount = 2;
			deadly.state = ENEMY_STATE_ID::EXTRA;
			deadly.alertTimer = 2000;
			deadly.chaseTimer = 4000;
			vec2 summonPos = { motion.position.x, motion.position.y };
			summonPos.x = motion.position.x - 325.f;
			summonPosReq.push_back(summonPos);
			summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
			summonPos.x = motion.position.x + 325.f;
			summonPosReq.push_back(summonPos);
			summonTypeReq.push_back(ENEMY_TYPE_ID::GUARDSUMMON);
		}
		else {
			if (deadly.alertTimer <= 0) {
				vec2 summonPos = { registry.motions.get(player).position.x,  0 };
				createProjectile(renderer, summonPos, summonPos, -1.f, PROJECTILE_TYPE_ID::FIREBALL);
				deadly.alertTimer = 2000;
			}
			if (deadly.chaseTimer <= 0) {
				vec2 summonPos = { registry.motions.get(player).position.x,  0 };
				createProjectile(renderer, summonPos, summonPos, motion.scale.x, PROJECTILE_TYPE_ID::ICICLE);
				summonPos.x += 75.f;
				createProjectile(renderer, summonPos, summonPos, motion.scale.x, PROJECTILE_TYPE_ID::ICICLE);
				summonPos.x -= 150.f;
				createProjectile(renderer, summonPos, summonPos, motion.scale.x, PROJECTILE_TYPE_ID::ICICLE);
				deadly.chaseTimer = 3000;
			}
		}
	}
	else if (deadly.state == ENEMY_STATE_ID::EXTRA) {
		if (boss.summonCount == 0) {
			// DEAD
		}
		else {
			if (deadly.alertTimer <= 0) {
				vec2 summonPos = { registry.motions.get(player).position.x,  0 };
				createProjectile(renderer, summonPos, summonPos, -1.f, PROJECTILE_TYPE_ID::FIREBALL);
				deadly.alertTimer = 1000;
			}
			if (deadly.chaseTimer <= 0) {
				vec2 summonPos = { registry.motions.get(player).position.x,  0 };
				createProjectile(renderer, summonPos, summonPos, motion.scale.x, PROJECTILE_TYPE_ID::ICICLE);
				summonPos.x += 75.f;
				createProjectile(renderer, summonPos, summonPos, motion.scale.x, PROJECTILE_TYPE_ID::ICICLE);
				summonPos.x -= 150.f;
				createProjectile(renderer, summonPos, summonPos, motion.scale.x, PROJECTILE_TYPE_ID::ICICLE);
				deadly.chaseTimer = 1500;
			}
		}
	}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}

	return;
}


void AISystem::templateMobAI(Entity& entity, RenderSystem* renderer) {
	Player& playerCom = registry.players.components[0];
	Deadly& deadly = registry.deadlys.get(entity);
	Mob& mob = registry.mobs.get(entity);
	Motion& motion = registry.motions.get(entity);
	Physics& physics = registry.physicsObjs.get(entity);
	Entity& player = registry.players.entities[0];
	GameManager& gm = registry.gameManagers.components[0];
	Patrol& patrol = registry.patrols.components[0];
	bool stealth = registry.stealth.has(player);


	vec2 playerPos = registry.motions.get(player).position;
	vec2 distVector = playerPos - motion.position;
	float dist = glm::length(distVector);
	vec2 dir = distVector / dist;
	vec2 enemyDir = glm::vec2(motion.scale.x > 0 ? -1.0f : 1.0f, 0.0f);
	float dotProduct = glm::dot(enemyDir, dir);
	float adjustedAggroDistance = deadly.aggroDistance * glm::abs(dotProduct);


	if (adjustedAggroDistance > dist && !gm.stealth && dotProduct > 0 && deadly.state != ENEMY_STATE_ID::CHASE) {
		if (patrol.detectionMeter >= 1 && deadly.state != ENEMY_STATE_ID::CHASE) {
			deadly.state = ENEMY_STATE_ID::CHASE;
		}
		else {
			if (patrol.detectionMeter <= 0) {
				createAlert(renderer, motion.position, ENEMY_STATE_ID::ALERT);
				physics.targetVelocity.x = 0;
				deadly.state = ENEMY_STATE_ID::ALERT;
			}
			patrol.detectionMeter += 0.003f;
			std::cout << patrol.detectionMeter << std::endl;
		}
	}
	else {
		if (patrol.detectionMeter > 0) {
			patrol.detectionMeter -= 0.002f;
			deadly.state = ENEMY_STATE_ID::ALERT;
		}
		else {
			deadly.state = ENEMY_STATE_ID::IDLE;
			doPatrol(motion, deadly, mob, physics, patrol);
		}
	}

	if (physics.velocity.x > 0) {
		motion.scale = vec2(abs(motion.scale.x) * -1.f, abs(motion.scale.y));
	}
	else {
		motion.scale = vec2(abs(motion.scale.x), abs(motion.scale.y));
	}

	return;
}

