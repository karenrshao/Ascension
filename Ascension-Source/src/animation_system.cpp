// internal
#include "animation_system.hpp"
#include "world_init.hpp"
#include <iostream>

void AnimationSystem::step(float elapsed_ms, RenderSystem& renderer) {
	auto& runAnimationContainer = registry.runAnimations;
	auto& jumpUpAnimationContainer = registry.jumpUpAnimations;
	auto& renderRequestsContainer = registry.renderRequests;
	auto& deathAnimationContainer = registry.deathAnimations;
	auto& attackAnimationContainer = registry.attackAnimations;
	auto& attackUpAnimationContainer = registry.attackUpAnimations;
	auto& sparkleAnimationContainer = registry.sparkleAnimations;
	auto& sleepAnimationContainer = registry.sleepAnimations;

	for (Entity entity : deathAnimationContainer.entities) {
		DeathAnimation& deathAnimation = deathAnimationContainer.get(entity);

		if (deathAnimation.currTime >= deathAnimation.frameSpeed) {
			//RenderRequest& request = renderRequestsContainer.get(entity);
			//request.used_texture = allAnimationsVector[deathAnimation.runFrameVectorIndex][deathAnimation.currentFrame];
			renderRequestsContainer.remove(entity);
			renderRequestsContainer.insert(entity, {
				allAnimationsVector[deathAnimation.runFrameVectorIndex][deathAnimation.currentFrame],
							EFFECT_ASSET_ID::TEXTURED,
							GEOMETRY_BUFFER_ID::SPRITE
				});

			//end of death animation cycle, we do not want it to loop
			if (deathAnimation.currentFrame + 1 >= deathAnimation.numFrames) {
				deathAnimationContainer.remove(entity);
			}
			else {
				deathAnimation.currentFrame++;
			}
			deathAnimation.currTime = 0;
		}
		else {
			deathAnimation.currTime += elapsed_ms;
		}
	}

	for (Entity entity : attackAnimationContainer.entities) {
		AttackAnimation& attackAnimation = attackAnimationContainer.get(entity);
		Motion& player_motion = registry.motions.get(registry.players.entities[0]);
		if (attackAnimation.currTime >= attackAnimation.frameSpeed) {
			//RenderRequest& request = renderRequestsContainer.get(entity);
			//request.used_texture = allAnimationsVector[attackAnimation.animationVectorIndex][attackAnimation.currentFrame];
			renderRequestsContainer.remove(entity);
			renderRequestsContainer.insert(entity, {
				allAnimationsVector[attackAnimation.animationVectorIndex][attackAnimation.currentFrame],
							EFFECT_ASSET_ID::TEXTURED,
							GEOMETRY_BUFFER_ID::SPRITE
				});
			//end of attack animation cycle, we do not want it to loop
			if (attackAnimation.currentFrame + 1 >= attackAnimation.numFrames) {
				attackAnimationContainer.remove(entity);
				// reset to standing sprite
				renderRequestsContainer.remove(entity);
				renderRequestsContainer.insert(entity, {
					TEXTURE_ASSET_ID::PLAYER,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
					});
				if (registry.players.has(entity)) {
					Player& playerStruct = registry.players.get(entity);
					playerStruct.is_up = false;
				}
			}
			else {
				attackAnimation.currentFrame++;
				if (attackAnimation.currentFrame == attackAnimation.frameToRenderSwordMesh && registry.players.has(entity)) {
					if (registry.motions.has(entity)) {
						int direction = 1;
						Motion motion = registry.motions.get(entity);
						if (player_motion.scale.x <= 0) {
							direction = -1;
						}
						Player& player = registry.players.get(entity);
						player.attack_sound = true;
						createDamage(&renderer, { motion.position.x + (motion.scale.x / 1.5),  motion.position.y + (motion.position.y/5.0)}, {150 * direction, 60}, 0);
					}
				}
				if (attackAnimation.currentFrame == attackAnimation.frameToImpulse && registry.players.has(entity)) {
					if (registry.physicsObjs.has(entity) && registry.motions.has(entity)) {
						Physics& physics = registry.physicsObjs.get(entity);
						Motion motion = registry.motions.get(entity);
						physics.velocity.x = motion.scale.x > 0 ? 500 : -500;
					}
				}
			}
			attackAnimation.currTime = 0;
		}
		else {
			attackAnimation.currTime += elapsed_ms;
		}
	}

	for (Entity entity : attackUpAnimationContainer.entities) {
		AttackUpAnimation& attackUpAnimation = attackUpAnimationContainer.get(entity);
		if (attackUpAnimation.currTime >= attackUpAnimation.frameSpeed) {
			//RenderRequest& request = renderRequestsContainer.get(entity);
			//request.used_texture = allAnimationsVector[attackUpAnimation.runFrameVectorIndex][attackUpAnimation.currentFrame];
			renderRequestsContainer.remove(entity);
			renderRequestsContainer.insert(entity, {
				allAnimationsVector[attackUpAnimation.runFrameVectorIndex][attackUpAnimation.currentFrame],
							EFFECT_ASSET_ID::TEXTURED,
							GEOMETRY_BUFFER_ID::SPRITE
				});
			//end of attack animation cycle, we do not want it to loop
			if (attackUpAnimation.currentFrame + 1 >= attackUpAnimation.numFrames) {
				attackUpAnimationContainer.remove(entity);
				// reset to standing sprite
				renderRequestsContainer.remove(entity);
				renderRequestsContainer.insert(entity, {
					TEXTURE_ASSET_ID::PLAYER,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
					});
				if (registry.players.has(entity)) {
					Player& playerStruct = registry.players.get(entity);
					playerStruct.is_up = false;
				}
			}
			else {
				attackUpAnimation.currentFrame++;
				if (attackUpAnimation.currentFrame == attackUpAnimation.frameToRenderSwordMesh && registry.players.has(entity)) {
					if (registry.motions.has(entity)) {
						Motion motion = registry.motions.get(entity);
						int direction = -1;
						if (motion.scale.x <= 0) {
							direction = 1;
						}
						Player& player = registry.players.get(entity);
						player.attack_sound = true;
						createDamage(&renderer, { motion.position.x , motion.position.y - (motion.scale.y / 2.0)}, { 80, 120*direction }, -M_PI / 2);
					}
				}
			}
			attackUpAnimation.currTime = 0;
		}
		else {
			attackUpAnimation.currTime += elapsed_ms;
		}
	}

	auto& slashAnimations = registry.slashAnimations;

	for (Entity entity : slashAnimations.entities) {
		SlashAnimation& slashAnimation = slashAnimations.get(entity);
		if (slashAnimation.currTime >= slashAnimation.frameSpeed) {
			/*RenderRequest& request = renderRequestsContainer.get(entity);
			request.used_texture = allAnimationsVector[slashAnimation.runFrameVectorIndex][slashAnimation.currentFrame];*/
			renderRequestsContainer.remove(entity);
			renderRequestsContainer.insert(entity, {
				allAnimationsVector[slashAnimation.runFrameVectorIndex][slashAnimation.currentFrame],
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
				});

			if (slashAnimation.currentFrame + 1 >= slashAnimation.numFrames) {
				// reset
				slashAnimations.remove(entity);
				renderRequestsContainer.remove(entity);
			}
			else {
				slashAnimation.currentFrame++;
			}
			slashAnimation.currTime = 0;
		}
		else {
			slashAnimation.currTime += elapsed_ms;
		}
	}

	handleRunAnimation(elapsed_ms, renderer, runAnimationContainer, renderRequestsContainer);

	handleSparkleAnimation(elapsed_ms, renderer, sparkleAnimationContainer, renderRequestsContainer);

	handleSleepAnimation(elapsed_ms, renderer, sleepAnimationContainer, renderRequestsContainer);
}

void AnimationSystem::handleSparkleAnimation(float elapsed_ms, RenderSystem& renderer, ComponentContainer<SparkleAnimation>& sparkleAnimationContainer, ComponentContainer<RenderRequest>& renderRequestsContainer) {
	for (Entity entity : sparkleAnimationContainer.entities) {
		SparkleAnimation& sparkleAnimation = sparkleAnimationContainer.get(entity);
		if (sparkleAnimation.paused) {
			sparkleAnimation.pauseTimer += elapsed_ms;
			if (sparkleAnimation.pauseTimer >= sparkleAnimation.pauseTimeMs) {
				sparkleAnimation.paused = false;
				sparkleAnimation.pauseTimer = 0;
			}
		}
		else {
			if (sparkleAnimation.currTime >= sparkleAnimation.frameSpeed) {
				//RenderRequest& request = renderRequestsContainer.get(entity);
				//request.used_texture = allAnimationsVector[sparkleAnimation.animationVectorIndex][sparkleAnimation.currentFrame];
				renderRequestsContainer.remove(entity);
				renderRequestsContainer.insert(entity, {
					allAnimationsVector[sparkleAnimation.animationVectorIndex][sparkleAnimation.currentFrame],
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
					});

				if (sparkleAnimation.currentFrame + 1 >= sparkleAnimation.numFrames) {
					sparkleAnimation.currentFrame = 0;
					sparkleAnimation.paused = true;
				}
				else {
					sparkleAnimation.currentFrame++;
				}
				sparkleAnimation.currTime = 0;
			}
			else {
				sparkleAnimation.currTime += elapsed_ms;
			}
		}
	}
}

void AnimationSystem::handleRunAnimation(float elapsed_ms, RenderSystem& renderer, ComponentContainer<RunAnimation>& runAnimationContainer, ComponentContainer<RenderRequest>& renderRequestsContainer) {
	for (Entity entity : runAnimationContainer.entities) {
		RunAnimation& runAnimation = runAnimationContainer.get(entity);
		if (runAnimation.currTime >= runAnimation.frameSpeed) {
			//RenderRequest& request = renderRequestsContainer.get(entity);
			//request.used_texture = allAnimationsVector[runAnimation.runFrameVectorIndex][runAnimation.currentFrame];
			renderRequestsContainer.remove(entity);
			renderRequestsContainer.insert(entity, {
				allAnimationsVector[runAnimation.runFrameVectorIndex][runAnimation.currentFrame],
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
				});

			if (runAnimation.currentFrame + 1 >= runAnimation.numFrames) {
				runAnimation.currentFrame = 0;
			}
			else {
				runAnimation.currentFrame++;
			}
			runAnimation.currTime = 0;
		}
		else {
			runAnimation.currTime += elapsed_ms;
		}
	}
}

void AnimationSystem::handleSleepAnimation(float elapsed_ms, RenderSystem& renderer, ComponentContainer<SleepAnimation>& sleepAnimationContainer, ComponentContainer<RenderRequest>& renderRequestsContainer) {
	for (Entity entity : sleepAnimationContainer.entities) {
		SleepAnimation& sleepAnimation = sleepAnimationContainer.get(entity);
		if (sleepAnimation.currTime >= sleepAnimation.frameSpeed) {
			//RenderRequest& request = renderRequestsContainer.get(entity);
			//request.used_texture = allAnimationsVector[runAnimation.runFrameVectorIndex][runAnimation.currentFrame];
			renderRequestsContainer.remove(entity);
			renderRequestsContainer.insert(entity, {
				allAnimationsVector[sleepAnimation.runFrameVectorIndex][sleepAnimation.currentFrame],
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
				});

			if (sleepAnimation.holdCounter < 7 && ((sleepAnimation.currentFrame == sleepAnimation.breathInHoldFrame) || (sleepAnimation.currentFrame == sleepAnimation.breathOutHoldFrame))) {
				sleepAnimation.holdCounter++;
			}
			else {
				sleepAnimation.holdCounter = 0;
				if (sleepAnimation.currentFrame + 1 >= sleepAnimation.numFrames) {
					sleepAnimation.currentFrame = 0;
				}
				else {
					sleepAnimation.currentFrame++;
				}
			}
			sleepAnimation.currTime = 0;
		}
		else {
			sleepAnimation.currTime += elapsed_ms;
		}
	}
}
