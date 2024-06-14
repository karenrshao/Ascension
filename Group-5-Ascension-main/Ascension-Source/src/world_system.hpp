#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "save_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods


class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer, LevelRegistry levels);
	void initPlayer();

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	void stepBackgrounds();
	// Check for collisions
	void handle_collisions();
	void handleDeath();

	// Should the game be over ?
	bool is_over()const;

	Entity getPlayer() {
		return player;
	}

	Entity getGameManager() {
		return gameManager;
	}

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_click(int button, int action, int mods);

	// restart level
	void init_game();
	void restart_game();
	void transitionLevel(Transition& t);

	void loadLevel(std::string filepath);
	void loadLevel(Level& level, LEVEL_INDEX li);

	// inventory and quests
	void checkQuests(bool canComplete, int curQuest);
	void addToInventory(Item& item);
	void enterBattle(bool enter);

	void doInteract(Entity entity, Entity entity_other);
	void doDialogue(Entity entity, Entity entity_other);
	void stepAutoSpeakerSystem();

	// OpenGL window handle
	GLFWwindow* window;

	// Number of bug eaten by the chicken, displayed in the window title
	unsigned int points;

	// Game state
	RenderSystem* renderer;
	float current_speed;
	float next_eagle_spawn;
	float next_bug_spawn;
	Entity player;
	Entity gameManager;
	Entity consumable;
	Entity changingItem;

	// buttons
	Entity resume_game;
	Entity quit_game;
	Entity start_game;
	Entity display_help;
	Entity pause_game;

	// music references
	Mix_Chunk* background_music;
	Mix_Chunk* second_background_music;
	Mix_Chunk* chicken_dead_sound;
	Mix_Chunk* chicken_eat_sound;

	// sound effects
	Mix_Chunk* door_open;
	Mix_Chunk* door_close;
	Mix_Chunk* door_shop;
	Mix_Chunk* sword_swing;
	Mix_Chunk* interactable_item;
	Mix_Chunk* landing;
	Mix_Chunk* damage_taken;
	Mix_Chunk* run_grass;
	Mix_Chunk* run_stone;
	Mix_Chunk* break_box;
	Mix_Chunk* stealth;
	Mix_Chunk* create_rock;

	void musicOn();
	void musicOff();

	void sfxOn();
	void sfxOff();

	bool music_play = true;
	bool sfx_play = true;

	LevelRegistry levels;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

};
