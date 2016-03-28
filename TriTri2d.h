#ifndef _TRI_TRI_2D_H
#define _TRI_TRI_2D_H

#include <utility>

typedef std::pair<double, double> TriPoint;

bool TriTri2D(const TriPoint *t1,
	const TriPoint *t2,
	double eps = 0.0, bool onBoundary = true);
void TriCollisionTestCases();

#endif //_TRI_TRI_2D_H
