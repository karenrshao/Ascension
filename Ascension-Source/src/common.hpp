#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string player_run_textures_path(const std::string& name) { return data_path() + "/textures/player_run/" + std::string(name); };
inline std::string player_jump_textures_path(const std::string& name) { return data_path() + "/textures/player_jump/" + std::string(name); };
inline std::string player_death_textures_path(const std::string& name) { return data_path() + "/textures/player_death/" + std::string(name); };
inline std::string player_attack_textures_path(const std::string& name) { return data_path() + "/textures/player_attack/" + std::string(name); };
inline std::string player_attack_up_textures_path(const std::string& name) { return data_path() + "/textures/player_up_attack/" + std::string(name); };
inline std::string attack_slash_textures_path(const std::string& name) { return data_path() + "/textures/attack_slash/" + std::string(name); };
inline std::string evil_tree_run_textures_path(const std::string& name) { return data_path() + "/textures/evil_tree_run/" + std::string(name); };
inline std::string sparkle_textures_path(const std::string& name) { return data_path() + "/textures/item_sparkle/" + std::string(name); };
inline std::string evil_bird_fly_textures_path(const std::string& name) { return data_path() + "/textures/evil_bird_fly/" + std::string(name); };
inline std::string slime_bounce_textures_path(const std::string& name) { return data_path() + "/textures/slime_bounce/" + std::string(name); };
inline std::string sewer_slime_textures_path(const std::string& name) { return data_path() + "/textures/sewer_slime/" + std::string(name); };
inline std::string guard_run_textures_path(const std::string& name) { return data_path() + "/textures/guard_run/" + std::string(name); };
inline std::string guard_captain_run_textures_path(const std::string& name) { return data_path() + "/textures/guard_captain_run/" + std::string(name); };
inline std::string sleeping_starseeker_textures_path(const std::string& name) { return data_path() + "/textures/sleeping_starseeker/" + std::string(name); };

inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string sfx_audio_path(const std::string& name) { return data_path() + "/audio/sfx/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};
inline std::string level_path(const std::string& name) { return data_path() + "/levels/" + std::string(name); };
inline std::string save_path(const std::string& name) { return data_path() + "/save_data/" + std::string(name); };
inline std::string font_path(const std::string& name) { return data_path() + "/fonts/" + std::string(name); };

const int window_width_px = 1280; 
const int window_height_px = 720;
 
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
