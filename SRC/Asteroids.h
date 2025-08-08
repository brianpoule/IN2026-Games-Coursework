#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"

enum GameState
{
	STATE_START_SCREEN, // The new start screen/menu
	STATE_PLAYING,      // The main game
	STATE_GAME_OVER
};

class GameObject;
class Spaceship;
class GUILabel;

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char* argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world);
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

private:
	// Game State
	GameState mState;

	// GUI Labels
	shared_ptr<GUILabel> mMenuTitleLabel;
	shared_ptr<GUILabel> mMenuStartLabel;
	shared_ptr<GUILabel> mMenuDifficultyLabel;
	shared_ptr<GUILabel> mMenuInstructionsLabel;
	shared_ptr<GUILabel> mMenuScoresLabel;

	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;

	// Game Objects
	shared_ptr<Spaceship> mSpaceship;
	list<shared_ptr<GameObject>> mObjectsToAdd;

	// Game State Variables
	uint mLevel;
	uint mAsteroidCount;

	// Private Methods
	void StartGame();
	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExtraLife();
	shared_ptr<GameObject> CreateExplosion();
	shared_ptr<GameObject> CreateShieldPowerUp();
	

	// Timer constants
	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;
	const static uint CREATE_POWERUP = 3;

	// Listeners and Keepers
	ScoreKeeper mScoreKeeper;
	Player mPlayer;
};

#endif