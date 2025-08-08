#ifndef __SHIELDPOWERUP_H__
#define __SHIELDPOWERUP_H__

#include "GameObject.h"

class ShieldPowerUp : public GameObject
{
public:
	ShieldPowerUp();
	~ShieldPowerUp();

	void Collision(shared_ptr<GameObject> other_object);
};

#endif