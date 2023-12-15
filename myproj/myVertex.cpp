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
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}

void myVertex::computeNormal()
{
	const auto* start = originof->adjacent_face->adjacent_halfedge;
	auto* current = start;
	myVector3D tmp(0,0,0);
	int i = 0;
	do
	{
		i++;
		tmp.dX += current->adjacent_face->normal->dX;
		tmp.dY += current->adjacent_face->normal->dY;
		tmp.dZ += current->adjacent_face->normal->dZ;
		current = current->twin->next;
	} while (current != start);
	std::cout << "Number of faces : " << i << std::endl;

	tmp.normalize();
	normal = new myVector3D(tmp.dX, tmp.dY, tmp.dZ);
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