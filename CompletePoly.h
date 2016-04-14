#ifndef _COMPLETE_POLY_H
#define _COMPLETE_POLY_H

#include "MapRenderTypes.h"

class PointInfo
{
public:
	double x, y;
	int edgeIndex; //-1 for no edge contact
	int64_t nid; //Node ID. Zero is used for virtual added nodes

	PointInfo(double x, double y, int edgeIndex, int64_t nid);
};

void CompletePolygonsInBbox(const ContoursWithIds &contours, 
	const std::vector<double> &bbox, 
	int direction, 
	double eps,
	std::vector<std::vector<class PointInfo> > &collectedLoopsOut,
	std::vector<std::vector<class PointInfo> > &internalLoopsOut,
	std::vector<std::vector<class PointInfo> > &reverseInternalLoopsOut);

void PointInfoVecToPolygons(const std::vector<std::vector<class PointInfo> > &loops, std::vector<Polygon> &out);

#endif //_COMPLETE_POLY_H
