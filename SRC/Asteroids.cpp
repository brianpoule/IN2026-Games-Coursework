#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "ExtraLife.h" 
#include "ShieldPowerUp.h"


// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char* argv[])
	: GameSession(argc, argv)
{
	mState = STATE_START_SCREEN;
	mLevel = 0;
	mAsteroidCount = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	// Create a spaceship and add it to the world
	mGameWorld->AddObject(CreateSpaceship());
	// Create some asteroids and add them to the world
	CreateAsteroids(10);

	//Create the GUI
	CreateGUI();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	// Handle input during the main game
	if (mState == STATE_PLAYING) {
		switch (key)
		{
		case ' ':
			mSpaceship->Shoot();
			break;

		case 't':
		case 'T':
		{ // Create a local scope for variables
			float world_width = mGameDisplay->GetWidth();
			float world_height = mGameDisplay->GetHeight();
			GLVector3f random_position;
			random_position.x = (float)(rand() % (int)world_width);
			random_position.y = (float)(rand() % (int)world_height);
			mSpaceship->SetPosition(random_position);
			break;
		}

		default:
			break;
		}
	}
	// Handle input on the start screen
	else if (mState == STATE_START_SCREEN)
	{
		if (key == 's' || key == 'S')
		{
			StartGame();
		}
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	// Only handle ship movement if the game is in the playing state
	if (mState == STATE_PLAYING) {
		switch (key)
		{
			// If up arrow key is pressed start applying forward thrust
		case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
			// If left arrow key is pressed start rotating anti-clockwise
		case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
			// If right arrow key is pressed start rotating clockwise
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
			// Default case - do nothing
		default: break;
		}
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
		// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
		// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
		// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
		// Default case - do nothing
	default: break;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());

		mObjectsToAdd.push_back(explosion); // Add to our waiting list

		mAsteroidCount--;
		if (mAsteroidCount <= 0)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}

		if (object->GetType() == GameObjectType("ExtraLife"))
		{
			// Get current lives, add one, and set it back
			int lives = mPlayer.GetLives();
			mPlayer.SetLives(lives + 1);

			// Update the GUI label
			std::ostringstream msg_stream;
			msg_stream << "Lives: " << mPlayer.GetLives();
			mLivesLabel->SetText(msg_stream.str());
		}
		if (object->GetType() == GameObjectType("Shield"))
		{
			mSpaceship->ActivateShield();
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);
	}

	if (value == CREATE_POWERUP)
	{
		// Randomly choose between Extra Life and Shield
		if (rand() % 2 == 0) {
			mGameWorld->AddObject(CreateExtraLife());
		}
		else {
			mGameWorld->AddObject(CreateShieldPowerUp());
		}
		SetTimer(15000, CREATE_POWERUP); // Reset the timer
	}

}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

shared_ptr<GameObject> Asteroids::CreateExtraLife()
{
	shared_ptr<GameObject> extra_life = make_shared<ExtraLife>();
	extra_life->SetBoundingShape(make_shared<BoundingSphere>(extra_life->GetThisPtr(), 10.0f));
	// You would want to create a proper power-up animation.
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
	shared_ptr<Sprite> sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	extra_life->SetSprite(sprite);
	extra_life->SetScale(0.1f); 
	return extra_life;
}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

	// In-Game UI (Score and Lives) - Initially hidden
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mGameDisplay->GetContainer()->AddComponent(mScoreLabel, GLVector2f(0.0f, 1.0f));
	mScoreLabel->SetVisible(false); // Hide at start

	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mGameDisplay->GetContainer()->AddComponent(mLivesLabel, GLVector2f(0.0f, 0.0f));
	mLivesLabel->SetVisible(false); // Hide at start

	// Game Over Label - Initially hidden
	mGameOverLabel = make_shared<GUILabel>("GAME OVER");
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameOverLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(mGameOverLabel, GLVector2f(0.5f, 0.5f));


	// These are visible by default because the game starts in STATE_START_SCREEN

	mMenuTitleLabel = make_shared<GUILabel>("ASTEROIDS");
	mMenuTitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(mMenuTitleLabel, GLVector2f(0.5f, 0.8f));

	mMenuStartLabel = make_shared<GUILabel>("Press 'S' to Start Game");
	mMenuStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(mMenuStartLabel, GLVector2f(0.5f, 0.6f));

	mMenuDifficultyLabel = make_shared<GUILabel>("Difficulty (Not Implemented)");
	mMenuDifficultyLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(mMenuDifficultyLabel, GLVector2f(0.5f, 0.5f));

	mMenuInstructionsLabel = make_shared<GUILabel>("Instructions (Not Implemented)");
	mMenuInstructionsLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(mMenuInstructionsLabel, GLVector2f(0.5f, 0.4f));

	mMenuScoresLabel = make_shared<GUILabel>("High Scores (Not Implemented)");
	mMenuScoresLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameDisplay->GetContainer()->AddComponent(mMenuScoresLabel, GLVector2f(0.5f, 0.3f));
}

void Asteroids::StartGame()
{

	mState = STATE_PLAYING;


	mMenuTitleLabel->SetVisible(false);
	mMenuStartLabel->SetVisible(false);
	mMenuDifficultyLabel->SetVisible(false);
	mMenuInstructionsLabel->SetVisible(false);
	mMenuScoresLabel->SetVisible(false);


	mScoreLabel->SetVisible(true);
	mLivesLabel->SetVisible(true);


	mGameWorld->AddObject(CreateSpaceship());
	CreateAsteroids(10);

	SetTimer(15000, CREATE_POWERUP);
}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnWorldUpdated(GameWorld* world)
{
	// Safely add all the objects that are waiting in our temporary list
	for (shared_ptr<GameObject> new_object : mObjectsToAdd)
	{
		mGameWorld->AddObject(new_object);
	}
	// Clear the list for the next frame
	mObjectsToAdd.clear();
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0)
	{
		SetTimer(1000, CREATE_NEW_PLAYER);
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}

shared_ptr<GameObject> Asteroids::CreateShieldPowerUp()
{
	shared_ptr<GameObject> shield_powerup = make_shared<ShieldPowerUp>();
	shield_powerup->SetBoundingShape(make_shared<BoundingSphere>(shield_powerup->GetThisPtr(), 10.0f));
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
	shared_ptr<Sprite> sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	shield_powerup->SetSprite(sprite);
	shield_powerup->SetScale(0.2f); // Make it a different size
	return shield_powerup;
}




