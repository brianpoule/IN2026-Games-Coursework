#ifndef __EXTRALIFE_H__
#define __EXTRALIFE_H__

#include "GameObject.h"

class ExtraLife : public GameObject
{
public:
	ExtraLife();
	~ExtraLife();

	void Collision(shared_ptr<GameObject> other_object);
};

#endif