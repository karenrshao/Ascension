#pragma once

#include <vector>
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "common.hpp"

class AnimationSystem
{
public:
    AnimationSystem() {

    }

    void step(float elapsed_ms, RenderSystem& renderer);
    void handleSparkleAnimation(float elapsed_ms, RenderSystem& renderer, ComponentContainer<SparkleAnimation>& sparkleAnimationContainer, ComponentContainer<RenderRequest>& renderRequestsContainer);
    void handleRunAnimation(float elapsed_ms, RenderSystem& renderer, ComponentContainer<RunAnimation>& runAnimationContainer, ComponentContainer<RenderRequest>& renderRequestsContainer);
    void handleSleepAnimation(float elapsed_ms, RenderSystem& renderer, ComponentContainer<SleepAnimation>& sleepAnimationContainer, ComponentContainer<RenderRequest>& renderRequestsContainer);

};