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
	int max = halfedges.size();
	int count = 0;
	for(int i = 0; i < max; ++i)
	{
		if(halfedges[i]->twin == nullptr)
		{
			for (int j = 0; j < max; ++j)
			{
				if (halfedges[j]->twin == nullptr && i != j)
				{
					if (
						halfedges[i]->source->point->X == halfedges[j]->next->source->point->X && 
						halfedges[i]->source->point->Y == halfedges[j]->next->source->point->Y && 
						halfedges[i]->source->point->Z == halfedges[j]->next->source->point->Z && 
						halfedges[i]->next->source->point->X == halfedges[j]->source->point->X &&
						halfedges[i]->next->source->point->Y == halfedges[j]->source->point->Y &&
						halfedges[i]->next->source->point->Z == halfedges[j]->source->point->Z
						)
					{
						halfedges[i]->twin = halfedges[j];
						halfedges[j]->twin = halfedges[i];
						count++;
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
	myPoint3D a = *(e1->source->point);
	myPoint3D b = *(e1->next->source->point);

	myPoint3D tmp;
	tmp = ((a + b)/2);
	*(p) = tmp;
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
	auto start_he = f->adjacent_halfedge;

	/**** TODO ****/
	auto * point = findCenter(f);
	auto * middle = new myVertex();
	auto * face1 = new myFace();
	auto * face2 = new myFace();
	auto * face3 = new myFace();
	auto * face4 = new myFace();

	// Middle Edge points
	auto * north = new myVertex();
	auto * south = new myVertex();
	auto * west = new myVertex();
	auto * east = new myVertex();

	middle->point = new myPoint3D();
	north->point = new myPoint3D();
	south->point = new myPoint3D();
	west->point = new myPoint3D();
	east->point = new myPoint3D();

	middle->label = 'f';
	north->label = 'e';
	south->label = 'e';
	west->label = 'e';
	east->label = 'e';

	middle->point = point;
	splitEdge(start_he, south->point);
	splitEdge(start_he->next, east->point);
	splitEdge(start_he->next->next, north->point);
	splitEdge(start_he->next->next->next, west->point);


	
	vertices.push_back(middle);

	/** Quad 1 **/
	auto * he1_1 = new myHalfedge();
	auto * he1_2 = new myHalfedge();
	auto * he1_3 = new myHalfedge();
	auto * he1_4 = new myHalfedge();

	he1_1->source = start_he->source;
	he1_2->source = south;
	he1_3->source = middle;
	he1_4->source = west;

	he1_1->next = he1_2;
	he1_2->next = he1_3;
	he1_3->next = he1_4;
	he1_4->next = he1_1;

	he1_1->prev = he1_4;
	he1_2->prev = he1_1;
	he1_3->prev = he1_2;
	he1_4->prev = he1_3;

	he1_1->adjacent_face = face1;
	he1_2->adjacent_face = face1;
	he1_3->adjacent_face = face1;
	he1_4->adjacent_face = face1;

	face1->adjacent_halfedge = he1_1;

	south->originof = he1_2;
	west->originof = he1_4;
	middle->originof = he1_3;
	start_he->source->originof = he1_1;


	//// Quad 2
	auto * he2_1 = new myHalfedge();
	auto * he2_2 = new myHalfedge();
	auto * he2_3 = new myHalfedge();
	auto * he2_4 = new myHalfedge();

	he2_1->source = south;
	he2_2->source = start_he->next->source;
	he2_3->source = east;
	he2_4->source = middle;

	he2_1->next = he2_2;
	he2_2->next = he2_3;
	he2_3->next = he2_4;
	he2_4->next = he2_1;

	he2_1->prev = he2_4;
	he2_2->prev = he2_1;
	he2_3->prev = he2_2;
	he2_4->prev = he2_3;

	he2_1->adjacent_face = face2;
	he2_2->adjacent_face = face2;
	he2_3->adjacent_face = face2;
	he2_4->adjacent_face = face2;

	face2->adjacent_halfedge = he2_1;

	south->originof = he2_1;
	east->originof = he2_3;
	middle->originof = he2_4;
	start_he->next->source->originof = he2_2;

	//// Quad 3
	auto * he3_1 = new myHalfedge();
	auto * he3_2 = new myHalfedge();
	auto * he3_3 = new myHalfedge();
	auto * he3_4 = new myHalfedge();

	he3_1->source = middle;
	he3_2->source = east;
	he3_3->source = start_he->next->next->source;
	he3_4->source = north;

	he3_1->next = he3_2;
	he3_2->next = he3_3;
	he3_3->next = he3_4;
	he3_4->next = he3_1;

	he3_1->prev = he3_4;
	he3_2->prev = he3_1;
	he3_3->prev = he3_2;
	he3_4->prev = he3_3;

	he3_1->adjacent_face = face3;
	he3_2->adjacent_face = face3;
	he3_3->adjacent_face = face3;
	he3_4->adjacent_face = face3;

	face3->adjacent_halfedge = he3_1;

	north->originof = he3_4;
	east->originof = he3_2;
	middle->originof = he3_1;
	start_he->next->next->source->originof = he3_3;

	//// Quad 4
	auto * he4_1 = new myHalfedge();
	auto * he4_2 = new myHalfedge();
	auto * he4_3 = new myHalfedge();
	auto * he4_4 = new myHalfedge();

	he4_1->source = west;
	he4_2->source = middle;
	he4_3->source = north;
	he4_4->source = start_he->next->next->next->source;

	he4_1->next = he4_2;
	he4_2->next = he4_3;
	he4_3->next = he4_4;
	he4_4->next = he4_1;

	he4_1->prev = he4_4;
	he4_2->prev = he4_1;
	he4_3->prev = he4_2;
	he4_4->prev = he4_3;

	he4_1->adjacent_face = face4;
	he4_2->adjacent_face = face4;
	he4_3->adjacent_face = face4;
	he4_4->adjacent_face = face4;

	face4->adjacent_halfedge = he4_1;

	north->originof = he4_3;
	west->originof = he4_1;
	middle->originof = he4_2;
	start_he->next->next->next->source->originof = he4_4;

	he1_2->twin = he2_4;
	he2_3->twin = he3_1;
	he3_4->twin = he4_2;
	he1_3->twin = he4_1;

	he2_4->twin = he1_2;
	he3_1->twin = he2_3;
	he4_2->twin = he3_4;
	he4_1->twin = he1_3;


	vertices.push_back(south);
	vertices.push_back(west);
	vertices.push_back(east);
	vertices.push_back(north);
	vertices.push_back(middle);
	faces.push_back(face1);
	faces.push_back(face2);
	faces.push_back(face3);
	faces.push_back(face4);
	halfedges.push_back(he1_1);
	halfedges.push_back(he1_2);
	halfedges.push_back(he1_3);
	halfedges.push_back(he1_4);
	halfedges.push_back(he2_1);
	halfedges.push_back(he2_2);
	halfedges.push_back(he2_3);
	halfedges.push_back(he2_4);
	halfedges.push_back(he3_1);
	halfedges.push_back(he3_2);
	halfedges.push_back(he3_3);
	halfedges.push_back(he3_4);
	halfedges.push_back(he4_1);
	halfedges.push_back(he4_2);
	halfedges.push_back(he4_3);
	halfedges.push_back(he4_4);

	halfedges.erase(remove(halfedges.begin(), halfedges.end(), start_he), halfedges.end());
	halfedges.erase(remove(halfedges.begin(), halfedges.end(), start_he->next), halfedges.end());
	halfedges.erase(remove(halfedges.begin(), halfedges.end(), start_he->next->next), halfedges.end());
	halfedges.erase(remove(halfedges.begin(), halfedges.end(), start_he->next->next->next), halfedges.end());
}

std::map<myVertex*, myPoint3D> myMesh::newedge_catmull()
{
	std::map<myVertex *, myPoint3D> dictionary;
	for(auto * edgepoint : vertices)
	{
		if (edgepoint->label == 'e')
		{
			auto* start = edgepoint->originof;
			auto* current = start;
			int i = 0;
			myPoint3D tmp(0, 0, 0);
			do
			{
				i++;
				tmp += *(current->next->source->point);
				if (current->twin == nullptr)
					break;
				current = current->twin->next;
			} while (start != current);
			tmp /= i;
			dictionary[edgepoint] = tmp;
		}
	}
	return dictionary;
}

std::map<myVertex*, myPoint3D> myMesh::newpoint_catmull()
{
	std::map<myVertex*, myPoint3D> dictionary;
	for (auto* edgepoint : vertices)
	{
		if (edgepoint->label == 'p')
		{
			myPoint3D pnew(0, 0, 0);
			myPoint3D Q(0, 0, 0);
			myPoint3D R(0, 0, 0);
			myPoint3D S(0, 0, 0);

			auto* start = edgepoint->originof;
			auto* current = start;
			int n = 0;
			int i = 0;

			// Calcul Q
			do
			{
				n++;
				if (current->next->next->source->label == 'f')
				{
					i++;
					Q += *(current->next->next->source->point);
					if (current->twin == nullptr)
						break;
					current = current->twin->next;
				}
			} while (start != current);
			Q /= i;

			current = start;
			// Calcul R
			i = 0;
			do
			{
				if (current->next->source->label == 'e')
				{
					i++;
					R += *(current->next->source->point);
					if (current->twin == nullptr)
						break;
					current = current->twin->next;
				}
			} while (start != current);
			R /= i;

			S = *(edgepoint->point);

			dictionary[edgepoint] = (Q + (R * 2) + S * (n - 3)) / n;
		}
	}
	return dictionary;
}


void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
	vector<myFace*> todelete;
	vector<myHalfedge*> he_delete;
	vector<myFace*> tochange;

	for (auto face : faces)
	{
		tochange.push_back(face);
	}

	for(auto face : tochange)
	{
		if(face != nullptr)
		{
			myPoint3D* tmp;
			splitFaceQUADS(face, tmp);
			todelete.push_back(face);
		}
	}
	for(auto face : todelete)
	{
		faces.erase(remove(faces.begin(), faces.end(), face), faces.end());
	}
	findtwins();

	std::map<myVertex*, myPoint3D> edges = newedge_catmull();
	
	std::map<myVertex*, myPoint3D> points = newpoint_catmull();

	for(auto pair : edges)
	{
		*(pair.first->point) = pair.second;
	}

	for (auto pair : points)
	{
		*(pair.first->point) = pair.second;
	}

	edges.clear();
	points.clear();

	// reset edge labeling 
	for(auto * v : vertices)
	{
		v->label = 'p'; 
	}
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
	//while(!all_triangles())
	//{
		for (auto* face : faces)
		{
			if (!check_triangle(face))
				triangulate(face);
		}
		findtwins();
	//}
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
