#pragma once
#include "mypoint3d.h"

class myHalfedge;
class myVector3D;

class myVertex
{
public:
	myPoint3D *point;
	myHalfedge *originof;
	char label;

	int index;  //use as you wish.

	myVector3D *normal;

	void computeNormal();
	bool checkPoint();
	myVertex(void);
	~myVertex(void);
};
