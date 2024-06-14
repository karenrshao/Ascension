// internal
#include "render_system.hpp"
#include <SDL.h>
#include <chrono> // clock()
#include "tiny_ecs_registry.hpp"

bool hasMovedX = false;

vec2 worldToScreenCoords(vec2 coord) {
	Camera& camera = registry.cameras.components[0];
	// xx = camerax - width
	// yy = cameray + height
	// drawx = cTTx - xx
	// drawy = cTTy + yy
	vec2 origin = { camera.position.x - camera.dims.x / 2, camera.position.y + camera.dims.y / 2 };
	return { camera.zoom * (coord.x - origin.x), camera.zoom * (origin.y - coord.y) };
}

vec2 RenderSystem::halignText(std::string text, float scale, vec2 coords) {
	float wOffset = 0.0;
	for (char c : text) {
		Character ch = m_ftCharacters[c];

		wOffset += scale * ch.Size.x;
		if (c == ' ' || c == '\n') {
			wOffset += scale * 0.846153 * m_ftCharacters['I'].Size.x;
		}
	}
	return coords - vec2({ wOffset / 2.0, 0. });
}

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	// rebind vao bc fontinit unbinds it
	glBindVertexArray(vao);
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	assert(registry.renderRequests.has(entity));
	RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		GLint flash_uloc = glGetUniformLocation(program, "flash");
		GLint initialHit_uloc = glGetUniformLocation(program, "initialHit");

		assert(flash_uloc >= 0);

		const int flash = registry.invincibilityTimers.has(entity) ? (registry.invincibilityTimers.get(entity).flash ? 1 : 0) : 0;

		const int initialHit = registry.invincibilityTimers.has(entity) ? ((registry.invincibilityTimers.get(entity).curr_flash_count == 0) ? 1 : 0) : 0;

		glUniform1i(flash_uloc, flash);
		glUniform1i(initialHit_uloc, initialHit);

		gl_has_errors();

	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN)
		{
			// Light up?
			GLint light_up_uloc = glGetUniformLocation(program, "light_up");
			assert(light_up_uloc >= 0);

			// !!! TODO A1: set the light_up shader variable using glUniform1i,
			// similar to the glUniform1f call below. The 1f or 1i specified the type, here a single int.
			gl_has_errors();
		}
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::WIND_GRASS) {
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");

		GLuint time_uloc = glGetUniformLocation(program, "time");
		glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));

		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	} else {
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();

	render_request.rendered = true;
}

// draw the intermediate texture to the screen, with some distortion to simulate
// wind
void RenderSystem::drawToScreen()
{
	// rebind vao bc fontinit unbinds it
	glBindVertexArray(vao);
	// Setting shaders
	// get the wind texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WIND]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint wind_program = effects[(GLuint)EFFECT_ASSET_ID::WIND];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(wind_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(wind_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(wind_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// fps counter and related fn adapted from : 
// https://stackoverflow.com/questions/28530798/how-to-make-a-basic-fps-counter
double RenderSystem::clockToMilliseconds(clock_t ticks) {
	// units/(units/time) => time (seconds) * 1000 = milliseconds
	return (ticks / (double)CLOCKS_PER_SEC) * 10000.0;
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// start time of frame
	clock_t start_time = clock();
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0.674, 0.847, 1.0 , 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
	// Draw all textured meshes that have a position and size component

	/*************************/
	// LAYERS
	for (RenderRequest& rr : registry.renderRequests.components)
	{
		rr.rendered = false;
	}
	// background
	
	for (Entity entity : registry.backgrounds.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}
	// stealth
	for (Entity entity : registry.stealth.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}
	// background decor
	for (Entity entity : registry.backgroundDecors.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}
	// level objects (doors)
	for (Entity entity : registry.backLevelObjs.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}
	// deadlies
	for (Entity entity : registry.deadlys.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}
	// player
	for (Entity entity : registry.players.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}
	// foreground obj
	for (Entity entity : registry.foreLevelObjs.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}
	// foreground decore
	for (Entity entity : registry.foregroundDecors.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}
	// world UI
	for (Entity entity : registry.worldUIelems.entities) {
		if (!registry.motions.has(entity) || 
			!registry.renderRequests.has(entity) ||
			(registry.invisibles.has(entity) && !debugging.in_debug_mode))
			continue;

		RenderRequest& rr = registry.renderRequests.get(entity);
		if (rr.rendered) continue;

		rr.rendered = true;
		
		drawTexturedMesh(entity, projection_2D);
	}

	// debug boxes
	for (Entity entity : registry.debugComponents.entities) {
		if (!registry.motions.has(entity) ||
			!registry.renderRequests.has(entity))
			continue;

		drawTexturedMesh(entity, projection_2D);
	}

	// for (Entity entity : registry.renderRequests.entities)
	// {
	// 	if (!registry.motions.has(entity) || (registry.invisibles.has(entity) && !debugging.in_debug_mode)) {
	// 		continue;
	// 	}

	// 	// Note, its not very efficient to access elements indirectly via the entity
	// 	// albeit iterating through all Sprites in sequence. A good point to optimize
	// }
	/*************************/

	// Truely render to the screen
	drawToScreen();

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// RENDER TEXT HERE!!!!! because glfwSwapBuffers clears screen 
	// renderText("TEST - text blablabla", 0.0f, 0.0f, 5.0f, glm::vec3(1.0f, 0.0f, 0.0f), glm::mat4(1.0f));
	
	// display initial movement controls

	// for (uint i = 0; i < registry.drawInits.components.size(); i++) {
	// 	Entity entity = registry.drawInits.entities[i];
	// 	float posx = registry.motions.get(entity).position.x + registry.motions.get(entity).scale.x / 2;
	// 	float posy = registry.motions.get(entity).position.y + registry.motions.get(entity).scale.y / 2;

	// 	vec2 drawPos = worldToScreenCoords(registry.motions.get(entity).position) - vec2({ 100., -32. });

	// 	if (registry.motions.get(entity).position.x == registry.drawInits.get(entity).x) { // check if x position has changed from initial
	// 		if (hasMovedX == false) {
	// 			renderText("[A][D] or arrow keys to move", drawPos.x - 64, drawPos.y, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
	// 		}
	// 	}
	// 	else {
	// 		hasMovedX = true;
	// 	}
	// 	if (registry.motions.get(entity).position.y >= registry.drawInits.get(entity).y && hasMovedX == true) { // check if y has decreased (has player jumped? may need to change if level shape goes down a lot)
	// 		renderText("[SPACE] to jump", drawPos.x, drawPos.y, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
	// 	}
	// 	else if (registry.motions.get(entity).position.y < registry.drawInits.get(entity).y && hasMovedX == true) {
	// 		registry.drawInits.remove(entity); // remove entity so help doesn't display after the first time
	// 	}
	// }

	// display text above interactable when colliding
	GameManager& gm = registry.gameManagers.components[0];
	if (gm.state == GAME_STATE::PLAY) {
		for (uint i = 0; i < registry.drawHelps.components.size(); i++) {
			Entity entity = registry.drawHelps.entities[i];
			vec2 position = worldToScreenCoords(registry.motions.get(entity).position);
			float width = 0.0;
			std::string helpText = registry.drawHelps.get(entity).text;
			vec2 centre = halignText(helpText, 0.4f, position);
			renderText(helpText, centre.x, centre.y + 48., 0.4f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
			// registry.drawHelps.remove(entity);
		}
	}

	//for (Entity entity : registry.drawEnemyHPs.entities) {
	//	if (registry.motions.has(entity) && registry.deadlys.has(entity)) {
	//		Deadly deadly = registry.deadlys.get(entity);
	//		Motion motion = registry.motions.get(entity);

	//		renderText("Lives: " + std::to_string(deadly.health), motion.position.x , motion.position.y - (motion.scale.y/2.0), 1, vec3(1, 1, 1), glm::mat4(1.0f));

	//	}
	//}


	if (gm.state == GAME_STATE::PAUSE) {
		std::string pauseStr = "GAME PAUSED";
		
		int width_px, height_px;
		glfwGetWindowSize(window, &width_px, &height_px);

		drawSpriteToScreen(TEXTURE_ASSET_ID::BUTTON_OVERLAY, {0, 0}, {width_px, height_px}, glm::mat4(1.0f));
		vec2 center = halignText(pauseStr, 1.2f, { registry.cameras.components[0].dims.x / 2, registry.cameras.components[0].dims.y * 3 / 4 });
		renderText(pauseStr, center.x + 4, center.y - 4, 1.2f, glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4(1.0f));
		renderText(pauseStr, center.x, center.y, 1.2f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
		for (Button& button : registry.buttons.components) {
			if (button.function == BUTTON_FUNCTION::RESUME_GAME || button.function == BUTTON_FUNCTION::QUIT_GAME || button.function == BUTTON_FUNCTION::DISPLAY_HELP) {
				renderButton(button);
			}
		}
		// add buttons
		//drawSpriteToScreen(TEXTURE_ASSET_ID::BREAKABLE_BOX, {registry.cameras.components[0].dims.x / 2.f - 250, registry.cameras.components[0].dims.y / 2}, {200, 100}, glm::mat4(1));
		//drawSpriteToScreen(TEXTURE_ASSET_ID::BREAKABLE_BOX, { 0, 0 }, { 200, 100 }, glm::mat4(1));

		//std::cout << "minx: " << (registry.cameras.components[0].dims.x / 2.f) - 250 - (200 / 2.f) << std::endl;
		//std::cout << "maxx: " << (registry.cameras.components[0].dims.x / 2.f) - 250 + (200 / 2.f) << std::endl;
		//std::cout << "miny: " << (registry.cameras.components[0].dims.y / 2.f) - (100 / 2.f) << std::endl;
		//std::cout << "maxy: " << (registry.cameras.components[0].dims.y / 2.f) + (100 / 2.f) << std::endl;

		//if (!registry.buttons.has(resume_game)) {
		//	//																						position	 	   + 	scale
		//	registry.buttons.insert(resume_game, Button(TEXTURE_ASSET_ID::BREAKABLE_BOX, (registry.cameras.components[0].dims.x / 2.f) - 250 - (200 / 2.f), (registry.cameras.components[0].dims.x / 2.f) - 250 + (200 / 2.f), (registry.cameras.components[0].dims.y / 2.f) - (100 / 2.f), (registry.cameras.components[0].dims.y / 2.f) + (100 / 2.f)));
		//}

		//drawSpriteToScreen(TEXTURE_ASSET_ID::BREAKABLE_BOX, {registry.cameras.components[0].dims.x / 2 + 50, registry.cameras.components[0].dims.y / 2}, {200, 100}, glm::mat4(1));
		//if (!registry.buttons.has(quit_game)) {
		//	registry.buttons.insert(quit_game, Button(TEXTURE_ASSET_ID::BREAKABLE_BOX, (registry.cameras.components[0].dims.x / 2.f) + 50 - (200 / 2.f), (registry.cameras.components[0].dims.x / 2.f) + 50 + (200 / 2.f), (registry.cameras.components[0].dims.y / 2.f) - (100 / 2.f), (registry.cameras.components[0].dims.y / 2.f) + (100 / 2.f)));
		//}

	} else if (gm.state == GAME_STATE::DIALOGUE) {
		drawDialogue(gm.dialogueText, "test", false);
	}
	else if (gm.state == GAME_STATE::MAP) {
		drawMap();
	}
	else if (gm.state == GAME_STATE::QUEST_MENU) {
		drawQuests();
	}
	else if (gm.state == GAME_STATE::MENU) {
		drawTitle();
		for (Button& button : registry.buttons.components) {
			if (button.function == BUTTON_FUNCTION::START_GAME) {
				renderButton(button);
			}
		}
	}
	else if (gm.state == GAME_STATE::HELP) {
		std::cout << "drawing help screen" << std::endl;
		drawHelpScreen();
		for (Button& button : registry.buttons.components) {
			if (button.function == BUTTON_FUNCTION::DISPLAY_HELP) {
				renderButton(button);
			}
		}
	}
	else { // PLAY
		Player& player = registry.players.get(registry.players.entities[0]);
		float start_x = 16.0;
		for (int i = 0; i < player.health; i++) {
			drawSpriteToScreen(TEXTURE_ASSET_ID::HEART, { start_x, registry.cameras.components[0].dims.y - 64. }, { 48., 48. }, glm::mat4(1.0f));
			start_x += 24;
		}
		// std::string lives_string = std::to_string(player.health);
		// renderText("HP: " + lives_string, 0.0f, registry.cameras.components[0].dims.y - 50, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
		// quest display
		QuestManager& qm = registry.questmanagers.components[0];
		float startY = registry.cameras.components[0].dims.y - 128;
		drawSpriteToScreen(TEXTURE_ASSET_ID::DIALOGUE_BOX, { 12., startY - 14}, { 140., 44. }, glm::mat4(1.0f));
		renderText("Quests [TAB]", 24.f, startY, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4(1.0f));
		for (int q : qm.openQuests) {
			if (questRegistry.quest_list[q]->stage != QUEST_STAGE::START && questRegistry.quest_list[q]->visible) {
				startY -= 36;
				std::string questName = questRegistry.quest_list[q]->name;
				vec3 renderColour = { 1.0, 1.0, 1.0 };
				// if (questRegistry.quest_list[q]->stage == QUEST_STAGE::END) renderColour = { 0.0, 1.0, 0.3 };
				renderText(questName, 16.f, startY, 0.4f, renderColour, glm::mat4(1.0f));
			}
		}	
	}

	// time to execute draw -- turn clock ticks into ms and add to running counter
	clock_t end_time = clock() - start_time;
	double ms = clockToMilliseconds(end_time);
	delta_t += ms;
	frames++;
	if (delta_t > 1000.0) {
		frames_string = std::to_string(frames);
		delta_t -= CLOCKS_PER_SEC;
		frames = 0;
	}

	// display fps counter if option turned on
	if (fps.show_fps) {
		renderText("fps: " + frames_string, registry.cameras.components[0].dims.x - 200, registry.cameras.components[0].dims.y - 50, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
		Motion& playerPos = registry.motions.get(registry.players.entities[0]);
		renderText("x: " + std::to_string(playerPos.position.x) + ", y: " + std::to_string(playerPos.position.y), registry.cameras.components[0].dims.x - 300, registry.cameras.components[0].dims.y - 100, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
	}

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

void RenderSystem::renderButton(Button button) {
	vec2 textCenter = halignText(button.label, 0.9f, {button.position.x + button.scale.x / 2, button.position.y + 0.35 * button.scale.y});
	if (button.is_mouse_over) {
		drawSpriteToScreen(TEXTURE_ASSET_ID::BUTTON_OVERLAY, button.position, button.scale, glm::mat4(1));
		renderText(button.label, textCenter.x, textCenter.y, 0.9f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
	}
	else {
		// drawSpriteToScreen(button.texture, button.position, button.scale, glm::mat4(1));
		renderText(button.label, textCenter.x, textCenter.y, 0.9f, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
	}
}

bool inRangeOfTether(CameraTether& tether, Motion& motion) {
	float halfWidth = tether.bounds.x / 2, halfHeight = tether.bounds.y / 2;
	vec2 pos = motion.position;

	return pos.x > tether.position.x - halfWidth &&
		pos.x < tether.position.x + halfWidth &&
		pos.y > tether.position.y - halfHeight &&
		pos.y < tether.position.y + halfHeight;
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	Entity player = registry.players.entities[0];
	auto& playerMotion = registry.motions.get(player);
	Camera& camera = registry.cameras.get(player);

	camera.targetPosition = playerMotion.position;
	if (registry.gameManagers.components[0].state == GAME_STATE::DIALOGUE) {
		float targetDialogueZoom = 0.8;
		if (camera.zoom >= targetDialogueZoom) {
			camera.zoom += 0.05 * (targetDialogueZoom - camera.zoom);
		}
	} else {
		if (camera.zoom <= camera.defaultZoom) {
			camera.zoom += 0.05 * (camera.defaultZoom - camera.zoom);
		}
		camera.targetPosition[0] += (playerMotion.scale[0] / abs(playerMotion.scale[0])) * camera.offsetDist;
	}
	camera.targetPosition[1] -= camera.offsetDist / 2;
	for (int i = 0; i < registry.cameraTethers.size(); i++) {
		auto& tether = registry.cameraTethers.components[i];
		if (inRangeOfTether(tether, playerMotion)) {
			camera.targetPosition = tether.position;
		}
	}

	int halfWidth = camera.zoom * camera.dims.x / 2, halfHeight = camera.zoom * camera.dims.y / 2;

	camera.targetPosition[0] = fmin(camera.targetPosition[0], camera.bounds[0] - halfWidth);
	camera.targetPosition[0] = fmax(camera.targetPosition[0], halfWidth - 32);
	camera.targetPosition[1] = fmin(camera.targetPosition[1], camera.bounds[1] - halfHeight - 16);
	camera.targetPosition[1] = fmax(camera.targetPosition[1], halfHeight + 32);

	camera.position += camera.interpSpeed * (camera.targetPosition - camera.position);

	float left = camera.position[0] - halfWidth;
	float top = camera.position[1] - halfHeight;

	gl_has_errors();
	float right = (float) camera.position[0] + halfWidth;
	float bottom = (float) camera.position[1] + halfHeight;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

void RenderSystem::drawDialogue(const std::string& dialogue, const std::string& speaker, bool conditional) {
	// draw text background

	// split the string up and render screen
	float width_bounds = 530.f, 
		box_half_width = window_width_px / 4,
		draw_x = registry.cameras.components[0].dims.x / 2.f - box_half_width, 
		draw_y = 616.f, 
		scale = 0.6f;
	float line_width = 0.f, word_width = 0.f;
	std::string line = "", next_word = "";

	// draw speaker name
	GameManager& gm = registry.gameManagers.components[0];
	if (gm.speakerName.size() > 0) {
		drawSpriteToScreen(TEXTURE_ASSET_ID::DIALOGUE_BOX, { draw_x + 24., 660 }, { 156, 48 }, glm::mat4(1.0f));
		renderText(gm.speakerName, draw_x + 40.f, 675., 0.55, glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4(1.0f));
	}

	drawSpriteToScreen(TEXTURE_ASSET_ID::DIALOGUE_BOX, { draw_x, 436 }, { 640, 240 }, glm::mat4(1.0f));
	// split line
	for (char c : dialogue) {
		Character ch = m_ftCharacters[c];

		next_word += c;
		word_width += scale * ch.Size.x;

		if (c == ' ' || c == '\n') {
			line_width += word_width + scale * 0.846153 * m_ftCharacters['I'].Size.x;
		}

		if (line_width >= width_bounds) {
			renderText(line, draw_x + 48., draw_y, scale, glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4(1.0f));

			line_width = word_width;
			draw_y -= 32.f;
			line = "";
		}

		if (c == ' ' || c == '\n') {
			line += next_word;

			word_width = 0.f;
			next_word = "";
		}
	}
	// termination, add last word and print last line.
	line_width += word_width;
	if (line_width >= width_bounds) {
		renderText(line, draw_x + 48., draw_y, scale, glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4(1.0f));

		line_width = word_width;
		draw_y -= 32.f;
		line = "";
	}
	line += next_word;
	renderText(line, draw_x + 48., draw_y, scale, glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4(1.0f));
	
	std::string optText = registry.gameManagers.components[0].optionText;
	renderText(optText, draw_x + 48., 416., 0.5, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
}

void RenderSystem::drawMap() {
	GameManager& gm = registry.gameManagers.components[0];

	vec2 drawPos; // do math here
	drawPos.x = 640. / window_width_px * registry.cameras.components[0].dims.x - gm.mapScale * gm.mapPos.x;
	drawPos.y =  - gm.mapScale * (10000. - gm.mapPos.y) + 360. / window_width_px * registry.cameras.components[0].dims.x;
	/*
	804 = 640.
	8648 = 360.

	x = 640. - gm.mapPos.x
	y = gm.mapPos.y - 1280.
	
	*/

	drawSpriteToScreen(TEXTURE_ASSET_ID::MINI_MAP, drawPos, { gm.mapScale * 10000., gm.mapScale * 10000. }, glm::mat4(1.0f));
	drawSpriteToScreen(TEXTURE_ASSET_ID::MAP_CTRLS, {32., 32.}, {96., 96.}, glm::mat4(1.0f));
}

void RenderSystem::drawQuests() {
	drawSpriteToScreen(TEXTURE_ASSET_ID::QUEST_MENU, { 0, 0 }, { registry.cameras.components[0].dims.x, registry.cameras.components[0].dims.y }, glm::mat4(1.0f));
	// quest display
	QuestManager& qm = registry.questmanagers.components[0];
	float startY = registry.cameras.components[0].dims.y - 84;
	renderText("Quests", registry.cameras.components[0].dims.x / 2 - 64, startY, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4(1.0f));

	float width_bounds = 780.f / window_width_px * registry.cameras.components[0].dims.x, 
		height_bounds = 220.f,
		title_bounds = 480.f / window_width_px * registry.cameras.components[0].dims.x,
		draw_x = registry.cameras.components[0].dims.x / 4.f - 72, 
		draw_y = 480.f, 
		scale = 0.6f, 
		progress_x = 0.66 * registry.cameras.components[0].dims.x;
	float line_width = 0.f, word_width = 0.f;
	std::string line = "", next_word = "";
	
	for (int q : qm.openQuests) {
		if (questRegistry.quest_list[q]->stage != QUEST_STAGE::START && questRegistry.quest_list[q]->visible) {
			if (draw_y <= height_bounds) {
				renderText("...", draw_x, draw_y, scale, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
			}
			std::string questName = questRegistry.quest_list[q]->name;
			std::string questDesc = questRegistry.quest_list[q]->description;

			if (questRegistry.quest_list[q]->stage == QUEST_STAGE::END) {
				renderText("Wrapping up...", progress_x, draw_y, scale, glm::vec3(0.0f, 0.8f, 0.3f), glm::mat4(1.0f));
			} else {
				renderText("In progress...", progress_x, draw_y, scale, glm::vec3(0.8f, 0.8f, 0.0f), glm::mat4(1.0f));
			}

			for (char c : questName) {
				Character ch = m_ftCharacters[c];

				next_word += c;
				word_width += scale * ch.Size.x;

				if (c == ' ' || c == '\n') {
					line_width += word_width + scale * 0.846153 * m_ftCharacters['I'].Size.x;
				}

				if (line_width >= title_bounds) {
					renderText(line, draw_x, draw_y, scale, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));

					line_width = word_width;
					draw_y -= 32.f;
					line = "";
				}

				if (c == ' ' || c == '\n') {
					line += next_word;

					word_width = 0.f;
					next_word = "";
				}
			}
			// termination, add last word and print last line.
			line_width += word_width;
			if (line_width >= title_bounds) {
				renderText(line, draw_x, draw_y, scale, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));

				line_width = word_width;
				draw_y -= 32.f;
				line = "";
			}
			line += next_word;
			renderText(line, draw_x, draw_y, scale, glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));

			draw_y -= 32.f;
			line = "";
			line_width = 0.f;
			word_width = 0.f;
			next_word = "";

			for (char c : questDesc) {
				Character ch = m_ftCharacters[c];

				next_word += c;
				word_width += 0.7f * scale * ch.Size.x;

				if (c == ' ' || c == '\n') {
					line_width += word_width + scale * 0.846153 * m_ftCharacters['I'].Size.x;
				}

				if (line_width >= width_bounds) {
					renderText(line, draw_x, draw_y, 0.7f * scale, glm::vec3(0.8f, 1.0f, 1.0f), glm::mat4(1.0f));

					line_width = word_width;
					draw_y -= 24.f;
					line = "";
				}

				if (c == ' ' || c == '\n') {
					line += next_word;

					word_width = 0.f;
					next_word = "";
				}
			}
			// termination, add last word and print last line.
			line_width += word_width;
			if (line_width >= width_bounds) {
				renderText(line, draw_x, draw_y, 0.7f * scale, glm::vec3(0.8f, 1.0f, 1.0f), glm::mat4(1.0f));

				line_width = word_width;
				draw_y -= 24.f;
				line = "";
			}
			line += next_word;
			renderText(line, draw_x, draw_y, 0.7f * scale, glm::vec3(0.8f, 1.0f, 1.0f), glm::mat4(1.0f));
			draw_y -= 44;
			line = "";
			line_width = 0.f;
			word_width = 0.f;
			next_word = "";
		}
	}
}

void RenderSystem::drawTitle() {
	drawSpriteToScreen(TEXTURE_ASSET_ID::ASCENSION_TITLE, { 0, 0 }, { registry.cameras.components[0].dims.x, registry.cameras.components[0].dims.y }, glm::mat4(1.0f));
	return;
}

void RenderSystem::drawHelpScreen() {
	drawSpriteToScreen(TEXTURE_ASSET_ID::HELP_SCREEN, { 0, 0 }, { registry.cameras.components[0].dims.x, registry.cameras.components[0].dims.y }, glm::mat4(1.0f));
	return;
}

void RenderSystem::renderText(const std::string& text, float x, float y,
	float scale, const glm::vec3& color,
	const glm::mat4& trans) {

	// activate the shaders!
	glUseProgram(m_font_shaderProgram);

	unsigned int textColor_location =
		glGetUniformLocation(
			m_font_shaderProgram,
			"textColor"
		);
	assert(textColor_location >= 0);
	glUniform3f(textColor_location, color.x, color.y, color.z);

	auto transform_location = glGetUniformLocation(
		m_font_shaderProgram,
		"transform"
	);
	assert(transform_location > -1);
	glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(trans));

	glBindVertexArray(m_font_VAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = m_ftCharacters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void RenderSystem::drawSpriteToScreen(const TEXTURE_ASSET_ID texture, vec2 position,
	vec2 scale, 
	const glm::mat4& trans) {

	auto entity = Entity();

	// activate the shaders!
	glUseProgram(m_screen_shaderProgram);

	auto transform_location = glGetUniformLocation(
		m_screen_shaderProgram,
		"transform"
	);
	assert(transform_location > -1);
	glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(trans));

	glBindVertexArray(m_font_VAO);

	float xpos = position.x;
	float ypos = position.y;

	float w = scale.x;
	float h = scale.y;

	// update VBO for each character
	float vertices[6][4] = {
		{ xpos,     ypos + h,   0.0f, 0.0f },
		{ xpos,     ypos,       0.0f, 1.0f },
		{ xpos + w, ypos,       1.0f, 1.0f },

		{ xpos,     ypos + h,   0.0f, 0.0f },
		{ xpos + w, ypos,       1.0f, 1.0f },
		{ xpos + w, ypos + h,   1.0f, 0.0f }
	};

	//registry.buttons.insert(entity, Button(texture, xpos - w / 2.f, xpos + w / 2.f, ypos - h / 2.f, ypos + h / 2.f));

	// render glyph texture over quad
	GLuint texture_id =
		texture_gl_handles[(GLuint) texture];
	glBindTexture(GL_TEXTURE_2D, texture_id);
	// std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

	// update content of VBO memory
	glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// render quad
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}