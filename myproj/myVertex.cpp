#include "myVertex.h"

#include <iostream>

#include "myvector3d.h"
#include "myHalfedge.h"
#include "myFace.h"

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0,1.0,1.0);
	label = 'p';
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}

void myVertex::computeNormal()
{
	auto* start = originof;
	auto* current = start;
	myVector3D tmp(0,0,0);
	int i = 0;
	do
	{
		if(current->twin != nullptr)
		{
			i++;
			tmp += *current->adjacent_face->normal;
			current = current->twin->next;
		}
	} while (current != start);
	if(i > 0)
	{
		tmp = tmp / static_cast<double>(i);
	}
	tmp.normalize();
	normal = new myVector3D(tmp);
}

/*
 * This function check of there is a point in the vertex or it's empty
 * Ii return a boolean value
 * false : error
 * true : good
 */
bool myVertex::checkPoint()
{
	if(point == nullptr)
		return false; 
	return true;
}