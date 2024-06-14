#pragma once

#include <array>
#include <utility>
#include <iostream>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// fonts
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
// matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// clock (for fps)
#include <time.h>

struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};


// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {

	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER, mesh_path("player.obj"))
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("nosprite.png"),
			textures_path("bg_central_background_back_blurred.jpg"),
			textures_path("bg_forest_1_0.png"),
			textures_path("bg_forest_2_0.png"),
			textures_path("bg_forest_2_1.png"),
			textures_path("bg_canopy_0.png"),
			textures_path("bg_cabin_0.png"),
			textures_path("bg_inn.png"),

			textures_path("bg_town_0.png"),
			textures_path("bg_town_1.png"),
			textures_path("bg_sewers.png"),
			textures_path("bg_sewers_1.png"),

			textures_path("bg_plaza.png"),
			textures_path("bg_plaza_2.png"),
			textures_path("bg_aristocrat.png"),
			textures_path("bg_aristocrat_1.png"),

			textures_path("bg_aristocrat_int.png"),
			textures_path("bg_greenway.png"),
			
			textures_path("dialogue_box.png"),
			textures_path("in_game_map.png"),
			textures_path("mapctrls.png"),
			textures_path("quests.png"),
			textures_path("health_heart.png"),

			textures_path("eagle.png"),
			textures_path("Player_standing.png"),
			textures_path("evil_tree.png"),
			textures_path("spr_greybox_solid.png"),
			textures_path("spr_greybox_ramp.png"),

			textures_path("/particles/part_1.png"),
			textures_path("rock.png"),

			textures_path("grass.png"),
			textures_path("spr_item.png"),
			textures_path("door.png"),
			textures_path("inn_base.png"),

			player_run_textures_path("Player_frame_1.png"),
			player_run_textures_path("Player_frame_2.png"),
			player_run_textures_path("Player_frame_3.png"),
			player_run_textures_path("Player_frame_4.png"),
			player_run_textures_path("Player_frame_5.png"),
			player_run_textures_path("Player_frame_6.png"),
			player_run_textures_path("Player_frame_7.png"),
			player_run_textures_path("Player_frame_8.png"),
			player_run_textures_path("Player_frame_9.png"),
			player_run_textures_path("Player_frame_10.png"),

			evil_tree_run_textures_path("Evil_Tree_Frame_1.png"),
			evil_tree_run_textures_path("Evil_Tree_Frame_2.png"),
			evil_tree_run_textures_path("Evil_Tree_Frame_3.png"),
			evil_tree_run_textures_path("Evil_Tree_Frame_4.png"),
			evil_tree_run_textures_path("Evil_Tree_Frame_5.png"),
			evil_tree_run_textures_path("Evil_Tree_Frame_6.png"),
			evil_tree_run_textures_path("Evil_Tree_Frame_7.png"),
			evil_tree_run_textures_path("Evil_Tree_Frame_8.png"),

			player_jump_textures_path("Up_In_Air_Frame_1.png"),
			player_jump_textures_path("Up_In_Air_Frame_2.png"),
			player_jump_textures_path("Up_In_Air_Frame_3.png"),
			player_jump_textures_path("Up_In_Air_Frame_4.png"),
			player_jump_textures_path("Up_In_Air_Frame_5.png"),
			player_jump_textures_path("Falling_Frame_1.png"),
			player_jump_textures_path("Falling_Frame_2.png"),		
			player_jump_textures_path("Falling_Frame_3.png"),
			player_jump_textures_path("Falling_Frame_4.png"),
			player_jump_textures_path("Falling_Frame_5.png"),

			textures_path("apple.png"),
			textures_path("blueberry.png"),
			textures_path("breakable_box.png"),

			player_death_textures_path("Death_Frame_1.png"),
			player_death_textures_path("Death_Frame_2.png"),
			player_death_textures_path("Death_Frame_3.png"),
			player_death_textures_path("Death_Frame_4.png"),
			player_death_textures_path("Death_Frame_5.png"),
			player_death_textures_path("Death_Frame_6.png"),
			player_death_textures_path("Death_Frame_7.png"),
			player_death_textures_path("Death_Frame_8.png"),
			player_death_textures_path("Death_Frame_9.png"),
			player_death_textures_path("Death_Frame_10.png"),
			player_death_textures_path("Death_Frame_11.png"),
			player_death_textures_path("Death_Frame_12.png"),
			player_death_textures_path("Death_Frame_13.png"),

			player_attack_textures_path("Player_Attack_Frame_1.png"),
			player_attack_textures_path("Player_Attack_Frame_2.png"),
			player_attack_textures_path("Player_Attack_Frame_3.png"),
			player_attack_textures_path("Player_Attack_Frame_4.png"),
			player_attack_textures_path("Player_Attack_Frame_5.png"),
			player_attack_textures_path("Player_Attack_Frame_6.png"),
			player_attack_textures_path("Player_Attack_Frame_7.png"),
			player_attack_textures_path("Player_Attack_Frame_8.png"),
			player_attack_textures_path("Player_Attack_Frame_9.png"),

			player_attack_up_textures_path("Up_Attack_Frame_1.png"),
			player_attack_up_textures_path("Up_Attack_Frame_2.png"),
			player_attack_up_textures_path("Up_Attack_Frame_3.png"),
			player_attack_up_textures_path("Up_Attack_Frame_4.png"),
			player_attack_up_textures_path("Up_Attack_Frame_5.png"),
			player_attack_up_textures_path("Up_Attack_Frame_6.png"),
			player_attack_up_textures_path("Up_Attack_Frame_7.png"),

			attack_slash_textures_path("Slash_Frame_1.png"),
			attack_slash_textures_path("Slash_Frame_2.png"),
			attack_slash_textures_path("Slash_Frame_3.png"),
			attack_slash_textures_path("Slash_Frame_4.png"),
			attack_slash_textures_path("Slash_Frame_5.png"),

			textures_path("orion.png"),
			textures_path("starseeker_statue.png"),

			sparkle_textures_path("Sparkle_Frame_1.png"),
			sparkle_textures_path("Sparkle_Frame_2.png"),
			sparkle_textures_path("Sparkle_Frame_3.png"),
			sparkle_textures_path("Sparkle_Frame_4.png"),
			sparkle_textures_path("Sparkle_Frame_5.png"),
			sparkle_textures_path("Sparkle_Frame_6.png"),
			sparkle_textures_path("Sparkle_Frame_7.png"),
			sparkle_textures_path("Sparkle_Frame_8.png"),
			sparkle_textures_path("Sparkle_Frame_9.png"),
			sparkle_textures_path("Sparkle_Frame_10.png"),
			sparkle_textures_path("Sparkle_Frame_11.png"),
			sparkle_textures_path("Sparkle_Frame_12.png"),
			sparkle_textures_path("Sparkle_Frame_13.png"),

			textures_path("leo.png"),
			textures_path("apothecary.png"),
			textures_path("venus.png"),
			textures_path("rope.png"),

			evil_bird_fly_textures_path("Evil_Bird_1.png"),
			evil_bird_fly_textures_path("Evil_Bird_2.png"),
			evil_bird_fly_textures_path("Evil_Bird_3.png"),
			evil_bird_fly_textures_path("Evil_Bird_4.png"),
			evil_bird_fly_textures_path("Evil_Bird_5.png"),
			evil_bird_fly_textures_path("Evil_Bird_6.png"),
			evil_bird_fly_textures_path("Evil_Bird_7.png"),
			evil_bird_fly_textures_path("Evil_Bird_8.png"),
			evil_bird_fly_textures_path("Evil_Bird_9.png"),
			evil_bird_fly_textures_path("Evil_Bird_10.png"),
			evil_bird_fly_textures_path("Evil_Bird_11.png"),
			evil_bird_fly_textures_path("Evil_Bird_12.png"),

			textures_path("egg.png"),

			textures_path("ascension_title.png"),
			textures_path("inn_keeper.png"),
			textures_path("help_screen.png"),
      
			textures_path("bg_central_no_islands.jpg"),
			textures_path("vase.png"),
			textures_path("bush.png"),
			textures_path("coins.png"),

			textures_path("btn_overlay.png"),
			sewer_slime_textures_path("Sewer_Slime_1.png"),
			sewer_slime_textures_path("Sewer_Slime_2.png"),
			sewer_slime_textures_path("Sewer_Slime_3.png"),
			sewer_slime_textures_path("Sewer_Slime_4.png"),
			sewer_slime_textures_path("Sewer_Slime_5.png"),
			sewer_slime_textures_path("Sewer_Slime_6.png"),
			sewer_slime_textures_path("Sewer_Slime_7.png"),
			sewer_slime_textures_path("Sewer_Slime_8.png"),
			sewer_slime_textures_path("Sewer_Slime_9.png"),
			slime_bounce_textures_path("Slime_Bounce_1.png"),
			slime_bounce_textures_path("Slime_Bounce_2.png"),
			slime_bounce_textures_path("Slime_Bounce_3.png"),
			slime_bounce_textures_path("Slime_Bounce_4.png"),
			slime_bounce_textures_path("Slime_Bounce_5.png"),
			slime_bounce_textures_path("Slime_Bounce_6.png"),
			slime_bounce_textures_path("Slime_Bounce_7.png"),
			slime_bounce_textures_path("Slime_Bounce_8.png"),
			slime_bounce_textures_path("Slime_Bounce_9.png"),
			slime_bounce_textures_path("Slime_Bounce_10.png"),
			slime_bounce_textures_path("Slime_Bounce_11.png"),
			
			slime_bounce_textures_path("Slime_Bounce_12.png"),
			slime_bounce_textures_path("Slime_Bounce_13.png"),
			slime_bounce_textures_path("Slime_Bounce_14.png"),
			textures_path("Starseeker_Captain.png"),
			textures_path("Standing_Starseeker.png"),
			textures_path("Mom.png"),
			textures_path("King.png"),
			textures_path("Guard_Captain.png"),
			textures_path("Guard.png"),
			textures_path("Gossiper_1.png"),
			textures_path("Gossiper_2.png"),
			textures_path("Child.png"),
			textures_path("Astronomer.png"),
			textures_path("Aristocrat_1.png"),
			textures_path("Aristocrat_2.png"),
			guard_run_textures_path("Guard_Run_1.png"),
			guard_run_textures_path("Guard_Run_2.png"),
			guard_run_textures_path("Guard_Run_3.png"),
			guard_run_textures_path("Guard_Run_4.png"),
			guard_run_textures_path("Guard_Run_5.png"),
			guard_run_textures_path("Guard_Run_6.png"),
			guard_run_textures_path("Guard_Run_7.png"),
			guard_run_textures_path("Guard_Run_8.png"),
			guard_run_textures_path("Guard_Run_9.png"),
			guard_run_textures_path("Guard_Run_10.png"),
			guard_captain_run_textures_path("Guard_Captain_1.png"),
			guard_captain_run_textures_path("Guard_Captain_2.png"),
			guard_captain_run_textures_path("Guard_Captain_3.png"),
			guard_captain_run_textures_path("Guard_Captain_4.png"),
			guard_captain_run_textures_path("Guard_Captain_5.png"),
			guard_captain_run_textures_path("Guard_Captain_6.png"),
			guard_captain_run_textures_path("Guard_Captain_7.png"),
			guard_captain_run_textures_path("Guard_Captain_8.png"),
			guard_captain_run_textures_path("Guard_Captain_9.png"),
			guard_captain_run_textures_path("Guard_Captain_10.png"),
			sleeping_starseeker_textures_path("Sleeping_1.png"),
			sleeping_starseeker_textures_path("Sleeping_2.png"),
			sleeping_starseeker_textures_path("Sleeping_3.png"),
			sleeping_starseeker_textures_path("Sleeping_4.png"),
			sleeping_starseeker_textures_path("Sleeping_5.png"),
			sleeping_starseeker_textures_path("Sleeping_6.png"),
			sleeping_starseeker_textures_path("Sleeping_7.png"),
			sleeping_starseeker_textures_path("Sleeping_8.png"),
			sleeping_starseeker_textures_path("Sleeping_9.png"),
			sleeping_starseeker_textures_path("Sleeping_10.png"),
			sleeping_starseeker_textures_path("Sleeping_11.png"),
			sleeping_starseeker_textures_path("Sleeping_12.png"),
			sleeping_starseeker_textures_path("Sleeping_13.png"),
			sleeping_starseeker_textures_path("Sleeping_14.png"),
			sleeping_starseeker_textures_path("Sleeping_15.png"),
			sleeping_starseeker_textures_path("Sleeping_16.png"),
			sleeping_starseeker_textures_path("Sleeping_17.png"),
			sleeping_starseeker_textures_path("Sleeping_18.png"),
			sleeping_starseeker_textures_path("Sleeping_19.png")

	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("chicken"),
		shader_path("textured"),
		shader_path("wind"),
		shader_path("wind_grass")};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	bool fontInit(const std::string& font_filename, unsigned int font_default_size);
	bool initScreenSpace();

	void updateScreenSpace();

	void renderButton(Button button);

	void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans);
	void drawSpriteToScreen(const TEXTURE_ASSET_ID texture, vec2 position, vec2 scale, const glm::mat4& trans);

	void drawDialogue(const std::string& dialogue, const std::string& speaker, bool conditional);

	void drawMap();
	void drawQuests();

	void drawTitle();
	void drawHelpScreen();

	double clockToMilliseconds(clock_t ticks);

private:

	int frames = 0;
	clock_t delta_t = 0;
	std::string frames_string = "";
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();

	vec2 halignText(std::string text, float scale, vec2 coords);

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;

	//vao
	GLuint vao;

	// fonts
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;

	GLuint m_screen_shaderProgram;
	GLuint m_screen_VAO;
	GLuint m_screen_VBO;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);


