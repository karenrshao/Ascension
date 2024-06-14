#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

class QuestSystem
{
public:
    QuestSystem();

    void init(Entity& manager);

    void step(float elapsed_ms);

    void checkQuests();
private:
    Entity manager;

    QuestRegistry quests;
};