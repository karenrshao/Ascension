#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<GameManager> gameManagers;
	ComponentContainer<LevelElement> levelElements;
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<BasicTimer> basicTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collider> colliders;
	ComponentContainer<Collision> collisions;

	ComponentContainer<Physics> physicsObjs;
	ComponentContainer<Gravity> gravityEntities;

	ComponentContainer<Speaker> speakers;
	ComponentContainer<Mob> mobs;
	ComponentContainer<Player> players;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<Patrol> patrols;
	ComponentContainer<Boss> boss;
	ComponentContainer<Projectile> projectiles;

	ComponentContainer<Stealth> stealth;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Eatable> eatables;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Solid> solids;
	ComponentContainer<Particle> particles;

	ComponentContainer<Item> items;
	ComponentContainer<Interactable> interactables;

	ComponentContainer<Breakable> breakables;
	ComponentContainer<Summonable> summonables;

	ComponentContainer<Transition> transitions;
	ComponentContainer<drawHelp> drawHelps;
	ComponentContainer<drawInitial> drawInits;
	ComponentContainer<Invisible> invisibles;

	ComponentContainer<Camera> cameras;
	ComponentContainer<CameraTether> cameraTethers;

	ComponentContainer<Background> backgrounds;
	ComponentContainer<BackgroundDecoration> backgroundDecors;
	ComponentContainer<BackLevelObj> backLevelObjs;
	ComponentContainer<ForeLevelObj> foreLevelObjs;
	ComponentContainer<ForegroundDecoration> foregroundDecors;
	ComponentContainer<WorldUI> worldUIelems;

	ComponentContainer<RunAnimation> runAnimations;
	ComponentContainer<JumpUpAnimation> jumpUpAnimations;
	ComponentContainer<DeathAnimation> deathAnimations;
	ComponentContainer<InvincibilityTimer> invincibilityTimers;
	ComponentContainer<DamageTimer> dmgTimers; 
	ComponentContainer<AttackAnimation> attackAnimations;
	ComponentContainer<AttackUpAnimation> attackUpAnimations;
	ComponentContainer<SlashAnimation> slashAnimations;
	ComponentContainer<SparkleAnimation> sparkleAnimations;

	ComponentContainer<Quest> quests;
	ComponentContainer<QuestManager> questmanagers;

	ComponentContainer<Button> buttons;

	ComponentContainer<SleepAnimation> sleepAnimations;
	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&gameManagers);
		registry_list.push_back(&levelElements);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&basicTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&colliders);
		registry_list.push_back(&collisions);
		
		registry_list.push_back(&physicsObjs);
		registry_list.push_back(&gravityEntities);

		registry_list.push_back(&speakers);
		registry_list.push_back(&mobs);
		registry_list.push_back(&players);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&boss);
		registry_list.push_back(&patrols);
		registry_list.push_back(&projectiles);

		registry_list.push_back(&stealth);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&eatables);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&solids);
		registry_list.push_back(&particles);

		registry_list.push_back(&items);
		registry_list.push_back(&interactables);

		registry_list.push_back(&breakables);
		registry_list.push_back(&summonables);

		registry_list.push_back(&transitions);
		registry_list.push_back(&drawHelps);
		registry_list.push_back(&drawInits);
		registry_list.push_back(&invisibles);

		registry_list.push_back(&cameras);
		registry_list.push_back(&cameraTethers);

		registry_list.push_back(&backgrounds);
		registry_list.push_back(&backgroundDecors);
		registry_list.push_back(&backLevelObjs);
		registry_list.push_back(&foreLevelObjs);
		registry_list.push_back(&foregroundDecors);
		registry_list.push_back(&worldUIelems);

		registry_list.push_back(&runAnimations);
		registry_list.push_back(&jumpUpAnimations);
		registry_list.push_back(&deathAnimations);
		registry_list.push_back(&invincibilityTimers);
		registry_list.push_back(&dmgTimers);
		registry_list.push_back(&attackAnimations);
		registry_list.push_back(&attackUpAnimations);
		registry_list.push_back(&slashAnimations);
		registry_list.push_back(&sparkleAnimations);

		registry_list.push_back(&quests);
		registry_list.push_back(&questmanagers);

		registry_list.push_back(&buttons);
		registry_list.push_back(&sleepAnimations);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;