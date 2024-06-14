
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"
#include "animation_system.hpp"
#include "components.hpp"
#include "save_system.hpp"

#include <nlohmann/json.hpp>

using Clock = std::chrono::high_resolution_clock;
using json = nlohmann::json;

QuestRegistry questRegistry;

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem aiSystem;
	AnimationSystem animationSystem;

	LevelRegistry levels = LevelRegistry();

	// Initializing window
	GLFWwindow* window = world.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}


	// initialize the main systems
	renderer.init(window);
	world.init(&renderer, levels);
	renderer.updateScreenSpace();

	// matrix transformation
	glm::mat4 trans = glm::mat4(1.0f);

	// matrix rotation
	trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, glm::vec3(0.25, 0.25, 1.0));

	// variable timestep loop
	auto t = Clock::now();
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;	

		int i = 0;

		world.step(elapsed_ms);
		physics.step(elapsed_ms);
		world.handle_collisions();
		aiSystem.step(elapsed_ms, &renderer);
		animationSystem.step(elapsed_ms, renderer);
		renderer.draw();		
	}

	return EXIT_SUCCESS;
}
