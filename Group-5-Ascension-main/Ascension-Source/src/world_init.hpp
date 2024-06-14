#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float BUG_BB_WIDTH = 0.6f * 165.f;
const float BUG_BB_HEIGHT = 0.6f * 165.f;
const float EAGLE_BB_WIDTH = 0.6f * 300.f;
const float EAGLE_BB_HEIGHT = 0.6f * 202.f;
const float PLAYER_BB_SIZE = 84.f;
const float ENEMY_BB_SIZE = 84.f;
const float BIRDO_BB_SIZE = 110.f;
const float WALL_TILE_SIZE = 64.f;

const float INN_WIDTH = 512.f;
const float INN_HEIGHT = 512.f;

Entity createGameManager();

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
Entity createDamage(RenderSystem* renderer, vec2 position, vec2 scale, float angle);

// level
Entity createBackground(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID background_asset, vec2 background_scale, float dist);
Entity createCameraTether(RenderSystem* renderer, vec2 pos, vec2 bounds, float zoom);
Entity createSolid(RenderSystem* renderer, vec2 pos, vec2 scale, bool oneWay, int slope, MATERIAL mat);
Entity createGrass(RenderSystem* renderer, vec2 pos);
Entity createHidingSpot(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID asset);

// interactables
Entity createItem(RenderSystem* renderer, vec2 pos, bool disappears, bool changes);
Entity createSpecificItem(RenderSystem* renderer, vec2 pos, vec2 scale, bool disappears, bool changes, TEXTURE_ASSET_ID sprite, ITEM_CONDITION_ID id);
Entity createDoor(RenderSystem* renderer, vec2 createPosition, vec2 targetPlayerPosition, vec2 scale, Level targetLevel, 
	LEVEL_INDEX targetLevelIndex, bool needsInteract, bool visible, std::vector<QUEST_INDEX> requirements, std::string doorText);
Entity createRespawnTether(vec2 pos);
// Entity renderDoor(Entity entity);

Entity createNPC(RenderSystem* renderer, NPCData data);

// combat
Entity createProjectile(RenderSystem* renderer, vec2 pos, vec2 dir, float hdir, PROJECTILE_TYPE_ID projType);
Entity createSummonable(RenderSystem* renderer, vec2 pos);
Entity createEnemy(RenderSystem* renderer, vec2 pos, ENEMY_TYPE_ID enemyType);
Entity createAlert(RenderSystem* renderer, vec2 pos, ENEMY_STATE_ID alert);
Entity createBreakable(RenderSystem* renderer, vec2 position, vec2 scale, TEXTURE_ASSET_ID id, int hp);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

void emitParticles(vec2 centre, int count, float magnitude, float size, vec3 colour);

Entity createInn(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID sprite, LEVEL_INDEX inn_level);