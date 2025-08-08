#include "ExtraLife.h"
#include "GameWorld.h"

ExtraLife::ExtraLife() : GameObject("ExtraLife")
{
}

ExtraLife::~ExtraLife()
{
}

// When a collision occurs with the spaceship, mark this object for removal
void ExtraLife::Collision(shared_ptr<GameObject> other_object)
{
	if(other_object->GetType() == GameObjectType("Spaceship"))
	{
		mWorld->FlagForRemoval(GetThisPtr());
	}
}