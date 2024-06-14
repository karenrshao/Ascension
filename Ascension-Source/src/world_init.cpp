#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

DialogueRegistry importDialogue;

Entity createGameManager() {
	auto entity = Entity();

	registry.questmanagers.emplace(entity);
	GameManager& gm = registry.gameManagers.emplace(entity);

	gm.state = GAME_STATE::PLAY;

	return entity;
} 

Entity createBackground(RenderSystem* renderer, 
	vec2 pos, 
	TEXTURE_ASSET_ID background_asset, 
	vec2 background_scale,
	float dist) 
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);

	// TODO: REMOVE POSITION DEPENDENCE AND USE SOLIDS TO RENDER POSITION
	motion.position = pos;
	motion.angle = 0.0f;
	motion.scale = background_scale;

	auto& background = registry.backgrounds.emplace(entity);

	background.initPos = pos;
	background.dist = dist;

	registry.renderRequests.insert(
		entity,
		{ background_asset, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	registry.levelElements.emplace(entity);

	return entity;
}

Entity createCameraTether(RenderSystem* renderer, vec2 pos, vec2 bounds, float zoom) {
	auto entity = Entity();

	auto& tether = registry.cameraTethers.emplace(entity);
	tether.position = pos;
	tether.bounds = bounds;
	tether.zoom = zoom;

	registry.levelElements.emplace(entity);
	return entity;
}

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	auto& mob = registry.mobs.emplace(entity);
	mob.moveSpeed = 512.f;
	mob.jumpSpeed = -950.f;
	mob.hurtSpeed = -200.f;
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PLAYER);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.0f;
	motion.scale = { PLAYER_BB_SIZE, PLAYER_BB_SIZE };

	auto& physics = registry.physicsObjs.emplace(entity);

	Collider& collider = registry.colliders.emplace(entity);
	// std::vector<vec2> initialVertices = collider.getConvexHull(mesh);
	std::vector<vec2> initialVertices = collider.getSquareHull(motion);

	for (vec2& cv : initialVertices) {
		cv.x *= 0.3;
		if (cv.y < 0) {
			cv.y = cv.y * 0.65 + 0.16;
		}
	}

	collider.hull = initialVertices;

	auto& camera = registry.cameras.emplace(entity);
	camera.position = pos + vec2({ 256.f, -128.f });
	camera.targetPosition = pos;

	registry.players.emplace(entity);
	registry.gravityEntities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createNPC(RenderSystem* renderer, NPCData data) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& npcMotion = registry.motions.emplace(entity);
	npcMotion.position = data.position;
	npcMotion.scale = data.scale;

	auto& interactable = registry.interactables.emplace(entity);
	interactable.disappears = false;
	interactable.changes = false;
	interactable.interactText = data.interactText;
	interactable.type = INTERACTABLE_TYPE_ID::NPC;

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(npcMotion);
	

	if (data.speaker != SPEAKER_ID::NO_SPEAKER) {
		Speaker& speaker = registry.speakers.emplace(entity);
		speaker.speaker = data.speaker;
		speaker.name = data.name;
		speaker.autoTriggerRadii = data.radius;
		speaker.complexDialogueSequences = importDialogue.complexSequences[data.speaker];
		speaker.simpleDialogueLines = importDialogue.simpleLines[data.speaker];
	}

	registry.renderRequests.insert(
		entity,
		{
			data.sprite,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	if (data.sprite == TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_1) {
		if (!registry.sleepAnimations.has(entity)) {
			registry.sleepAnimations.insert(entity, SleepAnimation(19, 14));
		}
	}

	registry.levelElements.emplace(entity);

	registry.backLevelObjs.emplace(entity);
	return entity;
}

Entity createSolid(RenderSystem* renderer, vec2 position, vec2 scale, bool oneWay, int slope, MATERIAL mat) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	auto& solid = registry.solids.emplace(entity);
	solid.position = position;
	solid.topFaceOnly = oneWay;
	solid.mat = mat;

	// TODO: REMOVE POSITION DEPENDENCE AND USE SOLIDS TO RENDER POSITION
	motion.position = position;
	motion.angle = 0.0f;
	motion.scale = scale; // vec2({ WALL_TILE_SIZE, WALL_TILE_SIZE });

	if (slope < 0) motion.scale.x = -motion.scale.x;
	
	auto& physics = registry.physicsObjs.emplace(entity);

	Collider& collider = registry.colliders.emplace(entity);
	if (slope != 0) {
		collider.hull = Collider::getSlopedHull(slope);
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::RAMP,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	} else {
		collider.hull = Collider::getSquareHull(motion);
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::SOLID,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}


	registry.backLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;
}

Entity createGrass(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);

	// TODO: REMOVE POSITION DEPENDENCE AND USE SOLIDS TO RENDER POSITION
	motion.position = position;
	motion.angle = 0.0f;
	motion.scale = vec2({ WALL_TILE_SIZE, WALL_TILE_SIZE });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GRASS,
			EFFECT_ASSET_ID::WIND_GRASS,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	registry.foregroundDecors.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;
}

Entity createItem(RenderSystem* renderer, vec2 position, bool disappears, bool changes) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
    auto& interactable = registry.interactables.emplace(entity);
	interactable.disappears = disappears;
	interactable.changes = changes;
	interactable.type = INTERACTABLE_TYPE_ID::ITEM;

	motion.position = position;
	motion.angle = 0.0f;
	motion.scale = { WALL_TILE_SIZE, WALL_TILE_SIZE };

	auto& physics = registry.physicsObjs.emplace(entity);
	// COULD ADD GRAVITY

	Collider& collider = registry.colliders.emplace(entity);	
	collider.hull = Collider::getSquareHull(motion);

	/******************TEMPORARY CODE*******************/
	Item& item = registry.items.insert(
		entity,
		{
			ITEM_CONDITION_ID::FRUIT
		}
	);
	/******************TEMPORARY CODE*******************/

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::APPLE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	registry.foreLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;
}

Entity createSpecificItem(RenderSystem* renderer, 
							vec2 position, 
							vec2 scale, 
							bool disappears, 
							bool changes, 
							TEXTURE_ASSET_ID sprite,
							ITEM_CONDITION_ID id
						) {
	Entity entity;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	auto& interactable = registry.interactables.emplace(entity);
	interactable.disappears = disappears;
	interactable.changes = changes;
	interactable.type = INTERACTABLE_TYPE_ID::ITEM;
	interactable.interactText = "[V] Pick up";

	motion.position = position;
	motion.angle = 0.0f;
	motion.scale = scale;

	auto& physics = registry.physicsObjs.emplace(entity);
	// COULD ADD GRAVITY

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(motion);

	Item& item = registry.items.emplace(entity);
	item.item_id = id;

	registry.renderRequests.insert(
		entity,
		{
			sprite,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	if (sprite == TEXTURE_ASSET_ID::SPARKLE_1) {
		std::cout << "should add to sparkle animation component container" << std::endl;
		// pause time for SparkleAnimation is in ms
		registry.sparkleAnimations.insert(entity, SparkleAnimation(13, 8, 2300));
	}

	registry.foreLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;
}
// creates a breakable object
Entity createBreakable(RenderSystem* renderer, vec2 position, vec2 scale, TEXTURE_ASSET_ID id, int hp) {
	auto entity = Entity();

	auto& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	auto& breakable = registry.breakables.emplace(entity);
	breakable.hp = hp;

	// make it collide w player
	auto& solid = registry.solids.emplace(entity);
	solid.position = position;
	Collider& collider = registry.colliders.emplace(entity);
	if (id == TEXTURE_ASSET_ID::BREAKABLE_BOX) {
		collider.hull = Collider::getSquareHull(motion);
	}

	registry.physicsObjs.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	registry.foreLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;

}
// creates a damage hitbox in front of the player
Entity createDamage(RenderSystem* renderer, vec2 position, vec2 scale, float angle) {
	auto entity = Entity();
	auto& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = scale;
	motion.angle = angle;

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(motion);
	auto& physics = registry.physicsObjs.emplace(entity);

	registry.foreLevelObjs.emplace(entity);
	registry.dmgTimers.emplace(entity);

	registry.slashAnimations.insert(entity, SlashAnimation(4, 7));

	return entity;
}
 
// refers to any transition that requires an interaction, not just doors
Entity createDoor(RenderSystem* renderer, 
					vec2 createPosition, 
					vec2 targetPlayerPosition,
					vec2 scale,
					Level targetLevel,
					LEVEL_INDEX targetLevelIndex,
					bool needsInteract,
					bool visible,
					std::vector<QUEST_INDEX> requirements,
					std::string doorText
				) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	 if (needsInteract) {
	 	auto& interactable = registry.interactables.emplace(entity);
	 	interactable.disappears = false; // TODO: change this to set with parameters only if necessary but for now we're assuming things like doors open/close but dont' disappear
	 	interactable.changes = true;
		interactable.type = INTERACTABLE_TYPE_ID::DOOR;
		interactable.interactText = "[V] " + doorText;
		if (visible) {
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::DOOR,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);
		}
	 }

	auto& transition = registry.transitions.emplace(entity);
	transition.nextLevel = targetLevel;
	transition.nextPos = targetPlayerPosition;
	transition.needsInteract = needsInteract;
	transition.nextLevelIndex = targetLevelIndex;
	transition.requirements = requirements;

	auto& motion = registry.motions.emplace(entity);
	motion.position = createPosition;
	motion.position.y -= WALL_TILE_SIZE / 2;
	motion.angle = 0.0f;
	motion.scale = scale; // TODO? change this to be the actual size of the door

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(motion);

	
	registry.backLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;
}

Entity createRespawnTether(vec2 pos) {
	auto entity = Entity();

	auto& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = { 128., 128. };

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(motion);

	auto& interactable = registry.interactables.emplace(entity);
	interactable.type = INTERACTABLE_TYPE_ID::RESPAWN_TETHER;
	interactable.disappears = false;
	interactable.changes = false;
	interactable.interactText = "[V] Save Progress";

	auto& speaker = registry.speakers.emplace(entity);
	speaker.speaker = SPEAKER_ID::RESPAWN_ANCHOR;
	speaker.name = "";
	speaker.autoTriggerRadii = motion.scale;
	speaker.simpleDialogueLines = importDialogue.simpleLines[speaker.speaker];

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::NOSPRITE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

// COPY OF OLD DOOR CODE FOR REFERENCE
/*Entity createDoor(RenderSystem* renderer, 
					vec2 position, 
					std::string filepath, 
					std::string currentLevel) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	auto& interactable = registry.interactables.emplace(entity);
	auto& transition = registry.transitions.emplace(entity);
	interactable.disappears = false; // TODO: change this to set with parameters only if necessary but for now we're assuming things like doors open/close but dont' disappear
	interactable.changes = true;
	transition.nextLocation = filepath;
	transition.appearsIn = currentLevel;
	// transition.startpos = some starting position we get from the parameters
	motion.position = position;
	motion.angle = 0.0f;
	motion.scale = { WALL_TILE_SIZE, WALL_TILE_SIZE * 2 }; // TODO? change this to be the actual size of the door

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	registry.levelElements.emplace(entity);
	return entity;
}*/

/*Entity renderDoor(Entity entity) {
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	return entity;
}*/

/*
Entity createBug(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 50 };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -BUG_BB_WIDTH, BUG_BB_HEIGHT });

	// Create an (empty) Bug component to be able to refer to all bug
	registry.eatables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BUG,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}
*/

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 pos, ENEMY_TYPE_ID enemyType) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& mob = registry.mobs.emplace(entity);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = pos;
	motion.scale = { ENEMY_BB_SIZE, ENEMY_BB_SIZE };

	auto& deadly = registry.deadlys.emplace(entity); 
	deadly.type = enemyType;
	deadly.state = ENEMY_STATE_ID::IDLE;
	deadly.patrolPosition = pos;

	auto& physics = registry.physicsObjs.emplace(entity);

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(motion);
	
	switch (enemyType) {
		case ENEMY_TYPE_ID::BIRDO:
			deadly.aggroDistance = 500;
			mob.moveSpeed = 400.f;
			mob.jumpSpeed = 0.f; 
			registry.patrols.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::EVIL_BIRD_FLY_1,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);

			registry.runAnimations.insert(entity, RunAnimation(12, 9));
			break;
		case ENEMY_TYPE_ID::BIRDMAN:
			deadly.aggroDistance = 450;
			mob.moveSpeed = 300.f;
			mob.jumpSpeed = 0.f;
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::EVIL_BIRD_FLY_1,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);
			motion.scale = { BIRDO_BB_SIZE, BIRDO_BB_SIZE };
			//registry.runAnimations.insert(entity, RunAnimation(12, 9));
			break;
		case ENEMY_TYPE_ID::DIVE_BIRDO:
			deadly.aggroDistance = 500;
			mob.moveSpeed = 400.f;
			mob.jumpSpeed = 0.f;
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::EVIL_BIRD_FLY_1,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);
			//registry.runAnimations.insert(entity, RunAnimation(12, 9));
			break;
		case ENEMY_TYPE_ID::TREE:
			deadly.aggroDistance = 300;
			mob.moveSpeed = 350.f;
			mob.jumpSpeed = -1000.f;
			registry.gravityEntities.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::ENEMY,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				} 
			);
			break;
		case ENEMY_TYPE_ID::RAT:
			deadly.aggroDistance = 200;
			mob.moveSpeed = 150.f;
			mob.jumpSpeed = -1000.f;
			motion.scale = { 60, 60 };

			registry.gravityEntities.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::SEWER_SLIME_1,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);

			registry.runAnimations.insert(entity, RunAnimation(9, 11));
			break;
		case ENEMY_TYPE_ID::GUARD:
			deadly.aggroDistance = 300;
			deadly.alertDistance = 800;
			mob.moveSpeed = 300.f;
			mob.jumpSpeed = -1000.f;
			motion.scale.x *= -1;
			registry.gravityEntities.emplace(entity);
			registry.patrols.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::GUARD,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);
			break;	
		case ENEMY_TYPE_ID::GUARDCAPTAIN:
			deadly.aggroDistance = 350;
			deadly.alertDistance = 800;
			mob.moveSpeed = 250.f;
			mob.jumpSpeed = -1000.f;
			registry.gravityEntities.emplace(entity);
			registry.patrols.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::GUARD_CAPTAIN,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);
			break;
		case ENEMY_TYPE_ID::GUARDSUMMON:
			mob.moveSpeed = 200.f;
			registry.gravityEntities.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::APPLE,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);
			break;
		case ENEMY_TYPE_ID::BOSS:
			registry.boss.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::KING,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);
			break;
	}
	registry.levelElements.emplace(entity);
	return entity;
}

Entity createAlert(RenderSystem* renderer, vec2 pos, ENEMY_STATE_ID state) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = { 20, 20 };

	Physics& physics = registry.physicsObjs.emplace(entity);
	physics.targetVelocity.y = -30.f;

	registry.basicTimers.emplace(entity);

	switch (state) {
		case ENEMY_STATE_ID::ALERT:
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::BLUEBERRY,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				});
			break;
		case ENEMY_STATE_ID::CHASE:
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::APPLE,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				});
			break;
		default:
			break;
	}

	registry.foreLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;
}

Entity createProjectile(RenderSystem* renderer, vec2 pos, vec2 dir, float hdir, PROJECTILE_TYPE_ID projType) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;


	Physics& physics = registry.physicsObjs.emplace(entity);

	Projectile& projectile = registry.projectiles.emplace(entity);
	projectile.projType = projType;

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(motion);


	switch (projType) {
		case PROJECTILE_TYPE_ID::ENEMY_EGG:
			motion.scale = { 30, 30 };
			physics.velocity = { 0.f, 100.f };
			projectile.enemy = true;
			registry.gravityEntities.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::EVIL_BIRD_EGG,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				});
			break;
		case PROJECTILE_TYPE_ID::DECOY:
			motion.scale = { 30, 30 }; 
			if (hdir > 0) physics.velocity = { 1250.f, -750.f };
			if (hdir < 0) physics.velocity = { -1250.f, -750.f };
			projectile.enemy = false;
			projectile.decoy = true;
			registry.gravityEntities.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::BLUEBERRY,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				});
			break;
		case PROJECTILE_TYPE_ID::HEART:
			motion.scale = { 30, 30 };
			projectile.enemy = false;
			projectile.noHurtEnemy = true;
			registry.gravityEntities.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::APPLE,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				});
			break;
		case PROJECTILE_TYPE_ID::FEATHER:
			motion.scale = { 30, 30 };
			projectile.enemy = true;
			physics.airDrag = 0;
			physics.velocity = dir * 350.f;
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::APPLE,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				});
			break;
		case PROJECTILE_TYPE_ID::FIREBALL:
			motion.scale = { 70, 70 };
			physics.velocity = { 0.f, 250.f };
			physics.airDrag = 0;
			projectile.enemy = true;
			projectile.boss = true;
			registry.basicTimers.emplace(entity);
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::APPLE,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				});
			break;
		case PROJECTILE_TYPE_ID::ICICLE:
			motion.scale = { 40, 40 };
			if (hdir > 0) physics.velocity = { 100.f, 200.f };
			if (hdir < 0) physics.velocity = { 100.f, 200.f };
			projectile.enemy = true;
			projectile.boss = true;
			physics.airDrag = 0;
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::BLUEBERRY,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				});
			break;
	}

	registry.foreLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;
}

Entity createSummonable(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = {72., 64.};

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(motion);

	Summonable& summ = registry.summonables.emplace(entity);

	registry.foreLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ROCK,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createHidingSpot(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID asset) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	auto& interactable = registry.interactables.emplace(entity);
	interactable.interactText = "[V] Hide";
	motion.position = pos - vec2({ 0., WALL_TILE_SIZE / 2});
	motion.angle = 0.0f;
	motion.scale = { 2 * WALL_TILE_SIZE, 2 * WALL_TILE_SIZE };
	interactable.type = INTERACTABLE_TYPE_ID::STEALTH;

	Collider& collider = registry.colliders.emplace(entity);
	collider.hull = Collider::getSquareHull(motion);

	registry.renderRequests.insert(
		entity,
		{
			asset,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	registry.backLevelObjs.emplace(entity);
	registry.levelElements.emplace(entity);
	return entity;
}

void emitParticles(vec2 centre, int count, float magnitude, float size, vec3 colour) {
	int maxParticles = 15;
	if (registry.particles.size() < maxParticles) {
		// create particles
		for (uint i = 0; i < count; i++) {
			Entity entity = Entity();

			Motion& m = registry.motions.emplace(entity);
			m.position = centre;
			m.scale = { size, size };

			Collider& coll = registry.colliders.emplace(entity);
			coll.hull = Collider::getSquareHull(m);

			Physics& phys = registry.physicsObjs.emplace(entity);

			Gravity& grav = registry.gravityEntities.emplace(entity);

			float theta = atan2 (-rand() % 50, rand() % 100 - 50);

			phys.velocity = { cos(theta) * magnitude, sin(theta) * magnitude };

			Particle& p = registry.particles.emplace(entity);
			p.ttl = rand() % 300 + 400;

			registry.colors.insert(entity, colour);

			registry.renderRequests.insert(
				entity,
				{
					TEXTURE_ASSET_ID::PARTICLE,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
				}
			);

			registry.foreLevelObjs.emplace(entity);
			registry.levelElements.emplace(entity);
		}
	}
}

Entity createInn(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID sprite, LEVEL_INDEX inn_level /* target level, sprite*/) {
	Entity entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& m = registry.motions.emplace(entity);
	m.position = position + vec2({ WALL_TILE_SIZE / 2, WALL_TILE_SIZE / 2 - INN_HEIGHT / 2 });
	m.scale = { INN_WIDTH, INN_HEIGHT };

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::INN_BASE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	
	registry.backgroundDecors.emplace(entity);
	registry.levelElements.emplace(entity);

	// create door
	
	Entity door = Entity();

	Mesh& doorMesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(door, &doorMesh);

	auto& interactable = registry.interactables.emplace(door);
	interactable.disappears = false; // TODO: change this to set with parameters only if necessary but for now we're assuming things like doors open/close but dont' disappear
	interactable.changes = false;
	interactable.type = INTERACTABLE_TYPE_ID::DOOR;
	interactable.interactText = "[V] Enter the inn";
	registry.renderRequests.insert(
		door,
		{
			TEXTURE_ASSET_ID::NOSPRITE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	LevelRegistry levels;

	auto& transition = registry.transitions.emplace(door);
	transition.nextLevel = *levels.level_list[(int) inn_level];
	transition.nextPos = { 640., 628. };
	transition.needsInteract = true;
	transition.nextLevelIndex = inn_level;

	auto& motion = registry.motions.emplace(door);
	motion.position = position;
	motion.position.y -= WALL_TILE_SIZE / 2;
	motion.angle = 0.0f;
	motion.scale = { WALL_TILE_SIZE, WALL_TILE_SIZE * 2}; // TODO? change this to be the actual size of the door

	Collider& collider = registry.colliders.emplace(door);
	collider.hull = Collider::getSquareHull(motion);
	
	registry.backLevelObjs.emplace(door);
	registry.levelElements.emplace(door);

	return entity;
}
