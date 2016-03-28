#ifndef _TRI_TRI_2D_H
#define _TRI_TRI_2D_H

#include <utility>

#define ENABLE_TRI_VALIDATION

typedef std::pair<double, double> TriPoint;

inline double Det2D(const TriPoint &p1, const TriPoint &p2, const TriPoint &p3) 
{
	return +p1.first*(p2.second-p3.second)
		+p2.first*(p3.second-p1.second)
		+p3.first*(p1.second-p2.second);
}

bool TriTri2D(const TriPoint *t1,
	const TriPoint *t2,
	double eps = 0.0, bool onBoundary = true);
void TriCollisionTestCases();

#endif //_TRI_TRI_2D_H
