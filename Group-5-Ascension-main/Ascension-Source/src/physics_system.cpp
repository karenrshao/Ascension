// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>


// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}


bool collides(const Motion& motion1, const Motion& motion2, vec2 at_pos)
{
	// new code
	// center-align the bounding boxes
	vec2 m2_pos = motion2.position;
	vec2 m1_bb = get_bounding_box(motion1), m2_bb = get_bounding_box(motion2);

	// idea: for both bounding boxes, check if any corner is within any of the other bounding boxes
	int m1_minx = at_pos[0] - m1_bb[0] / 2;
	int m1_miny = at_pos[1] - m1_bb[1] / 2;
	int m1_maxx = at_pos[0] + m1_bb[0] / 2;
	int m1_maxy = at_pos[1] + m1_bb[1] / 2;

	int m2_minx = m2_pos[0] - m2_bb[0] / 2;
	int m2_miny = m2_pos[1] - m2_bb[1] / 2;
	int m2_maxx = m2_pos[0] + m2_bb[0] / 2;
	int m2_maxy = m2_pos[1] + m2_bb[1] / 2;

	return (
		m1_minx <= m2_maxx &&
		m1_maxx >= m2_minx &&
		m1_miny <= m2_maxy &&
		m1_maxy >= m2_miny
	);

	// old code
	/*
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, anglemy_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
	*/
}

/*

MESH COLLISION SYSTEM
	- the hull is convex, and the vertices are in counter-clockwise order
	- collisions will compose implicit equations based on this
	- any object with motion must also have a collider component

ASSUME ENTITIES BOTH HAVE COLLIDER AND MOTION COMPONENT

*/

bool inCameraRange(Motion& m) {
	Camera& camera = registry.cameras.components[0];
	int margin = 128;
	return m.position.x > camera.position.x - window_width_px / 2 - margin &&
			m.position.x < camera.position.x + window_width_px / 2 + margin &&
			m.position.y > camera.position.y - window_height_px / 2 - margin &&
			m.position.y < camera.position.y + window_height_px / 2 + margin;
}

void draw_hitbox(Entity e, vec2 bbox) {

	Motion& motion = registry.motions.get(e);
	if (inCameraRange(motion)) {
		vec2 topLeft = motion.position - bbox / 2.0f;
		vec2 bottomRight = motion.position + bbox / 2.0f;
		Entity l1 = createLine({ (topLeft.x + bottomRight.x) / 2, topLeft.y }, { bbox.x,-2 });
		Entity l2 = createLine({ topLeft.x, (topLeft.y + bottomRight.y) / 2 }, { -2, bbox.y });
		Entity l3 = createLine({ (topLeft.x + bottomRight.x) / 2, bottomRight.y }, { bbox.x,-2 });
		Entity l4 = createLine({ bottomRight.x, (topLeft.y + bottomRight.y) / 2 }, { -2, bbox.y });
		vec3 color = { 255.f, 255.f, 255.f };
		if (registry.players.has(e)) { // Player hitbox: WHITE if on floor, YELLOW if in air.
			Physics& physics = registry.physicsObjs.get(e);
			if (physics.inAir) {
				color = { 255.f, 255.f, 0.f };
			}
		}
		else if (registry.deadlys.has(e)) { // Enemy hitbox: WHITE on idle, BLUE on alert, RED on chase
			if (registry.deadlys.get(e).state == ENEMY_STATE_ID::CHASE) {
				color = { 255.f, 0.f, 0.f };
			}
			else if (registry.deadlys.get(e).state == ENEMY_STATE_ID::ALERT) {
				color = { 0.f, 0.f, 255.f };
			}
			else {
				color = { 255.f, 255.f, 255.f };
			}
		}
		else if (registry.solids.has(e)) { // Floor hitbox: ORANGE default, GREEN collision
			if (registry.collisions.has(e)) {
				color = { 0.f, 255.f, 0.f };
			}
			else {
				color = { 255.f, 127.f, 80.f };
			}
		}
		else {
			color = { 0.f, 0.f, 0.f };
		}
		registry.colors.insert(l1, color);
		registry.colors.insert(l2, color);
		registry.colors.insert(l3, color);
		registry.colors.insert(l4, color);
	}
}

static vec2 apply_affine_transform(vec2 point, Motion& motion, vec2 offset) {
	// rotate
	point = {
		point.x * cos(motion.angle) - point.y * sin(motion.angle),
		point.x * sin(motion.angle) + point.y * cos(motion.angle)
	};
	// scale
	point = {
		abs(motion.scale[0]) * point.x,
		abs(motion.scale[1]) * point.y
	};
	// translate
	point += motion.position + offset;
	return point;
}

static std::vector<vec2> transformHull(Collider collider, Motion translatingMotion, vec2 offset) {
	std::vector<vec2> transformedHull;

	for (vec2 point : collider.hull) {
		transformedHull.push_back(apply_affine_transform({ point[0], point[1] }, translatingMotion, offset));
	}

	return transformedHull;
}

void drawTranslatedHull(Entity colliderEntity) {
	Collider collider = registry.colliders.get(colliderEntity);
	Motion motion = registry.motions.get(colliderEntity);
	
	std::vector<vec2> translatedHull = transformHull(collider, motion, { 0.f, 0.f });
	Entity l1, l2, l3, l4;
	float min_x = INFINITY, min_y = INFINITY, max_x = -INFINITY, max_y = -INFINITY;

	for (uint i = 0; i < translatedHull.size(); i++) {
		vec2 p1 = translatedHull[i], p2;
		if (i == translatedHull.size() - 1) {
			p2 = translatedHull[0];
		}
		else {
			p2 = translatedHull[i + 1];
		}

		vec2 diff = p2 - p1;
		vec2 centre = 0.5f * (p1 + p2);

		float dir = atan2(diff[1], diff[0]);
		float length = sqrtf(diff[0] * diff[0] + diff[1] * diff[1]);

		vec2 pos = translatedHull[i];
		Entity line = createLine(centre, { length, 2. });
		registry.motions.get(line).angle = dir;
	}
}

static bool SATCollidesAt(Entity targetEntity, Entity otherEntity, vec2 collidingOffset) {

	// get components
	auto& collider_registry = registry.colliders;
	auto& motion_registry = registry.motions;

	Collider targetCollider = collider_registry.get(targetEntity);
	Collider otherCollider = collider_registry.get(otherEntity);

	Motion targetMotion = motion_registry.get(targetEntity);
	Motion otherMotion = motion_registry.get(otherEntity);

	std::vector<vec2> targetHull = transformHull(targetCollider, targetMotion, collidingOffset);
	std::vector<vec2> otherHull = transformHull(otherCollider, otherMotion, { 0., 0. });

	// note: use -1 and handle the final face (n - 1, 0) separately

	// check collisions for targetCollider
	// TODO: offset by colliding position

	bool colliding = true;

	for (uint i = 0; i < targetHull.size(); i++) {
		vec2 p1 = targetHull[i], p2;
		if (i == targetHull.size() - 1) {
			p2 = targetHull[0];
		}
		else {
			p2 = targetHull[i + 1];
		}

		float dir;

		bool lineDivides = true;

		for (auto vertex : otherHull) {
			float x = vertex[0], y = vertex[1];
			dir = (p2[1] - p1[1]) * (x - p1[0]) - (p2[0] - p1[0]) * (y - p1[1]); // positive if there is separation
			lineDivides = dir <= 0 ? false : lineDivides;
		}


		colliding = lineDivides ? false : colliding;
		if (lineDivides) break;
	}

	// check collisions for otherCollider

	for (uint i = 0; i < otherHull.size() - 1; i++) {
		vec2 p1 = otherHull[i], p2;
		if (i == otherHull.size() - 1) {
			p2 = otherHull[0];
		}
		else {
			p2 = otherHull[i + 1];
		}

		float dir;

		bool lineDivides = true;

		for (auto vertex : targetHull) {
			float x = vertex[0], y = vertex[1];
			dir = (p2[1] - p1[1]) * (x - p1[0]) - (p2[0] - p1[0]) * (y - p1[1]); // positive if there is separation
			lineDivides = dir <= 0 ? false : lineDivides;
		}

		colliding = lineDivides ? false : colliding;
		if (lineDivides) break;
	}

	return colliding;
}

void stepPlayerMovement(float elapsed_ms) {
	if (
		registry.gameManagers.components[0].state == GAME_STATE::DIALOGUE ||
		registry.deathTimers.has(registry.players.entities[0])
		) {
		Physics& playerPhys = registry.physicsObjs.components[0];
		playerPhys.targetVelocity = { 0., 0. };
		return;
	}
	auto& player_registry = registry.players;
	for (uint i = 0; i < player_registry.size(); i++) {
		Entity playerEntity = player_registry.entities[i];
		Player& player = player_registry.get(playerEntity);
		Motion& motion = registry.motions.get(playerEntity);
		Physics& physics = registry.physicsObjs.get(playerEntity);
		Mob& mob = registry.mobs.get(playerEntity);

		if (!player.is_dead && !registry.stealth.has(playerEntity)) {
			int attacking = registry.attackAnimations.has(playerEntity) - registry.attackUpAnimations.has(playerEntity);
			int hdir = player.key_right - player.key_left;

			// if attacking, we should not move until the attack animation completes
			if (attacking || player.dash_timer > 0) {
				hdir = 0;
			}

			if (hdir) {
				motion.scale.x = hdir * abs(motion.scale).x;
			}

			if (registry.invincibilityTimers.has(playerEntity) && player.dash_timer > 0) {
				if (motion.scale.x > 0) {
					physics.targetVelocity.x = mob.moveSpeed * 2.5;
					physics.velocity.x = mob.moveSpeed * 2.5;
				}
				else {
					physics.targetVelocity.x = mob.moveSpeed * -2.5;
					physics.velocity.x = mob.moveSpeed * -2.5;
				}
			}
			else {
				physics.targetVelocity.x = mob.moveSpeed * hdir * 1.2;

				if (player.key_jump && !physics.inAir) {
					physics.velocity.x *= 1.2;
					physics.velocity.y = mob.jumpSpeed;
					physics.targetVelocity.y = mob.jumpSpeed;
				}

				// if (player.key_down && physics.inAir) {
				// 	if (physics.velocity.y <= -mob.jumpSpeed)
				// 		physics.velocity.y = -mob.jumpSpeed * 1.1;
				// }

				if (player.hurt && !physics.inAir) {
					std::cout << "player is hurt, make it jump" << std::endl;
					physics.velocity.y = mob.hurtSpeed;
					physics.targetVelocity.y = mob.hurtSpeed;
				}
			}
		}
		else {
			//std::cout << "player is dead, physics system should not apply" << std::endl;
			physics.targetVelocity = { 0.f, 0.f };
			physics.velocity = { 0.f, 0.f };
		}

		// running sound fx
		GameManager& gm = registry.gameManagers.components[0];
		if (!physics.inAir && abs(physics.velocity.x) > 100) {
			player.is_running = true;
		} else {
			player.is_running = false;
		}
	}
}

void doGravity(float elapsed_ms, ECSRegistry& registry) {

	auto& motion_registry = registry.motions;
	auto& gravity_registry = registry.gravityEntities;
	auto& mob_registry = registry.mobs;

	// Handle gravity
	for (uint i = 0; i < gravity_registry.size(); i++) {
		// TODO: ADD CHECK FOR COLLISIONS
		float step_seconds = elapsed_ms / 1000.f;
		Entity entity = gravity_registry.entities[i];

		// Motion& motion = motion_registry.get(entity);
		// Mob& mob = mob_registry.get(entity);

		Gravity& grav = gravity_registry.components[i];
		Physics& physics = registry.physicsObjs.get(entity);

		if (abs(physics.targetVelocity.y) <= grav.terminalVelocity && abs(physics.velocity.y) <= grav.terminalVelocity) {
			physics.targetVelocity.y += step_seconds * grav.grav;
			physics.velocity.y += step_seconds * grav.grav;
		}
		
		// prevent the player from mini-jumping infinitely
		if (registry.players.has(entity) && (physics.velocity.y <= 0)) {
			Player& playerStr = registry.players.get(entity);
			playerStr.hurt = false;
			
		}
		// play a jump sound when falling a big distance (850 big enough so sound wont play if you fall down one block)
		if (registry.players.has(entity) && physics.velocity.y > 750) {
			// abs(physics.velocity.y) > 850 will play a sound for sure when you hit space bar but will make a sound on ceilings
			Player& playerStr = registry.players.get(entity);
			playerStr.jump_sound = true;
		}
	}
}

void doPhysicsCollisions(float elapsed_ms, ECSRegistry& registry) {
	auto& physics_registry = registry.physicsObjs;

	auto& motion_registry = registry.motions;
	auto& gravity_registry = registry.gravityEntities;
	auto& mob_registry = registry.mobs;

	for(uint i = 0; i < physics_registry.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		Physics& physics = physics_registry.components[i];
		Entity entity = physics_registry.entities[i];

		Motion& motion = motion_registry.get(entity);

		float step_seconds = elapsed_ms / 1000.f;

		if (!physics.inAir || physics.targetVelocity.x != 0) {
			physics.velocity += (physics.targetVelocity - physics.velocity) * physics.groundDrag;
		}
		else {
			physics.velocity += (physics.targetVelocity - physics.velocity) * physics.airDrag;
		}
		

		vec2 positionBeforeMove = motion.position;

		// check vertical collisions
		vec2 msp = step_seconds * physics.velocity;
		float hsp = msp[0], vsp = msp[1];
		int shsp = (hsp > 0) - (hsp < 0), svsp = (vsp > 0) - (vsp < 0);


		// motion.position += step_seconds * velToMove;

		//////////////////////////////////////////////////
		// COLLISION CODE USING COLLIDERS
		auto& collider_registry = registry.colliders;

		physics.inAir = true;
		physics.unspawnable = physics.inAir;

		// we ONLY want to collide if the entity is not a solid and if it has a collider component, it already has a physics component as given
		if (!registry.solids.has(entity) && registry.colliders.has(entity)) {

			for (uint j = 0; j < collider_registry.size(); j++)
			{
				// get the motion component of other object
				Entity entity_j = collider_registry.entities[j];
				Motion motion_j = motion_registry.get(entity_j);
				
				if (motion_j.position[1] >= motion.position[1] + 256 || 
					motion_j.position[1] <= motion.position[1] - 256 ||
					motion_j.position[0] + motion_j.scale[0] / 2 <= motion.position[0] - 256 ||
					motion_j.position[0] - motion_j.scale[0] / 2 >= motion.position[0] + 256 ||
					entity_j == entity) {
					continue;
				}


				// check if the collision is between entity and a solid
				if (registry.solids.has(entity_j)) {
					Solid& solid = registry.solids.get(entity_j);
					
					if (
							SATCollidesAt(entity, entity_j, { 0., 3. }) && 
							vsp >= 0 &&
							!(
								solid.topFaceOnly &&
								motion.position.y > motion_j.position.y - abs(motion_j.scale.y) / 2 - abs(motion.scale.y) / 2
							)
						) {
						physics.inAir = false;

						if (registry.projectiles.has(entity) && !registry.projectiles.get(entity).landed) {
							Projectile& proj = registry.projectiles.get(entity);
							proj.timer = 2000;
							if (proj.projType == PROJECTILE_TYPE_ID::HEART) proj.timer += 2000;
							proj.landed = true;
							if (proj.decoy) {
								GameManager& gm = registry.gameManagers.components[0];
								gm.decoyPosition = registry.motions.get(entity).position;
								gm.decoyTimer = 6000;
							}
						}

						if (!registry.summonables.has(entity_j)) {
							physics.unspawnable = physics.inAir;
						}
					}
					if (SATCollidesAt(entity, entity_j, { 0., vsp }) &&
							!(
								solid.topFaceOnly &&
								motion.position.y > motion_j.position.y - abs(motion_j.scale.y) / 2 - abs(motion.scale.y) / 2
							) && 
							!(
								solid.topFaceOnly && registry.players.has(entity) && 
								registry.players.get(entity).key_down &&
								!registry.players.get(entity).is_summoning
							)
						)
					{
						/*
						int xplus = 0;
						int facing = (motion.scale.x > 0) - (motion.scale.x < 0);
						// use shsp to figure out direction of slipping, don't slip if you're moving in opposite direction

						while (SATCollidesAt(entity, entity_j, { vsp, xplus }) && 
								abs(xplus) <= abs(physics.rampSpeed * vsp)) {
							xplus += facing; // move in the direction of current horizontal velocity
						}
						if (SATCollidesAt(entity, entity_j, { xplus, vsp })) {
						*/
							// if it's a solid, reset to the previous position
							motion.position = positionBeforeMove;
							physics.targetVelocity.y = -(physics.elasticity * physics.targetVelocity.y);
							physics.velocity.y = -(physics.elasticity * physics.velocity.y);

							for (int i = 0; i < 2 * abs(vsp); i++) {
								if (SATCollidesAt(entity, entity_j, { 0., svsp + 1 })) break;
								motion.position[1] += svsp;
							}
							vsp = 0;
						/*
						} else {
							motion.position[0] += xplus;
						}
						*/

						// setting player running sfx
						if (registry.players.has(entity)) {
							Player& p = registry.players.get(entity);
							p.on_material = solid.mat;
						}

					}

					// if they're colliding at the incremented position:
					if (SATCollidesAt(entity, entity_j, { hsp, 0. }) && !solid.topFaceOnly)
					{
						// ADDED RAMP CHECK
						int yplus = 0; // positive offset, so we subtract

						while (SATCollidesAt(entity, entity_j, { hsp, -yplus }) && yplus <= abs(physics.rampSpeed * hsp)) {
							yplus++;
						}
						if (SATCollidesAt(entity, entity_j, { hsp, -yplus })) {
							motion.position[0] = positionBeforeMove[0];
							// get the entity it's colliding with
							physics.targetVelocity.x = -(physics.elasticity * physics.targetVelocity.x);
							physics.velocity.x = -(physics.elasticity * physics.velocity.x);

							for (int i = 0; i < 2 * abs(hsp); i++) {
								if (SATCollidesAt(entity, entity_j, { shsp, 0. })) break;
								motion.position[0] += shsp;
							}
							hsp = 0;
						} else {
							motion.position[1] -= yplus;
						}
					}
				}
			}
		}

		// physics.inAir = vsp != 0;

		motion.position += vec2({ (int) hsp, (int) vsp });
	}

}

void doSummonables(float elapsed_ms) {
	auto& summonable_registry = registry.summonables;
	float step_seconds = elapsed_ms / 1000.0f;
	for (uint i = 0; i < summonable_registry.size(); i++) {

		Entity sEntity = summonable_registry.entities[i];
		Summonable& summ = summonable_registry.components[i];
		Motion& sMotion = registry.motions.get(sEntity);
		
		if (!summ.active) continue;

		Motion& playerMotion = registry.motions.get(registry.players.entities[0]);

		int facing = (playerMotion.scale.x > 0) - (playerMotion.scale.x < 0);

		float xOffset = facing * 0.5 * summ.max_offset, yOffset = summ.max_offset;
		Player& playerComp = registry.players.components[0];
		if (playerComp.is_summoning && playerComp.key_down) {
			xOffset = facing * 3.5 * summ.max_offset;
			yOffset = 0.2 * summ.max_offset;
		}
		
		vec2 targetPos = playerMotion.position - vec2({ -xOffset, yOffset }) - sMotion.position;
		float angle = atan2(targetPos.y, targetPos.x);

		vec2 spd = summ.speed * step_seconds * vec2({ cos(angle), sin(angle) });

		sMotion.position += spd;
	}
}

void updateBreakables(float elapsed_ms) {
	auto& breakable_registry = registry.breakables;

	for (uint i = 0; i < breakable_registry.size(); i++) {
		Entity& bEnt = breakable_registry.entities[i];

		if (registry.physicsObjs.has(bEnt) && !registry.summonables.has(bEnt)) {
			Physics& phys = registry.physicsObjs.get(bEnt);
			if (phys.inAir) {
				// if it's in air, fall downwards
				if (registry.solids.has(bEnt)) registry.solids.remove(bEnt);

				if (!registry.gravityEntities.has(bEnt)) registry.gravityEntities.emplace(bEnt);
			} else {
				// else, solidify
				if (registry.gravityEntities.has(bEnt)) registry.gravityEntities.remove(bEnt);

				if (!registry.solids.has(bEnt)) registry.solids.emplace(bEnt);
			}
		}
	}
}

void PhysicsSystem::step(float elapsed_ms)
{

	// template pause system
	GameManager& gm = registry.gameManagers.components[0];
	if (gm.state == GAME_STATE::PAUSE || gm.state == GAME_STATE::MENU)
		return;

	elapsed_ms = min(elapsed_ms, 16.6666667f);
	stepPlayerMovement(elapsed_ms);
	doSummonables(elapsed_ms);

	auto& playerCom = registry.players.components[0];
	auto& playerEnt = registry.players.entities[0];

	// Move bug based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	auto& gravity_registry = registry.gravityEntities;
	auto& mob_registry = registry.mobs;

	doGravity(elapsed_ms, registry);
	doPhysicsCollisions(elapsed_ms, registry);

	updateBreakables(elapsed_ms);

	// Check for collisions between all moving entities
    ComponentContainer<Collider> &collider_container = registry.colliders;
	for(uint i = 0; i< collider_container.components.size(); i++)
	{
		Entity entity_i = collider_container.entities[i];
		Motion& motion_i = registry.motions.get(entity_i);
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j< collider_container.components.size(); j++)
		{
			Entity ej = registry.colliders.entities[j];
			if (!registry.motions.has(ej)) continue;
			Motion& motion_j = registry.motions.get(ej);

			if (motion_j.position[0] >= motion_i.position[0] + 128 || motion_j.position[0] <= motion_i.position[0] - 128 || 
				motion_j.position[1] >= motion_i.position[1] + 128 || motion_j.position[1] <= motion_i.position[1] - 128) {
				continue;
			}

			if (SATCollidesAt(entity_i, ej, {0., 0.}))
			{
				Entity entity_j = collider_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				
				if (!registry.solids.has(entity_i) || !registry.solids.has(entity_j)) {
					registry.collisions.emplace_with_duplicates(entity_i, entity_j);
					registry.collisions.emplace_with_duplicates(entity_j, entity_i);
				}
			}
		}
	}


	//Debug boxes
	for (auto& m : registry.colliders.entities) {
		if (debugging.in_debug_mode && registry.motions.has(m)) {
			if (inCameraRange(registry.motions.get(m))) {
				drawTranslatedHull(m);
			}

			draw_hitbox(m, get_bounding_box(registry.motions.get(m)));
		}
	}

	// update player's last position
	Entity& pEnt = registry.players.entities[0];
	Player& player = registry.players.get(pEnt);
	Motion& pMotion = registry.motions.get(pEnt);

	if (!registry.physicsObjs.get(pEnt).unspawnable) {
		player.landTime += elapsed_ms;
		if (player.landTime >= player.landTimer) {
			player.savedPosition = player.lastPosition;
			player.lastPosition = pMotion.position;
			player.landTime = 0;
		}
	} else {
		player.landTime = 0;
	}
}