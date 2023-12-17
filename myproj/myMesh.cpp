#include "myMesh.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"

#ifndef ANSI_COLORS_H
#define ANSI_COLORS_H

#define RED "\033[31m"     // Texte rouge
#define GREEN "\033[32m"   // Texte vert
#define RESET "\033[0m"    // Réinitialiser la couleur

#define PASSED "\033[32mPASSED\033[0m"     // Texte passed
#define FAILED "\033[31mFAILED\033[0m"   // Texte failed

#endif // ANSI_COLORS_H

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}


myMesh::~myMesh(void)
{
	/**** TODO ****/
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex *> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge *> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace *> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge *>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else cout << "Each edge has a twin!\n";
}


bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge** hedges;

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge*> twin_map;
	map<pair<int, int>, myHalfedge*>::iterator it;
	//vector<myVertex*> vertexTab;
	vector<int> nbVerInFaceTab;
	int nbVerInFace = 0;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;
		int index = 0;
		if (t == "g") {}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			cout << "v " << x << " " << y << " " << z << endl;
			myVertex* vertex = new myVertex();
			vertex->point = new myPoint3D(x, y, z);
			vertices.push_back(vertex);

		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			cout << "f";
			cout << endl;
			myFace* f = new myFace(); // allocate the new face
			f->index = index;
			index++;
			faceids.clear();

			while (myline >> u) {
				
				int faceID = atoi((u.substr(0, u.find("/"))).c_str()) -1;
			
				faceids.push_back(faceID);
					
				cout << " " << faceID << endl;
				
			}	
			if (faceids.size() < 3) {
				continue;
				// ignore degenerate faces
			}
			
			hedges = new myHalfedge * [faceids.size()]; // allocate the array for storing pointers to half-edges
			for (unsigned int i = 0; i < faceids.size(); i++) {
				hedges[i] = new myHalfedge(); // pre-allocate new half-edges
				hedges[i]->source = vertices[faceids[i]];
				
			}
			
			f->adjacent_halfedge = hedges[0]; // connect the face with incident edge

			for (unsigned int i = 0; i < faceids.size(); i++)
			{
				//next
				int iplusone = (i + 1) % faceids.size();
				//previous
				int iminusone = (i - 1 + faceids.size()) % faceids.size();

				// connect prevs, and next

				hedges[i]->next = hedges[iplusone];
				hedges[i]->prev = hedges[iminusone];
				hedges[i]->adjacent_face = f;
				hedges[i]->index = i;

				it = twin_map.find(make_pair(faceids[iplusone], faceids[i]));
				if (it == twin_map.end()) {
					twin_map[make_pair(faceids[i], faceids[iplusone])] = hedges[i];
				}
				else
				{
					hedges[i]->twin = it->second;
					it->second->twin = hedges[i];
				}
				// set originof
				
				vertices[faceids[i]]->originof = hedges[i];

				// push edges to halfedges in myMesh
				halfedges.push_back(hedges[i]);
			
			}
			// push face to faces in myMesh
			faces.push_back(f);
		}
			//nbVerInFaceTab.push_back(nbVerInFace);
			//cout << "nb: " << nbVerInFace << endl;
			//nbVerInFace = 0;
		}
	checkMesh();
	normalize();

	return true;
}

void myMesh::computeNormals()
{
	for(auto* f : faces)
	{
		f->computeNormal();
	}

	for (auto* v : vertices)
	{
		v->computeNormal();
	}

}

myPoint3D* myMesh::findMiddle(myHalfedge * h)
{
	auto* p1 = h->source->point;
	auto* p2 = h->next->source->point;
	auto* tmp = new myPoint3D((p1->X+p2->X)/2, (p1->Y + p2->Y) / 2, (p1->Z + p2->Z) / 2);
	return tmp;
}

void myMesh::simplify(int nb)
{
	for(int i = 1; i <= nb; i++)
	{
		simplify(minhedges());
	}
	cout << "Size : " << nb << endl;
}

void myMesh::findtwins()
{
	for(auto a : halfedges)
	{
		if(a->twin == nullptr)
		{
			for (auto b : halfedges)
			{
				if (b->twin == nullptr && a != b)
				{
					if (a->source->point == b->next->source->point && a->next->source->point == b->source->point)
					{
						a->twin = b;
						b->twin = a;
						break;
					}
				}
			}
		}
	}
}

myHalfedge * myMesh::minhedges()
{
	myHalfedge* min = nullptr;
	for (auto* h : halfedges)
	{
		if(min == nullptr)
			min = h;

		else if (min->source->point->dist(*(min->next->source->point)) > h->source->point->dist(*(h->next->source->point)))
		{
			min = h;
		}
	}
	return min;
}



void myMesh::simplify(myHalfedge * todelete)
{
	auto* middle = findMiddle(todelete);
	auto* vertex = new myVertex();
	vertex->originof = todelete->next;
	vertex->point = middle;
	vertex->normal = new myVector3D();
	auto* p1 = todelete->source->point;
	auto* p2 = todelete->next->source->point;

	for(auto* h : halfedges)
	{
			// Link prev and next
			if (h == todelete || (h == todelete->twin && todelete->twin != nullptr))
			{
				h->next->prev = h->prev;
				h->prev->next = h->next;

				if (h->adjacent_face->adjacent_halfedge == h)
				{
					h->adjacent_face->adjacent_halfedge = h->next;
				}

				// Check if it's a line
				if (h->adjacent_face->adjacent_halfedge == h->adjacent_face->adjacent_halfedge->next->next)
				{
					h->adjacent_face->adjacent_halfedge->twin->twin = h->adjacent_face->adjacent_halfedge->next->twin;
					h->adjacent_face->adjacent_halfedge->next->twin = h->adjacent_face->adjacent_halfedge->twin->twin;
					halfedges.erase(remove(halfedges.begin(), halfedges.end(), h->adjacent_face->adjacent_halfedge), halfedges.end());
					halfedges.erase(remove(halfedges.begin(), halfedges.end(), h->adjacent_face->adjacent_halfedge->next), halfedges.end());
					faces.erase(remove(faces.begin(), faces.end(), h->adjacent_face), faces.end());
					// Remove the two half-edges 
				}
			}
			else if (h->source->point == p1 || h->source->point == p2)
			{
				h->source = vertex;
				vertex->originof= h;
			}
	}

	vertices.erase(remove(vertices.begin(), vertices.end(), todelete->source), vertices.end());
	vertices.erase(remove(vertices.begin(), vertices.end(), todelete->next->source), vertices.end());
	vertices.push_back(vertex);
	halfedges.erase(remove(halfedges.begin(), halfedges.end(), todelete), halfedges.end());
	halfedges.erase(remove(halfedges.begin(), halfedges.end(), todelete->twin), halfedges.end());

}

void myMesh::normalize()
{
	if (vertices.size() < 1) return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}

myPoint3D* myMesh::findCenter(myFace* face)
{
	auto* current_edge = face->adjacent_halfedge;
	auto* center = new myPoint3D();
	int i = 0;
	do
	{
		i++;
		// center->operator+=(*(current_edge->source->point));

		center->X += current_edge->source->point->X;
		center->Y += current_edge->source->point->Y;
		center->Z += current_edge->source->point->Z;

		current_edge = current_edge->next;
	} while (current_edge != face->adjacent_halfedge);

	center->X /= i;
	center->Y /= i;
	center->Z /= i;

	return center;
}


void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}


void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
}

void myMesh::test()
{
		bool tmp;
		
		std::cout << "================================\n";
		std::cout << "   Mesh System Test Report\n";
		std::cout << "================================\n";

		std::cout << std::endl << "Vertices"<< std::endl;
		std::cout << std::left << std::setw(50) << "- Check if has a correct origin half-edge" << ": ";
		tmp = true;
		for (auto* v : vertices)
		{
			if (v->originof == nullptr || std::find(halfedges.begin(), halfedges.end(), v->originof) == halfedges.end())
			{
				tmp = false;
				break;
			}
		}
		std::cout << (tmp ? PASSED : FAILED) << std::endl;


		std::cout << std::endl << "Half-edges" << std::endl;
		std::cout << std::left << std::setw(50) << "- Check if has a correct source vertex" << ": ";
		tmp = true;
		for (auto* hg : halfedges)
		{
			if (hg->source == nullptr || std::find(vertices.begin(), vertices.end(), hg->source) == vertices.end())
			{
				tmp = false;
				break;
			}
		}
		std::cout << (tmp ? PASSED : FAILED) << std::endl;

		std::cout << std::left << std::setw(50) << "- Check if has a correct adjacent face" << ": ";
		tmp = true;
		for (auto* hg : halfedges)
		{
			if (hg->adjacent_face == nullptr || std::find(faces.begin(), faces.end(), hg->adjacent_face) == faces.end())
			{
				tmp = false;
				break;
			}
		}
		std::cout << (tmp ? PASSED : FAILED) << std::endl;

		std::cout << std::left << std::setw(50) << "- Check Next/Prev Link" << ": ";
		tmp = true;
		for (auto he : halfedges)
		{
			if(!(he->next->prev == he && he->prev->next == he))
			{
				tmp = false;
				break;
			}
		}
		std::cout << (tmp ? PASSED : FAILED) << std::endl;

		std::cout << std::left << std::setw(50) << "- Check Twin Link" << ": ";
		tmp = true;
		int nottwin = 0;
		for (auto it = halfedges.begin(); it != halfedges.end(); it++)
		{
			if ((*it)->twin == NULL)
			{
				tmp = false;
				nottwin++;
			}
		}
		std::cout << (tmp ? PASSED : FAILED) << " (" << halfedges.size() - nottwin<< "/"<< halfedges.size() <<")"<< std::endl;

		std::cout << std::left << std::setw(50) << "- Check Loop" << ": ";
		tmp = true;
		for(auto he : halfedges)
		{
			int i, n = 20; // "n" is a variable to be considered a loop
			auto* current = he->next;
			for (i = 0; current != he && i < n; i++)
			{
				current = current->next;
			}
			if (i > n)
			{
				tmp = false;
				break;
			}
		}
		std::cout << (tmp ? PASSED : FAILED) << std::endl;

		std::cout << std::endl << "Faces" << std::endl;
		std::cout << std::left << std::setw(50) << "- Check if all faces have an adjacent half-edge" << ": ";
		tmp = true;
		for (auto* f : faces)
		{
			if (f->adjacent_halfedge == nullptr || std::find(halfedges.begin(), halfedges.end(), f->adjacent_halfedge) == halfedges.end())
			{
				tmp = false;
				break;
			}
		}
		std::cout << (tmp ? PASSED : FAILED) << std::endl;

		std::cout << std::left << std::setw(50) << "- Check if an adjacent h-e has good adjacent face" << ": ";
		tmp = true;
		for (auto* f : faces)
		{
			if (f->adjacent_halfedge->adjacent_face != f)
			{
				tmp = false;
				break;
			}
		}
		std::cout << (tmp ? PASSED : FAILED) << std::endl;

}


void myMesh::triangulate()
{
	while(!all_triangles())
	{
		for (auto* face : faces)
		{
			if (!check_triangle(face))
				triangulate(face);
		}
		findtwins();
	}
	cout << endl;
}

//return false if already triangle, true othewise.
bool myMesh::triangulate(myFace *f)
{
	myHalfedge* start_edge = f->adjacent_halfedge;
	myHalfedge* final_edge = start_edge->next->next->next;

	// Check if the face is a triangle
	if (start_edge == final_edge) {return false;}
	
	auto current = start_edge->next;
	int nb;
	for(nb = 1; start_edge != current; nb++)
	{
		current = current->next;
	}

	auto middle = new myVertex();
	middle->point = findCenter(f);

	// here the current one is the start edge
	auto next = current;
	for(int i = 0; i < nb; i++)
	{
		auto a = current->next->source;
		auto b = current->source;
		next = current->next; // for the next face

		auto newFace = new myFace();
		auto toMiddle = new myHalfedge();
		auto fromMiddle = new myHalfedge();

		faces.push_back(newFace);
		halfedges.push_back(toMiddle);
		halfedges.push_back(fromMiddle);

		toMiddle->source = a;
		fromMiddle->source = middle;

		a->originof = toMiddle;
		b->originof = current;
		middle->originof = fromMiddle;

		current->next = toMiddle;
		toMiddle->next = fromMiddle;
		fromMiddle->next = current;

		current->prev = fromMiddle;
		toMiddle->prev = current;
		fromMiddle->prev = toMiddle;

		current->adjacent_face = newFace;
		toMiddle->adjacent_face = newFace;
		fromMiddle->adjacent_face = newFace;
		newFace->adjacent_halfedge = fromMiddle;
		current = next;
		faces.erase(remove(faces.begin(), faces.end(), f), faces.end());
	}
	vertices.push_back(middle);
	findtwins();
	return true;
}


bool myMesh::check_triangle(myFace * f)
{
	myHalfedge* start_edge = f->adjacent_halfedge;
	myHalfedge* final_edge = start_edge->next->next->next;

	// Check if the face is a triangle
	if (start_edge == final_edge) { return true; }
	return false;
}

bool myMesh::all_triangles()
{
	for(auto face : faces)
	{
		if(!check_triangle(face))
		{
			return false;
		}
	}
	return true;
}
