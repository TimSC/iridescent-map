//2D Triangle-Triangle collisions in C++

#include <vector>
#include <iostream>
#include <stdexcept>
#include "TriTri2d.h"
using namespace std;

inline double Det2D(const TriPoint &p1, const TriPoint &p2, const TriPoint &p3) 
{
	return +p1.first*(p2.second-p3.second)
		+p2.first*(p3.second-p1.second)
		+p3.first*(p1.second-p2.second);
}

bool BoundaryCollideChk(const TriPoint &p1, const TriPoint &p2, const TriPoint &p3, double eps)
{
	return Det2D(p1, p2, p3) < eps;
}

bool BoundaryDoesntCollideChk(const TriPoint &p1, const TriPoint &p2, const TriPoint &p3, double eps)
{
	return Det2D(p1, p2, p3) <= eps;
}

bool TriTri2D(const TriPoint *t1,
	const TriPoint *t2,
	double eps, bool onBoundary)
{
	//Trangles must be expressed anti-clockwise
	if(Det2D(t1[0], t1[1], t1[2]) < 0.0)
		throw std::runtime_error("triangle 1 has wrong winding direction");
	if(Det2D(t2[0], t2[1], t2[2]) < 0.0)
		throw std::runtime_error("triangle 2 has wrong winding direction");

	bool (*chkEdge)(const TriPoint &, const TriPoint &, const TriPoint &, double) = NULL;
	if(onBoundary) //Points on the boundary are considered as colliding
		chkEdge = BoundaryCollideChk;
	else //Points on the boundary are not considered as colliding
		chkEdge = BoundaryDoesntCollideChk;

	//For edge E of trangle 1,
	//check all points of trangle 2 lay on the external side of the edge E. If
	//they do, the triangles do not collide.
	if (chkEdge(t1[0], t1[1], t2[0], eps) &&
		chkEdge(t1[0], t1[1], t2[1], eps) &&
		chkEdge(t1[0], t1[1], t2[2], eps))
		return false;
	if (chkEdge(t1[1], t1[2], t2[0], eps) &&
		chkEdge(t1[1], t1[2], t2[1], eps) &&
		chkEdge(t1[1], t1[2], t2[2], eps))
		return false;
	if (chkEdge(t1[2], t1[0], t2[0], eps) &&
		chkEdge(t1[2], t1[0], t2[1], eps) &&
		chkEdge(t1[2], t1[0], t2[2], eps))
		return false;

	//For edge E of trangle 2,
	//check all points of trangle 1 lay on the external side of the edge E. If
	//they do, the triangles do not collide.
	if (chkEdge(t2[0], t2[1], t1[0], eps) &&
		chkEdge(t2[0], t2[1], t1[1], eps) &&
		chkEdge(t2[0], t2[1], t1[2], eps))
		return false;
	if (chkEdge(t2[1], t2[2], t1[0], eps) &&
		chkEdge(t2[1], t2[2], t1[1], eps) &&
		chkEdge(t2[1], t2[2], t1[2], eps))
		return false;
	if (chkEdge(t2[2], t2[0], t1[0], eps) &&
		chkEdge(t2[2], t2[0], t1[1], eps) &&
		chkEdge(t2[2], t2[0], t1[2], eps))
		return false;

	//The triangles collide
	return true;
}

void TriCollisionTestCases()
{
	{TriPoint t1[] = {TriPoint(0,0),TriPoint(5,0),TriPoint(0,5)};
	TriPoint t2[] = {TriPoint(0,0),TriPoint(5,0),TriPoint(0,6)};
	cout << TriTri2D(t1, t2) << "," << true << endl;}

	{TriPoint t1[] = {TriPoint(0,0),TriPoint(5,0),TriPoint(0,5)};
	TriPoint t2[] = {TriPoint(-10,0),TriPoint(-5,0),TriPoint(-1,6)};
	cout << TriTri2D(t1, t2) << "," << false << endl;}

	{TriPoint t1[] = {TriPoint(0,0),TriPoint(5,0),TriPoint(2.5,5)};
	TriPoint t2[] = {TriPoint(0,4),TriPoint(2.5,-1),TriPoint(5,4)};
	cout << TriTri2D(t1, t2) << "," << true << endl;}

	{TriPoint t1[] = {TriPoint(0,0),TriPoint(1,1),TriPoint(0,2)};
	TriPoint t2[] = {TriPoint(2,1),TriPoint(3,0),TriPoint(3,2)};
	cout << TriTri2D(t1, t2) << "," << false << endl;}

	{TriPoint t1[] = {TriPoint(0,0),TriPoint(1,1),TriPoint(0,2)};
	TriPoint t2[] = {TriPoint(2,1),TriPoint(3,-2),TriPoint(3,4)};
	cout << TriTri2D(t1, t2) << "," << false << endl;}

	//Barely touching
	{TriPoint t1[] = {TriPoint(0,0),TriPoint(1,0),TriPoint(0,1)};
	TriPoint t2[] = {TriPoint(1,0),TriPoint(2,0),TriPoint(1,1)};
	cout << TriTri2D(t1, t2, 0.0, true) << "," << true << endl;}

	//Barely touching
	{TriPoint t1[] = {TriPoint(0,0),TriPoint(1,0),TriPoint(0,1)};
	TriPoint t2[] = {TriPoint(1,0),TriPoint(2,0),TriPoint(1,1)};
	cout << TriTri2D(t1, t2, 0.0, false) << "," << false << endl;}

}
