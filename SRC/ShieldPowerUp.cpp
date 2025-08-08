#include "ShieldPowerUp.h"
#include "GameWorld.h"

ShieldPowerUp::ShieldPowerUp() : GameObject("Shield")
{
}

ShieldPowerUp::~ShieldPowerUp()
{
}

void ShieldPowerUp::Collision(shared_ptr<GameObject> other_object)
{
	if (other_object->GetType() == GameObjectType("Spaceship"))
	{
		mWorld->FlagForRemoval(GetThisPtr());
	}
}