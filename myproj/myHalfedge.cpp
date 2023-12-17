#include "myHalfedge.h"

myHalfedge::myHalfedge(void)
{
	source = NULL; 
	adjacent_face = NULL; 
	next = NULL;  
	prev = NULL;  
	twin = NULL;  
}

void myHalfedge::copy(myHalfedge *ie)
{
/**** TODO ****/
}

myHalfedge::~myHalfedge(void)
{
}

bool myHalfedge::checkNextPrev()
{
	if (this->prev->next != this && this->next->prev != this)
		return false;
	return true;
}

bool myHalfedge::checkTwin()
{
	//if (twin == nullptr)
	//	return true;
	//if (this->prev->twin != twin-> && this->next->prev != this)
	//	return false;
	return true;
}

bool myHalfedge::checkLoop()
{
	int n = 50;
	int i;
	auto* current = next;
	for(i = 0; current != this && i < n; i++)
	{
		current = current->next;
	}
	if (i < n)
		return false;
	return true;
}