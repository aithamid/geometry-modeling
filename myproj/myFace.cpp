#include "myFace.h"

#include <iostream>

#include "myvector3d.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <GL/glew.h>

myFace::myFace(void)
{
	adjacent_halfedge = NULL;
	normal = new myVector3D(1.0, 1.0, 1.0);
}

myFace::~myFace(void)
{
	if (normal) delete normal;
}

void myFace::computeNormal()
{
    auto* p1 = adjacent_halfedge->source->point;
    auto* p2 = adjacent_halfedge->prev->source->point;
    auto* p3 = (adjacent_halfedge->next)->source->point;

    auto vect1 = *p1 - *p3;
    auto vect2 = *p1 - *p2;

    myVector3D tmp = vect1.crossproduct(vect2);
    tmp.normalize();
    *normal = tmp;
}

bool myFace::checkAllEdgespartofFace()
{
    auto * current = adjacent_halfedge;
    do
    {
        if(current->adjacent_face != this)
            return false;
        current = current->next;
    } while (current != adjacent_halfedge);
    return true;
}