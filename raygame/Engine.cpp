#include "Engine.h"
#include "raylib.h"
#include "Transform2D.h"
#include "Scene.h"
#include "TestScene.h"

bool Engine::m_applicationShouldClose = false;
Scene** Engine::m_scenes = new Scene*;
ActorArray Engine::m_actorsToDelete = ActorArray();
int Engine::m_sceneCount = 0;
int Engine::m_currentSceneIndex = 0;


Engine::Engine()
{
	m_applicationShouldClose = false;
	m_scenes = new Scene*;
	m_camera = new Camera2D();
	m_currentSceneIndex = 0;
	m_sceneCount = 0;
}

void Engine::start()
{
	//Initialize window
	int screenWidth = 700;
	int screenHeight = 800;
	InitWindow(screenWidth, screenHeight, "Intro To C++");
	SetTargetFPS(0);

	//Start the scene
	m_currentSceneIndex = addScene(new TestScene());
	m_scenes[m_currentSceneIndex]->start();
}

void Engine::update(float deltaTime)
{
	//Clean up actors marked for destruction
	destroyActorsInList();

	//Update scene
	m_scenes[m_currentSceneIndex]->update(deltaTime);
	m_scenes[m_currentSceneIndex]->updateUI(deltaTime);
}

void Engine::draw()
{
	BeginDrawing();

	ClearBackground(BLACK);

	m_scenes[m_currentSceneIndex]->draw();
	m_scenes[m_currentSceneIndex]->drawUI();

	EndDrawing();
}

void Engine::end()
{
	m_scenes[m_currentSceneIndex]->end();
	CloseWindow();
}

void Engine::run()
{
	//Create window and start scene
	start();

	//Loop while the application shouldn't end
	while (!m_applicationShouldClose && !RAYLIB_H::WindowShouldClose())
	{
		//Calculate deltatime
		float deltaTime = RAYLIB_H::GetFrameTime();

		//Update scene
		update(deltaTime);

		//Draw current scene
		draw();
	}

	end();
}

Scene* Engine::getScene(int index)
{
	//Return nullptr if the scene is out of bounds
	if (index < 0 || index >= m_sceneCount)
		return nullptr;

	return m_scenes[index];
}

Scene* Engine::getCurrentScene()
{
	return m_scenes[m_currentSceneIndex];
}

int Engine::getCurrentSceneIndex()
{
	return m_currentSceneIndex;
}

int Engine::addScene(Scene* scene)
{
	//If the scene is null then return before running any other logic
	if (scene == NULL)
		return -1;
	//Create a new array with a size one greater than our old array
	Scene** temp = new Scene * [m_sceneCount + 1];
	//Copy the values from the old array to the new array
	for (int i = 0; i < m_sceneCount; i++)
	{
		temp[i] = m_scenes[i];
	}
	//Store the current index
	int currentIndex = m_sceneCount;
	//Set the last value in the new array to be the scene we want to add
	temp[currentIndex] = scene;
	//Set old array to hold the values of the new array
	m_scenes = temp;
	//Increase the scene count by one
	m_sceneCount++;
	//Return the index this scene is at
	return currentIndex;
}

void Engine::addActorToDeletionList(Actor* actor)
{
	//return if the actor is already going to be deleted
	if (m_actorsToDelete.contains(actor))
		return;

	//Add actor to deletion list
	m_actorsToDelete.addActor(actor);

	//Add all the actors children to the deletion list
	for (int i = 0; i < actor->getTransform()->getChildCount(); i++)
	{
		m_actorsToDelete.addActor(actor->getTransform()->getChildren()[i]->getOwner());
	}
}

bool Engine::removeScene(Scene* scene)
{
	//If the scene is null then return before running any other logic
	if (!scene)
		return -1;
	//Create a new array with a size one greater than our old array
	Scene** temp = new Scene * [m_sceneCount + 1];

	bool sceneRemoved = false;
	int j = 0;
	//Copy the values from the old array to the new array
	for (int i = 0; i < m_sceneCount; i++)
	{
		if (m_scenes[i] == scene)
		{
			sceneRemoved = true;
			continue;
		}
		temp[j] = m_scenes[i];
		j++;
	}
	//Store the current index
	int currentIndex = m_sceneCount;
	//Set the last value in the new array to be the scene we want to add
	temp[currentIndex] = scene;
	//Set old array to hold the values of the new array
	if (sceneRemoved)
	{
		m_scenes = temp;
		m_sceneCount--;
	}
	//Increase the scene count by one
	m_sceneCount++;
	//Return the index this scene is at
	return currentIndex;
}

void Engine::setCurrentScene(int index)
{
	//If the index is not within the range of the the array return
	if (index < 0 || index >= m_sceneCount)
		return;

	//Call end for the previous scene before changing to the new one
	if (m_scenes[m_currentSceneIndex]->getStarted())
		m_scenes[m_currentSceneIndex]->end();

	//Update the current scene index
	m_currentSceneIndex = index;
}

bool Engine::getKeyDown(int key)
{
	return RAYLIB_H::IsKeyDown((KeyboardKey)key);
}

bool Engine::getKeyPressed(int key)
{
	return RAYLIB_H::IsKeyPressed((KeyboardKey)key);
}

void Engine::destroy(Actor* actor)
{
	addActorToDeletionList(actor);
}

void Engine::destroyActorsInList()
{
	//Iterate through deletion list
	for (int i = 0; i < m_actorsToDelete.getLength(); i++)
	{
		//Remove actor from the scene
		Actor* actorToDelete = m_actorsToDelete.getActor(i);
		if (!getCurrentScene()->removeActor(actorToDelete))
			getCurrentScene()->removeUIElement(actorToDelete);

		//Call actors clean up functions
		actorToDelete->end();
		actorToDelete->onDestroy();

		//Delete the actor
		delete actorToDelete;
	}

	//Clear the array
	m_actorsToDelete = ActorArray();
}

void Engine::CloseApplication()
{
	Engine::m_applicationShouldClose = true;
}