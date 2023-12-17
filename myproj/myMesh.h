#pragma once

#include <map>

#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <vector>
#include <string>

class myMesh
{
public:
	std::vector<myVertex *> vertices;
	std::vector<myHalfedge *> halfedges;
	std::vector<myFace *> faces;
	std::string name;

	void checkMesh();
	bool readFile(std::string filename);
	void computeNormals();
	myPoint3D* findMiddle(myHalfedge* h);
	void simplify(myHalfedge* h);
	void simplify(int nb);
	myHalfedge* minhedges();
	void normalize();
	void findtwins();
	myPoint3D* findCenter(myFace* face);

	void subdivisionCatmullClark();

	void splitFaceTRIS(myFace *, myPoint3D *);

	void splitEdge(myHalfedge *, myPoint3D *);
	void splitFaceQUADS(myFace *, myPoint3D *);

	void triangulate();
	bool triangulate(myFace *);
	bool check_triangle(myFace*);
	bool all_triangles();
	std::map<myVertex*, myPoint3D> newedge_catmull();
	std::map<myVertex*, myPoint3D> newpoint_catmull();

	void clear();
	void test();

	myMesh(void);
	~myMesh(void);
};

