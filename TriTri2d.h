#ifndef _TRI_TRI_2D_H
#define _TRI_TRI_2D_H

#include <utility>

typedef std::pair<double, double> TriPoint;

bool TriTri2D(TriPoint *t1,
	TriPoint *t2,
	double eps = 0.0, bool allowReversed = false, bool onBoundary = true);
int TriCollisionTestCases();

#endif //_TRI_TRI_2D_H
