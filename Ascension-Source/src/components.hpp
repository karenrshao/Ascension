#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include "../ext/stb_image/stb_image.h"
#include <map>
#include <unordered_set>

enum class TEXTURE_ASSET_ID;
enum class ENEMY_TYPE_ID;
enum class ENEMY_STATE_ID;
enum class PLAYER_STATE_ID;
enum class INTERACTABLE_TYPE_ID;
enum class PROJECTILE_TYPE_ID;
enum class QUEST_INDEX;
enum class LEVEL_INDEX;
enum class MATERIAL;
const float ENEMY_ALERT_TIMER = 3000;

enum class GAME_STATE {
	MENU = 0,
	PLAY = MENU + 1,
	PAUSE = PLAY + 1,

	DIALOGUE = PAUSE + 1,
	MAP = DIALOGUE + 1,
	QUEST_MENU = MAP + 1,

	TRANSITION = QUEST_MENU + 1,

	END = TRANSITION + 1,

	HELP = END + 1, 

	STATE_COUNT = HELP + 1,
};
const int state_count = (int) GAME_STATE::STATE_COUNT;

struct GameManager
{
	GAME_STATE state;
	std::string dialogueText = "";
	std::string speakerName;
	std::string defaultOptionText = "[V] Next";
	std::string optionText = defaultOptionText;
	LEVEL_INDEX currentLevel;

	bool key_left, key_right, key_up, key_down;

	std::unordered_set<unsigned int> enemiesInBattle;
	float musicTimer;
	bool stealth;
	vec2 decoyPosition;
	float decoyTimer;
	vec2 bounds;
	float transitionTimer, transitionTime = 1000.f;

	float particleTimer, particleTime = 250;
	vec2 mapPos = { 3356., 8531. };
	float mapScale = 0.6;
	int mapTimer, mapTime = 400;
	// vec2 mapPos = { 804., 8646. };
};

struct SleepAnimation {
	int numFrames;
	float frameSpeed = (1.f / 24.f) * 1000.f;
	float currTime = 0.f;
	int currentFrame = 0;
	int runFrameVectorIndex;
	int breathInHoldFrame = 10;
	int breathOutHoldFrame = 1;
	int holdNumFrames = 6;
	int holdCounter = 0;
	bool hold = false;

	SleepAnimation(int numFrames, int index) {
		this->numFrames = numFrames;
		this->runFrameVectorIndex = index;
	};
};

// Player component
struct Player
{
	// KEY CONTROLS
	bool key_right, key_left, key_jump, key_down, key_dash;
	float key_right_release_timer = 0;
	float key_left_release_timer = 0;
	bool start_key_right_timer = false;
	bool start_key_left_timer = false;
	bool key_interact, key_decline;
	float dash_timer;
	bool key_summon, key_attack;

	// STATES
	bool in_battle = false;
	bool is_dead = false;
	bool is_falling = false;
	bool is_summoning = false;

	// STATS
	int spawnHealth = 5;
	int health = spawnHealth;
	bool hurt = false;
	bool is_up = false;

	// sound effects
	bool jump_sound = false;
	bool is_running = false;
	bool attack_sound = false;
	MATERIAL on_material;

	vec2 lastPosition = {0., 0.};
	vec2 savedPosition = {0., 0.};
	float landTime = 0.f, landTimer = 250.f;
};

struct Stealth
{

};

// Enemy Component
struct Deadly
{
	ENEMY_TYPE_ID type;
	ENEMY_STATE_ID state;
	float aggroDistance;
	float alertDistance;
	float attackTimer = 0; // For spacing out attacks
	float alertTimer = 0; // For retaining alert state
	float chaseTimer = 0; // For some mobs that will calc position every X frames
	bool attacking = false;
	vec2 targetPosition = { 0, 0 };
	vec2 originalPosition = { 0, 0 };
	vec2 patrolPosition; //Position where deadly will patrol back and forth from (x-30 <= patrolPosition <= x+30)
	int health = 3;
	bool hurt = false;
};

struct Patrol
{
	// While in detection radius, detection meter raises quickly. Enter ALERT.
	// When detection meter hits 1, enter CHASE.
	// If out, detection meter slowly drops to 0. Once it hits 0, enter PATROL.
	// MAYBE IMPLEMENT POST MILESTONE
	float detectionMeter;

	float patrolTimer = 0;
	bool reachEnd = false;
};

struct Boss
{
	int wave;
	int summonCount = -1;

	float startTimer = 2000;
};
 

struct Eatable
{

};

struct Camera
{
	vec2 targetPosition = { 0.f, 0.f };
	vec2 position = { 0.f, 0.f };
	vec2 dims = { 1280, 720 };
	float zoom = 1.0;
	float defaultZoom = 1.0;
	float interpSpeed = 0.03;
	float offsetDist = 196.f;

	vec2 bounds = { window_width_px, window_height_px };
};

struct CameraTether
{
	vec2 position;
	vec2 bounds;
	float zoom;
};

enum class SOLID_TYPE {
	BLOCK = 0,
	TOP_FACE_ONLY = BLOCK + 1,
	UP_RIGHT_SLOPE = TOP_FACE_ONLY + 1,
	UP_LEFT_SLOPE = UP_RIGHT_SLOPE + 1
};

enum class MATERIAL {
	STONE = 0,
	GRASSY = STONE + 1,
	MATERIAL_COUNT = GRASSY + 1
};
const int material_count = (int) MATERIAL::MATERIAL_COUNT;
// all things that the player can platform with or collide on
struct Solid
{
	vec2 position = { 0, 0 };
	bool topFaceOnly = false;
	SOLID_TYPE type; // UNUSED
	MATERIAL mat;
};

struct Particle
{
	float ttl = 3000;
};

struct InvincibilityTimer {
	float invinc_counter_ms;
	float flash_counter_ms;
	float flash_count;
	bool flash = true;
	int num_flash;
	int curr_flash_count = 0;

	InvincibilityTimer(float invinc_counter, int num_flashes) {
		this->invinc_counter_ms = invinc_counter;
		this->flash_counter_ms = invinc_counter / num_flashes;
		this->flash_count = invinc_counter / num_flashes;
		this->num_flash = num_flashes;
	}
};

struct SparkleAnimation {
	int numFrames;
	int animationVectorIndex;
	float frameSpeed = (1.f / 24.f) * 1000.f;
	float currTime = 0.f;
	int currentFrame = 0;
	float pauseTimeMs = 4000;
	float pauseTimer = 0;
	bool paused = false;

	SparkleAnimation(int numFrames, int index, float pauseTime) {
		this->numFrames = numFrames;
		this->animationVectorIndex = index;
		this->pauseTimeMs = pauseTime;
	}
};

struct AttackAnimation {
	int numFrames;
	float frameSpeed = (1.f / 24.f) * 1000.f;
	float currTime = 0.f;
	int currentFrame = 0;
	int animationVectorIndex;
	int frameToRenderSwordMesh;
	int frameToImpulse;

	AttackAnimation(int numFrames, int index, int frameToRenderSwordMesh, int frameToImpulse) {
		this->numFrames = numFrames;
		this->animationVectorIndex = index;
		this->frameToRenderSwordMesh = frameToRenderSwordMesh;
		this->frameToImpulse = frameToImpulse;
	};
};

struct AttackUpAnimation {
	int numFrames;
	float frameSpeed = (1.f / 24.f) * 1000.f;
	float currTime = 0.f;
	int currentFrame = 0;
	int runFrameVectorIndex;
	int frameToRenderSwordMesh;

	AttackUpAnimation(int numFrames, int index, int frameToRenderSwordMesh) {
		this->numFrames = numFrames;
		this->runFrameVectorIndex = index;
		this->frameToRenderSwordMesh = frameToRenderSwordMesh;
	};
};


// invisible things that show up only in debug mode
struct Invisible
{

};

// things the player can interact with upon keypress
struct Interactable {
	bool disappears = true;
	bool changes = false;
	INTERACTABLE_TYPE_ID type;
	std::string interactText = "[V] to interact";
};

enum class ITEM_CONDITION_ID {
	NO_ITEM = 0, // 0
	FRUIT = NO_ITEM + 1, // 1
	ROPE = FRUIT + 1, // 2
	BROOCH = ROPE + 1, // 3
	SCALE = BROOCH + 1,
	COIN = SCALE + 1,
	MEDICINE = COIN + 1,
	GADGET = MEDICINE + 1,
	CONSTELLATION_KEY = GADGET + 1,
	MAP = CONSTELLATION_KEY + 1,
	RING = MAP + 1,

	SPAWN_SET = RING + 1,

	TALONS = SPAWN_SET + 1,
	POLISHED_BROOCH = TALONS + 1,

	ITEM_COUNT = POLISHED_BROOCH + 1,
};
const int item_count = (int) ITEM_CONDITION_ID::ITEM_COUNT;

struct Item {
	ITEM_CONDITION_ID item_id;
};

struct Breakable {
	int hp;
};

// All elements with gravity will fall
struct Gravity {
	float grav = 9.8f * 256;
	float terminalVelocity = 2400;
};

struct Physics {
	vec2 velocity = { 0.f, 0.f };
	vec2 targetVelocity = { 0.f, 0.f };
	bool inAir = false;
	bool unspawnable = inAir;
	float elasticity = 0.0;// BETWEEN 0 and 1
	float airDrag = 0.01;
	float groundDrag = 0.15;
	float rampSpeed = 1.0; // BETWEEN 0 AND 1 
};

// Define animator parameters
struct RunAnimation {
	int numFrames;
	float frameSpeed = (1.f / 20.f) * 1000.f;
	float currTime = 0.f;
	int currentFrame = 0;
	int runFrameVectorIndex;
	RunAnimation(int numFrames, int index) {
		this->numFrames = numFrames;
		this->runFrameVectorIndex = index;
	};
};

struct JumpUpAnimation {
	int numFrames;
	float frameSpeed = (1.f / 24.f) * 1000.f;
	float currTime = 0.f;
	int currentFrame = 0;
	int runFrameVectorIndex;
	JumpUpAnimation(int numFrames, int index) {
		this->numFrames = numFrames;
		this->runFrameVectorIndex = index;
	};
};

struct DeathAnimation {
	int numFrames;
	float frameSpeed = (1.f / 24.f) * 1000.f;
	float currTime = 0.f;
	int currentFrame = 0;
	int runFrameVectorIndex;
	DeathAnimation(int numFrames, int index) {
		this->numFrames = numFrames;
		this->runFrameVectorIndex = index;
	};
};


struct SlashAnimation {
	int numFrames;
	float frameSpeed = (1.f / 24.f) * 1000.f;
	float currTime = 0.f;
	int currentFrame = 0;
	int runFrameVectorIndex;
	SlashAnimation(int numFrames, int index) {
		this->numFrames = numFrames;
		this->runFrameVectorIndex = index;
	};
};

// any entity with motion and can move (enemies, NPC, player)
struct Mob {

	float moveSpeed;
	float jumpSpeed;
	float hurtSpeed;


	/*
	vec2 targetVelocity = { 0, 0 };


	
	float fastFallMultiplier = 600.f;
	float fallMultiplier = 18.f;
	float lowJumpMultiplier = 10.f;
	float highJumpMultiplier = 7.f;
	float xFloorDrag = 0.7f;
	float xAirDrag = 0.7f;
	float dashMultiplier = 2.5f;
	float maxSpeed = 512.f;

	float dashLeftCounter_ms = 0.f;
	float dashRightCounter_ms = 0.f;

	bool applyGroundDrag = false;
	bool applyAirDrag = false;
	bool goingUp = false;
	bool fastFall = false;
	bool dashLeftTest = false;
	bool dashRightTest = false;
	bool moving = false;
	bool jumpHeld = false;

	bool falling = false;
	bool on_solid = true;
	float testVar = 0;
	*/

};

struct Projectile {
	PROJECTILE_TYPE_ID projType;
	vec2 target = { 0, 0 };
	bool enemy = false;
	bool boss = false;
	bool decoy = false;
	bool landed = false;
	bool noHurtEnemy = false;
	float timer;
};

enum class BUTTON_FUNCTION {
	RESUME_GAME = 0,
	QUIT_GAME = RESUME_GAME + 1,
	START_GAME = QUIT_GAME + 1,
	DISPLAY_HELP = START_GAME + 1,
};

struct Button  {
	bool is_mouse_over = false;
	vec2 position;
	vec2 scale;
	std::string label;
	TEXTURE_ASSET_ID texture;
	BUTTON_FUNCTION function;

	Button(vec2 position, vec2 scale, std::string label, TEXTURE_ASSET_ID texture, BUTTON_FUNCTION function) {
		this->texture = texture;
		this->position = position;
		this->label = label;
		this->scale = scale;
		this->function = function;
	}
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 direction = { 0, 0 };
	vec2 scale = { 10, 10 };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

struct FPS {
	bool show_fps = 0;
};
extern FPS fps;

struct drawHelp {

	std::string text = "";
};

struct drawInitial {
	float x = 0;
	float y = 0;
};

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
	float counter_ms = 3000;
};

struct BasicTimer
{
	float counter_ms = 2000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct Collider
{
	/*

	NOTES ON THE COLLIDER:
	- the hull is convex, and the vertices are in counter-clockwise order
	- collisions will compose implicit equations based on this
	- any object with motion must also have a collider component

	*/
	static std::vector<vec2> getConvexHull(Mesh mesh);
	static std::vector<vec2> getSquareHull(Motion motion);
	static std::vector<vec2> getSlopedHull(int direction);
	std::vector<vec2> hull; // this is the convex hull that constitutes the shape that the collider will interact with
	std::vector<vec2> workingHull;
};

struct Summonable {
	float offset_y = 0.f;
	float speed = 156.f;
	float max_offset = 72.f;
	bool active = true;
};

struct DamageTimer {
	float counter_ms = 50;
};

struct DrawEnemyHP {

};

/***********************************************/
// NOTE: KEEP THIS IN SYNC WITH THE push_back order of the registry
enum class QUEST_INDEX {
	TUTORIAL_WALK = 0,
	TUTORIAL_JUMP = TUTORIAL_WALK + 1,
	TUTORIAL_DROP = TUTORIAL_JUMP + 1,
	TUTORIAL_ATTACK = TUTORIAL_DROP + 1,
	TUTORIAL_MAP = TUTORIAL_ATTACK + 1,
	TUTORIAL_ROCK = TUTORIAL_MAP + 1,
	TUTORIAL_ROCK_2 = TUTORIAL_ROCK + 1,
	TUTORIAL_ROCK_3 = TUTORIAL_ROCK_2 + 1,
	TUTORIAL_INN = TUTORIAL_ROCK_3 + 1,

	TUTORIAL_STEALTH = TUTORIAL_INN + 1,
	GATHER_FRUITS = TUTORIAL_STEALTH + 1,
	FIX_BRIDGE = GATHER_FRUITS + 1,
	FIND_BROOCH = FIX_BRIDGE + 1,
	COINS_FROM_ORION = FIND_BROOCH + 1,
	BUY_MEDICINE = COINS_FROM_ORION + 1,
	STEAL_MONEY = BUY_MEDICINE + 1,

	SCALE_FOR_LEO = STEAL_MONEY + 1,
	SCALE_TO_ORION = SCALE_FOR_LEO + 1,
	NO_SCALE_LEO = SCALE_TO_ORION + 1,

	GIVE_GADGET = NO_SCALE_LEO + 1,
	GET_BROOCH = GIVE_GADGET + 1,
	GO_TO_SKYLANDS = GET_BROOCH + 1,
	TRAVERSE_GREENWAY = GO_TO_SKYLANDS + 1,
	COLOSSEUM = TRAVERSE_GREENWAY + 1,
	ASTRONOMER_END = COLOSSEUM + 1,
	ASTRONOMER_ENCHANT = ASTRONOMER_END + 1,

	KINGDOM = ASTRONOMER_ENCHANT + 1,

	UNLOCK_WINDOW = KINGDOM + 1,

	QUEST_COUNT = UNLOCK_WINDOW + 1,
};

// QUESTS
struct QuestManager {
	// component that is held by the player?
	int conditions[item_count] = { 0 };
	std::vector<int> openQuests = { 
		(int) QUEST_INDEX::FIX_BRIDGE,
		(int) QUEST_INDEX::TUTORIAL_WALK, 
		(int) QUEST_INDEX::TUTORIAL_ATTACK,
		(int) QUEST_INDEX::TUTORIAL_JUMP,
		(int) QUEST_INDEX::GATHER_FRUITS,
		(int) QUEST_INDEX::UNLOCK_WINDOW,
	};
	std::vector<int> completedQuests;
};

enum class QUEST_STAGE {
	START = 0,
	DURING = 1,
	END = 2,
};

struct Quest {
	std::string name = "";
	std::string description = "";
	QUEST_STAGE stage = QUEST_STAGE::START;
	std::map<ITEM_CONDITION_ID, int> requirements; // key items and how many of each
	std::vector<int> childQuests;
	std::vector<QUEST_INDEX> questsToClose;
	std::map<ITEM_CONDITION_ID, int> rewards; // key items and how many of each
	bool visible;
};


// NON COMPONENT HELPER STRUCTS FOR DIALOGUE
enum class LINE_SPEAKER {
	SPEAKER = 0,
	PLAYER = 1,
	WORLD = 2
};

struct DialogueLine {
	std::string text;
	LINE_SPEAKER line_speaker = LINE_SPEAKER::SPEAKER;
};

enum class DIALOGUE_BRANCH {
	PRIMARY = 0,
	YES = 1,
	NO = 2,
};

struct DialogueStage {
	/*
	DIALOGUE STAGE STRUCTURE:
	primary: what the main text of each dialogue line is
	autoTrigger: true if it will start when you're in range, false if not
	conditional: true if the dialogue has a yes/no option, false if not
	yesBranch: if conditional==true, and the player chooses yes, then go down this branch
	noBranch: if conditional==true, and the player chooses no, then go down this branch
	dialogue stage can loop primary, does not loop yes or no branch
	*/
	DIALOGUE_BRANCH cur_stage = DIALOGUE_BRANCH::PRIMARY;

	std::vector<DialogueLine> primary;

	std::vector<DialogueLine> yesBranch;
	std::vector<DialogueLine> noBranch;

	bool conditional = false;
	bool autoTrigger = false;
	std::string yesOption = "Confirm", noOption = "Decline";
};

struct DialogueSequence {
	// initialize map entries for each stage
	std::map<QUEST_STAGE, DialogueStage> sequence = {
		{QUEST_STAGE::START, DialogueStage()},
		{QUEST_STAGE::DURING, DialogueStage()},
		{QUEST_STAGE::END, DialogueStage()}
	};
};

enum class SPEAKER_ID {
	NO_SPEAKER = -1,

	VENUS = NO_SPEAKER + 1,
	BRIDGE = VENUS + 1,
	BRIDGE_RIGHT = BRIDGE + 1,
	ORION = BRIDGE_RIGHT + 1,
	LEO = ORION + 1,
	APOTHECARY = LEO + 1,

	INN_KEEPER = APOTHECARY + 1,
	GREENWAY_GUARD = INN_KEEPER + 1,
	POLARIS = GREENWAY_GUARD + 1,
	ASTRONOMER = POLARIS + 1,
	STARSEEKER_MONUMENT = ASTRONOMER + 1,
	TOWN_FAMILY = STARSEEKER_MONUMENT + 1,
	GOSSIP_GIRLS = TOWN_FAMILY + 1,

	RESPAWN_ANCHOR = GOSSIP_GIRLS + 1,

	TUTORIAL_WALK = RESPAWN_ANCHOR + 1,
	TUTORIAL_JUMP = TUTORIAL_WALK + 1,
	TUTORIAL_ATTACK = TUTORIAL_JUMP + 1,
	TUTORIAL_MAP = TUTORIAL_ATTACK + 1,
	TUTORIAL_ROCK = TUTORIAL_MAP + 1,
	TUTORIAL_ROCK_2 = TUTORIAL_ROCK + 1,
	TUTORIAL_ROCK_3 = TUTORIAL_ROCK_2 + 1,
	TUTORIAL_STEALTH = TUTORIAL_ROCK_3 + 1,
	TUTORIAL_INN_EXT = TUTORIAL_STEALTH + 1,
	TUTORIAL_INN_INT = TUTORIAL_INN_EXT + 1,

	MAIL = TUTORIAL_INN_INT + 1,
	WINDOW = MAIL + 1,
	ARISTOCRAT_1 = WINDOW + 1,
	ARISTOCRAT_2 = ARISTOCRAT_1 + 1,
};

// END NON COMPONENT HELPER STRUCTS FOR DIALOGUE

struct Speaker {
	int nextLine = 0;
	int complexLine = 0;
	SPEAKER_ID speaker;
	std::string name;
	std::string interactText;
	std::map <QUEST_INDEX, DialogueSequence> complexDialogueSequences;
	std::vector<std::string> simpleDialogueLines; // these will only be spoken by the speaker, there will not be back and forth
	vec2 autoTriggerRadii = { 256., 64.};
};

// ALL DATA PERTAINING TO LEVELS

struct Background {
	vec2 initPos = { 0.f, 0.f };
	vec2 position;
	vec2 scale;
	TEXTURE_ASSET_ID asset;
	float dist = .9f;
};

struct BackgroundDecoration {

};

struct ForegroundDecoration {

};

struct BackLevelObj {

};

struct ForeLevelObj {

};

struct WorldUI {

};

struct LevelElement {
	// so we can create a registry for level elements that get deleted on level reset
};

// data stored in a level to create the transitions on load
struct TransitionData {
	int levelIndex;
	vec2 nextPos;
	vec2 createPosition;
	vec2 scale;
	bool needsInteract;
	bool visible = true;
	std::string doorText;
	std::vector<QUEST_INDEX> requirements;
};

struct NPCData {
	SPEAKER_ID speaker = SPEAKER_ID::NO_SPEAKER;
	std::string name;
	std::string interactText = "[V] Speak";
	vec2 position;
	vec2 radius = { 256., 64.};
	vec2 scale;
	TEXTURE_ASSET_ID sprite;
};

// not a component in the same way that the other components are
struct Level {
	std::string levelFile;
	vec2 startPos; // not used with level transitions, but can be used to starting the world or respawning on save
	vec2 mapPos;
	std::vector<Background> backgrounds; // include them in reverse render order: last background gets rendered first
	std::vector<TransitionData> doors;
	std::vector<NPCData> npcs;
	std::string background_track;
	std::string second_background_track;
	LEVEL_INDEX inn_target_level; // if there is an inn in the world, which world does it target?
	TEXTURE_ASSET_ID inn_sprite;
};

// component used in game
struct Transition {
	// std::string nextLocation = "";
	// std::string appearsIn = "";
	// vec2 scale;
	Level nextLevel;
	LEVEL_INDEX nextLevelIndex;
	vec2 nextPos = { 0, 0 };
	bool needsInteract;
	std::vector<QUEST_INDEX> requirements;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	NOSPRITE = 0,
	BG_PRIMARY_BACK = NOSPRITE + 1,
	BG_FOREST_1_0 = BG_PRIMARY_BACK + 1,
	BG_FOREST_2_0 = BG_FOREST_1_0 + 1,
	BG_FOREST_2_1 = BG_FOREST_2_0 + 1,
	BG_CANOPY = BG_FOREST_2_1 + 1,
	BG_CABIN = BG_CANOPY + 1,
	BG_INN = BG_CABIN + 1,

	BG_TOWN = BG_INN + 1,
	BG_TOWN_1 = BG_TOWN + 1,
	BG_SEWERS = BG_TOWN_1 + 1,
	BG_SEWERS_1 = BG_SEWERS + 1,

	BG_PLAZA = BG_SEWERS_1 + 1,
	BG_PLAZA_2 = BG_PLAZA + 1,
	BG_ARISTOCRAT = BG_PLAZA_2 + 1,
	BG_ARISTOCRAT_1 = BG_ARISTOCRAT + 1,

	BG_ARISTOCRAT_HOUSE = BG_ARISTOCRAT_1 + 1,
	BG_GREENWAY = BG_ARISTOCRAT_HOUSE + 1,

	DIALOGUE_BOX = BG_GREENWAY + 1,
	MINI_MAP = DIALOGUE_BOX + 1,
	MAP_CTRLS = MINI_MAP + 1,
	QUEST_MENU = MAP_CTRLS + 1,
	HEART = QUEST_MENU + 1,

	EAGLE = HEART + 1,
	PLAYER = EAGLE + 1,
	ENEMY = PLAYER + 1,
	SOLID = ENEMY + 1,
	RAMP = SOLID + 1,

	PARTICLE = RAMP + 1,
	ROCK = PARTICLE + 1,

	GRASS = ROCK + 1,
	ITEM = GRASS + 1,
	DOOR = ITEM + 1,
	INN_BASE = DOOR + 1, // ADD ALL OTHER INN SPRITES HERE OR BELOW

	PLAYER_RUN_1 = INN_BASE + 1,
	PLAYER_RUN_2 = PLAYER_RUN_1 + 1,
	PLAYER_RUN_3 = PLAYER_RUN_2 + 1,
	PLAYER_RUN_4 = PLAYER_RUN_3 + 1,
	PLAYER_RUN_5 = PLAYER_RUN_4 + 1,
	PLAYER_RUN_6 = PLAYER_RUN_5 + 1,
	PLAYER_RUN_7 = PLAYER_RUN_6 + 1,
	PLAYER_RUN_8 = PLAYER_RUN_7 + 1,
	PLAYER_RUN_9 = PLAYER_RUN_8 + 1,
	PLAYER_RUN_10 = PLAYER_RUN_9 + 1,

	EVIL_TREE_RUN_1 = PLAYER_RUN_10 + 1,
	EVIL_TREE_RUN_2 = EVIL_TREE_RUN_1 + 1,
	EVIL_TREE_RUN_3 = EVIL_TREE_RUN_2 + 1,
	EVIL_TREE_RUN_4 = EVIL_TREE_RUN_3 + 1,
	EVIL_TREE_RUN_5 = EVIL_TREE_RUN_4 + 1,
	EVIL_TREE_RUN_6 = EVIL_TREE_RUN_5 + 1,
	EVIL_TREE_RUN_7 = EVIL_TREE_RUN_6 + 1,
	EVIL_TREE_RUN_8 = EVIL_TREE_RUN_7 + 1,

	PLAYER_JUMP_UP_1 = EVIL_TREE_RUN_8 + 1,
	PLAYER_JUMP_UP_2 = PLAYER_JUMP_UP_1 + 1,
	PLAYER_JUMP_UP_3 = PLAYER_JUMP_UP_2 + 1,
	PLAYER_JUMP_UP_4 = PLAYER_JUMP_UP_3 + 1,
	PLAYER_JUMP_UP_5 = PLAYER_JUMP_UP_4 + 1,
	PLAYER_FALL_1 = PLAYER_JUMP_UP_5 + 1,
	PLAYER_FALL_2 = PLAYER_FALL_1 + 1,
	PLAYER_FALL_3 = PLAYER_FALL_2 + 1,
	PLAYER_FALL_4 = PLAYER_FALL_3 + 1,
	PLAYER_FALL_5 = PLAYER_FALL_4 + 1,

	APPLE = PLAYER_FALL_5 + 1,
	BLUEBERRY = APPLE + 1,

	BREAKABLE_BOX = BLUEBERRY + 1,

	PLAYER_DEATH_1 = BREAKABLE_BOX + 1,
	PLAYER_DEATH_2 = PLAYER_DEATH_1 + 1,
	PLAYER_DEATH_3 = PLAYER_DEATH_2 + 1,
	PLAYER_DEATH_4 = PLAYER_DEATH_3 + 1,
	PLAYER_DEATH_5 = PLAYER_DEATH_4 + 1,
	PLAYER_DEATH_6 = PLAYER_DEATH_5 + 1,
	PLAYER_DEATH_7 = PLAYER_DEATH_6 + 1,
	PLAYER_DEATH_8 = PLAYER_DEATH_7 + 1,
	PLAYER_DEATH_9 = PLAYER_DEATH_8 + 1,
	PLAYER_DEATH_10 = PLAYER_DEATH_9 + 1,
	PLAYER_DEATH_11 = PLAYER_DEATH_10 + 1,
	PLAYER_DEATH_12 = PLAYER_DEATH_11 + 1,
	PLAYER_DEATH_13 = PLAYER_DEATH_12 + 1,

	PLAYER_ATTACK_1 = PLAYER_DEATH_13 + 1,
	PLAYER_ATTACK_2 = PLAYER_ATTACK_1 + 1,
	PLAYER_ATTACK_3 = PLAYER_ATTACK_2 + 1,
	PLAYER_ATTACK_4 = PLAYER_ATTACK_3 + 1,
	PLAYER_ATTACK_5 = PLAYER_ATTACK_4 + 1,
	PLAYER_ATTACK_6 = PLAYER_ATTACK_5 + 1,
	PLAYER_ATTACK_7 = PLAYER_ATTACK_6 + 1,
	PLAYER_ATTACK_8 = PLAYER_ATTACK_7 + 1,
	PLAYER_ATTACK_9 = PLAYER_ATTACK_8 + 1,

	PLAYER_ATTACK_UP_1 = PLAYER_ATTACK_9 + 1,
	PLAYER_ATTACK_UP_2 = PLAYER_ATTACK_UP_1 + 1,
	PLAYER_ATTACK_UP_3 = PLAYER_ATTACK_UP_2 + 1,
	PLAYER_ATTACK_UP_4 = PLAYER_ATTACK_UP_3 + 1,
	PLAYER_ATTACK_UP_5 = PLAYER_ATTACK_UP_4 + 1,
	PLAYER_ATTACK_UP_6 = PLAYER_ATTACK_UP_5 + 1,
	PLAYER_ATTACK_UP_7 = PLAYER_ATTACK_UP_6 + 1,

	SLASH_1 = PLAYER_ATTACK_UP_7 + 1,
	SLASH_2 = SLASH_1 + 1,
	SLASH_3 = SLASH_2 + 1,
	SLASH_4 = SLASH_3 + 1,
	SLASH_5 = SLASH_4 + 1,
	ORION = SLASH_5 + 1,
	STATUE = ORION + 1,
	SPARKLE_1 = STATUE + 1,
	SPARKLE_2 = SPARKLE_1 + 1,
	SPARKLE_3 = SPARKLE_2 + 1,
	SPARKLE_4 = SPARKLE_3 + 1,
	SPARKLE_5 = SPARKLE_4 + 1,
	SPARKLE_6 = SPARKLE_5 + 1,
	SPARKLE_7 = SPARKLE_6 + 1,
	SPARKLE_8 = SPARKLE_7 + 1,
	SPARKLE_9 = SPARKLE_8 + 1,
	SPARKLE_10 = SPARKLE_9 + 1,
	SPARKLE_11 = SPARKLE_10 + 1,
	SPARKLE_12 = SPARKLE_11 + 1,
	SPARKLE_13 = SPARKLE_12 + 1,

	LEO = SPARKLE_13 + 1,
	APOTHECARY = LEO + 1,
	VENUS = APOTHECARY + 1,
	ROPE = VENUS + 1,

	EVIL_BIRD_FLY_1 = ROPE + 1,
	EVIL_BIRD_FLY_2 = EVIL_BIRD_FLY_1 + 1,
	EVIL_BIRD_FLY_3 = EVIL_BIRD_FLY_2 + 1,
	EVIL_BIRD_FLY_4 = EVIL_BIRD_FLY_3 + 1,
	EVIL_BIRD_FLY_5 = EVIL_BIRD_FLY_4 + 1,
	EVIL_BIRD_FLY_6 = EVIL_BIRD_FLY_5 + 1,
	EVIL_BIRD_FLY_7 = EVIL_BIRD_FLY_6 + 1,
	EVIL_BIRD_FLY_8 = EVIL_BIRD_FLY_7 + 1,
	EVIL_BIRD_FLY_9 = EVIL_BIRD_FLY_8 + 1,
	EVIL_BIRD_FLY_10 = EVIL_BIRD_FLY_9 + 1,
	EVIL_BIRD_FLY_11 = EVIL_BIRD_FLY_10 + 1,
	EVIL_BIRD_FLY_12 = EVIL_BIRD_FLY_11 + 1,

	EVIL_BIRD_EGG = EVIL_BIRD_FLY_12 + 1,

	ASCENSION_TITLE = EVIL_BIRD_EGG + 1,

	INN_KEEPER = ASCENSION_TITLE + 1,

	HELP_SCREEN = INN_KEEPER + 1,
	BG_PRIMARY_BACK_BLANK = HELP_SCREEN + 1,

	VASE = BG_PRIMARY_BACK_BLANK + 1,
	BUSH = VASE + 1,
	COIN = BUSH + 1,

	BUTTON_OVERLAY = COIN + 1,

	SEWER_SLIME_1 = BUTTON_OVERLAY + 1,
	SEWER_SLIME_2 = SEWER_SLIME_1 + 1,
	SEWER_SLIME_3 = SEWER_SLIME_2 + 1,
	SEWER_SLIME_4 = SEWER_SLIME_3 + 1,
	SEWER_SLIME_5 = SEWER_SLIME_4 + 1,
	SEWER_SLIME_6 = SEWER_SLIME_5 + 1,
	SEWER_SLIME_7 = SEWER_SLIME_6 + 1,
	SEWER_SLIME_8 = SEWER_SLIME_7 + 1,
	SEWER_SLIME_9 = SEWER_SLIME_8 + 1,

	SLIME_BOUNCE_1 = SEWER_SLIME_9 + 1,
	SLIME_BOUNCE_2 = SLIME_BOUNCE_1 + 1,
	SLIME_BOUNCE_3 = SLIME_BOUNCE_2 + 1,
	SLIME_BOUNCE_4 = SLIME_BOUNCE_3 + 1,
	SLIME_BOUNCE_5 = SLIME_BOUNCE_4 + 1,
	SLIME_BOUNCE_6 = SLIME_BOUNCE_5 + 1,
	SLIME_BOUNCE_7 = SLIME_BOUNCE_6 + 1,
	SLIME_BOUNCE_8 = SLIME_BOUNCE_7 + 1,
	SLIME_BOUNCE_9 = SLIME_BOUNCE_8 + 1,
	SLIME_BOUNCE_10 = SLIME_BOUNCE_9 + 1,
	SLIME_BOUNCE_11 = SLIME_BOUNCE_10 + 1,
	SLIME_BOUNCE_12 = SLIME_BOUNCE_11 + 1,
	SLIME_BOUNCE_13 = SLIME_BOUNCE_12 + 1,
	SLIME_BOUNCE_14 = SLIME_BOUNCE_13 + 1,

	STARSEEKER_CAPTAIN = SLIME_BOUNCE_14 + 1,
	STANDING_STARSEEKER = STARSEEKER_CAPTAIN + 1,
	MOM = STANDING_STARSEEKER + 1,
	KING = MOM + 1,
	GUARD_CAPTAIN = KING + 1,
	GUARD = GUARD_CAPTAIN + 1,
	TOWN_GOSSIPER_1 = GUARD + 1,
	TOWN_GOSSIPER_2 = TOWN_GOSSIPER_1 + 1,
	CHILD = TOWN_GOSSIPER_2 + 1,
	ASTRONOMER = CHILD + 1,
	ARISTOCRAT_1 = ASTRONOMER + 1,
	ARISTOCRAT_2 = ARISTOCRAT_1 + 1,

	GUARD_RUN_1 = ARISTOCRAT_2 + 1,
	GUARD_RUN_2 = GUARD_RUN_1 + 1,
	GUARD_RUN_3 = GUARD_RUN_2 + 1,
	GUARD_RUN_4 = GUARD_RUN_3 + 1,
	GUARD_RUN_5 = GUARD_RUN_4 + 1,
	GUARD_RUN_6 = GUARD_RUN_5 + 1,
	GUARD_RUN_7 = GUARD_RUN_6 + 1,
	GUARD_RUN_8 = GUARD_RUN_7 + 1,
	GUARD_RUN_9 = GUARD_RUN_8 + 1,
	GUARD_RUN_10 = GUARD_RUN_9 + 1,

	GUARD_CAPTAIN_RUN_1 = GUARD_RUN_10 + 1,
	GUARD_CAPTAIN_RUN_2 = GUARD_CAPTAIN_RUN_1 + 1,
	GUARD_CAPTAIN_RUN_3 = GUARD_CAPTAIN_RUN_2 + 1,
	GUARD_CAPTAIN_RUN_4 = GUARD_CAPTAIN_RUN_3 + 1,
	GUARD_CAPTAIN_RUN_5 = GUARD_CAPTAIN_RUN_4 + 1,
	GUARD_CAPTAIN_RUN_6 = GUARD_CAPTAIN_RUN_5 + 1,
	GUARD_CAPTAIN_RUN_7 = GUARD_CAPTAIN_RUN_6 + 1,
	GUARD_CAPTAIN_RUN_8 = GUARD_CAPTAIN_RUN_7 + 1,
	GUARD_CAPTAIN_RUN_9 = GUARD_CAPTAIN_RUN_8 + 1,
	GUARD_CAPTAIN_RUN_10 = GUARD_CAPTAIN_RUN_9 + 1,

	SLEEPING_STARSEEKER_1 = GUARD_CAPTAIN_RUN_10 + 1,
	SLEEPING_STARSEEKER_2 = SLEEPING_STARSEEKER_1 + 1,
	SLEEPING_STARSEEKER_3 = SLEEPING_STARSEEKER_2 + 1,
	SLEEPING_STARSEEKER_4 = SLEEPING_STARSEEKER_3 + 1,
	SLEEPING_STARSEEKER_5 = SLEEPING_STARSEEKER_4 + 1,
	SLEEPING_STARSEEKER_6 = SLEEPING_STARSEEKER_5 + 1,
	SLEEPING_STARSEEKER_7 = SLEEPING_STARSEEKER_6 + 1,
	SLEEPING_STARSEEKER_8 = SLEEPING_STARSEEKER_7 + 1,
	SLEEPING_STARSEEKER_9 = SLEEPING_STARSEEKER_8 + 1,
	SLEEPING_STARSEEKER_10 = SLEEPING_STARSEEKER_9 + 1,
	SLEEPING_STARSEEKER_11 = SLEEPING_STARSEEKER_10 + 1, 
	SLEEPING_STARSEEKER_12 = SLEEPING_STARSEEKER_11 + 1,
	SLEEPING_STARSEEKER_13 = SLEEPING_STARSEEKER_12 + 1,
	SLEEPING_STARSEEKER_14 = SLEEPING_STARSEEKER_13 + 1,
	SLEEPING_STARSEEKER_15 = SLEEPING_STARSEEKER_14 + 1,
	SLEEPING_STARSEEKER_16 = SLEEPING_STARSEEKER_15 + 1,
	SLEEPING_STARSEEKER_17 = SLEEPING_STARSEEKER_16 + 1,
	SLEEPING_STARSEEKER_18 = SLEEPING_STARSEEKER_17 + 1,
	SLEEPING_STARSEEKER_19 = SLEEPING_STARSEEKER_18 + 1,

	TEXTURE_COUNT = SLEEPING_STARSEEKER_19 + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

const std::vector<TEXTURE_ASSET_ID> playerRunFrames {
		TEXTURE_ASSET_ID::PLAYER_RUN_1,
		TEXTURE_ASSET_ID::PLAYER_RUN_2,
		TEXTURE_ASSET_ID::PLAYER_RUN_3,
		TEXTURE_ASSET_ID::PLAYER_RUN_4,
		TEXTURE_ASSET_ID::PLAYER_RUN_5,
		TEXTURE_ASSET_ID::PLAYER_RUN_6,
		TEXTURE_ASSET_ID::PLAYER_RUN_7,
		TEXTURE_ASSET_ID::PLAYER_RUN_8,
		TEXTURE_ASSET_ID::PLAYER_RUN_9,
		TEXTURE_ASSET_ID::PLAYER_RUN_10 };

const std::vector<TEXTURE_ASSET_ID> evilTreeRunFrames{
		TEXTURE_ASSET_ID::EVIL_TREE_RUN_1,
		TEXTURE_ASSET_ID::EVIL_TREE_RUN_2,
		TEXTURE_ASSET_ID::EVIL_TREE_RUN_3,
		TEXTURE_ASSET_ID::EVIL_TREE_RUN_4,
		TEXTURE_ASSET_ID::EVIL_TREE_RUN_5,
		TEXTURE_ASSET_ID::EVIL_TREE_RUN_6,
		TEXTURE_ASSET_ID::EVIL_TREE_RUN_7,
		TEXTURE_ASSET_ID::EVIL_TREE_RUN_8 };

const std::vector<TEXTURE_ASSET_ID> playerJumpUpFrames{
	TEXTURE_ASSET_ID::PLAYER_JUMP_UP_1,
	TEXTURE_ASSET_ID::PLAYER_JUMP_UP_2,
	TEXTURE_ASSET_ID::PLAYER_JUMP_UP_3,
	TEXTURE_ASSET_ID::PLAYER_JUMP_UP_4,
	TEXTURE_ASSET_ID::PLAYER_JUMP_UP_5
};

const std::vector<TEXTURE_ASSET_ID> playerFallFrames{
	TEXTURE_ASSET_ID::PLAYER_FALL_1,
	TEXTURE_ASSET_ID::PLAYER_FALL_2,
	TEXTURE_ASSET_ID::PLAYER_FALL_3,
	TEXTURE_ASSET_ID::PLAYER_FALL_4,
	TEXTURE_ASSET_ID::PLAYER_FALL_5
};

const std::vector<TEXTURE_ASSET_ID> playerDeathFrames{
	TEXTURE_ASSET_ID::PLAYER_DEATH_1,
	TEXTURE_ASSET_ID::PLAYER_DEATH_2,
	TEXTURE_ASSET_ID::PLAYER_DEATH_3,
	TEXTURE_ASSET_ID::PLAYER_DEATH_4,
	TEXTURE_ASSET_ID::PLAYER_DEATH_5,
	TEXTURE_ASSET_ID::PLAYER_DEATH_6,
	TEXTURE_ASSET_ID::PLAYER_DEATH_7,
	TEXTURE_ASSET_ID::PLAYER_DEATH_8,
	TEXTURE_ASSET_ID::PLAYER_DEATH_9,
	TEXTURE_ASSET_ID::PLAYER_DEATH_10,
	TEXTURE_ASSET_ID::PLAYER_DEATH_11,
	TEXTURE_ASSET_ID::PLAYER_DEATH_12,
	TEXTURE_ASSET_ID::PLAYER_DEATH_13
};

const std::vector<TEXTURE_ASSET_ID> playerAttackFrames{
	TEXTURE_ASSET_ID::PLAYER_ATTACK_1,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_2,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_3,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_4,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_5,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_6,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_7,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_8,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_9,
};

const std::vector<TEXTURE_ASSET_ID> playerAttackUpFrames{
	TEXTURE_ASSET_ID::PLAYER_ATTACK_UP_1,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_UP_2,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_UP_3,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_UP_4,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_UP_5,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_UP_6,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_UP_7
};

const std::vector<TEXTURE_ASSET_ID> attackSlashFrames{
	TEXTURE_ASSET_ID::SLASH_1,
	TEXTURE_ASSET_ID::SLASH_2,
	TEXTURE_ASSET_ID::SLASH_3,
	TEXTURE_ASSET_ID::SLASH_4,
	TEXTURE_ASSET_ID::SLASH_5
};

const std::vector<TEXTURE_ASSET_ID> itemSparkleFrames{
	TEXTURE_ASSET_ID::SPARKLE_1,
	TEXTURE_ASSET_ID::SPARKLE_2,
	TEXTURE_ASSET_ID::SPARKLE_3,
	TEXTURE_ASSET_ID::SPARKLE_4,
	TEXTURE_ASSET_ID::SPARKLE_5,
	TEXTURE_ASSET_ID::SPARKLE_6,
	TEXTURE_ASSET_ID::SPARKLE_7,
	TEXTURE_ASSET_ID::SPARKLE_8,
	TEXTURE_ASSET_ID::SPARKLE_9,
	TEXTURE_ASSET_ID::SPARKLE_10,
	TEXTURE_ASSET_ID::SPARKLE_11,
	TEXTURE_ASSET_ID::SPARKLE_12,
	TEXTURE_ASSET_ID::SPARKLE_13
};

const std::vector<TEXTURE_ASSET_ID> evilBirdFlyFrames{
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_1,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_2,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_3,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_4,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_5,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_6,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_7,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_8,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_9,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_10,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_11,
	TEXTURE_ASSET_ID::EVIL_BIRD_FLY_12,
};

const std::vector<TEXTURE_ASSET_ID> slimeBounceFrames{
	TEXTURE_ASSET_ID::SLIME_BOUNCE_1,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_2,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_3,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_4,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_5,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_6,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_7,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_8,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_9,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_10,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_11,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_12,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_13,
	TEXTURE_ASSET_ID::SLIME_BOUNCE_14
};

const std::vector<TEXTURE_ASSET_ID> sewerSlimeFrames{
	TEXTURE_ASSET_ID::SEWER_SLIME_1,
	TEXTURE_ASSET_ID::SEWER_SLIME_2,
	TEXTURE_ASSET_ID::SEWER_SLIME_3,
	TEXTURE_ASSET_ID::SEWER_SLIME_4,
	TEXTURE_ASSET_ID::SEWER_SLIME_5,
	TEXTURE_ASSET_ID::SEWER_SLIME_6,
	TEXTURE_ASSET_ID::SEWER_SLIME_7,
	TEXTURE_ASSET_ID::SEWER_SLIME_8,
	TEXTURE_ASSET_ID::SEWER_SLIME_9
};

const std::vector<TEXTURE_ASSET_ID> guardRunFrames{
	TEXTURE_ASSET_ID::GUARD_RUN_1,
	TEXTURE_ASSET_ID::GUARD_RUN_2,
	TEXTURE_ASSET_ID::GUARD_RUN_3,
	TEXTURE_ASSET_ID::GUARD_RUN_4,
	TEXTURE_ASSET_ID::GUARD_RUN_5,
	TEXTURE_ASSET_ID::GUARD_RUN_6,
	TEXTURE_ASSET_ID::GUARD_RUN_7,
	TEXTURE_ASSET_ID::GUARD_RUN_8,
	TEXTURE_ASSET_ID::GUARD_RUN_9,
	TEXTURE_ASSET_ID::GUARD_RUN_10,
};

const std::vector<TEXTURE_ASSET_ID> guardCaptainRunFrames{
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_1,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_2,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_3,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_4,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_5,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_6,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_7,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_8,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_9,
	TEXTURE_ASSET_ID::GUARD_CAPTAIN_RUN_10,
};

const std::vector<TEXTURE_ASSET_ID> sleepingStarseekerFrames{
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_1,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_2,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_3,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_4,	
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_5,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_6,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_7,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_8,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_9,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_10,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_11,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_12,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_13,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_14,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_15,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_16,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_17,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_18,
	TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_19,
};

const std::vector< std::vector<TEXTURE_ASSET_ID>> allAnimationsVector{
	//when initializing an animation component for an entity, make sure the vector number corresponds
	// to the index here
	playerRunFrames, //0
	evilTreeRunFrames, //1
	playerJumpUpFrames, //2
	playerFallFrames, //3
	playerDeathFrames, //4
	playerAttackFrames, //5
	playerAttackUpFrames, //6
	attackSlashFrames, //7
	itemSparkleFrames, //8
	evilBirdFlyFrames, //9
	slimeBounceFrames, //10
	sewerSlimeFrames, //11
	guardRunFrames, //12
	guardCaptainRunFrames, //13
	sleepingStarseekerFrames //14
};

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	WIND = TEXTURED + 1,
	WIND_GRASS = WIND + 1,
	EFFECT_COUNT = WIND_GRASS + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	PLAYER = 0,
	SPRITE = PLAYER + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class PLAYER_STATE_ID {
	IDLE = 0,
	HIDING = 1,
	PLAYER_COUNT = HIDING + 1
};
const int player_state_count = (int)PLAYER_STATE_ID::PLAYER_COUNT;

enum class PROJECTILE_TYPE_ID {
	ENEMY_EGG = 0,
	DECOY = 1,
	HEART = 2,
	FEATHER = 3,
	ICICLE = 4,
	FIREBALL = 5,
	PROJECTILE_COUNT = FIREBALL + 1
};
const int projectile_type_count = (int)PROJECTILE_TYPE_ID::PROJECTILE_COUNT;

enum class ENEMY_TYPE_ID {
	TREE = 0,
	BIRDO = 1,
	DIVE_BIRDO = 2,
	GUARD = 3,
	GUARDSUMMON = 4,
	GUARDCAPTAIN = 5,
	BIRDMAN = 6,
	RAT = 7,
	BOSS = 8,
	ENEMY_TYPE_COUNT = BOSS + 1
};
const int enemy_type_count = (int)ENEMY_TYPE_ID::ENEMY_TYPE_COUNT;

enum class ENEMY_STATE_ID {
	IDLE = 0,
	ALERT = 1,
	CHASE = 2,
	EXTRA = 3,
	ENEMY_COUNT = EXTRA + 1
};
const int enemy_state_count = (int)ENEMY_STATE_ID::ENEMY_COUNT;

enum class INTERACTABLE_TYPE_ID {
	ITEM = 0,
	DOOR = ITEM + 1,
	STEALTH = DOOR + 1,
	NPC = STEALTH + 1,
	RESPAWN_TETHER = NPC + 1,
	INTERACTABLE_COUNT = RESPAWN_TETHER + 1
};
const int interactable_state_count = (int)INTERACTABLE_TYPE_ID::INTERACTABLE_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	bool rendered = false;
};

/***********************************************/
// NOTE: KEEP THIS IN SYNC WITH THE push_back order of the registry
enum class LEVEL_INDEX {
	CABIN = 0,
	FOREST_1 = CABIN + 1,
	FOREST_2 = FOREST_1 + 1,
	CANOPY = FOREST_2 + 1,
	BRIDGE = CANOPY + 1,
	TOWN = BRIDGE + 1,
	PLAZA = TOWN + 1,
	SEWERS = PLAZA + 1,

	UNDERCITY = SEWERS + 1,
	ARISTOCRAT = UNDERCITY + 1,
	ARISTOCRAT_HOUSE = ARISTOCRAT + 1,
	LEO_SHOP = ARISTOCRAT_HOUSE + 1,
	GREENWAY = LEO_SHOP + 1,
	SKYLANDS = GREENWAY + 1,
	PALACE_EXT = SKYLANDS + 1,
	PALACE_INT_1 = PALACE_EXT + 1,
	PALACE_INT_2 = PALACE_INT_1 + 1,
	THRONE_ROOM = PALACE_INT_2 + 1,

	INN_TOWN = THRONE_ROOM + 1,
	INN_UNDERCITY = INN_TOWN + 1,
	INN_CITY = INN_UNDERCITY + 1,
	INN_SKYLANDS = INN_CITY + 1,

	LEVEL_COUNT = INN_SKYLANDS + 1,
};
const int level_count = (int) LEVEL_INDEX::LEVEL_COUNT;


/**************/
class LevelRegistry {

private:
	static Background defineBackground(
		vec2 pos, 
		vec2 scale, 
		TEXTURE_ASSET_ID asset, 
		float parallax_dist
	) {
		Background back = Background();
		back.initPos = pos;
		back.position = pos;
		back.scale = scale;
		back.asset = asset;
		back.dist = parallax_dist;
		return back;
	}

	static TransitionData defineTransition(
		vec2 createPos, 
		vec2 scale, 
		vec2 toPos, 
		LEVEL_INDEX toLevel, 
		std::string doorText,
		bool interact
	) {
		TransitionData t = TransitionData();
		t.levelIndex = (int) toLevel;
		t.createPosition = createPos;
		t.nextPos = toPos;
		t.scale = scale;
		t.needsInteract = interact;
		t.requirements = {};
		t.doorText = doorText;
		return t;
	}

	static NPCData defineNPC(
		SPEAKER_ID speaker,
		std::string name,
		vec2 position,
		vec2 scale,
		TEXTURE_ASSET_ID sprite
	) {
		NPCData data;
		data.speaker = speaker;
		data.name = name;
		data.position = position;
		data.scale = scale;
		data.sprite = sprite;
		return data;
	}

	static NPCData defineInspectable(
		SPEAKER_ID speaker,
		std::string name,
		vec2 position,
		vec2 scale,
		TEXTURE_ASSET_ID sprite
	) {
		NPCData data;
		data.speaker = speaker;
		data.name = name;
		data.position = position;
		data.scale = scale;
		data.sprite = sprite;
		data.interactText = "[V] Inspect";
		return data;
	}

	void initForest_1() {
		forest_1.levelFile = "forest_1.txt";
		forest_1.startPos = { 1024., 1332. };
		forest_1.mapPos = { 1618., 7200. };

		// define backgrounds
		Background farBack = defineBackground(
			vec2(4000., 920.),
			vec2(3375., 960.),
			TEXTURE_ASSET_ID::BG_PRIMARY_BACK,
			0.97f
		);
		/*
		Background midBack = defineBackground(
			vec2(5748., 596.),
			vec2(1.004 * 11520., 1.004 * 1216.),
			TEXTURE_ASSET_ID::BG_FOREST_1_2,
			0.5f
		);
		Background closeBack = defineBackground(
			vec2(5748., 596.),
			vec2(1.004 * 11520., 1.004 * 1216.),
			TEXTURE_ASSET_ID::BG_FOREST_1_1,
			0.2f
		);
		*/
		Background foreground = defineBackground(
			vec2(3106., 842.),
			vec2(7616. - 68, 2448. - 32),
			TEXTURE_ASSET_ID::BG_FOREST_1_0,
			0.0f
		);

		// forest_1.backgrounds.push_back(farBack);
		// forest_1.backgrounds.push_back(midBack);
		// forest_1.backgrounds.push_back(closeBack);
		forest_1.backgrounds.push_back(farBack);
		forest_1.backgrounds.push_back(foreground);

		// define transitions
		TransitionData toCabin = defineTransition(
			{ 1024., 1344. },
			{ 64., 128. },
			{ 256., 628. },
			LEVEL_INDEX::CABIN,
			"Enter cabin",
			true
		);
		// TEMP, change to tree later
		TransitionData toCanopy = defineTransition(
			{ 5760., 1600. },
			{ 64., 128. },
			{ 200., 692. },
			LEVEL_INDEX::CANOPY,
			"Climb tree",
			true
		);
		toCanopy.requirements.push_back(QUEST_INDEX::GATHER_FRUITS);

		TransitionData toBridge = defineTransition(
			{ 6256., 1530. },
			{ 64., 256. },
			{ 96., 696. },
			LEVEL_INDEX::FOREST_2,
			"Cross the bridge",
			false
		);
		toBridge.requirements.push_back(QUEST_INDEX::FIX_BRIDGE);

		forest_1.doors.push_back(toCabin);
		forest_1.doors.push_back(toCanopy);
		forest_1.doors.push_back(toBridge);

		// define NPCs
		NPCData tutJump = defineNPC(
			SPEAKER_ID::TUTORIAL_JUMP,
			"",
			{ 3320., 1052. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		NPCData tutAtk = defineNPC(
			SPEAKER_ID::TUTORIAL_ATTACK,
			"",
			{ 5700., 1600. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		NPCData bridge = defineInspectable(
			SPEAKER_ID::BRIDGE_RIGHT,
			"Bridge",
			{ 6232., 1600. },
			{ 128., 84. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		tutJump.interactText = "";
		tutAtk.interactText = "";
		forest_1.npcs.push_back(tutJump);
		forest_1.npcs.push_back(tutAtk);
		forest_1.npcs.push_back(bridge);

		// define music
		forest_1.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";
		forest_1.second_background_track = "DANCE_OF_THE_PIXIES_-_CHASE.wav";
	}

	void initCabin() {
		cabin.startPos = {256., 308. };
		cabin.levelFile = "cabin.txt";
		cabin.mapPos = { 937., 7364. };

		// define backgrounds
		Background farBack = defineBackground(
			vec2(1280., 720.),
			vec2(960., 256.),
			TEXTURE_ASSET_ID::BG_PRIMARY_BACK,
			0.97f
		);
		Background foreground = defineBackground(
			vec2(704., 384.),
			vec2(2496., 1424.),
			TEXTURE_ASSET_ID::BG_CABIN,
			0.0f
		);
		cabin.backgrounds.push_back(farBack);
		cabin.backgrounds.push_back(foreground);


		// define transitions
		TransitionData toForest = defineTransition(
			{ 256., 640. },
			{ 64., 128. },
			{ 1024., 1332. },
			LEVEL_INDEX::FOREST_1,
			"Exit to forest",
			true
		);

		cabin.doors.push_back(toForest);

		// define NPCs
		NPCData venus = defineNPC(
			SPEAKER_ID::VENUS,
			"Venus",
			{ 600., 628. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::VENUS
		);
		NPCData tutWalk = defineNPC(
			SPEAKER_ID::TUTORIAL_WALK,
			"",
			{ 256., 308. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		tutWalk.interactText = "";
		cabin.npcs.push_back(venus);
		cabin.npcs.push_back(tutWalk);

		// define music
		cabin.background_track = "BEGINNINGS.wav";
		cabin.second_background_track = "BEGINNINGS.wav";
	}

	void initForest_2() {
		forest_2.startPos = { 640., 630. };
		forest_2.levelFile = "forest_2.txt";
		forest_2.mapPos = { 3393., 7068. };

		Background farBack = defineBackground(
			vec2(6432. - 74., 390.),
			vec2(2700., 720.),
			TEXTURE_ASSET_ID::BG_PRIMARY_BACK,
			0.97f
		);
		Background midBack = defineBackground(
			vec2(6432. - 74., 416. + 94.),
			vec2(14144. - 48, 1552.),
			TEXTURE_ASSET_ID::BG_FOREST_2_1,
			0.08f
		);
		Background foreground = defineBackground(
			vec2(6432. - 74., 416. + 94.),
			vec2(14144. - 48, 1552.),
			TEXTURE_ASSET_ID::BG_FOREST_2_0,
			0.0f
		);
		forest_2.backgrounds.push_back(farBack);
		forest_2.backgrounds.push_back(midBack);
		forest_2.backgrounds.push_back(foreground);

		// define backgrounds
		/*
		Background BACKGROUND = defineBackground(
			POSITION,
			SCALE,
			ASSET,
			PARALLAX_DISTANCE
		);

		forest_2.backgrounds.push_back(BACKGROUND);
		*/

		// define transitions
		TransitionData toTown = defineTransition(
			{ 12400., 256. },
			{ 64., 256. },
			{ 128., 1516. },
			LEVEL_INDEX::TOWN,
			"Enter Town",
			false
		);
		TransitionData toCanopy = defineTransition(
			{ 640., 636. },
			{ 64., 128. },
			{ 6912., 820. },
			LEVEL_INDEX::CANOPY,
			"Climb tree",
			true
		);
		TransitionData toBridge = defineTransition(
			{ 32., 632. },
			{ 64., 256. },
			{ 6096., 1588. },
			LEVEL_INDEX::FOREST_1,
			"Cross bridge",
			false
		);
		toBridge.requirements.push_back(QUEST_INDEX::FIX_BRIDGE);

		forest_2.doors.push_back(toTown);
		forest_2.doors.push_back(toCanopy);
		forest_2.doors.push_back(toBridge);

		// define NPCs
		/*
		NPCData NPC = defineNPC(
			SPEAKER_ID,
			CREATE_POSITION,
			SCALE,
			SPRITE
		);

		ROOM.npcs.push_back(NPC);
		*/
		NPCData orion = defineNPC(
			SPEAKER_ID::ORION,
			"Orion",
			{ 10800., 308. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::ORION
		);
		NPCData bridge = defineInspectable(
			SPEAKER_ID::BRIDGE,
			"Bridge",
			{ 128., 696. },
			{ 64., 84. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		// forest_2.npcs.push_back(orion);
		forest_2.npcs.push_back(bridge);

		forest_2.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";
		forest_2.second_background_track = "DANCE_OF_THE_PIXIES_-_CHASE.wav";

	}

	void initCanopy() {
		canopy.startPos = { 200., 692. };
		canopy.levelFile = "canopy.txt";
		canopy.mapPos = { 2277., 7122. };

		// define backgrounds
		Background foreground = defineBackground(
			vec2(3584. - 64, 544. - 28),
			vec2(8448. - 80, 1808. - 28),
			TEXTURE_ASSET_ID::BG_CANOPY,
			0.0f
		);
		canopy.backgrounds.push_back(foreground);

		// define transitions
		TransitionData toForest_1 = defineTransition(
			{ 200., 704. },
			{ 64., 128. },
			{ 5760., 1588 },
			LEVEL_INDEX::FOREST_1,
			"Climb down to forest",
			true
		);
		toForest_1.visible = false;
		TransitionData toForest_2 = defineTransition(
			{ 6912., 832. },
			{ 64., 128. },
			{ 640., 630. },
			LEVEL_INDEX::FOREST_2,
			"Climb down to forest",
			true
		);
		toForest_2.visible = false;

		canopy.doors.push_back(toForest_1);
		canopy.doors.push_back(toForest_2);

		// define NPCs

		canopy.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";
		canopy.second_background_track = "DANCE_OF_THE_PIXIES_-_CHASE.wav";
	}

	void initBridge() {

	}

	void initTown() {
		town.startPos = { 192., 1500. };
		town.levelFile = "town.txt";
		town.mapPos = { 6294., 7210. };

		// define backgrounds
		Background farBack = defineBackground(
			vec2(3040. - 64., 1032.),
			vec2(4050., 1080.),
			TEXTURE_ASSET_ID::BG_PRIMARY_BACK,
			0.96f
		);
		Background midBack = defineBackground(
			vec2(3040. - 64., 992. + 234.),
			vec2(7360. - 88, 2704. - 36.),
			TEXTURE_ASSET_ID::BG_TOWN_1,
			0.08f
		);
		Background foreground = defineBackground(
			vec2(3040. - 64., 992. + 234.),
			vec2(7360. - 88, 2704. - 64.),
			TEXTURE_ASSET_ID::BG_TOWN,
			0.0f
		);
		town.backgrounds.push_back(farBack);
		town.backgrounds.push_back(midBack);
		town.backgrounds.push_back(foreground);
		/*
		Background BACKGROUND = defineBackground(
			POSITION,
			SCALE,
			ASSET,
			PARALLAX_DISTANCE
		);

		ROOM.backgrounds.push_back(BACKGROUND);
		*/

		// define transitions
		TransitionData toForest_2 = defineTransition(
			{ 48., 1500. },
			{ 128., 256. },
			{ 12240., 300. },
			LEVEL_INDEX::FOREST_2,
			"",
			false
		);

		TransitionData toSewers = defineTransition(
			{ 864., 2076. },
			{ 256., 128. },
			{ 3968., 0. },
			LEVEL_INDEX::SEWERS,
			"",
			false
		);
		TransitionData toPlaza = defineTransition(
			{ 5916., 848. },
			{ 64., 256. },
			{ 96., 756. },
			LEVEL_INDEX::PLAZA,
			"",
			false
		);
		toSewers.requirements.push_back(QUEST_INDEX::GIVE_GADGET);

		town.doors.push_back(toForest_2);
		town.doors.push_back(toSewers);
		town.doors.push_back(toPlaza);

		// set up inns
		town.inn_sprite = TEXTURE_ASSET_ID::INN_BASE;
		town.inn_target_level = LEVEL_INDEX::INN_TOWN;

		// define NPCs
		/*
		NPCData NPC = defineNPC(
			SPEAKER_ID,
			CREATE_POSITION,
			SCALE,
			SPRITE
		);

		ROOM.npcs.push_back(NPC);
		*/
		NPCData orion = defineNPC(
			SPEAKER_ID::ORION,
			"Orion",
			{ 1096., 1526. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::ORION
		);
		town.npcs.push_back(orion);

		NPCData leo = defineNPC(
			SPEAKER_ID::LEO,
			"Leo",
			{ 2064., 1520. },
			{ 96., 96. },
			TEXTURE_ASSET_ID::LEO
		);
		town.npcs.push_back(leo);

		NPCData apothecary = defineNPC(
			SPEAKER_ID::APOTHECARY,
			"Apothecary",
			{ 1900., 1910. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::APOTHECARY
		);
		town.npcs.push_back(apothecary);
		NPCData tutInn = defineNPC(
			SPEAKER_ID::TUTORIAL_INN_EXT,
			"",
			{ 1605., 1525. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		tutInn.interactText = "";
		tutInn.radius = { 64., 64. };
		NPCData innkeeper = defineNPC(
			SPEAKER_ID::INN_KEEPER,
			"Skye",
			{ 1507., 1520. },
			{ 96., 96. },
			TEXTURE_ASSET_ID::INN_KEEPER
		);
		innkeeper.radius = {48., 64.};
		town.npcs.push_back(tutInn);
		town.npcs.push_back(innkeeper);
		
		NPCData child = defineNPC(
			SPEAKER_ID::NO_SPEAKER,
			"",
			{ 3042., 1333. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::CHILD
		);
		child.interactText = "";
		NPCData mom = defineNPC(
			SPEAKER_ID::NO_SPEAKER,
			"",
			{ 3162., 1525. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::MOM
		);
		mom.interactText = "";
		NPCData family = defineNPC(
			SPEAKER_ID::TOWN_FAMILY,
			"",
			{ 3120., 1500. },
			{ 128., 128. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		family.radius = {128., 128.};
		family.interactText = "[V] Listen";
		town.npcs.push_back(child);
		town.npcs.push_back(mom);
		town.npcs.push_back(family);

		NPCData aristocrat_2 = defineNPC(
			SPEAKER_ID::ARISTOCRAT_2,
			"",
			{ 5248., 885. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::ARISTOCRAT_2
		);
		aristocrat_2.interactText = "[V] Listen";
		town.npcs.push_back(aristocrat_2);

		town.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";

	}

	void initPlaza() {
		plaza.startPos = { 96., 757. };
		plaza.levelFile = "plaza.txt";
		plaza.mapPos = { 7230., 7013. };


		Background farBack = defineBackground(
			vec2(1280., 480.),
			vec2(2700., 720.),
			TEXTURE_ASSET_ID::BG_PRIMARY_BACK,
			0.95f
		);
		Background closeBack = defineBackground(
			vec2(1280., 416.),
			vec2(3904, 1616),
			TEXTURE_ASSET_ID::BG_PLAZA_2,
			0.1f
		);
		Background foreground = defineBackground(
			vec2(1280., 416.),
			vec2(3904, 1616),
			TEXTURE_ASSET_ID::BG_PLAZA,
			0.0f
		);
		plaza.backgrounds.push_back(farBack);
		plaza.backgrounds.push_back(closeBack);
		plaza.backgrounds.push_back(foreground);

		TransitionData toTown = defineTransition(
			{ 24., 740. },
			{ 64., 256. },
			{ 5832., 885. },
			LEVEL_INDEX::TOWN,
			"",
			false
		);
		TransitionData toAristocrat = defineTransition(
			{ 2516., 740. },
			{ 64., 256. },
			{ 96., 2804. },
			LEVEL_INDEX::ARISTOCRAT,
			"Cross bridge",
			false
		);
		plaza.doors.push_back(toTown);
		plaza.doors.push_back(toAristocrat);

		NPCData gossiper_1 = defineNPC(
			SPEAKER_ID::NO_SPEAKER,
			"",
			{ 893., 757. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::TOWN_GOSSIPER_1
		);
		gossiper_1.interactText = "";

		NPCData gossiper_2 = defineNPC(
			SPEAKER_ID::NO_SPEAKER,
			"",
			{ 832., 757. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::TOWN_GOSSIPER_2
		);
		gossiper_2.interactText = "";

		NPCData gossiper_mid = defineNPC(
			SPEAKER_ID::GOSSIP_GIRLS,
			"",
			{ 864., 757. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		gossiper_mid.interactText = "[V] Listen";

		NPCData monument = defineInspectable(
			SPEAKER_ID::STARSEEKER_MONUMENT,
			"",
			{ 1320., 600. },
			{ 200., 400. },
			TEXTURE_ASSET_ID::STATUE
		);
		monument.interactText = "";
		NPCData monumentInspectable = defineInspectable(
			SPEAKER_ID::STARSEEKER_MONUMENT,
			"",
			{ 1320., 727. },
			{ 300., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		NPCData aristocrat_1 = defineNPC(
			SPEAKER_ID::ARISTOCRAT_1,
			"",
			{ 1659., 757. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::ARISTOCRAT_1
		);

		plaza.npcs.push_back(monument);
		plaza.npcs.push_back(gossiper_1);
		plaza.npcs.push_back(gossiper_2);
		plaza.npcs.push_back(gossiper_mid);
		plaza.npcs.push_back(monumentInspectable);
		plaza.npcs.push_back(aristocrat_1);

		plaza.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";
		plaza.second_background_track = "DANCE_OF_THE_PIXIES_-_CHASE.wav";
	}

	void initSewers() {
		sewers.startPos = { 3968., 0. };
		sewers.levelFile = "sewers.txt";
		sewers.mapPos = { 4527., 7842. };

		// define backgrounds
		Background foreground = defineBackground(
			vec2(3392. - 64., 512. - 26.),
			vec2(6784. - 64., 1024. - 8.),
			TEXTURE_ASSET_ID::BG_SEWERS,
			0.0f
		);
		Background background = defineBackground(
			vec2(3392. - 64. - 640, 512. - 26. - 360),
			vec2(6784. - 64. + 1280, 1024. - 8. + 720),
			TEXTURE_ASSET_ID::BG_SEWERS_1,
			1.0f
		);
		sewers.backgrounds.push_back(background);
		sewers.backgrounds.push_back(foreground);

		// define transitions
		TransitionData toForest_2 = defineTransition(
			{ 128., -64. },
			{ 256., 128. },
			{ 3580., 768. },
			LEVEL_INDEX::FOREST_2,
			"",
			false
		);

		sewers.doors.push_back(toForest_2);

		// define NPCs
		/*
		NPCData venus = defineNPC(
			SPEAKER_ID::VENUS,
			{ 512., 310. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::PLAYER
		);
		cabin.npcs.push_back(venus);
		*/

		// define music
		sewers.background_track = "sewers.wav";
		sewers.second_background_track = "sewers.wav";
	}
	
	void initUndercity() {

	}
	
	void initAristocrat() {
		aristocrat.startPos = { 96., 2804. };
		aristocrat.levelFile = "aristocrat.txt";
		aristocrat.mapPos = { 8434., 6622. };

		Background farBack = defineBackground(
			vec2(-2160., 1504.),
			vec2(4050., 1440.),
			TEXTURE_ASSET_ID::BG_PRIMARY_BACK_BLANK,
			0.98f
		);
		aristocrat.backgrounds.push_back(farBack);
		Background foreground = defineBackground(
			vec2(960., 1504.),
			vec2(3264, 3792),
			TEXTURE_ASSET_ID::BG_ARISTOCRAT,
			0.0f
		);
		aristocrat.backgrounds.push_back(foreground);

		plaza.startPos = { 96., 757. };

		TransitionData toPlaza = defineTransition(
			{ 24., 2780. },
			{ 64., 256. },
			{ 2440., 757. },
			LEVEL_INDEX::PLAZA,
			"",
			false
		);
		TransitionData toAristocratInt = defineTransition(
			{ 1440., 2048. },
			{ 64., 128. },
			{ 160., 2933. },
			LEVEL_INDEX::ARISTOCRAT_HOUSE,
			"Enter",
			true
		);
		toAristocratInt.visible = false;
		TransitionData toAristocratLowerWindow = defineTransition(
			{ 1762., 1536. },
			{ 128., 128. },
			{ 2500., 2229. },
			LEVEL_INDEX::ARISTOCRAT_HOUSE,
			"Climb in through window",
			true
		);
		toAristocratLowerWindow.visible = false;
		TransitionData toAristocratUpperWindow = defineTransition(
			{ 1126., 512. },
			{ 128., 128. },
			{ 160., 437. },
			LEVEL_INDEX::ARISTOCRAT_HOUSE,
			"Climb in through window",
			true
		);
		toAristocratUpperWindow.visible = false;
		toAristocratUpperWindow.requirements.push_back(QUEST_INDEX::UNLOCK_WINDOW);
		// toAristocratUpperWindow.requirements = //
		aristocrat.doors.push_back(toPlaza);
		aristocrat.doors.push_back(toAristocratInt);
		aristocrat.doors.push_back(toAristocratLowerWindow);
		aristocrat.doors.push_back(toAristocratUpperWindow);

		aristocrat.inn_sprite = TEXTURE_ASSET_ID::INN_BASE;
		aristocrat.inn_target_level = LEVEL_INDEX::INN_CITY;

		aristocrat.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";
		aristocrat.second_background_track = "DANCE_OF_THE_PIXIES_-_CHASE.wav";
	}
	
	void initAristocratHouse() {
		aristocrat_house.startPos = { 160., 2933. };
		aristocrat_house.levelFile = "aristocrat_int.txt";
		aristocrat_house.mapPos = { 8434., 6622. };

		Background foreground = defineBackground(
			vec2(1344., 1568.),
			vec2(4032, 3920),
			TEXTURE_ASSET_ID::BG_ARISTOCRAT_HOUSE,
			0.0f
		);
		aristocrat_house.backgrounds.push_back(foreground);

		TransitionData toAristocratExt = defineTransition(
			{ 160., 2944. },
			{ 64., 128. },
			{ 1440., 2037. },
			LEVEL_INDEX::ARISTOCRAT,
			"Enter",
			true
		);
		TransitionData toAristocratLowerWindow = defineTransition(
			{ 2500., 2240. },
			{ 128., 128. },
			{ 1762., 1525. },
			LEVEL_INDEX::ARISTOCRAT,
			"Climb out through window",
			true
		);
		toAristocratLowerWindow.visible = false;
		TransitionData toAristocratUpperWindow = defineTransition(
			{ 160., 448. },
			{ 128., 128. },
			{ 1126., 501. },
			LEVEL_INDEX::ARISTOCRAT,
			"Escape",
			true
		);
		toAristocratUpperWindow.visible = false;
		// toAristocratUpperWindow.requirements = //
		TransitionData toGreenway = defineTransition(
			{ 1688., 64. },
			{ 64., 128. },
			{ 3840., 2229. },
			LEVEL_INDEX::GREENWAY,
			"",
			false
		);
		toAristocratUpperWindow.requirements.push_back(QUEST_INDEX::UNLOCK_WINDOW);
		aristocrat_house.doors.push_back(toAristocratExt);
		aristocrat_house.doors.push_back(toAristocratLowerWindow);
		aristocrat_house.doors.push_back(toAristocratUpperWindow);
		aristocrat_house.doors.push_back(toGreenway);


		NPCData window = defineNPC(
			SPEAKER_ID::WINDOW,
			"",
			{ 160., 454. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		NPCData tutRock = defineNPC(
			SPEAKER_ID::TUTORIAL_ROCK,
			"",
			{ 688, 2933. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		window.interactText = "";
		tutRock.interactText = "";
		aristocrat_house.npcs.push_back(tutRock);
		aristocrat_house.npcs.push_back(window);

		aristocrat_house.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";
		aristocrat_house.second_background_track = "DANCE_OF_THE_PIXIES_-_CHASE.wav";
	}
	
	void initLeoShop() {

	}
	
	void initGreenway() {
		greenway.startPos = { 3840., 2100. };
		greenway.levelFile = "greenway.txt";
		greenway.mapPos = { 7630., 5166. };

		Background farBack = defineBackground(
			vec2(1000., 1300.),
			vec2(4050., 1480.),
			TEXTURE_ASSET_ID::BG_PRIMARY_BACK_BLANK,
			0.98f
		);
		greenway.backgrounds.push_back(farBack);
		Background foreground = defineBackground(
			vec2(1984., 1152.),
			vec2(5312, 3088),
			TEXTURE_ASSET_ID::BG_GREENWAY,
			0.0f
		);
		greenway.backgrounds.push_back(foreground);

		TransitionData toAristocratHouse = defineTransition(
			{ 3840., 2240. },
			{ 64., 128. },
			{ 1600., 64. },
			LEVEL_INDEX::ARISTOCRAT_HOUSE,
			"Enter",
			true
		);
		greenway.doors.push_back(toAristocratHouse);

		NPCData tutRock2 = defineNPC(
			SPEAKER_ID::TUTORIAL_ROCK_2,
			"",
			{ 3228., 2230. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		tutRock2.interactText = "";
		greenway.npcs.push_back(tutRock2);

		NPCData tutRock3 = defineNPC(
			SPEAKER_ID::TUTORIAL_ROCK_3,
			"",
			{ 1360., 2230. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		tutRock3.interactText = "";
		greenway.npcs.push_back(tutRock3);
		NPCData greenwayGuard = defineNPC(
			SPEAKER_ID::GREENWAY_GUARD,
			"",
			{ 3403., 2230. },
			{ 84., 84. },
			TEXTURE_ASSET_ID::SLEEPING_STARSEEKER_1
		);
		greenway.npcs.push_back(greenwayGuard);

		greenway.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";
		greenway.second_background_track = "DANCE_OF_THE_PIXIES_-_CHASE.wav";
	}
	
	void initSkylands() {
		skylands.startPos = { 3840., 2100. };
		skylands.levelFile = "skylands.txt";
		skylands.mapPos = { 3664., 3282. };

		Background farBack = defineBackground(
			vec2(0., 1300.),
			vec2(4050., 1480.),
			TEXTURE_ASSET_ID::BG_PRIMARY_BACK_BLANK,
			1.0f
		);
		skylands.backgrounds.push_back(farBack);
		// Background foreground = defineBackground(
		// 	vec2(1984., 1152.),
		// 	vec2(5312, 3088),
		// 	TEXTURE_ASSET_ID::BG_GREENWAY,
		// 	0.0f
		// );
		// skylands.backgrounds.push_back(foreground);

		skylands.background_track = "DANCE_OF_THE_PIXIES_-_IDLE.wav";
		skylands.second_background_track = "DANCE_OF_THE_PIXIES_-_CHASE.wav";
	}
	
	void initPalaceExt() {

	}
	
	void initPalaceInt1() {

	}
	
	void initPalaceInt2() {

	}
	
	void initThroneRoom() {

	}
	
	void initInnTown() {
		inn_town.startPos = { 640., 628. };
		inn_town.levelFile = "inn.txt";
		inn_town.mapPos = { 6000., 7359. };

		Background foreground = defineBackground(
			vec2(704., 384.),
			vec2(2496., 1424.),
			TEXTURE_ASSET_ID::BG_INN,
			0.0f
		);
		// cabin.backgrounds.push_back(farBack);
		inn_town.backgrounds.push_back(foreground);


		// define transitions
		TransitionData toTown = defineTransition(
			{ 640., 640. },
			{ 64., 128. },
			{ 1608., 1524. },
			LEVEL_INDEX::TOWN,
			"Exit to Town",
			true
		);

		inn_town.doors.push_back(toTown);

		// define NPCs
		NPCData mail = defineNPC(
			SPEAKER_ID::MAIL,
			"",
			{ 156., 576. },
			{ 128., 128. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		mail.radius = {128., 128.};
		mail.interactText = "[V] Send mail";
		inn_town.npcs.push_back(mail);

		NPCData tutInn = defineNPC(
			SPEAKER_ID::TUTORIAL_INN_INT,
			"",
			{ 800., 630. },
			{ 64., 64. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		tutInn.radius = {64., 64.};
		tutInn.interactText = "";
		inn_town.npcs.push_back(tutInn);

		// define music
		inn_town.background_track = "BEGINNINGS.wav";
		inn_town.second_background_track = "BEGINNINGS.wav";
	

	}
	
	void initInnUndercity() {
		inn_undercity.startPos = { 640., 628. };
		inn_undercity.levelFile = "inn.txt";
		inn_undercity.mapPos = { 5557., 8011. };

		Background foreground = defineBackground(
			vec2(704., 384.),
			vec2(2496., 1424.),
			TEXTURE_ASSET_ID::BG_INN,
			0.0f
		);
		// cabin.backgrounds.push_back(farBack);
		inn_undercity.backgrounds.push_back(foreground);


		// define transitions
		TransitionData toTown = defineTransition(
			{ 640., 640. },
			{ 64., 128. },
			{ 1608., 1524. },
			LEVEL_INDEX::TOWN,
			"Exit to Undercity",
			true
		);

		inn_undercity.doors.push_back(toTown);

		// define NPCs

		// define music
		inn_undercity.background_track = "BEGINNINGS.wav";
		inn_undercity.second_background_track = "BEGINNINGS.wav";

	}
	
	void initInnCity() {
		inn_city.startPos = { 640., 628. };
		inn_city.levelFile = "inn.txt";
		inn_city.mapPos = { 8456., 7111. };

		Background foreground = defineBackground(
			vec2(704., 384.),
			vec2(2496., 1424.),
			TEXTURE_ASSET_ID::BG_INN,
			0.0f
		);
		// cabin.backgrounds.push_back(farBack);
		inn_city.backgrounds.push_back(foreground);


		// define transitions
		TransitionData toTown = defineTransition(
			{ 640., 640. },
			{ 64., 128. },
			{ 1152, 2805. },
			LEVEL_INDEX::ARISTOCRAT,
			"Exit to Aristocrat's Quarters",
			true
		);

		inn_city.doors.push_back(toTown);

		// define NPCs
		NPCData mail = defineNPC(
			SPEAKER_ID::MAIL,
			"",
			{ 156., 576. },
			{ 128., 128. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		mail.interactText = "[V] Send mail";
		inn_city.npcs.push_back(mail);

		// define music
		inn_city.background_track = "BEGINNINGS.wav";
		inn_city.second_background_track = "BEGINNINGS.wav";

	}
	
	void initInnSkylands() {
		inn_skylands.startPos = { 640., 628. };
		inn_skylands.levelFile = "inn.txt";
		inn_skylands.mapPos = { 5557., 8011. };

		Background foreground = defineBackground(
			vec2(704., 384.),
			vec2(2496., 1424.),
			TEXTURE_ASSET_ID::BG_INN,
			0.0f
		);
		// cabin.backgrounds.push_back(farBack);
		inn_skylands.backgrounds.push_back(foreground);


		// define transitions
		TransitionData toTown = defineTransition(
			{ 640., 640. },
			{ 64., 128. },
			{ 1608., 1524. },
			LEVEL_INDEX::TOWN,
			"Exit to Skylands",
			true
		);

		inn_skylands.doors.push_back(toTown);

		// define NPCs
		NPCData mail = defineNPC(
			SPEAKER_ID::MAIL,
			"",
			{ 156., 576. },
			{ 128., 128. },
			TEXTURE_ASSET_ID::NOSPRITE
		);
		mail.interactText = "[V] Send mail";
		inn_skylands.npcs.push_back(mail);

		// define music
		inn_skylands.background_track = "BEGINNINGS.wav";
		inn_skylands.second_background_track = "BEGINNINGS.wav";

	}

	/*
	TEMPLATE void initNEWROOM() {
		ROOM.startPos = { x., y. };
		ROOM.levelFile = "file.txt";

		// define backgrounds
		Background BACKGROUND = defineBackground(
			POSITION,
			SCALE,
			ASSET,
			PARALLAX_DISTANCE
		);

		ROOM.backgrounds.push_back(BACKGROUND);

		// define transitions
		TransitionData TRANSITION = defineTransition(
			CREATE_POSITION,
			SCALE,
			TARGET_POSITION,
			TARGET_LEVEL,
			NEEDS_INTERACT
		);

		ROOM.doors.push_back(TRANSITION);

		// set up inns
		ROOM.inn_sprite = TEXTURE_ASSET_ID::
		ROOM.inn_target_level = LEVEL_INDEX::

		// define NPCs
		NPCData NPC = defineNPC(
			SPEAKER_ID,
			CREATE_POSITION,
			SCALE,
			SPRITE
		);

		ROOM.npcs.push_back(NPC);

		ROOM.background_track = "TRACK_FILE.wav";
	}
	*/

	void initLevels() {
		initCabin();
		initForest_1();
		initForest_2();
		initCanopy();
		initBridge();
		initTown();
		initPlaza();
		initSewers();
		initUndercity();

		initAristocrat();
		initAristocratHouse();
		initLeoShop();
		initGreenway();
		initSkylands();
		initPalaceExt();
		initPalaceInt1();
		initPalaceInt2();
		initThroneRoom();
		
		initInnTown();
		initInnUndercity();
		initInnCity();
		initInnSkylands();
	}

public:
	std::vector<Level*> level_list;

	Level forest_1 = Level();
	Level cabin = Level();
	Level canopy = Level();
	Level forest_2 = Level();
	Level bridge = Level();
	Level town = Level();
	Level plaza = Level();
	Level sewers = Level();
	Level undercity = Level();
	Level aristocrat = Level();
	Level aristocrat_house = Level();
	Level leo_shop = Level();
	Level greenway = Level();
	Level skylands = Level();
	Level palace_ext = Level();
	Level palace_int_1 = Level();
	Level palace_int_2 = Level();
	Level throne_room = Level();
	Level inn_town = Level();
	Level inn_undercity = Level();
	Level inn_city = Level();
	Level inn_skylands = Level();

	LevelRegistry() {
		level_list.push_back(&cabin);
		level_list.push_back(&forest_1);
		level_list.push_back(&forest_2);
		level_list.push_back(&canopy);
		level_list.push_back(&bridge);
		level_list.push_back(&town);
		level_list.push_back(&plaza);
		level_list.push_back(&sewers);

		level_list.push_back(&undercity);
		level_list.push_back(&aristocrat);
		level_list.push_back(&aristocrat_house);
		level_list.push_back(&leo_shop);
		level_list.push_back(&greenway);
		level_list.push_back(&skylands);
		level_list.push_back(&palace_ext);
		level_list.push_back(&palace_int_1);
		level_list.push_back(&palace_int_2);
		level_list.push_back(&throne_room);

		level_list.push_back(&inn_town);
		level_list.push_back(&inn_undercity);
		level_list.push_back(&inn_city);
		level_list.push_back(&inn_skylands);

		initLevels();
	}

};
/***********************************************/

// QUEST REGISTRY
class QuestRegistry {
private:

	void initTutorials() {
		// Quest tutorialMove, tutorialJump, tutorialAttack, tutorialMap, tutorialRock, tutorialStealth;
		tutorialMove = Quest();
		tutorialJump = Quest();
		tutorialDrop = Quest();

		tutorialAttack = Quest();
		tutorialMap = Quest();

		tutorialRock = Quest();
		tutorialRock_2 = Quest();
		tutorialRock_3 = Quest();

		tutorialInn = Quest();

		tutorialStealth = Quest();

		tutorialMove.stage = QUEST_STAGE::END;
		tutorialJump.stage = QUEST_STAGE::END;
		tutorialDrop.stage = QUEST_STAGE::END;

		tutorialAttack.stage = QUEST_STAGE::END;
		tutorialMap.stage = QUEST_STAGE::END;

		tutorialRock.stage = QUEST_STAGE::END;
		tutorialRock_2.stage = QUEST_STAGE::END;
		tutorialRock_3.stage = QUEST_STAGE::END;

		tutorialInn.requirements = {
			{ITEM_CONDITION_ID::SPAWN_SET, 1}
		};

		tutorialStealth.stage = QUEST_STAGE::END;

		tutorialMove.name = "tutorial";
		tutorialMove.visible = false;
		tutorialJump.name = "tutorial";
		tutorialJump.visible = false;
		tutorialDrop.name = "tutorial";
		tutorialDrop.visible = false;

		tutorialAttack.name = "tutorial";
		tutorialAttack.visible = false;
		tutorialMap.name = "tutorial";
		tutorialMap.visible = false;

		tutorialRock.name = "tutorial";
		tutorialRock.visible = false;
		tutorialRock_2.name = "tutorial";
		tutorialRock_2.visible = false;
		tutorialRock_3.name = "tutorial";
		tutorialRock_3.visible = false;

		tutorialInn.name = "";
		tutorialInn.visible = false;

		tutorialStealth.name = "tutorial";
		tutorialStealth.visible = false;
	}

	void initGatherFruits() {
		//
		gatherFruits = Quest();
		gatherFruits.requirements = {
			{ITEM_CONDITION_ID::FRUIT, 3}
		};
		gatherFruits.rewards = {
			{ITEM_CONDITION_ID::MAP, 1}
		};
		gatherFruits.childQuests.push_back((int)QUEST_INDEX::FIND_BROOCH);
		gatherFruits.name = "Treetop Scavenger";
		gatherFruits.description = "Find three fruits and berries in the forest and bring them back to Venus in your cabin.";
		gatherFruits.visible = true;
	}

	void initFixBridge() {
		//
		fixBridge = Quest();
		fixBridge.requirements = {
			{ITEM_CONDITION_ID::ROPE, 1}
		};
		fixBridge.name = "Fix the bridge";
		fixBridge.visible = false;
	}

	void initFindBrooch() {
		findBrooch = Quest();
		findBrooch.requirements = {
			{ITEM_CONDITION_ID::BROOCH, 1}
		};
		findBrooch.childQuests.push_back((int)QUEST_INDEX::COINS_FROM_ORION);
		findBrooch.childQuests.push_back((int)QUEST_INDEX::BUY_MEDICINE);
		findBrooch.childQuests.push_back((int)QUEST_INDEX::STEAL_MONEY);
		findBrooch.childQuests.push_back((int)QUEST_INDEX::TUTORIAL_MAP);
		findBrooch.childQuests.push_back((int)QUEST_INDEX::TUTORIAL_INN);
		findBrooch.name = "Nested and Nestled";
		findBrooch.description = "Bring back Venus's brooch from the birds in the treetop canopies.";
		findBrooch.visible = true;
	}

	void initCoinsFromOrion() {
		getCoinsFromOrion = Quest();
		getCoinsFromOrion.stage = QUEST_STAGE::END;
		getCoinsFromOrion.rewards = {
			{ITEM_CONDITION_ID::COIN, 1}
		};
		getCoinsFromOrion.childQuests.push_back((int)QUEST_INDEX::SCALE_TO_ORION);
		getCoinsFromOrion.childQuests.push_back((int)QUEST_INDEX::STEAL_MONEY);
		getCoinsFromOrion.name = "Talk to Orion";
		getCoinsFromOrion.visible = false;
	}

	void initBuyMedicine() {
		buyMedicine = Quest();
		buyMedicine.requirements = {
			{ITEM_CONDITION_ID::MEDICINE, 1}
		};
		buyMedicine.rewards = {
			{ITEM_CONDITION_ID::GADGET, 1}
		};
		buyMedicine.childQuests.push_back((int)QUEST_INDEX::GIVE_GADGET);
		buyMedicine.name = "Apothecariate";
		buyMedicine.description = "Find the Apothecary in town and buy some medicine for Venus.";
		buyMedicine.visible = true;
	}

	void initStealMoney() {
		stealMoney = Quest();
		stealMoney.requirements = {
			{ITEM_CONDITION_ID::COIN, 2}
		};
		stealMoney.rewards = {
			{ITEM_CONDITION_ID::MEDICINE, 1}
		};
		stealMoney.name = "Tipping the Scales";
		stealMoney.description = "Find a way to get or steal 50 scales to buy medicine."; // remove the part about stealing later on once we add the thief npc
		stealMoney.visible = true;
	}

	void initScaleForResearcher() {
		scaleForResearcher = Quest();
		scaleForResearcher.requirements = {
			{ITEM_CONDITION_ID::SCALE, 1}
		};
		scaleForResearcher.rewards = {
			{ITEM_CONDITION_ID::RING, 1}
		};
		scaleForResearcher.questsToClose.push_back(QUEST_INDEX::SCALE_TO_ORION);
		scaleForResearcher.childQuests.push_back((int)QUEST_INDEX::TUTORIAL_ROCK);
		scaleForResearcher.childQuests.push_back((int)QUEST_INDEX::TUTORIAL_ROCK_2);
		scaleForResearcher.childQuests.push_back((int)QUEST_INDEX::TUTORIAL_ROCK_3);
		scaleForResearcher.childQuests.push_back((int)QUEST_INDEX::GET_BROOCH);
		scaleForResearcher.name = "The Researcher's Prize";
		scaleForResearcher.description = "Find the glittering blue scale in the sewers and bring it back to Leo for his research.";
		scaleForResearcher.visible = true;
	}

	void initNoScaleLeo() {
		noScaleLeo = Quest();

		noScaleLeo.stage = QUEST_STAGE::END;

		noScaleLeo.rewards = {
			{ITEM_CONDITION_ID::RING, 1}
		};
		noScaleLeo.childQuests.push_back((int)QUEST_INDEX::TUTORIAL_ROCK);
		noScaleLeo.childQuests.push_back((int)QUEST_INDEX::TUTORIAL_ROCK_2);
		noScaleLeo.childQuests.push_back((int)QUEST_INDEX::TUTORIAL_ROCK_3);
		noScaleLeo.childQuests.push_back((int)QUEST_INDEX::GET_BROOCH);
		noScaleLeo.name = "The Researcher's Prize";
		noScaleLeo.description = "Visit Leo in the town and inform him of the bad news.";
		noScaleLeo.visible = true;
	}

	void initScaleToOrion() {
		scaleToOrion = Quest();
		scaleToOrion.requirements = {
			{ITEM_CONDITION_ID::SCALE, 1}
		};
		scaleToOrion.childQuests.push_back((int) QUEST_INDEX::NO_SCALE_LEO);
		scaleToOrion.questsToClose.push_back(QUEST_INDEX::SCALE_FOR_LEO);
		scaleToOrion.name = "For Me?";
		scaleToOrion.description = "Find something shiny as a gift for Orion's daughter.";
		scaleToOrion.visible = true;

	}

	void initGiveGadget() {
		giveGadget = Quest();
		giveGadget.requirements = {
			{ITEM_CONDITION_ID::GADGET, 1}
		};
		giveGadget.childQuests.push_back((int)QUEST_INDEX::SCALE_FOR_LEO);
		giveGadget.name = "The One Ring";
		giveGadget.description = "Find someone crafty in town to fix up your ring";
		giveGadget.visible = true;
	}

	void initGetBrooch() {
		getBrooch = Quest();
		getBrooch.rewards = {
			{ITEM_CONDITION_ID::COIN, 5},
			{ITEM_CONDITION_ID::POLISHED_BROOCH, 5}
		};
		getBrooch.childQuests.push_back((int)QUEST_INDEX::GO_TO_SKYLANDS);
		getBrooch.childQuests.push_back((int)QUEST_INDEX::TRAVERSE_GREENWAY);
		getBrooch.name = "Unexpected Package";
		getBrooch.description = "Check your mail at an inn.";
		getBrooch.visible = true;
	}

	void initGoSkylands() {
		// just put a speaker in the skylands to end the quest, don't have requirements
		goSkylands = Quest();
		goSkylands.childQuests.push_back((int) QUEST_INDEX::COLOSSEUM);
		goSkylands.name = "Ascend";
		goSkylands.description = "Orion is urging you to seek an audience with the king. You don't know what for, but it seems important. Make your way through the greenway to the skylands.";
		goSkylands.visible = true;
	}

	void initTraverseGreenway() {
		traverseGreenway = Quest();
		traverseGreenway.name = "The Guarded Greenways?";
		traverseGreenway.description = "It looks like Orion neglected to warn the guards of your arrival. Find a way over the gate and make your way to the skyship landing.";
		traverseGreenway.visible = true;
	}

	void initColosseum() {
		colosseum = Quest();
		colosseum.name = "Metal on Mettle";
		colosseum.childQuests.push_back((int)QUEST_INDEX::ASTRONOMER_END);
		colosseum.description = "The Starseeker captain doesn't think very highly of you and refuses to let you proceed to the castle. Defeat the enemies in the Colosseum to change her mind.";
		colosseum.visible = true;
	}

	void initAstronomer() {
		// astronomerend and astronomerenchant
		astronomer_end = Quest();
		astronomer_end.childQuests.push_back((int)QUEST_INDEX::ASTRONOMER_ENCHANT);
		astronomer_end.stage = QUEST_STAGE::END;
		astronomer_end.visible = false;

		astronomer_enchant = Quest();
		astronomer_enchant.requirements = {
			{ITEM_CONDITION_ID::TALONS, 7}
		};
		astronomer_enchant.name = "Wings and Talons";
		astronomer_enchant.description = "Collect 7 talons for the astronomer to re-enchant the skyship.";
		astronomer_enchant.visible = true;
	}

	void initKingdom() {
		kingdom = Quest();
	}

	void initUnlockWindow() {
		unlockWindow = Quest();
		unlockWindow.stage = QUEST_STAGE::END;
		unlockWindow.visible = false;
	}

	void init() {
		initTutorials();

		initGatherFruits();
		initFixBridge();
		initFindBrooch();

		initCoinsFromOrion();
		initBuyMedicine();
		initStealMoney();

		initScaleForResearcher();
		initScaleToOrion();
		initNoScaleLeo();
		initGiveGadget();

		initGetBrooch();
		initGoSkylands();
		initTraverseGreenway();
		initColosseum();
		initAstronomer();
		initKingdom();

		initUnlockWindow();
	}
public:
	std::vector<Quest*> quest_list;

	Quest tutorialMove, 
		tutorialJump, 
		tutorialDrop,
		tutorialAttack, 
		tutorialMap, 
		tutorialRock, tutorialRock_2, tutorialRock_3,
		tutorialInn,
		tutorialStealth; 
	Quest gatherFruits;
	Quest fixBridge;
	Quest findBrooch;

	Quest getCoinsFromOrion;
	Quest buyMedicine;
	Quest stealMoney;
	Quest scaleForResearcher;
	Quest scaleToOrion;
	Quest noScaleLeo;
	Quest giveGadget;

	Quest getBrooch;
	Quest goSkylands;
	Quest traverseGreenway;
	Quest colosseum;
	Quest astronomer_end;
	Quest astronomer_enchant;

	Quest kingdom;

	Quest unlockWindow;

	QuestRegistry() {
		quest_list.push_back(&tutorialMove);
		quest_list.push_back(&tutorialJump);
		quest_list.push_back(&tutorialDrop);
		quest_list.push_back(&tutorialAttack);
		quest_list.push_back(&tutorialMap);

		quest_list.push_back(&tutorialRock);
		quest_list.push_back(&tutorialRock_2);
		quest_list.push_back(&tutorialRock_3);
		quest_list.push_back(&tutorialInn);
		
		quest_list.push_back(&tutorialStealth);

		quest_list.push_back(&gatherFruits);
		quest_list.push_back(&fixBridge);
		quest_list.push_back(&findBrooch);

		quest_list.push_back(&getCoinsFromOrion);
		quest_list.push_back(&buyMedicine);
		quest_list.push_back(&stealMoney);

		quest_list.push_back(&scaleForResearcher);
		quest_list.push_back(&scaleToOrion);
		quest_list.push_back(&noScaleLeo);
		quest_list.push_back(&giveGadget);
		
		quest_list.push_back(&getBrooch);
		quest_list.push_back(&goSkylands);
		quest_list.push_back(&traverseGreenway);
		quest_list.push_back(&colosseum);
		quest_list.push_back(&astronomer_end);
		quest_list.push_back(&astronomer_enchant);

		quest_list.push_back(&kingdom);

		quest_list.push_back(&unlockWindow);

		init();
	}
};
extern QuestRegistry questRegistry;

class DialogueRegistry {
private:
	void init() {
	/*********** TUTORIALS ***********/
		DialogueSequence tutorialWalk = DialogueSequence(), 
			tutorialJump = DialogueSequence(),
			tutorialAttack = DialogueSequence(),
			tutorialMap = DialogueSequence(),
			tutorialRock = DialogueSequence(),
			tutorialRock2 = DialogueSequence(),
			tutorialRock3 = DialogueSequence(),
			tutorialInnExt = DialogueSequence(),
			tutorialInnInt = DialogueSequence(),
			tutorialStealth = DialogueSequence();

		DialogueStage& tWalkEnd = tutorialWalk.sequence[QUEST_STAGE::END];
		tWalkEnd.autoTrigger = true;
		DialogueLine tWalk_P1, tWalk_P2;
		tWalk_P1.text = "Use arrow keys [<][>] to walk.";
		tWalk_P2.text = "Press [v] to drop down one-way platforms.";
		tWalkEnd.primary.push_back(tWalk_P1);
		tWalkEnd.primary.push_back(tWalk_P2);

		DialogueStage& tJumpEnd = tutorialJump.sequence[QUEST_STAGE::END];
		tJumpEnd.autoTrigger = true;
		DialogueLine tJump_P1;
		tJump_P1.text = "Press [SPACE] to jump up.";
		tJumpEnd.primary.push_back(tJump_P1);

		DialogueStage& tAtkEnd = tutorialAttack.sequence[QUEST_STAGE::END];
		tAtkEnd.autoTrigger = true;
		DialogueLine tAtkEnd_P1, tAtkEnd_P2;
		tAtkEnd_P1.text = "Press [Z] to swing your sword."; 
		tAtkEnd_P2.text = "Press [^] + [Z] to attack upwards.";
		tAtkEnd.primary.push_back(tAtkEnd_P1);
		tAtkEnd.primary.push_back(tAtkEnd_P2);

		DialogueStage& tMapEnd = tutorialMap.sequence[QUEST_STAGE::END];
		tMapEnd.autoTrigger = true;
		DialogueLine tMapEnd_P1;
		tMapEnd_P1.text = "Press [M] to open your map.";
		tMapEnd.primary.push_back(tMapEnd_P1);

		DialogueStage& tStlthEnd = tutorialStealth.sequence[QUEST_STAGE::END];
		tStlthEnd.autoTrigger = true;
		DialogueLine tStlthEnd_P1;
		tStlthEnd_P1.text = "Press [V] over any hiding spot to hide from enemies.";
		tStlthEnd.primary.push_back(tStlthEnd_P1);

		DialogueStage& tRockEnd = tutorialRock.sequence[QUEST_STAGE::END];
		tRockEnd.autoTrigger = true;
		DialogueLine tRockEnd_P1, tRockEnd_P2, tRockEnd_P3;
		tRockEnd_P1.text = "Press [X] to bend a rock.";
		tRockEnd_P2.text = "Press [Z] to throw the rock at an enemy. Run side to side to aim your throw.";
		tRockEnd.primary.push_back(tRockEnd_P1);
		tRockEnd.primary.push_back(tRockEnd_P2);

		DialogueStage& tRockEnd2 = tutorialRock2.sequence[QUEST_STAGE::END];
		tRockEnd2.autoTrigger = true;
		DialogueLine tRockEnd2_P1, tRockEnd2_P2;
		tRockEnd2_P1.text = "While bending a rock, you can recast [X] to solidify the rock into a platform you can jump onto.";
		tRockEnd2_P2.text = "Try using it to jump over the gate.";
		tRockEnd2.primary.push_back(tRockEnd2_P1);
		tRockEnd2.primary.push_back(tRockEnd2_P2);

		DialogueStage& tRockEnd3 = tutorialRock3.sequence[QUEST_STAGE::END];
		tRockEnd3.autoTrigger = true;
		DialogueLine tRockEnd3_P1, tRockEnd3_P2;
		tRockEnd3_P1.text = "While bending a rock, you can press [v] to push the rock in front of you.";
		tRockEnd3_P2.text = "Recast [X] to solidify the rock into a platform you can jump onto.";
		tRockEnd3.primary.push_back(tRockEnd3_P1);
		tRockEnd3.primary.push_back(tRockEnd3_P2);

		DialogueStage& tInnStart = tutorialInnExt.sequence[QUEST_STAGE::START];
		tInnStart.autoTrigger = true;
		DialogueLine tInnStart_P1, tInnStart_P2;
		tInnStart_P1.text = "Inns are checkpoints in the world";
		tInnStart_P2.text = "Inside, you can send mail, which you can use to finish quests, or you can take a long rest, which regens your health and sets your spawn.";
		tInnStart.primary.push_back(tInnStart_P1);
		tInnStart.primary.push_back(tInnStart_P2);

		DialogueStage& tInnDuring = tutorialInnInt.sequence[QUEST_STAGE::DURING];
		tInnDuring.autoTrigger = true;
		DialogueLine tInnDuring_P1;
		tInnDuring_P1.text = "Check in at the receptionist to the right to take a rest.";
		tInnDuring.primary.push_back(tInnDuring_P1);

		DialogueStage& tInnEnd = tutorialInnInt.sequence[QUEST_STAGE::END];
		tInnEnd.autoTrigger = true;
		DialogueLine tInnEnd_P1, tInnEnd_P2;
		tInnEnd_P1.text = "If you have ongoing quests, you can finish quests by mailing items to people around Pureia.";
		tInnEnd_P2.text = "Visit the mail counter to the left to give it a try.";
		tInnEnd.primary.push_back(tInnEnd_P1);
		tInnEnd.primary.push_back(tInnEnd_P2);
		
	/*********** END OF TUTORIALS ***********/
	
	/*********** MAILBOX ***********/
		/****************************/

		DialogueSequence mailMedicineSeq = DialogueSequence();

		/****************************/

		DialogueStage& mailMedicineEnd = mailMedicineSeq.sequence[QUEST_STAGE::END];
		mailMedicineEnd.conditional = true;
		mailMedicineEnd.yesOption = "Send";
		mailMedicineEnd.noOption = "Leave";

		DialogueLine mailMedicineEnd_P1, mailMedicineEnd_Y1, mailMedicineEnd_N1;
		mailMedicineEnd_P1.text = "Send mail?";
		mailMedicineEnd_Y1.text = "You mail the medicine home.";
		mailMedicineEnd_N1.text = "Come again next time!";

		mailMedicineEnd_P1.line_speaker = LINE_SPEAKER::WORLD;
		mailMedicineEnd_Y1.line_speaker = LINE_SPEAKER::WORLD;
		mailMedicineEnd_N1.line_speaker = LINE_SPEAKER::WORLD;

		mailMedicineEnd.primary.push_back(mailMedicineEnd_P1);
		mailMedicineEnd.yesBranch.push_back(mailMedicineEnd_Y1);
		mailMedicineEnd.noBranch.push_back(mailMedicineEnd_N1);

		/****************************/

		DialogueSequence mailGadgetSeq = DialogueSequence();

		/****************************/

		DialogueStage& mailGadgetStart = mailGadgetSeq.sequence[QUEST_STAGE::START];
		mailGadgetStart.autoTrigger = true;

		DialogueLine mailGadgetStart_P1, mailGadgetStart_P2, mailGadgetStart_P3, mailGadgetStart_P4, mailGadgetStart_P5, mailGadgetStart_P6, mailGadgetStart_P7;
		mailGadgetStart_P1.text = "(Hmm, looks like I received a letter.)";
		mailGadgetStart_P1.line_speaker = LINE_SPEAKER::PLAYER;
		mailGadgetStart_P2.text = "You open the letter and a ring falls out.";
		mailGadgetStart_P2.line_speaker = LINE_SPEAKER::WORLD;
		mailGadgetStart_P3.text = "'I didn't think we'd run out of medicine so soon. Thank you'";
		mailGadgetStart_P3.line_speaker = LINE_SPEAKER::WORLD;
		mailGadgetStart_P4.text = "'Take a look at this, I found this ring while rummaging through the cabinets. I believe it used to be yours, so I wanted to return it to you.'";
		mailGadgetStart_P4.line_speaker = LINE_SPEAKER::WORLD;
		mailGadgetStart_P5.text = "'It doesn't seem to be in the best shape, but maybe you'll be able to find someone in town who knows how to fix it.'";
		mailGadgetStart_P5.line_speaker = LINE_SPEAKER::WORLD;
		mailGadgetStart_P6.text = "'I'll be waiting at home as usual, but please don't feel pressured to hurry back.'";
		mailGadgetStart_P6.line_speaker = LINE_SPEAKER::WORLD;
		mailGadgetStart_P7.text = "'- Venus'";
		mailGadgetStart_P7.line_speaker = LINE_SPEAKER::WORLD;

		mailGadgetStart.primary.push_back(mailGadgetStart_P1);
		mailGadgetStart.primary.push_back(mailGadgetStart_P2);
		mailGadgetStart.primary.push_back(mailGadgetStart_P3);
		mailGadgetStart.primary.push_back(mailGadgetStart_P4);
		mailGadgetStart.primary.push_back(mailGadgetStart_P5);
		mailGadgetStart.primary.push_back(mailGadgetStart_P6);
		mailGadgetStart.primary.push_back(mailGadgetStart_P7);

		/****************************/

		DialogueSequence mailBroochSeq = DialogueSequence();

		/****************************/

		DialogueStage& mailBroochEnd = mailBroochSeq.sequence[QUEST_STAGE::END];

		DialogueLine mailBroochEnd_P1, mailBroochEnd_P2, mailBroochEnd_P3, mailBroochEnd_P4;
		mailBroochEnd_P1.text = "It's a letter from Venus.";
		mailBroochEnd_P2.text = "'Hey dear, are you still in town today? You know, I was cleaning my brooch earlier and thought that I should give it to you to wear instead.'";
		mailBroochEnd_P3.text = "'I never really get the chance to go outside while you're always out and about, so I've also included a few scales in the parcel for you to spend.'";
		mailBroochEnd_P4.text = "'Have fun and come home safe. - Venus'";

		mailBroochEnd_P1.line_speaker = LINE_SPEAKER::WORLD;
		mailBroochEnd_P2.line_speaker = LINE_SPEAKER::WORLD;
		mailBroochEnd_P3.line_speaker = LINE_SPEAKER::WORLD;
		mailBroochEnd_P4.line_speaker = LINE_SPEAKER::WORLD;

		mailBroochEnd.primary.push_back(mailBroochEnd_P1);
		mailBroochEnd.primary.push_back(mailBroochEnd_P2);
		mailBroochEnd.primary.push_back(mailBroochEnd_P3);
		mailBroochEnd.primary.push_back(mailBroochEnd_P4);
		
	/*********** END OF MAILBOX ***********/

	/*********** VENUS ***********/
		// DEFINE DIALOGUE LINES
		DialogueSequence berriesSeq = DialogueSequence();

		/****************************/

		DialogueStage& berriesStart = berriesSeq.sequence[QUEST_STAGE::START];
		berriesStart.autoTrigger = true;

		DialogueLine berriesStart_P1, berriesStart_World, berriesStart_P2, berriesStart_P3;
		berriesStart_P1.text = "Rise and shine. I've prepared bread and fruit on the table, if you'd like to join me for breakfast.";
		// world dialogue after inspecting table: "You grab an apple and a loaf of bread off the table and start eating."
		berriesStart_World.text = "(You grab an apple and a loaf of bread off the table and start eating.)";
		berriesStart_World.line_speaker = LINE_SPEAKER::WORLD;
		berriesStart_P2.text = "We're running low on supplies again... Could you please help me gather some more fruit?";
		berriesStart_P3.text = "I would love to go outside with you and enjoy the sun, but I'm feeling a bit under the weather again today... Sorry.";

		berriesStart.primary.push_back(berriesStart_P1);
		berriesStart.primary.push_back(berriesStart_World);
		berriesStart.primary.push_back(berriesStart_P2);
		berriesStart.primary.push_back(berriesStart_P3);

		/****************************/

		DialogueStage& berriesDuring = berriesSeq.sequence[QUEST_STAGE::DURING];

		DialogueLine berriesDuring_P1;
		berriesDuring_P1.text = "All we need are a few apples and berries. I can take care of the baking for you here.";

		berriesDuring.primary.push_back(berriesDuring_P1);

		/****************************/

		DialogueStage& berriesEnd = berriesSeq.sequence[QUEST_STAGE::END];

		DialogueLine berriesEnd_P1;
		berriesEnd_P1.text = "Oh, thank you. We can make do for the next couple of days with this much.";

		berriesEnd.primary.push_back(berriesEnd_P1);

		/****************************/

		DialogueSequence broochSeq = DialogueSequence();

		/****************************/

		DialogueStage& broochStart = broochSeq.sequence[QUEST_STAGE::START];
		broochStart.autoTrigger = true;

		DialogueLine broochStart_P1, broochStart_P2, broochStart_P3, broochStart_P4, broochStart_P5;
		broochStart_P1.text = "Oh, before you leave!";
		broochStart_P2.text = "My brooch, I left it by the window for a moment, and a bird flew away with it.";
		broochStart_P3.text = "It's a bit hefty for a bird of that size, so I can't imagine it got very far. Please let me know if you see it out there.";
		broochStart_P4.text = "Here's a map so you don't get lost wandering around in the forest.";
		broochStart_P5.text = "Press [M] to pull up a world map.";
		broochStart_P5.line_speaker = LINE_SPEAKER::WORLD;

		broochStart.primary.push_back(broochStart_P1);
		broochStart.primary.push_back(broochStart_P2);
		broochStart.primary.push_back(broochStart_P3);
		broochStart.primary.push_back(broochStart_P4);
		broochStart.primary.push_back(broochStart_P5);

		/****************************/
		
		DialogueStage& broochDuring = broochSeq.sequence[QUEST_STAGE::DURING];
		
		DialogueLine broochDuring_P1;
		broochDuring_P1.text = "I hope it's still nearby... Hm? Oh, I mean my brooch.";

		broochDuring.primary.push_back(broochDuring_P1);

		/****************************/

		DialogueStage& broochEnd = broochSeq.sequence[QUEST_STAGE::END];

		DialogueLine broochEnd_P1, broochEnd_P2, broochEnd_P3;
		broochEnd_P1.text = "Ah, you found it! You're always so reliable. I can't imagine what I'd do without you.";
		broochEnd_P2.text = "It's all dusty... I suppose I'll have to clean and polish it later.";
		// world dialogue: "Venus coughs lightly."
		broochEnd_P3.text = "Ahaha, it looks like that was a bit too much excitement for the day.";

		broochEnd.primary.push_back(broochEnd_P1);
		broochEnd.primary.push_back(broochEnd_P2);
		broochEnd.primary.push_back(broochEnd_P3);

		/****************************/

		DialogueSequence medicineSeq = DialogueSequence();

		/****************************/

		DialogueStage& medicineStart = medicineSeq.sequence[QUEST_STAGE::START];
		medicineStart.autoTrigger = true;

		DialogueLine medicineStart_P1;
		medicineStart_P1.line_speaker = LINE_SPEAKER::PLAYER;
		medicineStart_P1.text = "(Her condition seems to be worsening again. I'll have to go into town and buy more medicine)";

		medicineStart.primary.push_back(medicineStart_P1);


		/****************************/

		DialogueStage& medicineDuring = medicineSeq.sequence[QUEST_STAGE::DURING];

		DialogueLine medicineDuring_P1;
		medicineDuring_P1.text = "I'll be resting for a bit, but let me know if you need anything.";

		medicineDuring.primary.push_back(medicineDuring_P1);

		/****************************/

		DialogueStage& medicineEnd = medicineSeq.sequence[QUEST_STAGE::END];

		DialogueLine medicineEnd_P1, medicineEnd_P2;
		medicineEnd_P1.text = "Is that for me? I didn't think we'd run out so soon...";
		medicineEnd_P2.text = "Thank you.";

		medicineEnd.primary.push_back(medicineEnd_P1);
		medicineEnd.primary.push_back(medicineEnd_P2);

		/****************************/

		DialogueSequence gadgetSeq = DialogueSequence();

		/****************************/

		DialogueStage& gadgetStart = gadgetSeq.sequence[QUEST_STAGE::START];
		gadgetStart.autoTrigger = true;

		DialogueLine gadgetStart_P1, gadgetStart_P2;
		gadgetStart_P1.text = "Take a look at this, I found this ring while rummaging through the cabinets. I believe it used to be yours, so I wanted to give it to you.";
		gadgetStart_P2.text = "It doesn't seem to be in the best shape, but maybe you'll be able to find someone in town who knows how to fix it.";

		gadgetStart.primary.push_back(gadgetStart_P1);
		gadgetStart.primary.push_back(gadgetStart_P2);




	/*********** END VENUS ***********/

	/*********** LEO ***********/
		// DEFINE DIALOGUE LINES
		DialogueSequence gadgetSeqLeo = DialogueSequence();

		/****************************/

		DialogueStage& gadgetDuringLeo = gadgetSeqLeo.sequence[QUEST_STAGE::DURING];

		DialogueLine gadgetDuringLeo_P1, gadgetDuringLeo_P2;
		gadgetDuringLeo_P1.text = "Hmm.. What if I..";
		gadgetDuringLeo_P2.text = "Oh, I didn't see you there.";

		gadgetDuringLeo.primary.push_back(gadgetDuringLeo_P1);
		gadgetDuringLeo.primary.push_back(gadgetDuringLeo_P2);

		/****************************/

		DialogueStage& gadgetEndLeo = gadgetSeqLeo.sequence[QUEST_STAGE::END];

		DialogueLine gadgetEndLeo_P1, gadgetEndLeo_P2, gadgetEndLeo_P3, gadgetEndLeo_P4, gadgetEndLeo_P5;
		gadgetEndLeo_P1.text = "What's that you have with you?";
		gadgetEndLeo_P2.text = "..I see. Yes, I do think I could fix it up for you. Would you let me have it?";
		gadgetEndLeo_P3.text = "Name's Leo, by the way.";
		gadgetEndLeo_P4.text = "I like interesting things.";
		gadgetEndLeo_P5.text = "And also making interesting things.";

		gadgetEndLeo.primary.push_back(gadgetEndLeo_P1);
		gadgetEndLeo.primary.push_back(gadgetEndLeo_P2);
		gadgetEndLeo.primary.push_back(gadgetEndLeo_P3);
		gadgetEndLeo.primary.push_back(gadgetEndLeo_P4);
		gadgetEndLeo.primary.push_back(gadgetEndLeo_P5);

		/****************************/

		DialogueSequence leoScaleSeq = DialogueSequence();

		/****************************/

		DialogueStage& leoScaleStart = leoScaleSeq.sequence[QUEST_STAGE::START];
		leoScaleStart.autoTrigger = true;

		DialogueLine leoScaleStart_P1, leoScaleStart_P2, leoScaleStart_P3, leoScaleStart_P4, leoScaleStart_P5, leoScaleStart_P6, leoScaleStart_P7, leoScaleStart_P8, leoScaleStart_P9;
		leoScaleStart_P1.text = "Say, if you're not busy.";
		leoScaleStart_P2.text = "Hear me out ?";
		leoScaleStart_P3.text = "Your ring, it reminds me of this, this glittering blue scale I found the other day.";
		leoScaleStart_P4.text = "But I dropped it.";
		leoScaleStart_P5.text = "Down the drain.";
		leoScaleStart_P6.text = "When I went to clean it.";
		leoScaleStart_P7.text = "Oops.";
		leoScaleStart_P8.text = "Anyways, here's the key to the sewers.";
		leoScaleStart_P9.text = "Best of luck!";

		leoScaleStart.primary.push_back(leoScaleStart_P1);
		leoScaleStart.primary.push_back(leoScaleStart_P2);
		leoScaleStart.primary.push_back(leoScaleStart_P3);
		leoScaleStart.primary.push_back(leoScaleStart_P4);
		leoScaleStart.primary.push_back(leoScaleStart_P5);
		leoScaleStart.primary.push_back(leoScaleStart_P6);
		leoScaleStart.primary.push_back(leoScaleStart_P7);
		leoScaleStart.primary.push_back(leoScaleStart_P8);
		leoScaleStart.primary.push_back(leoScaleStart_P9);

		/****************************/

		DialogueStage& leoScaleDuring = leoScaleSeq.sequence[QUEST_STAGE::DURING];

		DialogueLine leoScaleDuring_P1, leoScaleDuring_P2, leoScaleDuring_P3, leoScaleDuring_P4;
		leoScaleDuring_P1.text = "You find anything yet?";
		leoScaleDuring_P2.text = "I know it might be a lost cause.";
		leoScaleDuring_P3.text = "I liked that scale though.";
		leoScaleDuring_P4.text = "Seemed to have some strange properties.";
		// selene, upon picking up the scale: (looks like I won't be able to get out of here the same way I came in.)

		leoScaleDuring.primary.push_back(leoScaleDuring_P1);
		leoScaleDuring.primary.push_back(leoScaleDuring_P2);
		leoScaleDuring.primary.push_back(leoScaleDuring_P3);
		leoScaleDuring.primary.push_back(leoScaleDuring_P4);

		/****************************/

		DialogueStage& leoScaleEnd = leoScaleSeq.sequence[QUEST_STAGE::END];

		DialogueLine leoScaleEnd_P1, leoScaleEnd_P2, leoScaleEnd_P3, leoScaleEnd_P4, leoScaleEnd_P5, leoScaleEnd_P6;
		leoScaleEnd_P1.text = "Oh, you've returned! Would it be bad if I said I didn't expect you to?";
		leoScaleEnd_P2.text = "Well, no matter. You're here now, and with that scale intact too.";
		leoScaleEnd_P3.text = "Here, take this.";
		leoScaleEnd_P4.text = "I've fixed that ring you gave me.";
		leoScaleEnd_P5.text = "And made it extra fun.";
		leoScaleEnd_P6.text = "As thanks.";
		// TODO: add the no scale dialogue (check that the player has given the scale away)

		leoScaleEnd.primary.push_back(leoScaleEnd_P1);
		leoScaleEnd.primary.push_back(leoScaleEnd_P2);
		leoScaleEnd.primary.push_back(leoScaleEnd_P3);
		leoScaleEnd.primary.push_back(leoScaleEnd_P4);
		leoScaleEnd.primary.push_back(leoScaleEnd_P5);
		leoScaleEnd.primary.push_back(leoScaleEnd_P6);

		/****************************/

		DialogueSequence noScaleSeq = DialogueSequence();

		DialogueStage& noScaleEnd = noScaleSeq.sequence[QUEST_STAGE::END];

		DialogueLine noScaleEnd_P1, noScaleEnd_P2, noScaleEnd_P3, noScaleEnd_P4, noScaleEnd_P5, noScaleEnd_P6, noScaleEnd_P7;
		noScaleEnd_P1.text = "No luck? I see… a valiant effort nonetheless.";
		noScaleEnd_P2.text = "I'll just have to try again myself.";
		noScaleEnd_P3.text = "Oh, right.";
		noScaleEnd_P4.text = "I've fixed your ring.";
		noScaleEnd_P5.text = "Tinkered with it a little bit too.";
		noScaleEnd_P6.text = "Here, take it.";
		noScaleEnd_P7.text = "Goodbye then.";

		noScaleEnd.primary.push_back(noScaleEnd_P1);
		noScaleEnd.primary.push_back(noScaleEnd_P2);
		noScaleEnd.primary.push_back(noScaleEnd_P3);
		noScaleEnd.primary.push_back(noScaleEnd_P4);
		noScaleEnd.primary.push_back(noScaleEnd_P5);
		noScaleEnd.primary.push_back(noScaleEnd_P6);
		noScaleEnd.primary.push_back(noScaleEnd_P7);

		/****************************/
	/*********** END LEO ***********/

	/*********** ORION ***********/
		// DEFINE DIALOGUE LINES
		DialogueSequence coinsOrion = DialogueSequence();

		/****************************/

		DialogueStage& coinsEndOrion = coinsOrion.sequence[QUEST_STAGE::END];

		DialogueLine coinsEndOrion_P1, coinsEndOrion_P2, coinsEndOrion_P3, coinsEndOrion_P4, coinsEndOrion_P5;
		coinsEndOrion_P1.text = "Hey, don't believe I've seen your face 'round here before.";
		coinsEndOrion_P2.text = "Well, I guess I don't come down here that often.";
		coinsEndOrion_P3.text = "I'm Orion, a Starseeker from Above. What might you be up to today?";
		coinsEndOrion_P4.text = "...Medicine, is it? I'm afraid I don't have any on hand, but I can offer you a few scales.";
		coinsEndOrion_P5.text = "Orion hands you 10 scales.";
		coinsEndOrion_P5.line_speaker = LINE_SPEAKER::WORLD;


		coinsEndOrion.primary.push_back(coinsEndOrion_P1);
		coinsEndOrion.primary.push_back(coinsEndOrion_P2);
		coinsEndOrion.primary.push_back(coinsEndOrion_P3);
		coinsEndOrion.primary.push_back(coinsEndOrion_P4);
		coinsEndOrion.primary.push_back(coinsEndOrion_P5);

		/****************************/

		DialogueSequence orionScaleSeq = DialogueSequence();

		/****************************/

		DialogueStage& orionScaleStart = orionScaleSeq.sequence[QUEST_STAGE::START];
		orionScaleStart.autoTrigger = true;

		DialogueLine orionScaleStart_P1, orionScaleStart_P2, orionScaleStart_P3;
		orionScaleStart_P1.text = "Say, would you do me a favour?";
		orionScaleStart_P2.text = "I came here in search of a souvenir for my daughter. She's been feeling down recently, so I wanted to get her a little gift for her birthday.";
		orionScaleStart_P3.text = "Do let me know if you see anything pretty or sparkly that a young girl might like, yeah?";

		orionScaleStart.primary.push_back(orionScaleStart_P1);
		orionScaleStart.primary.push_back(orionScaleStart_P2);
		orionScaleStart.primary.push_back(orionScaleStart_P3);

		/****************************/

		DialogueStage& orionScaleDuring = orionScaleSeq.sequence[QUEST_STAGE::DURING];
		
		DialogueLine orionScaleDuring_P1;
		orionScaleDuring_P1.text = "Oh, hello again. What are you up to?"; // default line

		orionScaleDuring.primary.push_back(orionScaleDuring_P1);

		/****************************/

		DialogueStage& orionScaleEnd = orionScaleSeq.sequence[QUEST_STAGE::END];
		orionScaleEnd.autoTrigger = true;
		orionScaleEnd.conditional = true;
		orionScaleEnd.yesOption = "Give";
		orionScaleEnd.noOption = "Keep";

		DialogueLine orionScaleEnd_P1, orionScaleEnd_P2, orionScaleEnd_Y1, orionScaleEnd_Y2, orionScaleEnd_Y3, orionScaleEnd_Y4, orionScaleEnd_Y5, orionScaleEnd_N1;
		orionScaleEnd_P1.text = "My, what's that you're holding? It's so... pretty. Is it important to you?";
		orionScaleEnd_P2.text = "If not, might you consider parting with it? I think my little girl would really like it.";

		orionScaleEnd_Y1.text = "You have my gratitude. Little Callie's going to be thrilled to receive this.";
		orionScaleEnd_Y2.text = "Here, how about I give you this? It's something of a good luck charm to me.";
		orionScaleEnd_Y3.text = "This is a small painting of our kingdom under the sun. Seems simple, but it's the story that captured my heart.";
		orionScaleEnd_Y4.text = "In a time of despair, when the land was torn apart, they say that the king himself held the kingdom together and rebuilt it from the ground up.";
		orionScaleEnd_Y5.text = "Anyways, what I'm saying is, mayhap it will bring you hope as well.";

		orionScaleEnd_N1.text = "I see. That's a shame.";

		orionScaleEnd.primary.push_back(orionScaleEnd_P1);
		orionScaleEnd.primary.push_back(orionScaleEnd_P2);

		orionScaleEnd.yesBranch.push_back(orionScaleEnd_Y1);
		orionScaleEnd.yesBranch.push_back(orionScaleEnd_Y2);
		orionScaleEnd.yesBranch.push_back(orionScaleEnd_Y3);
		orionScaleEnd.yesBranch.push_back(orionScaleEnd_Y4);
		orionScaleEnd.yesBranch.push_back(orionScaleEnd_Y5);

		orionScaleEnd.noBranch.push_back(orionScaleEnd_N1);

		/****************************/

		DialogueSequence orionBroochSeq = DialogueSequence();

		/****************************/

		DialogueStage& orionBroochStart = orionBroochSeq.sequence[QUEST_STAGE::START];
		orionBroochStart.autoTrigger = true;

		DialogueLine orionBroochStart_P1, orionBroochStart_P2, orionBroochStart_P3, orionBroochStart_P4, orionBroochStart_P5, orionBroochStart_P6;
		orionBroochStart_P1.text = "... ..!!";
		orionBroochStart_P2.text = "That emblem... Isn't that-";
		orionBroochStart_P3.text = "I must report to the king immediately.";
		orionBroochStart_P4.text = "Could you make your way to the skylands as well? I think you should seek an audience with the king";
		orionBroochStart_P5.text = "Don't worry, you haven't done anything wrong. On the contrary, I think you might be able to get permission to move to the skylands with us.";
		orionBroochStart_P6.text = "Please hurry on ahead. We'll explain things once you get there.";

		orionBroochStart.primary.push_back(orionBroochStart_P1);
		orionBroochStart.primary.push_back(orionBroochStart_P2);
		orionBroochStart.primary.push_back(orionBroochStart_P3);
		orionBroochStart.primary.push_back(orionBroochStart_P4);
		orionBroochStart.primary.push_back(orionBroochStart_P5);
		orionBroochStart.primary.push_back(orionBroochStart_P6);

		/****************************/

		DialogueStage& orionBroochDuring = orionBroochSeq.sequence[QUEST_STAGE::DURING];
		
		DialogueLine orionBroochDuring_P1;
		orionBroochDuring_P1.text = "Please head up through the Gilded Greenway to the skyship landing."; // default line

		orionBroochDuring.primary.push_back(orionBroochDuring_P1);

	/*********** END ORION ***********/

	/*********** APOTHECARY ***********/
		// DEFINE DIALOGUE LINES
		DialogueSequence apothecarySeq = DialogueSequence();

		/****************************/

		DialogueStage& apothecaryStart = apothecarySeq.sequence[QUEST_STAGE::START];
		
		DialogueLine apothecaryStart_P1, apothecaryStart_P2, apothecaryStart_P3, apothecaryStart_P4;
		apothecaryStart_P1.text = "What can I do for you?";
		apothecaryStart_P2.text = "Medicine, is that right? It'll be 50 scales. I hate to raise the price, but the woods have been getting real dangerous recently.";
		apothecaryStart_P3.text = "As you can imagine, nobody is willing to chance it in there just for some herbs.";
		apothecaryStart_P4.text = "(I really need that medicine. There must be something I can do around town.)";
		apothecaryStart_P4.line_speaker = LINE_SPEAKER::PLAYER;

		apothecaryStart.primary.push_back(apothecaryStart_P1);
		apothecaryStart.primary.push_back(apothecaryStart_P2);
		apothecaryStart.primary.push_back(apothecaryStart_P3);
		apothecaryStart.primary.push_back(apothecaryStart_P4);

		/****************************/

		DialogueStage& apothecaryDuring = apothecarySeq.sequence[QUEST_STAGE::DURING];

		DialogueLine apothecaryDuring_P1, apothecaryDuring_P2;
		apothecaryDuring_P1.text = "Sorry, miss. Like I said, it's 50 scales for the medicine.";
		apothecaryDuring_P2.text = "(I'll have to find a way to pay for it somehow.)";
		apothecaryDuring_P2.line_speaker = LINE_SPEAKER::PLAYER;
		// optional dialogue from selene: (I'll have to find a way to pay for it somehow)

		apothecaryDuring.primary.push_back(apothecaryDuring_P1);
		apothecaryDuring.primary.push_back(apothecaryDuring_P2);

		/****************************/

		DialogueStage& apothecaryEnd = apothecarySeq.sequence[QUEST_STAGE::END];

		DialogueLine apothecaryEnd_P1, apothecaryEnd_P2, apothecaryEnd_P3, apothecaryEnd_P4;
		apothecaryEnd_P1.text = "Yep, that sure looks like 50 scales. Here you go.";
		apothecaryEnd_P2.text = "The apothecary hands you a small pouch.";
		apothecaryEnd_P2.line_speaker = LINE_SPEAKER::WORLD;
		apothecaryEnd_P3.text = "You don't seem like you're from very high up, so I didn't think you could afford it.";
		apothecaryEnd_P4.text = "Won't ask you where you got it from though.";

		apothecaryEnd.primary.push_back(apothecaryEnd_P1);
		apothecaryEnd.primary.push_back(apothecaryEnd_P2);
		apothecaryEnd.primary.push_back(apothecaryEnd_P3);
		apothecaryEnd.primary.push_back(apothecaryEnd_P4);

		/****************************/

	/*********** END APOTHECARY ***********/

	/*********** INNKEEPER ***********/
		// DEFINE DIALOGUE LINES
		DialogueSequence innkeeperSeq = DialogueSequence();

		/****************************/

		DialogueStage& innkeeperStart = innkeeperSeq.sequence[QUEST_STAGE::START];
		innkeeperStart.autoTrigger = true;
		
		DialogueLine innkeeperStart_P1, innkeeperStart_P2, innkeeperStart_P3;
		innkeeperStart_P1.text = "Wait, miss!";
		innkeeperStart_P2.text = "A parcel meant for you arrived at the inn just a moment ago.";
		innkeeperStart_P3.text = "Come on in! We'll hand it to you at the counter.";

		innkeeperStart.primary.push_back(innkeeperStart_P1);
		innkeeperStart.primary.push_back(innkeeperStart_P2);
		innkeeperStart.primary.push_back(innkeeperStart_P3);

		/****************************/
		
		DialogueSequence innkeeperTipSeq = DialogueSequence();

		/****************************/

		DialogueStage& innkeeperTipDuring = innkeeperTipSeq.sequence[QUEST_STAGE::DURING];
		innkeeperTipDuring.autoTrigger = true;
		
		DialogueLine innkeeperTipDuring_P1, innkeeperTipDuring_P2, innkeeperTipDuring_P3;
		innkeeperTipDuring_P1.text = "I heard the old aristocrat living in the manor east of here has plenty of gold by the way...";
		innkeeperTipDuring_P2.text = "More than he knows what to do with.";
		innkeeperTipDuring_P3.text = "You didn't hear it from me though.";

		innkeeperTipDuring.primary.push_back(innkeeperTipDuring_P1);
		innkeeperTipDuring.primary.push_back(innkeeperTipDuring_P2);
		innkeeperTipDuring.primary.push_back(innkeeperTipDuring_P3);

	/*********** END INNKEEPER ***********/

	/*********** GREENWAY GUARD ***********/
		// DEFINE DIALOGUE LINES
		DialogueSequence greenwayGuardSeq = DialogueSequence();

		/****************************/

		DialogueStage& greenwayGuardStart = greenwayGuardSeq.sequence[QUEST_STAGE::START];
		
		DialogueLine greenwayGuardStart_P1, greenwayGuardStart_P2, greenwayGuardStart_P3, greenwayGuardStart_P4;
		greenwayGuardStart_P1.text = "... ..hm? Who are you?";
		greenwayGuardStart_P2.text = "...Oh. I don't care what you think Orion has to say. I'm not letting a commoner through.";
		greenwayGuardStart_P3.text = "..Zzz....";
		greenwayGuardStart_P4.text = "(He seems sleepy. Maybe I can sneak around him somehow)";
		greenwayGuardStart_P4.line_speaker = LINE_SPEAKER::PLAYER;

		greenwayGuardStart.primary.push_back(greenwayGuardStart_P1);
		greenwayGuardStart.primary.push_back(greenwayGuardStart_P2);
		greenwayGuardStart.primary.push_back(greenwayGuardStart_P3);
		greenwayGuardStart.primary.push_back(greenwayGuardStart_P4);

		/****************************/

		DialogueStage& greenwayGuardDuring = greenwayGuardSeq.sequence[QUEST_STAGE::DURING];

		DialogueLine greenwayGuardDuring_P1;
		greenwayGuardDuring_P1.text = "Zzz...Go away.";

		greenwayGuardDuring.primary.push_back(greenwayGuardDuring_P1);

	/*********** END GREENWAY GUARD ***********/

	/*********** POLARIS ***********/

		DialogueSequence polarisColosseumSeq = DialogueSequence();

		DialogueStage& polarisColosseumStart = polarisColosseumSeq.sequence[QUEST_STAGE::START];

		DialogueLine polarisColosseumStart_P1, polarisColosseumStart_P2, polarisColosseumStart_P3, polarisColosseumStart_P4, polarisColosseumStart_P5, polarisColosseumStart_P6;
		DialogueLine polarisColosseumStart_Y1, polarisColosseumStart_N1;
		polarisColosseumStart_P1.text = "Halt.";
		polarisColosseumStart_P2.text = "I don't recognize you. State your purpose.";
		polarisColosseumStart_P3.text = "...Is that so. I did receive a message of that sort from Orion.";
		polarisColosseumStart_P4.text = "Unfortunately for you, I'm not as softhearted as he is.";
		polarisColosseumStart_P5.text = "How about you prove yourself? We've got a few recruits who could learn from observing a battle or two. In other words, I won't let you through unless your performance in the Colosseum impresses me.";
		polarisColosseumStart_P6.text = "So? Do you have what it takes to move forward?";

		polarisColosseumStart_Y1.text = "That's what I like to hear. Guards! Let the preparations begin!";

		polarisColosseumStart_N1.text = "Didn't take you for a coward. I'll be waiting in case you change your mind.";

		polarisColosseumStart.primary.push_back(polarisColosseumStart_P1);
		polarisColosseumStart.primary.push_back(polarisColosseumStart_P2);
		polarisColosseumStart.primary.push_back(polarisColosseumStart_P3);
		polarisColosseumStart.primary.push_back(polarisColosseumStart_P4);
		polarisColosseumStart.primary.push_back(polarisColosseumStart_P5);
		
		polarisColosseumStart.yesBranch.push_back(polarisColosseumStart_Y1);
		
		polarisColosseumStart.noBranch.push_back(polarisColosseumStart_N1);

	/*********** END POLARIS ***********/
	
	/*********** ASTRONOMER ***********/

		DialogueSequence astronomerEndSeq = DialogueSequence();

		DialogueStage& astronomerEnd = astronomerEndSeq.sequence[QUEST_STAGE::END];

		DialogueLine astronomerEnd_P1, astronomerEnd_P2, astronomerEnd_P3;
		astronomerEnd_P1.text = "Hi.";
		astronomerEnd_P2.text = "...You want a flight enchantment for the skyship? I don't really feel like it though.";
		astronomerEnd_P3.text = "Bye.";

		astronomerEnd.primary.push_back(astronomerEnd_P1);
		astronomerEnd.primary.push_back(astronomerEnd_P2);
		astronomerEnd.primary.push_back(astronomerEnd_P3);

		DialogueSequence astronomerEnchantSeq = DialogueSequence();

		DialogueStage& astronomerEnchantStart = astronomerEnchantSeq.sequence[QUEST_STAGE::START];
		DialogueLine astronomerEnchantStart_P1, astronomerEnchantStart_P2, astronomerEnchantStart_P3;

		astronomerEnchantStart_P1.text = "...By orders of Captain Polaris? Fine. I'll do it if you do something for me first.";
		astronomerEnchantStart_P2.text = "Go collect some black talons for me. I need them for a new spell I'm trying to develop.";
		astronomerEnchantStart_P3.text = "You'll find them around these islands. Be prepared to get your hands dirty.";

		astronomerEnchantStart.primary.push_back(astronomerEnchantStart_P1);
		astronomerEnchantStart.primary.push_back(astronomerEnchantStart_P2);
		astronomerEnchantStart.primary.push_back(astronomerEnchantStart_P3);
		
		DialogueStage& astronomerEnchantDuring = astronomerEnchantSeq.sequence[QUEST_STAGE::DURING];
		DialogueLine astronomerEnchantDuring_P1, astronomerEnchantDuring_P2, astronomerEnchantDuring_P3;

		astronomerEnchantDuring_P1.text = "Hurry up and fetch me those talons. That dagger's not just for show, is it?";

		astronomerEnchantDuring.primary.push_back(astronomerEnchantDuring_P1);

		DialogueStage& astronomerEnchantEnd = astronomerEnchantSeq.sequence[QUEST_STAGE::END];
		DialogueLine astronomerEnchantEnd_P1, astronomerEnchantEnd_P2;

		astronomerEnchantEnd_P1.text = "Thanks. I'll be done by the time you reach the landing.";
		astronomerEnchantEnd_P2.text = "Run along, then.";

		astronomerEnchantEnd.primary.push_back(astronomerEnchantEnd_P1);
		astronomerEnchantEnd.primary.push_back(astronomerEnchantEnd_P2);


	/*********** END ASTRONOMER ***********/

	/*********** BRIDGE ***********/
		// DEFINE DIALOGUE LINES
		DialogueSequence bridgeSeq = DialogueSequence();

		/****************************/

		DialogueStage& bridgeStart = bridgeSeq.sequence[QUEST_STAGE::START];
		DialogueLine bridgeStart_P1, bridgeStart_P2;
		bridgeStart_P1.text = "(The bridge seems broken...)";
		bridgeStart_P2.text = "(I might be able to fix it with some rope.)";
		bridgeStart_P1.line_speaker = LINE_SPEAKER::PLAYER;
		bridgeStart_P2.line_speaker = LINE_SPEAKER::PLAYER;

		bridgeStart.primary.push_back(bridgeStart_P1);
		bridgeStart.primary.push_back(bridgeStart_P2);

		/****************************/

		DialogueStage& bridgeDuring = bridgeSeq.sequence[QUEST_STAGE::DURING];

		DialogueLine bridgeDuring_P1;
		bridgeDuring_P1.text = "(Maybe I can find a rope to fix this with.)";
		bridgeDuring_P1.line_speaker = LINE_SPEAKER::PLAYER;

		bridgeDuring.primary.push_back(bridgeDuring_P1);

		/****************************/

		DialogueStage& bridgeEnd = bridgeSeq.sequence[QUEST_STAGE::END];

		DialogueLine bridgeEnd_P1;
		bridgeEnd_P1.text = "(Looks like I can cross safely now.)";
		bridgeEnd_P1.line_speaker = LINE_SPEAKER::PLAYER;

		bridgeEnd.primary.push_back(bridgeEnd_P1);

		/****************************/
		// DEFINE DIALOGUE LINES
		DialogueSequence bridgeRightSeq = DialogueSequence();

		/****************************/

		DialogueStage& bridgeRightStart = bridgeRightSeq.sequence[QUEST_STAGE::START];
		DialogueLine bridgeRightStart_P1, bridgeRightStart_P2;
		bridgeRightStart_P1.text = "(The bridge seems broken...)";
		bridgeRightStart_P2.text = "(I don't think I can fix it from this side.)";
		bridgeRightStart_P1.line_speaker = LINE_SPEAKER::PLAYER;
		bridgeRightStart_P2.line_speaker = LINE_SPEAKER::PLAYER;

		bridgeRightStart.primary.push_back(bridgeRightStart_P1);
		bridgeRightStart.primary.push_back(bridgeRightStart_P2);

		/****************************/

		DialogueStage& bridgeRightDuring = bridgeRightSeq.sequence[QUEST_STAGE::DURING];

		DialogueLine bridgeRightDuring_P1;
		bridgeRightDuring_P1.text = "(Agh, I think I have to fix it from the other side.)";
		bridgeRightDuring_P1.line_speaker = LINE_SPEAKER::PLAYER;

		bridgeRightDuring.primary.push_back(bridgeRightDuring_P1);

		/****************************/
	/*********** END BRIDGE ***********/

	/*********** BRIDGE ***********/
		// DEFINE DIALOGUE LINES
		DialogueSequence windowSeq = DialogueSequence();

		DialogueStage& windowEnd = windowSeq.sequence[QUEST_STAGE::END];

		DialogueLine windowEnd_P1;
		windowEnd_P1.text = "Window unlocked.";
		windowEnd_P1.line_speaker = LINE_SPEAKER::WORLD;

		windowEnd.primary.push_back(windowEnd_P1);

		/****************************/
	/*********** END BRIDGE ***********/
	

		// ADD IT TO THE COMPLEX SEQUENCES
		complexSequences = {
			{ SPEAKER_ID::VENUS,
				{ //map
					{ QUEST_INDEX::GATHER_FRUITS, berriesSeq },
					{ QUEST_INDEX::FIND_BROOCH, broochSeq },
					{ QUEST_INDEX::BUY_MEDICINE, medicineSeq },
					{ QUEST_INDEX::GIVE_GADGET, gadgetSeq },
				},
			},
			{ SPEAKER_ID::LEO,
				{ //map
					{ QUEST_INDEX::GIVE_GADGET, gadgetSeqLeo },
					{ QUEST_INDEX::SCALE_FOR_LEO, leoScaleSeq },
					{ QUEST_INDEX::NO_SCALE_LEO, noScaleSeq },
				},
			},
			{ SPEAKER_ID::ORION,
				{ //map
					{ QUEST_INDEX::COINS_FROM_ORION, coinsOrion },
					{ QUEST_INDEX::SCALE_TO_ORION, orionScaleSeq },
					{ QUEST_INDEX::GO_TO_SKYLANDS, orionBroochSeq },
				},
			},
			{
				SPEAKER_ID::APOTHECARY,
				{
					{ QUEST_INDEX::STEAL_MONEY, apothecarySeq },
				}
			},
			{
				SPEAKER_ID::BRIDGE,
				{
					{ QUEST_INDEX::FIX_BRIDGE, bridgeSeq },
				}
			},
			{
				SPEAKER_ID::BRIDGE_RIGHT,
				{
					{ QUEST_INDEX::FIX_BRIDGE, bridgeRightSeq },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_WALK,
				{
					{ QUEST_INDEX::TUTORIAL_WALK, tutorialWalk },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_JUMP,
				{
					{ QUEST_INDEX::TUTORIAL_JUMP, tutorialJump },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_ATTACK,
				{
					{ QUEST_INDEX::TUTORIAL_ATTACK, tutorialAttack },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_MAP,
				{
					{ QUEST_INDEX::TUTORIAL_MAP, tutorialMap },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_ROCK,
				{
					{ QUEST_INDEX::TUTORIAL_ROCK, tutorialRock },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_ROCK,
				{
					{ QUEST_INDEX::TUTORIAL_ROCK, tutorialRock },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_ROCK_2,
				{
					{ QUEST_INDEX::TUTORIAL_ROCK_2, tutorialRock2 },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_ROCK_3,
				{
					{ QUEST_INDEX::TUTORIAL_ROCK_3, tutorialRock3 },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_INN_EXT,
				{
					{ QUEST_INDEX::TUTORIAL_INN, tutorialInnExt },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_INN_INT,
				{
					{ QUEST_INDEX::TUTORIAL_INN, tutorialInnInt },
				}
			},
			{
				SPEAKER_ID::TUTORIAL_STEALTH,
				{
					{ QUEST_INDEX::TUTORIAL_STEALTH, tutorialStealth },
				}
			},
			{
				SPEAKER_ID::MAIL,
				{
					{ QUEST_INDEX::BUY_MEDICINE, mailMedicineSeq },
					{ QUEST_INDEX::GIVE_GADGET, mailGadgetSeq },
					{ QUEST_INDEX::GET_BROOCH, mailBroochSeq },
				}
			},
			{
				SPEAKER_ID::WINDOW,
				{
					{ QUEST_INDEX::UNLOCK_WINDOW, windowSeq },
				}
			},
			{
				SPEAKER_ID::INN_KEEPER,
				{
					{ QUEST_INDEX::STEAL_MONEY, innkeeperTipSeq },
					{ QUEST_INDEX::GET_BROOCH, innkeeperSeq },
				}
			},
		};

		simpleLines = {
			{
				SPEAKER_ID::VENUS,
				{ "Taking a break from your adventures?" }
			},
			{
				SPEAKER_ID::LEO,
				{ "Hmm.. What if I..",
					"No, no.. that's not right. How about-"} // find a way to add this - Selene: (Seems like he�s preoccupied. Better leave him alone for now.)
			},
			{
				SPEAKER_ID::ORION,
				{ "Oh, hello again. What are you up to? "} // make sure this is not triggered before his introductory dialogue
			},
			{
				SPEAKER_ID::APOTHECARY,
				{ "What can I help you with?" }
			},
			{
				SPEAKER_ID::GREENWAY_GUARD,
				{ "Zzz...Go away." }
			},
			{
				SPEAKER_ID::POLARIS,
				{ "Back for another round in the Colosseum? ..Relax, I'm just joking. Mostly." }
			},
			{
				SPEAKER_ID::ASTRONOMER,
				{ "...I'm busy." }
			},
			{
				SPEAKER_ID::TOWN_FAMILY,
				{ "Mommy look! I can jump so high now! Hehe, I bet I can reach the skylands soon.",
					"Of course you will, dear. Now come back down for dinner." }
			},
			{
				SPEAKER_ID::GOSSIP_GIRLS,
				{ "It looks like there's a request for 15 scales to fetch herbs from the forest.",
					"Aren't there more monsters there than usual these days?",
					"You know, I heard Leo say that monsters are attracted to irregularities in the flow of magical energy... Whatever that means.",
					"Didn't that man come from Below? I don't know if we should trust him. Besides, there's nothing all that magical about this run-down place." }
			},
			{
				SPEAKER_ID::BRIDGE,
				{ "(Hmm, this bridge might help me get back to the other side.)" }
			},
			{
				SPEAKER_ID::BRIDGE_RIGHT,
				{ "(I think this takes me across the river.)" }
			},
			{
				SPEAKER_ID::RESPAWN_ANCHOR,
				{ "Game Saved." }
			},
			{
				SPEAKER_ID::TUTORIAL_WALK, {}
			},
			{
				SPEAKER_ID::TUTORIAL_JUMP, {}
			},
			{
				SPEAKER_ID::TUTORIAL_ATTACK, {}
			},
			{
				SPEAKER_ID::TUTORIAL_MAP, {}
			},
			{
				SPEAKER_ID::TUTORIAL_ROCK, {}
			},
			{
				SPEAKER_ID::TUTORIAL_STEALTH, {}
			},
			{
				SPEAKER_ID::MAIL, 
				{ "No new mail." }
			},
			{
				SPEAKER_ID::STARSEEKER_MONUMENT, 
				{ "In honour of the Starseekers, who brought us sanctuary and led us to prosperity in our time of need." }
			},
			{
				SPEAKER_ID::STARSEEKER_MONUMENT, 
				{ "In honour of the Starseekers, who brought us sanctuary and led us to prosperity in our time of need." }
			},
			{
				SPEAKER_ID::ARISTOCRAT_1, 
				{ "My, if it isn't a commoner.", 
					"What brings you up here? Trying to climb the ranks?", 
					"You'll find you won't get far on your own." }
			},
			{
				SPEAKER_ID::ARISTOCRAT_2, 
				{ "I like watching the peasants skitter around like bugs. Heeheehee. They could never reach our level." }
			},
		};
	}

public:
	std::map<SPEAKER_ID, std::map <QUEST_INDEX, DialogueSequence>> complexSequences;
	std::map<SPEAKER_ID, std::vector<std::string> > simpleLines;

	DialogueRegistry() {
		init();
	}
};