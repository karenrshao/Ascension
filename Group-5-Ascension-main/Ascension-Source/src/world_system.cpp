// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <fstream>		// for reading text files
#include <iostream>
#include <string>
#include <cmath>
#include "physics_system.hpp"
#include "render_system.hpp"

using namespace std;

// Game configuration
const size_t MAX_EAGLES = 1;
const size_t MAX_BUG = 05;
const size_t EAGLE_DELAY_MS = 2000 * 3;
const size_t BUG_DELAY_MS = 5000 * 3;
bool showMovementControls = true;
std::string currLevel = "";
std::string defaultLevel = "../../../src/levels/level1.txt";

// Create the bug world
WorldSystem::WorldSystem()
	: points(0)
	, next_eagle_spawn(0.f)
	, next_bug_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeChunk(background_music);
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int width_px = mode->width;
	int height_px = mode->height;

	// set 4th parameter to "glfwGetPrimaryMonitor()" to make it full screen
	window = glfwCreateWindow(width_px, height_px, "Ascension", glfwGetPrimaryMonitor(), nullptr);

	// windowed version of create window for debugging purposes
	// window = glfwCreateWindow(1280, 720, "Ascension", nullptr, nullptr);

	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto cursor_click_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) {((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, cursor_click_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	// music
	background_music = Mix_LoadWAV(audio_path("BEGINNINGS.wav").c_str());
	second_background_music = Mix_LoadWAV(audio_path("ASCENSION.wav").c_str());
	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());

	if (background_music == nullptr || second_background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present\n",
			audio_path("DANCE_OF_THE_PIXIES_-_IDLE.wav").c_str(),
			audio_path("DANCE_OF_THE_PIXIES_-_CHASE.wav").c_str(),
			audio_path("chicken_dead.wav").c_str(),
			audio_path("chicken_eat.wav").c_str());
		return nullptr;
	}

	// sound effects
	door_open = Mix_LoadWAV(sfx_audio_path("sfx_door_open.wav").c_str());
	door_close = Mix_LoadWAV(sfx_audio_path("sfx_door_close.wav").c_str());
	door_shop = Mix_LoadWAV(sfx_audio_path("sfx_door_shop.wav").c_str());
	sword_swing = Mix_LoadWAV(sfx_audio_path("sfx_sword_swing.wav").c_str());
	interactable_item = Mix_LoadWAV(sfx_audio_path("sfx_interactable_item.wav").c_str());
	landing = Mix_LoadWAV(sfx_audio_path("sfx_landing.wav").c_str());
	damage_taken = Mix_LoadWAV(sfx_audio_path("sfx_damage_taken.wav").c_str());
	run_grass = Mix_LoadWAV(sfx_audio_path("sfx_run_grass.wav").c_str());
	run_stone = Mix_LoadWAV(sfx_audio_path("sfx_run_stone.wav").c_str());
	break_box = Mix_LoadWAV(sfx_audio_path("sfx_break_box.wav").c_str());
	stealth = Mix_LoadWAV(sfx_audio_path("sfx_stealth.wav").c_str());
	create_rock = Mix_LoadWAV(sfx_audio_path("sfx_create_rock.wav").c_str());

	// TODO: add other sound fx to printed string
	if (door_open == nullptr || door_close == nullptr || door_shop == nullptr || sword_swing == nullptr || interactable_item == nullptr ||
		landing == nullptr || damage_taken == nullptr || run_grass == nullptr || run_stone == nullptr || break_box == nullptr || 
		stealth == nullptr || create_rock == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n %s\n make sure the data directory is present\n",
			sfx_audio_path("sfx_door_open.wav").c_str(),
			sfx_audio_path("sfx_door_close.wav").c_str(),
			sfx_audio_path("sfx_door_shop.wav").c_str(),
			sfx_audio_path("sfx_sword_swing.wav").c_str());
		return nullptr;
	}

	return window;
}

// audio on/off functions
void WorldSystem::musicOn() {
	std::cout << "mplay is true" << std::endl;
	music_play = true;
	Mix_Volume(6, 128);
	Mix_Volume(7, 20);
}

void WorldSystem::musicOff() {
	std::cout << "mplay is false" << std::endl;
	music_play = false;
	Mix_Volume(6, 0);
	Mix_Volume(7, 0);
}

void WorldSystem::sfxOn() {
	sfx_play = true;
	Mix_Volume(0, 128);
	Mix_Volume(1, 128);
	Mix_Volume(2, 128);
	Mix_Volume(3, 32); // lower hurt volume
	Mix_Volume(4, 32); // lower door volume
	Mix_Volume(5, 128);
	Mix_Volume(8, 64); // run_stone
	Mix_Volume(9, 128);
	Mix_Volume(10, 16); // break_box quieter
	Mix_Volume(11, 16); // create_rock
}

void WorldSystem::sfxOff() {
	sfx_play = false;
	Mix_Volume(0, 0);
	Mix_Volume(1, 0);
	Mix_Volume(2, 0);
	Mix_Volume(3, 0);
	Mix_Volume(4, 0);
	Mix_Volume(5, 0);
	Mix_Volume(8, 0);
	Mix_Volume(9, 0);
	Mix_Volume(10, 0);
	Mix_Volume(11, 128);
}

void WorldSystem::init(RenderSystem* renderer_arg, LevelRegistry levels) {
	this->renderer = renderer_arg;
	// allocating max audio channels to 20
	Mix_AllocateChannels(20);
	musicOn();
	sfxOn();
	fprintf(stderr, "Loaded music\n");
	// Playing background music indefinitely

	initPlayer();
	gameManager = createGameManager();
	GameManager& gm = registry.gameManagers.get(gameManager);
	gm.state = GAME_STATE::MENU;
	saveData.init();
	registry.gameManagers.get(gameManager).currentLevel = LEVEL_INDEX::CABIN;
	json data = saveData.getGameData();
	if (data.find("current_level") != data.end()) {
		registry.gameManagers.get(gameManager).currentLevel = (LEVEL_INDEX) data["current_level"];
		
	}

	init_game();

	if(data.contains("player")){
		json playerData = data["player"];
		Player& playerStr = registry.players.get(player);
		playerStr.health = playerData["health"];

		Motion& playerMotion = registry.motions.get(player);
		json position = playerData["position"];
		playerMotion.position = {position["x"], position["y"]};
	}

	QuestManager& qm = registry.questmanagers.get(gameManager);

	if (data.contains("quests")) {
		for (int i = 0; i < data["quests"]["conditions"].size(); i++) {
			int item = data["quests"]["conditions"][i];
			qm.conditions[i] = item;
		}
		qm.openQuests = {};
		for (json quest : data["quests"]["open_quests"]) {
			auto quest_pair = quest.items().begin();
			int quest_index = std::stoi(quest_pair.key());

			qm.openQuests.push_back(quest_index);
			questRegistry.quest_list[quest_index]->stage = (QUEST_STAGE) quest_pair.value();
		}
		for (int closed_quest : data["quests"]["closed_quests"]) {
			if (std::find(qm.completedQuests.begin(), qm.completedQuests.end(), closed_quest) == qm.completedQuests.end()) {
				qm.completedQuests.push_back(closed_quest);
			}
		}
	}

	// Player& playerStr = registry.players.get(player);
	// playerStr.health = data["player"]["health"];

	// Motion& playerMotion = registry.motions.get(player);
	// json position = data["player"]["position"];
	// playerMotion.position = {position["x"], position["y"]};

	levels = levels;

	// initPlayer();

	// Set all states to default
}

void WorldSystem::initPlayer() {
	player = createPlayer(renderer, { 256, 628 });
	Camera& cam = registry.cameras.components[0];

	// aspect ratio scaling
	int width_px, height_px;
	glfwGetWindowSize(window, &width_px, &height_px);
	cam.dims[0] = ((float) width_px / (float) height_px) * cam.dims[1];
}

void stepMap(float elapsed_ms) {
	int mapSpeed = 360.;
	float step_seconds = elapsed_ms / 1000.f;
	GameManager& gm = registry.gameManagers.components[0];
	if (gm.state == GAME_STATE::MAP) {

		float mult = 1 + gm.mapTimer / gm.mapTime;
		mult = fmin(4.0f, mult);

		mapSpeed *= mult;

		float hsp, vsp;
		hsp = (gm.key_right - gm.key_left) * step_seconds * mapSpeed;
		vsp = (gm.key_down - gm.key_up) * step_seconds * mapSpeed;
		gm.mapPos += vec2({ hsp, vsp });

		if (hsp != 0 || vsp != 0) {
			gm.mapTimer += elapsed_ms;
		} else {
			gm.mapTimer = 0;
		}
	} else {
		gm.mapTimer = 0;
	}
	float offset_x = registry.cameras.components[0].dims.x / 2 / gm.mapScale;
	float offset_y = registry.cameras.components[0].dims.y / 2 / gm.mapScale;

	gm.mapPos.x = fmax(gm.mapPos.x, offset_x);
	gm.mapPos.y = fmax(gm.mapPos.y, offset_y);

	gm.mapPos.x = fmin(gm.mapPos.x, 10000.f - offset_x);
	gm.mapPos.y = fmin(gm.mapPos.y, 10000.f - offset_y);
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	GameManager& gameManager = registry.gameManagers.components[0];
	gameManager.stealth = registry.stealth.has(registry.players.entities[0]);
	stepMap(elapsed_ms_since_last_update);
	// Handle left/right key releases
	// There was a "bug" previously where if you press the right key while pressing the left key (or vice versa)
	// The player would moonwalk for a brief period of time because keyboard callback can't detect the key release and key
	// presses fast enough, this solution uses a timer whenever the left/right keys are released and if the timer
	// passes a threshold, then the player would stop running, otherwise it will keep running
	Player& playerCom = registry.players.components[0];
	if (playerCom.start_key_left_timer) {
		if (playerCom.key_left_release_timer > 30.f) {
			if (!playerCom.key_right && !playerCom.is_dead) {
				registry.runAnimations.remove(player);
				registry.renderRequests.remove(player);
				registry.renderRequests.insert(
					player,
					{
						TEXTURE_ASSET_ID::PLAYER,
						EFFECT_ASSET_ID::TEXTURED,
						GEOMETRY_BUFFER_ID::SPRITE
					}
				);
			}
			playerCom.start_key_left_timer = false;
		}
		playerCom.key_left_release_timer += elapsed_ms_since_last_update;
	}

	if (playerCom.start_key_right_timer) {
		if (playerCom.key_right_release_timer > 30.f) {
			if (!playerCom.key_left && !playerCom.is_dead) {
				registry.runAnimations.remove(player);
				registry.renderRequests.remove(player);
				registry.renderRequests.insert(
					player,
					{
						TEXTURE_ASSET_ID::PLAYER,
						EFFECT_ASSET_ID::TEXTURED,
						GEOMETRY_BUFFER_ID::SPRITE
					}
				);
			}
			playerCom.start_key_right_timer = false;
		}
		playerCom.key_right_release_timer += elapsed_ms_since_last_update;
	}

	// if you attack while holding down the right/left arrow keys, the key callbacks won't detect
	// the pressing of the right/left keys afterwards and the player will moonwalk, this fixes that bug
	if ((playerCom.key_right || playerCom.key_left) && !registry.attackAnimations.has(player) && !registry.attackUpAnimations.has(player)) {
		if (!registry.runAnimations.has(player)) {
			registry.runAnimations.insert(player, RunAnimation(10, 0));
		}
	}

	if (playerCom.attack_sound) {
		Mix_PlayChannel(2, sword_swing, 0);
		playerCom.attack_sound = false;
	}

	if (gameManager.enemiesInBattle.size() == 0) {
		playerCom.in_battle = false;
	}
	else {
		playerCom.in_battle = true;
	}
	stepBackgrounds();
	Physics& playerPhys = registry.physicsObjs.get(registry.players.entities[0]);
	if (playerPhys.velocity.y == 0 && playerCom.jump_sound) {
		Mix_PlayChannel(1, landing, 0);
		playerCom.jump_sound = false;
	}

	if (gameManager.state == GAME_STATE::PLAY && playerCom.is_running) {
		Player& p = registry.players.components[0];
		if (gameManager.particleTimer <= 0) {
			vec2 pm = registry.motions.get(player).position;	
			emitParticles(pm + vec2({ 0., 32. }), 2, 200, 4, { .5f, .5f, .5f });
			gameManager.particleTimer = gameManager.particleTime;
		}
		else {
			gameManager.particleTimer -= elapsed_ms_since_last_update;
		}
		if (p.on_material == MATERIAL::GRASSY) {
			if (Mix_Playing(8) != 0) {
				Mix_HaltChannel(8);
			}
			if (Mix_Playing(0) == 0) {
				Mix_PlayChannel(0, run_grass, -1);
			}
			
		}
		else if (p.on_material == MATERIAL::STONE) {
			if (Mix_Playing(0) != 0) {
				Mix_HaltChannel(0);
			}
			if (Mix_Playing(8) == 0) {
				Mix_PlayChannel(8, run_stone, -1);
			}
		}
	}
	else {
		Mix_HaltChannel(0);
		Mix_HaltChannel(8);
	}

	std::stringstream title_ss;

	title_ss << gameManager.enemiesInBattle.size();

	glfwSetWindowTitle(window, title_ss.str().c_str());


	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
		Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if (!registry.players.has(motions_registry.entities[i]))// don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// Processing the chicken state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];

	float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
			restart_game();
			return true;
		}
	}

	auto& playerMotion = registry.motions.get(player);

	for (uint i = 0; i < registry.summonables.size(); i++) {
		Entity summonable = registry.summonables.entities[i];

		if (!registry.summonables.get(summonable).active) {
			if (registry.breakables.has(summonable)) {
				Breakable& b = registry.breakables.get(summonable);
				b.hp -= elapsed_ms_since_last_update;
				if (b.hp <= 0) {
					emitParticles(registry.motions.get(summonable).position, 4, 150, 6, { .3f, .3f, .3f });
					registry.remove_all_components_of(summonable);
				}
				if (b.hp <= 1250) {
					if (!registry.invincibilityTimers.has(summonable)) {
						registry.invincibilityTimers.insert(summonable, InvincibilityTimer(1250, 6));
					}
				}
			}
			if (registry.physicsObjs.has(summonable) &&
				!registry.physicsObjs.get(summonable).inAir &&
				registry.dmgTimers.has(summonable)) {
				emitParticles(registry.motions.get(summonable).position, 4, 150, 6, { .3f, .3f, .3f });
				registry.remove_all_components_of(summonable);
			}
			continue;
		}

		if (playerCom.is_summoning && playerCom.key_attack) {

			auto& sMotion = registry.motions.get(summonable);

			float diff = playerMotion.position.x - sMotion.position.x;

			for (int i = 0; i < 10; i++) {
				int facing = (diff >= 0) - (diff < 0);
				float offset = i * 48.0;
				/*
				equation: y = -(offset - pm.x)^2 + pm.y + 64
				f(0) = 0;
				- sqrt(0 - q) 
				*/
				float q = -256.f;
				float p = sqrt(-q);
				// float yy = 0.0006 * pow(offset , 2) + 0.003 * -abs(diff) * offset;
				float yy = pow(3.f/abs(diff) * offset - p, 2) + q;
				Entity l1 = createLine({sMotion.position.x + facing * offset, sMotion.position.y + yy}, {4, 4});
				registry.colors.insert(l1, {255.0, 255.0, 255.0});
			}
		}
	}

	if (gameManager.musicTimer > 0) {
		gameManager.musicTimer -= elapsed_ms_since_last_update;
	}
	if (gameManager.decoyTimer > 0) {
		gameManager.decoyTimer -= elapsed_ms_since_last_update;
	}

	if (playerCom.dash_timer > 0) {
		playerCom.dash_timer -= elapsed_ms_since_last_update;
	}

	if (registry.boss.entities.size() > 0){
		Boss& boss = registry.boss.components[0];
		if (boss.startTimer > 0) {
			boss.startTimer -= elapsed_ms_since_last_update;
		}
	}

	for (Entity e : registry.basicTimers.entities) {
		BasicTimer& timer = registry.basicTimers.get(e);
		if (timer.counter_ms > 0) {
			timer.counter_ms -= elapsed_ms_since_last_update;
		}
		else {
			registry.remove_all_components_of(e);
		}
	}


	for (Entity e : registry.patrols.entities) {
		Patrol& patrol = registry.patrols.get(e);
		if (patrol.patrolTimer > 0) {
			patrol.patrolTimer -= elapsed_ms_since_last_update;
		}
	}

	for (Entity entity : registry.invincibilityTimers.entities) {
		InvincibilityTimer& invincibilityTimer = registry.invincibilityTimers.get(entity);
		if (invincibilityTimer.flash_count > 0) {
			invincibilityTimer.flash_count -= elapsed_ms_since_last_update;
		}
		else {
			invincibilityTimer.flash_count = invincibilityTimer.flash_counter_ms;
			invincibilityTimer.flash = !invincibilityTimer.flash;
			invincibilityTimer.curr_flash_count++;
		}
		if (invincibilityTimer.invinc_counter_ms > 0) {
			invincibilityTimer.invinc_counter_ms -= elapsed_ms_since_last_update;
		}
		else {
			if (registry.players.has(entity)) {
				Player& playerStr = registry.players.get(entity);
				playerStr.hurt = false;
			}
			else if (registry.breakables.has(entity)) {
				Breakable& breakable = registry.breakables.get(entity);
				breakable.hp--;
				if (breakable.hp <= 0) {
					emitParticles(registry.motions.get(entity).position, 4, 150, 6, { .3f, .3f, .3f });
					registry.remove_all_components_of(entity);
					continue;
				}
			}
			registry.invincibilityTimers.remove(entity);
		}

	}

	for (Entity entity : registry.dmgTimers.entities) {
		DamageTimer& dmgTimer = registry.dmgTimers.get(entity);
		Player& playerStr = registry.players.get(player);
		if (dmgTimer.counter_ms > 0) {
			dmgTimer.counter_ms -= elapsed_ms_since_last_update;
			Motion& motion = registry.motions.get(entity);
			Motion& player_motion = registry.motions.get(player);
			// up attacks
			if (playerStr.is_up && registry.attackUpAnimations.has(player)) {
				motion.position.x = player_motion.position.x;
				motion.position.y = player_motion.position.y - (player_motion.scale.y / 2.0);
			}
			else if (registry.attackAnimations.has(player)) { // side attacks
				motion.position.x = player_motion.position.x + (player_motion.scale.x / 1.5);
				motion.position.y = player_motion.position.y + (player_motion.scale.y / 5.0);
			}
			
	
		} else {
			registry.remove_all_components_of(entity);
		}
	}

	for (Entity entity : registry.deadlys.entities) {
		if (registry.motions.has(entity)) {
			Deadly& deadly = registry.deadlys.get(entity);
			Physics& physics = registry.physicsObjs.get(entity);

			//the run animation of birds should stop if they are stationary (they should still flap their wings)
			if (deadly.type != ENEMY_TYPE_ID::BIRDO) {
				if (physics.velocity.x != 0) {
					if (!registry.runAnimations.has(entity)) {
						switch (deadly.type) {
							case ENEMY_TYPE_ID::TREE:
								registry.runAnimations.insert(entity, RunAnimation(8, 1));
								break;
							case ENEMY_TYPE_ID::GUARD:
								registry.runAnimations.insert(entity, RunAnimation(10, 12));
								break;
							case ENEMY_TYPE_ID::GUARDCAPTAIN:
								registry.runAnimations.insert(entity, RunAnimation(10, 13));
								break;
							case ENEMY_TYPE_ID::RAT:
								registry.runAnimations.insert(entity, RunAnimation(9, 11));
								break;

						}
						//registry.runAnimations.insert(entity, RunAnimation(8, 1));
					}
		
				}
				else {
					switch (deadly.type) {
						case ENEMY_TYPE_ID::TREE:
							registry.runAnimations.remove(entity);
							registry.renderRequests.remove(entity);
							registry.renderRequests.insert(
								entity,
								{
									TEXTURE_ASSET_ID::ENEMY,
									EFFECT_ASSET_ID::TEXTURED,
									GEOMETRY_BUFFER_ID::SPRITE
								}
							);
							break;
						case ENEMY_TYPE_ID::GUARD:
							registry.runAnimations.remove(entity);
							registry.renderRequests.remove(entity);
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
							registry.runAnimations.remove(entity);
							registry.renderRequests.remove(entity);
							registry.renderRequests.insert(
								entity,
								{
									TEXTURE_ASSET_ID::GUARD_CAPTAIN,
									EFFECT_ASSET_ID::TEXTURED,
									GEOMETRY_BUFFER_ID::SPRITE
								}
							);
							break;
						//case ENEMY_TYPE_ID::RAT:
						//	registry.runAnimations.remove(entity);
						//	registry.renderRequests.remove(entity);
						//	registry.renderRequests.insert(
						//		entity,
						//		{
						//			TEXTURE_ASSET_ID::SEWER_SLIME_1,
						//			EFFECT_ASSET_ID::TEXTURED,
						//			GEOMETRY_BUFFER_ID::SPRITE
						//		}
						//	);
					}
				}
			}

			if (deadly.alertTimer > 0) {
				deadly.alertTimer -= elapsed_ms_since_last_update;
			}
			if (deadly.attackTimer > 0) {
				deadly.attackTimer -= elapsed_ms_since_last_update;
			}
			if (deadly.chaseTimer > 0) {
				deadly.chaseTimer -= elapsed_ms_since_last_update;
			}
		}
	}

	for (auto& p : registry.projectiles.entities) {
		Projectile& proj = registry.projectiles.get(p);
		if (proj.timer > 0) {
			proj.timer -= elapsed_ms_since_last_update;
		}
		else if (proj.landed) {
			registry.remove_all_components_of(p);
		}
	}

	Motion& motion = registry.motions.get(player);
	Mob& mob = registry.mobs.get(player);

	// assuming the player will always be present here
	Physics& physics = registry.physicsObjs.get(player);
	enterBattle(playerCom.in_battle);
	
	bool inSpeakerRange = false;

	// check speakers here
	for (auto entity_speaker : registry.speakers.entities) {
		// if in range, do dialogue. let doDialogue handle key check vs. no key check
		Motion& playerMotion = registry.motions.get(player);
		Motion& speakerMotion = registry.motions.get(entity_speaker);
		Speaker& speaker = registry.speakers.get(entity_speaker);

		vec2 playerPos = playerMotion.position, speakerPos = speakerMotion.position;

		if (
			playerPos.x > speakerPos.x - speaker.autoTriggerRadii.x &&
			playerPos.x < speakerPos.x + speaker.autoTriggerRadii.x &&
			playerPos.y > speakerPos.y - speaker.autoTriggerRadii.y &&
			playerPos.y < speakerPos.y + speaker.autoTriggerRadii.y
			) {
			if (gameManager.state == GAME_STATE::PLAY || (
				(playerCom.key_interact || playerCom.key_decline) && gameManager.state == GAME_STATE::DIALOGUE
				)) {
				doDialogue(player, entity_speaker);
			}
			inSpeakerRange = true;
		}
	}
	if (!inSpeakerRange && gameManager.state == GAME_STATE::DIALOGUE) gameManager.state = GAME_STATE::PLAY;

	//if (mob.falling && physics.inAir) {
	//if (physics.inAir) {
	//	//std::cout << "world system detected that the player is falling " << elapsed_ms_since_last_update << std::endl;
	//	registry.jumpUpAnimations.remove(player);
	//	registry.runAnimations.remove(player);
	//	registry.jumpUpAnimations.insert(player, JumpUpAnimation(5, 3));
	//	playerStruct.is_falling = true;
	//}

	//if (!physics.inAir && playerStruct.is_falling && physics.velocity.y == 0) {
	//	//std::cout << "world system detected that the player has landed from a fall " << elapsed_ms_since_last_update << std::endl;
	//	playerStruct.is_falling = false;
	//	registry.renderRequests.remove(player);
	//	registry.renderRequests.insert(
	//		player,
	//		{
	//			TEXTURE_ASSET_ID::PLAYER,
	//			EFFECT_ASSET_ID::TEXTURED,
	//			GEOMETRY_BUFFER_ID::SPRITE
	//		}
	//	);
	//}


	//Player& playerStruct = registry.players.get(player);

	//if (motion.on_solid && !playerStruct.key_left && !playerStruct.key_right && !playerStruct.key_jump) {
	//	//registry.runAnimations.remove(player);
	//	if()
	//	registry.renderRequests.remove(player);
	//	registry.renderRequests.insert(
	//		player,
	//		{
	//			TEXTURE_ASSET_ID::PLAYER,
	//			EFFECT_ASSET_ID::TEXTURED,
	//			GEOMETRY_BUFFER_ID::SPRITE
	//		}
	//	);

	//}
	
	if (registry.motions.get(player).position.y > gameManager.bounds.y + 256.) {
		Player& playerComp = registry.players.get(player);
		Motion& pMotion = registry.motions.get(player);
		
		if (!registry.deathTimers.has(player)) {
			playerComp.health -= 1;

			if (playerComp.health <= 0) {
				playerComp.health = 0;
				registry.deathTimers.emplace(player);
				Mix_PlayChannel(-1, chicken_dead_sound, 0);
			} else {
				pMotion.position = playerComp.savedPosition;
				if (!registry.invincibilityTimers.has(player)) {
					registry.invincibilityTimers.insert(player, InvincibilityTimer(800, 6));
				}
			}
		}
	}

	// reduce window brightness if any of the present chickens is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	for (Entity particle : registry.particles.entities) {
		Particle& p = registry.particles.get(particle);
		if (p.ttl <= 0) registry.remove_all_components_of(particle);
		p.ttl -= elapsed_ms_since_last_update;
	}

	return true;
}


void WorldSystem::checkQuests(bool canComplete, int curQuest) {
	QuestManager& qm = registry.questmanagers.get(gameManager);

	std::vector<int> completedQuests; // indexes of open quests in openQuests
	std::vector<int> questsToClose;
	// 
	for (int i = qm.openQuests.size() - 1; i >= 0; i--) {
		int q = qm.openQuests[i];
		bool questComplete = true;

		Quest& tq = *questRegistry.quest_list[q];

		if (tq.stage == QUEST_STAGE::START) continue;
		
		map<ITEM_CONDITION_ID, int>::iterator it;
		std::cout << "Checking quest " << tq.name << "..." << std::endl;

		for (it = tq.requirements.begin(); it != tq.requirements.end(); it++) {
			std::cout << "  Checking inventory for " << (int)it->first << ": has " << qm.conditions[(int)it->first] << ", needs " << it->second;
			if (qm.conditions[(int) it->first] < it->second) {
				questComplete = false;
			}
		}

		if (questComplete) {
			// set conditions for deletion
			if (tq.stage == QUEST_STAGE::END && canComplete && q == curQuest) {
				std::cout << " Quest Complete!" << std::endl;
				// subtract from conditions
				for (it = tq.requirements.begin(); it != tq.requirements.end(); it++) {
					qm.conditions[(int)it->first] -= it->second;
				}
				// add rewards
				for (it = tq.rewards.begin(); it != tq.rewards.end(); it++) {
					qm.conditions[(int)it->first] += it->second;
				}

				for (int nq : tq.childQuests) {
					Quest openQuest = *questRegistry.quest_list[nq];
					std::cout << "New quest unlocked: " << openQuest.name << std::endl;
					if (std::find(qm.openQuests.begin(), qm.openQuests.end(), nq) == qm.openQuests.end())
						// if quest doesn't exist, add it
						qm.openQuests.push_back(nq);
				}

				// if the quest is already ended, prepare it for deletion
				// completedQuests.push_back(i);
				questsToClose.push_back(q);

				// add quest to completed quests list
				if (std::find(qm.completedQuests.begin(), qm.completedQuests.end(), q) == qm.completedQuests.end()) {
					qm.completedQuests.push_back(q); // add quest_index q to completedQuests
				}
				for (QUEST_INDEX qi : tq.questsToClose) {
					// find their index in 
					questsToClose.push_back((int) qi);
				}
			} else {
				// otherwise, end it
				tq.stage = QUEST_STAGE::END;
				std::cout << "Entering end stage" << std::endl;
			}
		}
		else {
			std::cout << "Still not done" << std::endl;
		}

	}

	for (int qtc : questsToClose) {
		std::vector<int>::iterator it;

		// if the quest is open, close it
		if ( (it = std::find(qm.openQuests.begin(), qm.openQuests.end(), qtc)) != qm.openQuests.end() ) {
			qm.openQuests.erase(it);
			// unlock new quests
			if ((QUEST_INDEX) qtc == QUEST_INDEX::GATHER_FRUITS || (QUEST_INDEX)qtc == QUEST_INDEX::FIND_BROOCH) {
				saveData.setSpawn();
			}
		}
	}

	saveData.savePlayer();
	saveData.saveGame();
}

void WorldSystem::addToInventory(Item& item) {
	QuestManager& qm = registry.questmanagers.get(gameManager);

	qm.conditions[(int)item.item_id]++;

	for (int item_id = 0; item_id < sizeof(qm.conditions) / sizeof(int); item_id++) {
		std::cout << "   ITEM_" << item_id << ": " << qm.conditions[item_id] << std::endl;
	}
}

void WorldSystem::stepBackgrounds() {
	auto& playerCamera = registry.cameras.get(player);
	auto& background_registry = registry.backgrounds;

	for (uint i = 0; i < background_registry.size(); i++) {
		Entity backgroundEntity = background_registry.entities[i];

		Background& background = background_registry.get(backgroundEntity);
		Motion& backgroundMotion = registry.motions.get(backgroundEntity);

		backgroundMotion.position[0] = background.dist * (playerCamera.position[0] - background.initPos[0]) + background.initPos[0];
		backgroundMotion.position[1] = 0.8 * background.dist * (playerCamera.position[1] - background.initPos[1]) + background.initPos[1];
	}

	return;
}

void loadGeometryFile(RenderSystem* renderer, Level& level, bool loadFromNew) {

	std::string levelFile = level_path(level.levelFile);;

	fstream file(levelFile);
	float starting_x_coord = 0.0, max_width = 0.0;
	int line_num = 0;

	// TEMP
	// createDoor(renderer, { 512., 628. }, { 1000., 1000. }, levels.forestLevel, false);

	if (file.is_open()) {
		string stringLine;
		while (getline(file, stringLine)) {
			for (uint i = 0; i < stringLine.size(); i++) {
				char c = stringLine[i];
				// 1, 5, r, l: stone
				// G, g, <, >: grassy
				if (c == '1' || c == '5' || c == 'r' || c == 'l' ||
					c == 'G' || c == 'g' || c == '>' || c == '<' ) {
					int solidRowSize = 1;
					uint j = i + 1;
					float end_x_coord = starting_x_coord;
					while (j < stringLine.size() && stringLine[j] == c) {
						j++;
						solidRowSize++;
						end_x_coord += WALL_TILE_SIZE;
					}
					i = j - 1;
					float centre_x = (starting_x_coord + end_x_coord) / 2, width = solidRowSize * WALL_TILE_SIZE;
					Entity solid = createSolid(renderer, 
							{ centre_x, line_num * WALL_TILE_SIZE }, 
							{ width, WALL_TILE_SIZE }, 
							(c == '5' || c == 'g'), 
							(c == 'r' || c == 'l' || c == '<' || c == '>') ? (int) (c == 'r' || c == '>') - (c == 'l' || c == '<') : 0,
							(c == '1' || c == '5' || c == 'r' || c == 'l') ? MATERIAL::STONE : MATERIAL::GRASSY
						);
					starting_x_coord += (solidRowSize - 1) * WALL_TILE_SIZE;
					registry.invisibles.emplace(solid);
				}
				else if (c == '2') {
					createGrass(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE });
				}
				else if (c == 'i' && loadFromNew) {
					createSpecificItem(renderer,
						{ starting_x_coord, line_num * WALL_TILE_SIZE },
						{ 64, 64 },
						true, false,
						TEXTURE_ASSET_ID::APPLE,
						ITEM_CONDITION_ID::FRUIT);
				}
				else if (c == 'c' && loadFromNew) {
					createSpecificItem(renderer,
						{ starting_x_coord, line_num * WALL_TILE_SIZE },
						{ 64, 64 },
						true, false,
						TEXTURE_ASSET_ID::COIN,
						ITEM_CONDITION_ID::COIN);
				}
				else if (c == '3' && loadFromNew) {
					createEnemy(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, ENEMY_TYPE_ID::TREE);
				}
				else if (c == '4') {
					createCameraTether(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, { 1080, 720 }, 1.4);
				}
				else if (c == '6' && loadFromNew) {
					createEnemy(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, ENEMY_TYPE_ID::BIRDO);
				}
				else if (c == 'b') {
					createHidingSpot(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, TEXTURE_ASSET_ID::BUSH);
				}
				else if (c == 'p') {
					createHidingSpot(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, TEXTURE_ASSET_ID::VASE);
				}
				else if (c == '7' && loadFromNew) {
					createEnemy(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, ENEMY_TYPE_ID::GUARD);
				}
				else if (c == '8' && loadFromNew) {
					createEnemy(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, ENEMY_TYPE_ID::GUARDCAPTAIN);
				}
				else if (c == '9' && loadFromNew) {
					createEnemy(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, ENEMY_TYPE_ID::BIRDMAN);
				}
				else if (c == 'Z' && loadFromNew) {
					createEnemy(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, ENEMY_TYPE_ID::RAT);
				}
				else if (c == 'V' && loadFromNew) {
					createEnemy(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, ENEMY_TYPE_ID::BOSS);
				}
				else if (c == 'B' && loadFromNew) {
					createSpecificItem(renderer,
						{ starting_x_coord, line_num * WALL_TILE_SIZE }, 
						{ 64, 64 }, 
						true, false, 
						TEXTURE_ASSET_ID::SPARKLE_1,
						ITEM_CONDITION_ID::BROOCH);
				}
				else if (c == 'R' && loadFromNew) {
					createSpecificItem(renderer,
						{ starting_x_coord, line_num * WALL_TILE_SIZE },
						{ 64, 64 },
						true, false,
						TEXTURE_ASSET_ID::ROPE,
						ITEM_CONDITION_ID::ROPE);
				}
				else if (c == 'S' && loadFromNew) {
					createSpecificItem(renderer,
						{ starting_x_coord, line_num * WALL_TILE_SIZE },
						{ 64, 64 },
						true, false,
						TEXTURE_ASSET_ID::SPARKLE_1,
						ITEM_CONDITION_ID::SCALE);
				}
				else if (c == 'X' && loadFromNew) {
					createBreakable(renderer, 
						{ starting_x_coord, line_num * WALL_TILE_SIZE }, 
						{ 64, 64 },
						TEXTURE_ASSET_ID::BREAKABLE_BOX,
						1);
				}
				else if (c == 'T') {
					createRespawnTether({ starting_x_coord, line_num * WALL_TILE_SIZE });
				} 
				else if (c == 'I') {
					// create inn
					createInn(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, level.inn_sprite, level.inn_target_level);
				}
				else if (c == 'N' && loadFromNew) {
					createEnemy(renderer, { starting_x_coord, line_num * WALL_TILE_SIZE }, ENEMY_TYPE_ID::RAT);
				}
				starting_x_coord += WALL_TILE_SIZE;
			}
			max_width = starting_x_coord > max_width ? starting_x_coord : max_width;
			starting_x_coord = 0.0;
			line_num++;
		}
	}

	file.close();

	Camera& cam = registry.cameras.components[0];
	GameManager& game = registry.gameManagers.components[0];
	cam.bounds = { max_width, line_num * WALL_TILE_SIZE };
	game.bounds = cam.bounds;
}


void WorldSystem::loadLevel(Level& level, LEVEL_INDEX li) {
	// with the current window size, this function expects an 11 x 21 rectangle of 0s and 1s

	for (auto& b : level.backgrounds) {
		createBackground(
			renderer,
			b.initPos,
			b.asset,
			b.scale,
			b.dist
		);
	}

	// Create a new chicken

	// render the arrow keys to move prompt above player
	if (showMovementControls == true) {
		showMovementControls = false;
		registry.drawInits.emplace(player);
		registry.drawInits.get(player).x = registry.motions.get(player).position.x;
		registry.drawInits.get(player).y = registry.motions.get(player).position.y;
	}

	json data = saveData.getGameData();

	if ((int) li >= data["levels"].size() || data["levels"][(int) li] == nullptr) {
		loadGeometryFile(renderer, level, true);
	}
	else {
		json thisLevel = data["levels"][(int)li];
		loadGeometryFile(renderer, level, false);
		for (json item : thisLevel["items"]) {
			vec2 pos = { item["position"]["x"], item["position"]["y"] };
			createSpecificItem(renderer, 
				pos, 
				{ 64., 64. }, 
				true, false, 
				(TEXTURE_ASSET_ID) item["sprite"],
				(ITEM_CONDITION_ID) item["type"]
			);
		}
		for (json enemy : thisLevel["enemy"]) {
			vec2 pos = { enemy["position"]["x"], enemy["position"]["y"] };
			int enemyType = enemy["type"];
			createEnemy(renderer, pos, (ENEMY_TYPE_ID) enemyType);
		}
		for (json breakable : thisLevel["breakables"]) {
			vec2 pos = { breakable["position"]["x"], breakable["position"]["y"] };
			vec2 scale = { breakable["scale"]["x"], breakable["scale"]["y"] };
			createBreakable(renderer, 
				pos, 
				scale,
				(TEXTURE_ASSET_ID) breakable["sprite"],
				(int) breakable["hp"]);
		}
	}

	// load transitions
	for (auto& t : level.doors) {
		createDoor(renderer, t.createPosition, t.nextPos, t.scale, *levels.level_list[t.levelIndex], 
			(LEVEL_INDEX) t.levelIndex, t.needsInteract, t.visible, t.requirements, t.doorText);
	}

	for (auto& n : level.npcs) {
		createNPC(renderer, n);
	}

	registry.gameManagers.get(gameManager).mapPos = level.mapPos;
}

// Reset the world state to its initial state
void WorldSystem::init_game() {
	json data = saveData.getGameData();

	if (data.find("current_level") != data.end()) {
		registry.gameManagers.get(gameManager).currentLevel = (LEVEL_INDEX)data["current_level"];
	}
	// DO RESPAWN HERE, RESTART AT SPAWNPOINT
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;


	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.levelElements.entities.size() > 0)
	    registry.remove_all_components_of(registry.levelElements.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	resume_game = Entity();
	quit_game = Entity();
	start_game = Entity();
	display_help = Entity();
	pause_game = Entity();

	// dont do this or else buttons will be cleared on level change
	// registry.levelElements.emplace(resume_game);
	// registry.levelElements.emplace(quit_game);
	// registry.levelElements.emplace(start_game);

	registry.buttons.insert(resume_game, Button({ registry.cameras.components[0].dims.x / 2.f - 300, registry.cameras.components[0].dims.y/2.f }, {200, 100}, "Resume", TEXTURE_ASSET_ID::BREAKABLE_BOX, BUTTON_FUNCTION::RESUME_GAME));
	registry.buttons.insert(quit_game, Button({ registry.cameras.components[0].dims.x / 2.f + 100, registry.cameras.components[0].dims.y / 2.f }, { 200, 100 }, "Quit", TEXTURE_ASSET_ID::BREAKABLE_BOX, BUTTON_FUNCTION::QUIT_GAME));
	registry.buttons.insert(start_game, Button({ registry.cameras.components[0].dims.x / 2.f - 400, registry.cameras.components[0].dims.y / 2.f - 300 }, { 200, 100 }, "Start", TEXTURE_ASSET_ID::BREAKABLE_BOX, BUTTON_FUNCTION::START_GAME));
	registry.buttons.insert(display_help, Button({ registry.cameras.components[0].dims.x - 80, registry.cameras.components[0].dims.y - 80 }, { 70, 70 }, "?", TEXTURE_ASSET_ID::BREAKABLE_BOX, BUTTON_FUNCTION::DISPLAY_HELP));

	registry.players.get(player).is_dead = false;
	registry.players.get(player).health = 5;
	
	LEVEL_INDEX li = registry.gameManagers.get(gameManager).currentLevel;
	Level& levelToLoad = *levels.level_list[(int) li];

	registry.motions.get(player).position = levelToLoad.startPos;
	registry.players.get(player).is_dead = false;
	registry.players.get(player).in_battle = false;

	background_music = Mix_LoadWAV(audio_path(levelToLoad.background_track).c_str());
	second_background_music = Mix_LoadWAV(audio_path(levelToLoad.second_background_track).c_str());

	// Playing background music indefinitely
	Mix_PlayChannel(6, background_music, -1); //CHANNEL 6: normal theme
	Mix_PlayChannel(7, second_background_music, -1); //CHANNEL 7: battle theme 

	// reset player position
	if (data.contains("player")) {
		json playerData = data["player"];
		Player& playerStr = registry.players.get(player);
		playerStr.health = playerData["health"];

		Motion& playerMotion = registry.motions.get(player);
		json position = playerData["position"];
		playerMotion.position = { position["x"], position["y"] };
	}

	QuestManager& qm = registry.questmanagers.get(gameManager);

	// load level
	loadLevel(levelToLoad, li); // default level

}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	/*
	* 
	gameManager = createGameManager();
	registry.gameManagers.get(gameManager).currentLevel = LEVEL_INDEX::CABIN;
	json data = saveData.getGameData();

	restart_game();
	*/
	json data = saveData.getRespawnData();
	saveData.setGameData(data);
	if (data.find("current_level") != data.end()) {
		registry.gameManagers.get(gameManager).currentLevel = (LEVEL_INDEX)data["current_level"];
	}
	// DO RESPAWN HERE, RESTART AT SPAWNPOINT
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;


	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.levelElements.entities.size() > 0)
	    registry.remove_all_components_of(registry.levelElements.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();


	// // dont do this or else buttons will be cleared on level change
	// // registry.levelElements.emplace(resume_game);
	// // registry.levelElements.emplace(quit_game);
	// // registry.levelElements.emplace(start_game);

	// registry.buttons.insert(resume_game, Button({ registry.cameras.components[0].dims.x / 2.f - 300, registry.cameras.components[0].dims.y/2.f }, {200, 100}, TEXTURE_ASSET_ID::BREAKABLE_BOX, BUTTON_FUNCTION::RESUME_GAME));
	// registry.buttons.insert(quit_game, Button({ registry.cameras.components[0].dims.x / 2.f + 100, registry.cameras.components[0].dims.y / 2.f }, { 200, 100 }, TEXTURE_ASSET_ID::BREAKABLE_BOX, BUTTON_FUNCTION::QUIT_GAME));

	// registry.players.get(player).is_dead = false;
	// registry.players.get(player).health = 5;
	// registry.renderRequests.remove(player);
	// registry.renderRequests.insert(
	// 	player,
	// 	{
	// 		TEXTURE_ASSET_ID::PLAYER,
	// 		EFFECT_ASSET_ID::TEXTURED,
	// 		GEOMETRY_BUFFER_ID::SPRITE
	// 	}
	// );
	registry.renderRequests.remove(player);
	registry.renderRequests.insert(
		player,
		{
			TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	
	LEVEL_INDEX li = registry.gameManagers.get(gameManager).currentLevel;
	Level& levelToLoad = *levels.level_list[(int) li];

	GameManager& gm = registry.gameManagers.components[0];
	gm.enemiesInBattle.clear();
  
	Player& pc = registry.players.get(player);

	registry.motions.get(player).position = levelToLoad.startPos;
	pc.is_dead = false;
	pc.health = pc.spawnHealth;
	pc.in_battle = false;

	// reset player position
	if (data.contains("player")) {
		json playerData = data["player"];
		Player& playerStr = registry.players.get(player);
		playerStr.health = playerData["health"];

		Motion& playerMotion = registry.motions.get(player);
		json position = playerData["position"];
		playerMotion.position = { position["x"], position["y"] };
	}

	QuestManager& qm = registry.questmanagers.get(gameManager);

	if (data.contains("quests")) {
		for (int i = 0; i < data["quests"]["conditions"].size(); i++) {
			int item = data["quests"]["conditions"][i];
			qm.conditions[i] = item;
		}
		qm.openQuests = {};
		for (json quest : data["quests"]["open_quests"]) {
			auto quest_pair = quest.items().begin();
			int quest_index = std::stoi(quest_pair.key());

			qm.openQuests.push_back(quest_index);
			questRegistry.quest_list[quest_index]->stage = (QUEST_STAGE)quest_pair.value();
		}
		for (int closed_quest : data["quests"]["closed_quests"]) {
			qm.completedQuests.push_back(closed_quest);
		}
	}

	Player& p = registry.players.get(player);
	std::cout << "resetting player in battle to false" << std::endl;
	p.in_battle = false;

	// load level
	loadLevel(levelToLoad, li); // default level

}

void WorldSystem::transitionLevel(Transition& t) {
	// Debugging for memory/component leaks
	registry.list_all_components();

	Level targetLevel = t.nextLevel;
	QuestManager& qm = registry.questmanagers.get(gameManager);

	// check gameManager
	for (QUEST_INDEX req : t.requirements) {
		// if one of the reqs isn't complete, don't transition
		if (std::find(qm.completedQuests.begin(), qm.completedQuests.end(), (int) req) == qm.completedQuests.end()) {
			return;
		}
	}

	saveData.saveLevel();

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.transitions.entities.size() > 0)
		registry.remove_all_components_of(registry.transitions.entities.back());

	while (registry.speakers.entities.size() > 0)
		registry.remove_all_components_of(registry.speakers.entities.back());

	while (registry.levelElements.entities.size() > 0)
		registry.remove_all_components_of(registry.levelElements.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();
	
	/*************************/
	// SWITCHING SCENE MUSIC
	// Mix_HaltMusic();

	// maybe don't need halt, just try loading new track and playing

	// Mix_FreeMusic(background_music);
	background_music = Mix_LoadWAV(audio_path(targetLevel.background_track).c_str());
	second_background_music = Mix_LoadWAV(audio_path(targetLevel.second_background_track).c_str());
	Mix_PlayChannel(6, background_music, -1);
	Mix_PlayChannel(7, second_background_music, -1);
	
	/************************/

	registry.players.get(player).is_dead = false;
	registry.players.get(player).in_battle = false;

	registry.gameManagers.get(gameManager).currentLevel = t.nextLevelIndex;


	// WE CAN DO LEVEL LOAD AND UNLOAD ANIMATIONS HERE IF YOU WANT
	Camera& cam = registry.cameras.components[0];
	Motion& playerMotion = registry.motions.get(player);

	playerMotion.position = t.nextPos;

	// load level
	loadLevel(targetLevel, t.nextLevelIndex);

	int halfWidth = cam.zoom * cam.dims.x / 2, halfHeight = cam.zoom * cam.dims.y / 2;

	cam.targetPosition[0] = fmin(playerMotion.position[0], cam.bounds[0] - halfWidth);
	cam.targetPosition[0] = fmax(playerMotion.position[0], halfWidth - 32);
	cam.targetPosition[1] = fmin(playerMotion.position[1], cam.bounds[1] - halfHeight - 16);
	cam.targetPosition[1] = fmax(playerMotion.position[1], halfHeight + 32);

	cam.position = cam.targetPosition;

	GameManager& gm = registry.gameManagers.components[0];
	gm.enemiesInBattle.clear();
	saveData.savePlayer();
	// writing to file 
	saveData.saveGame();
}

void WorldSystem::doDialogue(Entity entity, Entity entity_other) {
	Speaker& speaker = registry.speakers.get(entity_other);
	GameManager& gm = registry.gameManagers.get(gameManager);
	QuestManager& qm = registry.questmanagers.get(gameManager);
	// break into complex dialogue
	int foundQuest = -1;
	std::map <QUEST_INDEX, DialogueSequence>::iterator it;

	if (speaker.nextLine == 0 && 
		speaker.complexLine == 0 && 
		!registry.players.get(player).key_decline && 
		registry.players.get(player).key_interact &&
		gm.state == GAME_STATE::DIALOGUE) {
		gm.state = GAME_STATE::PLAY;
		registry.players.get(player).key_decline = false;
		registry.players.get(player).key_interact = false;
	}
	else {
		for (int q : qm.openQuests) {
			if ((it = speaker.complexDialogueSequences.find((QUEST_INDEX)q)) != speaker.complexDialogueSequences.end()) {

				/**************************** CHECK THIS QUEST ****************************/
				Quest& tq = *questRegistry.quest_list[q];
				QUEST_STAGE stage = tq.stage;

				Player& playerComp = registry.players.get(player);

				// check quests 
				if (stage == QUEST_STAGE::DURING && playerComp.key_interact) {
					checkQuests(true, q);
				}
				stage = tq.stage;

				foundQuest = q;

				//
				DialogueSequence& foundDS = it->second;

				// found a quest, now choose which one to display
				// get the current line
				DialogueStage& dStage = foundDS.sequence[stage];

				if (
					playerComp.key_interact || 
					dStage.autoTrigger || 
					(
						playerComp.key_decline && 
						dStage.cur_stage == DIALOGUE_BRANCH::PRIMARY
					)
				) {
					std::cout << "Starting dialogue" << std::endl;
					vector<DialogueLine> branch;
					// trigger next dialogue stage
					if (dStage.cur_stage == DIALOGUE_BRANCH::PRIMARY) {
						branch = dStage.primary;
						if (speaker.complexLine == -1) {
							std::cout << "End of dialogue" << std::endl;
							if (playerComp.key_interact) {
								dStage.cur_stage = DIALOGUE_BRANCH::YES;
								std::cout << "Player chose yes" << std::endl;
								branch = dStage.yesBranch;

							} else if (playerComp.key_decline) {
								dStage.cur_stage = DIALOGUE_BRANCH::NO;
								std::cout << "Player chose no" << std::endl;
								branch = dStage.noBranch;

							}
							speaker.complexLine = 0;
						} else {
							if (playerComp.key_decline) break;
						}
					}
					// isolate this from previous if
					if (dStage.cur_stage == DIALOGUE_BRANCH::YES) {
						branch = dStage.yesBranch;
					}
					else if (dStage.cur_stage == DIALOGUE_BRANCH::NO) {
						branch = dStage.noBranch;
					}
					if (branch.size() <= 0) break;
					dStage.autoTrigger = false;
					gm.dialogueText = branch[speaker.complexLine].text;
					if (branch[speaker.complexLine].line_speaker == LINE_SPEAKER::PLAYER) {
						gm.speakerName = "Selene";
					} else if (branch[speaker.complexLine].line_speaker == LINE_SPEAKER::WORLD) {
						gm.speakerName = "";
					} else {
						gm.speakerName = speaker.name;
					}
					gm.state = GAME_STATE::DIALOGUE;

					speaker.complexLine++;

					if (speaker.complexLine >= branch.size()) {
						// check if this is the primary, yes, or no branch
						// if primary, check 
						if (dStage.cur_stage == DIALOGUE_BRANCH::PRIMARY && dStage.conditional) {
							gm.optionText = "[V] " + dStage.yesOption + " - [C] " + dStage.noOption;
							speaker.complexLine = -1;
						} else {
							gm.optionText = gm.defaultOptionText;
							// not conditional, or if conditional, we've finished a different branch
							if (stage == QUEST_STAGE::START && dStage.cur_stage != DIALOGUE_BRANCH::NO) {
								// if we've finished the starting branch and we're not on a "no" branch
								tq.stage = QUEST_STAGE::DURING;
							} else if (stage == QUEST_STAGE::DURING) {
								checkQuests(true, q);
							} else if (stage == QUEST_STAGE::END && dStage.cur_stage != DIALOGUE_BRANCH::NO) {
								checkQuests(true, q);
							}

							playerComp.key_interact = false;
							speaker.complexLine = 0;
						}
					}
				}

				/*
				// get the text from that line
				gm.dialogueText = dl.text;
				// if dl.playerSpeaking, draw the players name, otherwise draw the NPCs name

				speaker.complexLine++;
				if (speaker.complexLine >= foundDS.sequence[stage].size()) {
					std::cout << "Finished this sequence" << std::endl;
					// we've finished the dialogue sequence
					if (stage == QUEST_STAGE::START) {
						tq.stage = QUEST_STAGE::DURING;
					}
					if (stage == QUEST_STAGE::DURING || stage == QUEST_STAGE::END) {
						checkQuests();
					}

					speaker.complexLine = 0;
				}
				*/
				/**************************** CHECK THIS QUEST ****************************/
				break;
			}
		}

		// ELSE: SIMPLE DIALOGUE
		if (foundQuest == -1 && registry.players.get(player).key_interact && speaker.simpleDialogueLines.size() > 0) {
			if (speaker.simpleDialogueLines.size() <= 0) return;

			gm.dialogueText = speaker.simpleDialogueLines[speaker.nextLine];
			gm.speakerName = speaker.name;

			speaker.nextLine++;

			if (speaker.nextLine >= speaker.simpleDialogueLines.size()) {
				speaker.nextLine = 0;
			}
			gm.state = GAME_STATE::DIALOGUE;
		}
	}

}

void WorldSystem::doInteract(Entity entity, Entity entity_other) {
	
	Interactable& interactable = registry.interactables.get(entity_other);

	if (interactable.type == INTERACTABLE_TYPE_ID::STEALTH) {
		if (!Mix_Playing(9)) {
			Mix_PlayChannel(9, stealth, 0);
		}
		if (!registry.stealth.has(entity)) {
			registry.stealth.emplace_with_duplicates(entity);
		}
		else {
			registry.stealth.remove(entity);
		}		
	} 
	else if (interactable.type == INTERACTABLE_TYPE_ID::DOOR) {
		Transition& transition = registry.transitions.get(entity_other);
		if (!Mix_Playing(4)){
			Mix_PlayChannel(4, door_open, 0);
		}
		transitionLevel(transition);

	} 
	else if (interactable.type == INTERACTABLE_TYPE_ID::ITEM) {
		Item& item = registry.items.get(entity_other);
		Mix_PlayChannel(5, interactable_item, 0);
		addToInventory(item);
		checkQuests(false, -1);
		if (interactable.disappears) {
			registry.remove_all_components_of(entity_other);
		}
	}
	else if (interactable.type == INTERACTABLE_TYPE_ID::RESPAWN_TETHER) {
		registry.players.components[0].health = registry.players.components[0].spawnHealth;
		
		QuestManager& qm = registry.questmanagers.get(gameManager);

		qm.conditions[(int) ITEM_CONDITION_ID::SPAWN_SET]++;
		saveData.setSpawn();
	}
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	registry.drawHelps.clear();
	auto& collisionsRegistry = registry.collisions;
	GameManager& gm = registry.gameManagers.components[0];
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// For now, we are only interested in collisions that involve the chicken
		if (registry.players.has(entity)) {
			Player& playerComponent = registry.players.get(entity);
			Motion& playerMotion = registry.motions.get(entity);

			// Checking Player - NON deadly collisions
			if (registry.interactables.has(entity_other)) {
				registry.drawHelps.emplace_with_duplicates(entity_other);
				registry.drawHelps.get(entity_other).text = registry.interactables.get(entity_other).interactText;
				
				auto interactable = registry.interactables.get(entity_other);
				if (interactable.type == INTERACTABLE_TYPE_ID::DOOR) {
					Transition& transition = registry.transitions.get(entity_other);
					QuestManager& qm = registry.questmanagers.components[0];
					for (QUEST_INDEX req : transition.requirements) {
						// if one of the reqs isn't complete, don't transition
						if (std::find(qm.completedQuests.begin(), qm.completedQuests.end(), (int) req) == qm.completedQuests.end()) {	
							registry.drawHelps.get(entity_other).text = "Locked";
						}
					}
				} 

				if (playerComponent.key_interact) {
					doInteract(entity, entity_other);
					playerComponent.key_interact = false;
				}
			}

			if (registry.transitions.has(entity_other) && !registry.transitions.get(entity_other).needsInteract) {
				Transition& transition = registry.transitions.get(entity_other);
				transitionLevel(transition);
			}


			// Checking Player - Deadly collisions
			//if (registry.deadlys.has(entity_other) && !registry.stealth.has(entity)) { //for future stealth
			if (registry.deadlys.has(entity_other)) {
				if (!registry.stealth.has(player)) {
					if (!playerComponent.is_dead && !registry.invincibilityTimers.has(entity) && !registry.attackAnimations.has(player) && !registry.attackUpAnimations.has(player)) {
						Physics& physics = registry.physicsObjs.get(entity);
						physics.velocity.x = (physics.velocity.x > 0) ? -200 : 200;
						playerComponent.hurt = true;
						std::cout << "player collided with enemy, should jump back" << std::endl;
						registry.invincibilityTimers.insert(entity, InvincibilityTimer(1000, 6));
						Mix_PlayChannel(3, damage_taken, 0);
						playerComponent.health--;
						handleDeath();
					}
				}
				else {
					playerComponent.hurt = false;
				}
			}
			else if (registry.projectiles.has(entity_other)) {
				Projectile& proj = registry.projectiles.get(entity_other);
				if (!registry.stealth.has(player) && proj.enemy) {
					if (!playerComponent.is_dead && !registry.invincibilityTimers.has(entity)) {
						Physics& physics = registry.physicsObjs.get(entity);
						physics.velocity.x = (physics.velocity.x > 0) ? -140 : 140;
						playerComponent.hurt = true;
						//std::cout << "player collided with enemy, should jump back" << std::endl;
						registry.invincibilityTimers.insert(entity, InvincibilityTimer(1000, 6));
						playerComponent.health--;
						Mix_PlayChannel(3, damage_taken, 0);
						handleDeath();
						registry.remove_all_components_of(entity_other);
					}
				}
				else if (proj.projType == PROJECTILE_TYPE_ID::HEART) {
					playerComponent.hurt = false;
					if (playerComponent.health < 5) playerComponent.health++;
					registry.remove_all_components_of(entity_other);
				}
				else {
					playerComponent.hurt = false;
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, chicken_eat_sound, 0);
					++points;
				}
			}
		}

		// checking enemy collisions with sword
		if (registry.deadlys.has(entity) && !registry.boss.has(entity)) {
			if (registry.dmgTimers.has(entity_other)) {
				Deadly& enemy = registry.deadlys.get(entity);
				Physics& physics = registry.physicsObjs.get(entity);
				if (!registry.invincibilityTimers.has(entity)) {
					if (enemy.health > 0) {
						// decrement hp and knock back
						registry.invincibilityTimers.insert(entity, InvincibilityTimer(500, 4));
						physics.velocity.x = (physics.velocity.x > 0) ? -1500 : 1500;
						enemy.hurt = true;
						enemy.health--;
						// if enemy hp is 0 we delete it
						if (enemy.health <= 0) {
							if (gm.enemiesInBattle.size() == 1 && gm.musicTimer <= 0) {
								gm.musicTimer = 2000;
							}
							gm.enemiesInBattle.erase(entity);
							if (enemy.type == ENEMY_TYPE_ID::GUARDSUMMON) {
								if (!registry.boss.entities.empty()) {
									Boss& boss = registry.boss.components[0];
									boss.summonCount--;
								}
								Motion& motion = registry.motions.get(entity);
								createProjectile(renderer, motion.position, motion.position, 0.f, PROJECTILE_TYPE_ID::HEART);
							}
							registry.remove_all_components_of(entity);
						}
						//std::cout << "Enemy health is currently: " << enemy.health << std::endl;
					}
				}
			}
			if (registry.projectiles.has(entity_other)) {
				Projectile& proj = registry.projectiles.get(entity_other);
				if (!proj.noHurtEnemy && !proj.decoy && !proj.enemy || proj.boss) {

					if (registry.deadlys.has(entity)) {
						Deadly& enemy = registry.deadlys.get(entity);
						Physics& physics = registry.physicsObjs.get(entity);
						if (!registry.invincibilityTimers.has(entity)) {
							if (enemy.health > 0) {
								// decrement hp and knock back
								registry.invincibilityTimers.insert(entity, InvincibilityTimer(500, 4));
								physics.velocity.x = (physics.velocity.x > 0) ? -1000 : 1000;
								enemy.hurt = true;
								enemy.health--;
								// if enemy hp is 0 we delete it
								if (enemy.health <= 0) {
									if (gm.enemiesInBattle.size() == 1 && gm.musicTimer <= 0) {
										gm.musicTimer = 2000;
									}
									gm.enemiesInBattle.erase(entity);
									if (enemy.type == ENEMY_TYPE_ID::GUARDSUMMON) {
										if (!registry.boss.entities.empty()) {
											Boss& boss = registry.boss.components[0];
											boss.summonCount--;
										}
										Motion& motion = registry.motions.get(entity);
										createProjectile(renderer, motion.position, motion.position, 0.f, PROJECTILE_TYPE_ID::HEART);
									}
									registry.remove_all_components_of(entity);
								}
								//std::cout << "Enemy health is currently: " << enemy.health << std::endl;
							}
						}
					}
				}
			}
		}

		if (registry.breakables.has(entity)) {
			if (registry.dmgTimers.has(entity_other) && !registry.invincibilityTimers.has(entity)) {
				Mix_PlayChannel(10, break_box, 0);
				registry.invincibilityTimers.insert(entity, InvincibilityTimer(100, 1));
			}
		}
	}

	// If the projectile makes contact with a solid ** currently only floor **
	/*
	for (auto& p: registry.projectiles.entities) {
		Physics& physics = registry.physicsObjs.get(p);
		if (physics.velocity.y == 0 || physics.velocity.x != 0) {
			registry.remove_all_components_of(p);
		}
	
	}
	*/
	// Remove all collisions from this simulation step
	
	Player& playerComponent = registry.players.get(player);
	playerComponent.key_interact = false;
	playerComponent.key_decline = false;
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	GameManager& gameManager = registry.gameManagers.components[0];

	/* the button now quits game
	// QUIT GAME ON ESCAPE: NECESSARY FOR FULL SCREEN
	if (action == GLFW_REPEAT && key == GLFW_KEY_ESCAPE) {
		saveData.savePlayer();
		saveData.saveLevel();
		saveData.saveGame();
		glfwSetWindowShouldClose(window, 1);
	}
	*/
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		if (gameManager.state == GAME_STATE::PLAY) {
			gameManager.state = GAME_STATE::PAUSE;
			std::cout << "State changed to pause" << std::endl;
		}
		else {
			gameManager.state = GAME_STATE::PLAY;
			std::cout << "State changed to play" << std::endl;
		}
	}
	Mob& mob = registry.mobs.get(player);

	// MAP

	if (action == GLFW_RELEASE && key == GLFW_KEY_M) {
		QuestManager& qm = registry.questmanagers.components[0];
		if (gameManager.state != GAME_STATE::MAP && qm.conditions[(int) ITEM_CONDITION_ID::MAP] > 0) {
			gameManager.state = GAME_STATE::MAP;
			std::cout << "State changed to map" << std::endl;
		}
		else if (gameManager.state == GAME_STATE::MAP) {
			gameManager.state = GAME_STATE::PLAY;
			std::cout << "State changed to play" << std::endl;
		}
		
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_TAB) {
		if (gameManager.state != GAME_STATE::QUEST_MENU) {
			gameManager.state = GAME_STATE::QUEST_MENU;
			std::cout << "State changed to quest menu" << std::endl;
		}
		else if (gameManager.state == GAME_STATE::QUEST_MENU) {
			gameManager.state = GAME_STATE::PLAY;
			std::cout << "State changed to play" << std::endl;
		}

	}
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Movement 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && (key == GLFW_KEY_A || key == GLFW_KEY_LEFT)) {
		gameManager.key_left = true;
		
		Player& playerCt = registry.players.get(player);
		if (gameManager.state == GAME_STATE::PLAY) {
			if (!playerCt.is_dead) {
				playerCt.key_left = true;
				playerCt.key_left_release_timer = 0;
				if (!registry.runAnimations.has(player) && !registry.attackAnimations.has(player) && !registry.attackUpAnimations.has(player)) {
					registry.runAnimations.insert(player, RunAnimation(10, 0));
				}
			}
		}
		else {
			playerCt.key_left = false;
			playerCt.key_left_release_timer = 0;
			registry.runAnimations.remove(player);

			// If the death animation is not playing then reset the player to standing
			if (!playerCt.is_dead) {
				RenderRequest& rr = registry.renderRequests.get(player);
				rr.used_texture = TEXTURE_ASSET_ID::PLAYER;
			}
		}
	}

	if (action == GLFW_RELEASE && (key == GLFW_KEY_A || key == GLFW_KEY_LEFT)) {
		gameManager.key_left = false;

		Player& playerCt = registry.players.get(player);
		playerCt.key_left = false;
		playerCt.start_key_left_timer = true;
	}

	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT)) {
		gameManager.key_right = true;
		
		Player& playerCt = registry.players.get(player);
		if (gameManager.state == GAME_STATE::PLAY) {
			if (!playerCt.is_dead) {
				playerCt.key_right = true;
				playerCt.key_right_release_timer = 0;

				if (!registry.runAnimations.has(player) && !registry.attackAnimations.has(player) && !registry.attackUpAnimations.has(player)) {
					registry.runAnimations.insert(player, RunAnimation(10, 0));
				}
			}
		}
		else {
			playerCt.key_right = false;
			playerCt.key_right_release_timer = 0;
			registry.runAnimations.remove(player);
			// If the death animation is not playing then reset the player to standing
			if (!playerCt.is_dead) {
				RenderRequest& rr = registry.renderRequests.get(player);
				rr.used_texture = TEXTURE_ASSET_ID::PLAYER;
			}
		}
	}
  
	if (action == GLFW_RELEASE && (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT)) {
		gameManager.key_right = false;

		Player& playerCt = registry.players.get(player);
		playerCt.key_right = false;
		playerCt.start_key_right_timer = true;
	}

	if (action == GLFW_PRESS && (key == GLFW_KEY_SPACE)) {
		if (registry.motions.has(player) && !registry.deathTimers.has(player)) {
			Motion& motion = registry.motions.get(player);
			Player& playerComp = registry.players.get(player);
			playerComp.key_jump = true;
		}
	}

	if (action == GLFW_RELEASE && (key == GLFW_KEY_SPACE)) {
		if (registry.motions.has(player) && !registry.deathTimers.has(player)) {
			Player& playerComp = registry.players.get(player);
			playerComp.key_jump = false;
		}
	}

	if (action == GLFW_PRESS && (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)) {
		gameManager.key_down = true;

		if (registry.motions.has(player) && !registry.deathTimers.has(player)) {
			Motion& motion = registry.motions.get(player);
			Player& playerComp = registry.players.get(player);
			playerComp.key_down = true;
		}
	}

	if (action == GLFW_RELEASE && (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)) {
		gameManager.key_down = false;

		if (registry.motions.has(player) && !registry.deathTimers.has(player)) {
			Player& playerComp = registry.players.get(player);
			playerComp.key_down = false;
		}
	}

	if (action == GLFW_PRESS && (key == GLFW_KEY_LEFT_SHIFT)) {
		if (registry.motions.has(player) && !registry.deathTimers.has(player)) {
			registry.invincibilityTimers.remove(player);
			Player& playerComp = registry.players.get(player);
			playerComp.dash_timer = 300;
			registry.invincibilityTimers.insert(player, InvincibilityTimer(300, 0));
		}
	}
  
	// setting upwards attack if 'w' or up pressed
	if (action == GLFW_PRESS && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		gameManager.key_up = true;

		if (registry.motions.has(player) && !registry.deathTimers.has(player)) {
			Player& playerComp = registry.players.get(player);
			playerComp.is_up = true;
		}
	}

	if (action == GLFW_RELEASE && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		gameManager.key_up = false;

		if (registry.motions.has(player) && !registry.deathTimers.has(player)) {
			Player& playerComp = registry.players.get(player);
			playerComp.is_up = false;
		}
	}
/*
	if (action == GLFW_RELEASE && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		if (registry.motions.has(player) && !registry.deathTimers.has(player)) {
			Player& playerComp = registry.players.get(player);
			playerComp.is_up = false;
		}
	} */

	// player attacking
	if (action == GLFW_PRESS && key == GLFW_KEY_Z) {
		bool is_up = false;
		Player& playerCt = registry.players.get(player);
		playerCt.key_attack = true;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			is_up = true;
		}
		if (gameManager.state == GAME_STATE::PLAY && !playerCt.is_dead && !playerCt.is_summoning) {
			if (registry.motions.has(player) && !registry.dmgTimers.has(player)) {
				if (registry.dmgTimers.entities.size() == 0) {
					Player p = registry.players.get(player);
					Motion& m = registry.motions.get(player);
					//Physics& physics = registry.physicsObjs.get(player);

					if (!registry.attackUpAnimations.has(player) && !registry.attackAnimations.has(player)) {
						if (is_up) {
							registry.runAnimations.remove(player);
							registry.attackUpAnimations.insert(player, AttackUpAnimation(7, 6, 1));
							// run the upwards animation here
						}
						else {
							registry.runAnimations.remove(player);
							registry.attackAnimations.insert(player, AttackAnimation(9, 5, 1, 2));
							//physics.velocity.x = m.scale.x > 0 ? 500 : -500;
							// don't make the run and attack animations clash
						}
					}
				}
			}
		}
		else {
			registry.attackAnimations.remove(player);
			registry.attackUpAnimations.remove(player);

			// If the player is not dead then reset the player to standing
			if (!playerCt.is_dead) {
				std::cout << "player reset to standing" << std::endl;
				RenderRequest& rr = registry.renderRequests.get(player);
				rr.used_texture = TEXTURE_ASSET_ID::PLAYER;
			}
			else {
				std::cout << "player is dead" << std::endl;
			}
			//RenderRequest& rr = registry.renderRequests.get(player);
			//rr.used_texture = TEXTURE_ASSET_ID::PLAYER;
		}
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_Z) {
		Player& playerCt = registry.players.get(player);
		playerCt.key_attack = false;
		if (playerCt.is_summoning) {
			// create projectile
			auto& summonable_registry = registry.summonables;
			for (uint i = 0; i < summonable_registry.size(); i++) {

				Motion& playerMotion = registry.motions.get(player);

				Entity sEntity = summonable_registry.entities[i];
				Summonable& summ = summonable_registry.components[i];

				if (!summ.active) continue;

				summ.active = false;
				
				Motion& sMotion = registry.motions.get(sEntity);

				float diff = playerMotion.position.x - sMotion.position.x;

				int dir = (diff >= 0) - (diff < 0);

				registry.gravityEntities.emplace(sEntity);

				Physics& physics = registry.physicsObjs.emplace(sEntity);
				float angle = atan2(-1000.f, 10.f * diff);
				float projectileSpeed = 2000.f;

				physics.velocity = { cos(angle) * projectileSpeed, sin(angle) * projectileSpeed };
				physics.inAir = true;

				Projectile& projectile = registry.projectiles.emplace(sEntity);
				projectile.enemy = false;

				DamageTimer& dmg = registry.dmgTimers.emplace(sEntity);
				dmg.counter_ms = 5000;
				
			}
			playerCt.is_summoning = false;
		}
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_X) {
		Player& playerComp = registry.players.get(player);
		Physics& playerPhys = registry.physicsObjs.get(player);
		QuestManager& qm = registry.questmanagers.components[0];
		playerComp.key_summon = true;
		if (!playerPhys.inAir && qm.conditions[(int) ITEM_CONDITION_ID::RING] > 0 /* other condition met*/) {
			if (!playerComp.is_summoning) {
				vec2 createPos = registry.motions.get(player).position + vec2({0., 32.});
				createSummonable(renderer, createPos);
				emitParticles(createPos, 15, 600, 8, { .3f, .3f, .3f });
				Mix_PlayChannel(11, create_rock, 0);
			} else {
				auto& summonable_registry = registry.summonables;
				for (uint i = 0; i < summonable_registry.size(); i++) {
					Entity sEntity = summonable_registry.entities[i];
					Summonable& summ = summonable_registry.components[i];

					if (!summ.active) continue;
					summ.active = false;

					Solid& s = registry.solids.emplace(sEntity);
					s.topFaceOnly = true;
					s.mat = MATERIAL::STONE;

					Breakable& b = registry.breakables.emplace(sEntity);
					b.hp = 7000;

					emitParticles(registry.motions.get(sEntity).position, 6, 300, 6, { .3f, .3f, .3f });
				}
			}

			playerComp.is_summoning = !playerComp.is_summoning;
			playerComp.key_down = false;
		}
	}

	// temp throwable
	if (action == GLFW_RELEASE && key == GLFW_KEY_X) {
		Player& playerComp = registry.players.get(player);
		playerComp.key_summon = false;

/*
		if (registry.motions.has(player) && !registry.dmgTimers.has(player)) {
			Motion& motion = registry.motions.get(player);
			createProjectile(renderer, motion.position, motion.position, motion.scale.x, PROJECTILE_TYPE_ID::DECOY);
		}
*/
	}

	// sound control (FOR NOW)
	if (action == GLFW_PRESS && key == GLFW_KEY_7) {
		if (sfx_play) {
			sfxOff();
		}
		else {
			sfxOn();
		}
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_6) {
		if (music_play) {
			musicOff();
		}
		else {
			musicOn();
		}
	}

	
	// transitions
	if (action == GLFW_PRESS && key == GLFW_KEY_V) {
		Player& playerComp = registry.players.get(player);
		playerComp.key_interact = true;
	}
	
	if (action == GLFW_PRESS && key == GLFW_KEY_C) {
		Player& playerComp = registry.players.get(player);
		playerComp.key_decline = true;
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_APOSTROPHE && action == GLFW_PRESS) {
		debugging.in_debug_mode = !debugging.in_debug_mode;
	}
  
  // toggle fps
	if (action == GLFW_RELEASE && key == GLFW_KEY_F) {
		fps.show_fps = !fps.show_fps;
  }

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);

	Camera& camera = registry.cameras.components[0];

	if ((action == GLFW_RELEASE || action == GLFW_REPEAT) && (mod & GLFW_MOD_CONTROL) && key == GLFW_KEY_EQUAL) {
		camera.zoom -= 0.05f;
	}
	if ((action == GLFW_RELEASE || action == GLFW_REPEAT) && (mod & GLFW_MOD_CONTROL) && key == GLFW_KEY_MINUS) {
		camera.zoom += 0.05f;
	}
	//camera.zoom = fmax(0.7f, camera.zoom);
	//camera.zoom = fmin(2.0f, camera.zoom);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE CHICKEN ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the chicken's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(vec2)mouse_position; // dummy to avoid compiler warning
	float mouse_x = mouse_position.x;
	float mouse_y = mouse_position.y;
	// std::cout << "mouse position: " << mouse_x << ", " << mouse_y << std::endl;

	//std::cout << "fb to window width ratio: " << frame_buffer_to_window_width_ratio << ", fb to window height ratio" << frame_buffer_to_window_height_ratio << std::endl;

	for (Button& button: registry.buttons.components) {
		
		int width_px, height_px;
		glfwGetWindowSize(window, &width_px, &height_px);
		
		float xmult = width_px / registry.cameras.components[0].dims.x;
		float ymult = height_px / registry.cameras.components[0].dims.y;

		float posx = xmult * button.position.x;
		float scalex = xmult * button.scale.x;
		float posy = ymult * (window_height_px - button.position.y);
		float scaley = ymult * button.scale.y;

		float minX = posx;
		float maxX = posx + scalex;
		float minY = posy - scaley;
		float maxY = posy;
		//if (button.function == BUTTON_FUNCTION::START_GAME) {
		//	std::cout << "button area min max: " << minX << " " << maxX << " " << minY << " " << maxY << std::endl;
		//}
		// std::cout << "button area min max: " << minX << " " << maxX << " " << minY << " " << maxY << std::endl;

		if (minX <= mouse_x && mouse_x <= maxX && minY <= mouse_y && mouse_y <= maxY) {
			//std::cout << "WITHIN BOUNDS!!! w function: " << (int)button.function << std::endl;
			button.is_mouse_over = true;
		} else {
			button.is_mouse_over = false;
		}	
	}
}

void WorldSystem::on_mouse_click(int button, int action, int mods)
{
	GameManager& gm = registry.gameManagers.components[0];
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		if (gm.state == GAME_STATE::PAUSE) {
			// if mouse_over, check function to determine what to do 
			for (Button& button : registry.buttons.components) {
				if (button.is_mouse_over) {
					if (button.function == BUTTON_FUNCTION::RESUME_GAME) {
						gm.state = GAME_STATE::PLAY;
					}
					if (button.function == BUTTON_FUNCTION::QUIT_GAME) {
						saveData.savePlayer();
						saveData.saveLevel();
						saveData.saveGame();
						glfwSetWindowShouldClose(window, 1);
					}
					if (button.function == BUTTON_FUNCTION::DISPLAY_HELP) {
						std::cout << "changing to help" << std::endl;
						gm.state = GAME_STATE::HELP;
						return;
					}
					std::cout << "clicked on a button!" << std::endl;
				}
			}
		}

		if (gm.state == GAME_STATE::MENU) {
			for (Button& button : registry.buttons.components) {
				if (button.is_mouse_over) {
					if (button.function == BUTTON_FUNCTION::START_GAME) {
						std::cout << "changing to play" << std::endl;
						gm.state = GAME_STATE::PLAY;
					}
				}
			}
		}
		
		if (gm.state == GAME_STATE::HELP) {
			for (Button& button : registry.buttons.components) {
				if (button.is_mouse_over) {
					if (button.function == BUTTON_FUNCTION::DISPLAY_HELP) {
						gm.state = GAME_STATE::PAUSE;
					}
				}
			}
		}
		
	}
}

void WorldSystem::enterBattle(bool enter) {
	GameManager& gm = registry.gameManagers.components[0];
	if(enter && music_play){
		if (gm.musicTimer > 0) {
			int volume = static_cast<int> ((30.0 / 2000.0) * (2000.0 - gm.musicTimer));
			Mix_Volume(7, volume);
			Mix_Volume(6, 30-volume);
		}
		else {
			Mix_Volume(6, 0);
		}
	} else if (!enter && music_play) {
		if (gm.musicTimer > 0) {
			int volume = static_cast<int> ((30.0 / 2000.0) * (2000.0 - gm.musicTimer));
			Mix_Volume(7, 30 - volume);
			Mix_Volume(6, volume);
			
		}
		else {
			Mix_Volume(7, 0);
			Mix_Volume(6, 30);
			
		}
	}					
}

void WorldSystem::handleDeath() {
	Player& playerComponent = registry.players.components[0];
	if (playerComponent.health <= 0) {
		if (!registry.deathTimers.has(player)) {
			registry.deathTimers.emplace(player);
			Mix_PlayChannel(-1, chicken_dead_sound, 0);
		}
		playerComponent.is_dead = true;
		playerComponent.key_jump = false;
		playerComponent.key_left = false;
		playerComponent.key_right = false;
		playerComponent.key_down = false;

		registry.runAnimations.remove(player);
		registry.jumpUpAnimations.remove(player);
		registry.attackAnimations.remove(player);
		registry.attackUpAnimations.remove(player);
		registry.invincibilityTimers.remove(player);
		registry.deathAnimations.insert(player, DeathAnimation(13, 4));
	}
}
