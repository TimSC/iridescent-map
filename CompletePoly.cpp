#include <utility>
#include <vector>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <stdint.h>
#include "drawlib/LineLineIntersect.h"
using namespace std;
typedef std::pair<double, double> Point;
typedef std::pair<int64_t, Point> PointWithId;
typedef std::vector<Point> Contour;
typedef std::vector<PointWithId> ContourWithIds;

int CompletePoly()
{


}

bool IsPointInBbox(const Point &pt, const std::vector<double> &bbox)
{
	if(pt.first < bbox[0]) return false;
	if(pt.first >= bbox[2]) return false;
	if(pt.second < bbox[1]) return false;
	if(pt.second >= bbox[3]) return false;
	return true;
}

int IsPointOnEdge(const Point &pt,
	double eps,
	const std::vector<double> &bbox)
{
	//bbox defined as left,bottom,right,top
	double leftMinusEps = bbox[0] - eps;
	double leftPlusEps = bbox[0] + eps;
	double bottomMinusEps = bbox[1] - eps;
	double bottomPlusEps = bbox[1] + eps;
	double rightMinusEps = bbox[2] - eps;
	double rightPlusEps = bbox[2] + eps;
	double topMinusEps = bbox[3] - eps;
	double topPlusEps = bbox[3] + eps;
	int approxEdge = -1;
	
	//Left
	if(pt.first >= leftMinusEps && pt.first <= leftPlusEps)
	{
		if(pt.second >= bbox[1] && pt.second <= bbox[3])
			return 0;
		if(pt.second >= bottomMinusEps && pt.second <= topPlusEps)
			approxEdge = 0;
	}
	
	//Bottom
	if(pt.second >= bottomMinusEps && pt.second <= bottomPlusEps)
	{
		if(pt.first >= bbox[0] && pt.first <= bbox[2])
			return 1;
		if(pt.first >= leftMinusEps && pt.first <= rightPlusEps)
			approxEdge = 1;
	}

	//Right
	if(pt.first >= rightMinusEps && pt.first <= rightPlusEps)
	{
		if(pt.second >= bbox[1] && pt.second <= bbox[3])
			return 2;
		if(pt.second >= bottomMinusEps && pt.second <= topPlusEps)
			approxEdge = 2;
	}

	//Top
	if(pt.second >= topMinusEps && pt.second <= topPlusEps)
	{
		if(pt.first >= bbox[0] && pt.first <= bbox[2])
			return 3;
		if(pt.first >= leftMinusEps && pt.first <= rightPlusEps)
			approxEdge = 3;
	}

	return approxEdge;
}

class Crossing
{
public:
	double ix, iy;
	size_t edgeIndex;
	bool startInside;
	bool endInside;
};

void DetectLineBboxEntryExit(const PointWithId &pt1, const PointWithId &pt2, const std::vector<double> &bbox,
	std::vector<class Crossing> &crossingsSortedOut)
{
	//bbox defined as left,bottom,right,top
	const Point &pt1pos = pt1.second;
	const Point &pt2pos = pt2.second;

	//Crossings found
	std::vector<class Crossing> crossings;

	//Vertical edges
	for(int edgeIndex = 0; edgeIndex < 4; edgeIndex += 2)
	{
		bool pt1LeftOfEdge = (pt1pos.first < bbox[edgeIndex]);
		bool pt2LeftOfEdge = (pt2pos.first < bbox[edgeIndex]);
		double ix = -1.0, iy = -1.0;
		if(pt1LeftOfEdge != pt2LeftOfEdge)
		{
			//Potential line cross edge detected
			bool ok = LineLineIntersect(pt1pos.first, pt1pos.second, //Line 1 start
				pt2pos.first, pt2pos.second, //Line 1 end
				bbox[edgeIndex], bbox[1], //Line 2 start
				bbox[edgeIndex], bbox[3], //Line 2 end
				ix, iy);
			if(ok && iy >= bbox[1] && iy <= bbox[3])
			{
				class Crossing crossing;
				crossing.ix = ix;
				crossing.iy = iy;
				crossing.edgeIndex = edgeIndex;
				if(edgeIndex == 0)
				{
					crossing.startInside = !pt1LeftOfEdge;
					crossing.endInside = !pt2LeftOfEdge;
				}
				else
				{
					crossing.startInside = pt1LeftOfEdge;
					crossing.endInside = pt2LeftOfEdge;
				}
				crossings.push_back(crossing);
			}
		}
	}

	//Horizontal edges
	for(int edgeIndex = 1; edgeIndex < 4; edgeIndex += 2)
	{
		bool pt1BelowEdge = (pt1pos.second < bbox[edgeIndex]);
		bool pt2BelowEdge = (pt2pos.second < bbox[edgeIndex]);
		double ix = -1.0, iy = -1.0;
		if(pt1BelowEdge != pt2BelowEdge)
		{
			//Potential line cross edge detected
			bool ok = LineLineIntersect(pt1pos.first, pt1pos.second, //Line 1 start
				pt2pos.first, pt2pos.second, //Line 1 end
				bbox[0], bbox[edgeIndex], //Line 2 start
				bbox[2], bbox[edgeIndex], //Line 2 end
				ix, iy);
			if(ok && ix >= bbox[0] && ix <= bbox[2])
			{
				class Crossing crossing;
				crossing.ix = ix;
				crossing.iy = iy;
				crossing.edgeIndex = edgeIndex;
				if(edgeIndex == 1)
				{
					crossing.startInside = !pt1BelowEdge;
					crossing.endInside = !pt2BelowEdge;
				}
				else
				{
					crossing.startInside = pt1BelowEdge;
					crossing.endInside = pt2BelowEdge;
				}
				crossings.push_back(crossing);
			}
		}
	}

	//Prepare to sort by distance
	vector<bool> crossingsMask;
	crossingsMask.resize(crossings.size());
	for(size_t i=0; i < crossings.size(); i++)
		crossingsMask[i] = true;
	
	vector<double> crossingDistSq;
	crossingDistSq.resize(crossings.size());
	for(size_t i=0; i < crossings.size(); i++)
	{
		class Crossing &crossing = crossings[i];
		double distSq = pow(crossing.ix - pt1pos.first, 2.0) + pow(crossing.iy - pt1pos.second, 2.0);
		crossingDistSq[i] = distSq;
	}

	//(Pretty much) selection sort by distance of collision point from line start (pt1)
	crossingsSortedOut.resize(0);
	crossingsSortedOut.reserve(crossings.size());

	while(crossingsSortedOut.size() < crossings.size())
	{
		double minDistSq = -1.0;
		bool minSet = false;
		size_t minIndex = 0;
		for(size_t i=0; i<crossings.size(); i++)
		{
			if(crossingsMask[i] == false) continue;
			if(!minSet || crossingDistSq[i] < minDistSq)
			{
				minDistSq = crossingDistSq[i];
				minIndex = i;
				minSet = true;
			}
		}
		if(!minSet)
			throw runtime_error("Internal error during sort");
		crossingsSortedOut.push_back(crossings[minIndex]);
		crossingsMask[minIndex] = false;
	}
}

class PointInfo
{
public:
	double x, y;
	int edgeIndex; //-1 for no edge contact
	int64_t nid; //Node ID. Zero is used for virtual added nodes

	PointInfo(double x, double y, int edgeIndex, int64_t nid)
	{
		this->x = x;
		this->y = y;
		this->edgeIndex = edgeIndex;
		this->nid = nid;
	}
};

void AnalyseContour(const ContourWithIds &contour, 
	const std::vector<double> &bbox, 
	double eps,
	std::vector<std::vector<class PointInfo> > &pathsWithinBboxOut)
{
	unsigned int numInside = 0;
	unsigned int numOutside = 0;
	if(contour.size() < 2) return;

	const PointWithId *prevPt = &contour[0];
	const Point &prevPtPos = prevPt->second;
	bool currentlyInsideBbox = IsPointInBbox(prevPtPos, bbox);
	pathsWithinBboxOut.clear();
	std::vector<class PointInfo> pathWithinBbox;
	if(currentlyInsideBbox)
		pathWithinBbox.push_back(PointInfo(prevPtPos.first, prevPtPos.second, -1, prevPt->first));
	
	for(size_t i=1; i < contour.size(); i++)
	{
		const PointWithId *pt = &contour[i];
		const Point &ptPos = pt->second;

		std::vector<class Crossing> crossingsSorted;
		DetectLineBboxEntryExit(*prevPt, *pt, bbox, crossingsSorted);

		if(crossingsSorted.size() == 0)
		{
			//No crossing
			if(currentlyInsideBbox)
				pathWithinBbox.push_back(PointInfo(ptPos.first, ptPos.second, -1, pt->first));
		}

		if(crossingsSorted.size() == 1)
		{
			//Single crossing
			currentlyInsideBbox = !currentlyInsideBbox;
			if(!currentlyInsideBbox)
			{
				//Path within bbox completed
				pathWithinBbox.push_back(PointInfo(crossingsSorted[0].ix, crossingsSorted[0].iy, crossingsSorted[0].edgeIndex, 0));
				pathsWithinBboxOut.push_back(pathWithinBbox);
				pathWithinBbox.clear();
			}
			else
			{
				//Starting new path in bbox
				pathWithinBbox.push_back(PointInfo(crossingsSorted[0].ix, crossingsSorted[0].iy, crossingsSorted[0].edgeIndex, 0));
				pathWithinBbox.push_back(PointInfo(ptPos.first, ptPos.second, -1, pt->first));
			}

		}

		if(crossingsSorted.size() >= 2)
		{
			//Crossing in and out
			//More than two crossings should not be possible
			pathWithinBbox.push_back(PointInfo(crossingsSorted[0].ix, crossingsSorted[0].iy, crossingsSorted[0].edgeIndex, 0));
			pathWithinBbox.push_back(PointInfo(crossingsSorted[1].ix, crossingsSorted[1].iy, crossingsSorted[1].edgeIndex, 0));
			pathsWithinBboxOut.push_back(pathWithinBbox);
			pathWithinBbox.clear();
			currentlyInsideBbox = false;
		}

		if(currentlyInsideBbox && i == contour.size() - 1)
		{
			pathsWithinBboxOut.push_back(pathWithinBbox);
			pathWithinBbox.clear();
		}

		prevPt = pt;
	}

	//Extra step of finding start/end points very close to edges, even if line does not cross.
	//Consider this as colliding if the line is not closed.
	for(size_t i=0; i<pathsWithinBboxOut.size();i ++)
	{
		std::vector<class PointInfo> &path = pathsWithinBboxOut[i];
		if(path.size() < 2) continue;
		class PointInfo &startPt = path[0];
		class PointInfo &endPt = path[path.size()-1];
		if(startPt.nid == endPt.nid && startPt.nid != 0)
			continue; //Path is closed, so skip
		if(startPt.edgeIndex == -1)
			startPt.edgeIndex = IsPointOnEdge(Point(startPt.x, startPt.y), eps, bbox);
		if(endPt.edgeIndex == -1)
			endPt.edgeIndex = IsPointOnEdge(Point(endPt.x, endPt.y), eps, bbox);
	}
	
}

void PrintPathsWithinBbox(const std::vector<std::vector<class PointInfo> > &pathsWithinBbox)
{
	for(size_t i=0;i<pathsWithinBbox.size();i++)
	{
		const std::vector<class PointInfo> &line = pathsWithinBbox[i];
		for(size_t j=0; j< line.size(); j++)
		{
			const class PointInfo &pt = line[j];
			cout << pt.x << "," << pt.y << "," << pt.edgeIndex << "," << pt.nid << endl;
		}
	}
}

int main()
{
	//Coastlines have land on the left, sea on the right

	//left,bottom,right,top
	std::vector<double> bbox;
	bbox.push_back(0.0);
	bbox.push_back(0.0);
	bbox.push_back(1.0);
	bbox.push_back(1.0);

	cout << "line1" << endl;
	ContourWithIds line1;
	line1.push_back(PointWithId(1, Point(0.5, -0.1)));
	line1.push_back(PointWithId(2, Point(0.5, 0.4)));
	line1.push_back(PointWithId(3, Point(0.5, 1.1)));

	std::vector<std::vector<class PointInfo> > pathsWithinBbox;
	AnalyseContour(line1, bbox, 1e-6, pathsWithinBbox);
	PrintPathsWithinBbox(pathsWithinBbox);

	cout << "line2" << endl;
	ContourWithIds line2;
	line2.push_back(PointWithId(1, Point(-0.1, 0.5)));
	line2.push_back(PointWithId(2, Point(0.5, 1.1)));

	AnalyseContour(line2, bbox, 1e-6, pathsWithinBbox);
	PrintPathsWithinBbox(pathsWithinBbox);

	cout << "line3" << endl;
	ContourWithIds line3;
	line3.push_back(PointWithId(1, Point(-0.1, 0.5)));
	line3.push_back(PointWithId(2, Point(0.1, 0.5)));
	line3.push_back(PointWithId(3, Point(0.9, 0.5)));
	line3.push_back(PointWithId(4, Point(1.1, 0.5)));

	AnalyseContour(line3, bbox, 1e-6, pathsWithinBbox);
	PrintPathsWithinBbox(pathsWithinBbox);

	cout << "line4" << endl;
	ContourWithIds line4;
	line4.push_back(PointWithId(1, Point(0.5, 0.25)));
	line4.push_back(PointWithId(2, Point(0.75, 0.5)));
	line4.push_back(PointWithId(3, Point(0.5, 0.75)));
	line4.push_back(PointWithId(4, Point(0.25, 0.5)));

	AnalyseContour(line4, bbox, 1e-6, pathsWithinBbox);
	PrintPathsWithinBbox(pathsWithinBbox);

	cout << "line5" << endl;
	ContourWithIds line5;
	line5.push_back(PointWithId(1, Point(0.5, 1e-8)));
	line5.push_back(PointWithId(2, Point(0.5, 1-1e-8)));

	AnalyseContour(line5, bbox, 1e-6, pathsWithinBbox);
	PrintPathsWithinBbox(pathsWithinBbox);
}

