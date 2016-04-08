#include <utility>
#include <vector>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include "drawlib/LineLineIntersect.h"
using namespace std;
typedef std::pair<double, double> Point;
typedef std::vector<Point> Contour;

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

class Crossing
{
public:
	double ix, iy;
	size_t edgeIndex;
	bool startInside;
	bool endInside;
};

void DetectLineBboxEntryExit(const Point &pt1, const Point &pt2, const std::vector<double> &bbox,
	std::vector<class Crossing> &crossingsSortedOut)
{
	//bbox defined as left,bottom,right,top

	//Crossings found
	std::vector<class Crossing> crossings;

	//Vertical edges
	for(int edgeIndex = 0; edgeIndex < 4; edgeIndex += 2)
	{
		bool pt1LeftOfEdge = (pt1.first < bbox[edgeIndex]);
		bool pt2LeftOfEdge = (pt2.first < bbox[edgeIndex]);
		double ix = -1.0, iy = -1.0;
		if(pt1LeftOfEdge != pt2LeftOfEdge)
		{
			//Potential line cross edge detected
			bool ok = LineLineIntersect(pt1.first, pt1.second, //Line 1 start
				pt2.first, pt2.second, //Line 1 end
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
		bool pt1BelowEdge = (pt1.second < bbox[edgeIndex]);
		bool pt2BelowEdge = (pt2.second < bbox[edgeIndex]);
		double ix = -1.0, iy = -1.0;
		if(pt1BelowEdge != pt2BelowEdge)
		{
			//Potential line cross edge detected
			bool ok = LineLineIntersect(pt1.first, pt1.second, //Line 1 start
				pt2.first, pt2.second, //Line 1 end
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
		double distSq = pow(crossing.ix - pt1.first, 2.0) + pow(crossing.iy - pt1.second, 2.0);
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

	PointInfo(double x, double y, int edgeIndex)
	{
		this->x = x;
		this->y = y;
		this->edgeIndex = edgeIndex;
	}
};

void AnalyseContour(const Contour &contour, const std::vector<double> &bbox, std::vector<std::vector<class PointInfo> > &pathsWithinBboxOut)
{
	unsigned int numInside = 0;
	unsigned int numOutside = 0;
	if(contour.size() < 2) return;

	const Point *prevPt = &contour[0];
	bool currentlyInsideBbox = IsPointInBbox(*prevPt, bbox);
	pathsWithinBboxOut.clear();
	std::vector<class PointInfo> pathWithinBbox;
	if(currentlyInsideBbox)
		pathWithinBbox.push_back(PointInfo(prevPt->first, prevPt->second, -1));
	
	for(size_t i=1; i < contour.size(); i++)
	{
		const Point *pt = &contour[i];

		std::vector<class Crossing> crossingsSorted;
		DetectLineBboxEntryExit(*prevPt, *pt, bbox, crossingsSorted);

		if(crossingsSorted.size() == 0)
		{
			//No crossing
			if(currentlyInsideBbox)
				pathWithinBbox.push_back(PointInfo(pt->first, pt->second, -1));
		}

		if(crossingsSorted.size() == 1)
		{
			//Single crossing
			currentlyInsideBbox = !currentlyInsideBbox;
			if(!currentlyInsideBbox)
			{
				//Path within bbox completed
				pathWithinBbox.push_back(PointInfo(crossingsSorted[0].ix, crossingsSorted[0].iy, crossingsSorted[0].edgeIndex));
				pathsWithinBboxOut.push_back(pathWithinBbox);
				pathWithinBbox.clear();
			}
			else
			{
				//Starting new path in bbox
				pathWithinBbox.push_back(PointInfo(crossingsSorted[0].ix, crossingsSorted[0].iy, crossingsSorted[0].edgeIndex));
				pathWithinBbox.push_back(PointInfo(pt->first, pt->second, -1));
			}

		}

		if(crossingsSorted.size() >= 2)
		{
			//Crossing in and out
			//More than two crossings should not be possible
			pathWithinBbox.push_back(PointInfo(crossingsSorted[0].ix, crossingsSorted[0].iy, crossingsSorted[0].edgeIndex));
			pathWithinBbox.push_back(PointInfo(crossingsSorted[1].ix, crossingsSorted[1].iy, crossingsSorted[1].edgeIndex));
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

	
}

void PrintPathsWithinBbox(const std::vector<std::vector<class PointInfo> > &pathsWithinBbox)
{
	for(size_t i=0;i<pathsWithinBbox.size();i++)
	{
		const std::vector<class PointInfo> &line = pathsWithinBbox[i];
		for(size_t j=0; j< line.size(); j++)
		{
			const class PointInfo &pt = line[j];
			cout << pt.x << "," << pt.y << "," << pt.edgeIndex << endl;
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
	Contour line1;
	line1.push_back(Point(0.5, -0.1));
	line1.push_back(Point(0.5, 0.4));
	line1.push_back(Point(0.5, 1.1));

	std::vector<std::vector<class PointInfo> > pathsWithinBbox;
	AnalyseContour(line1, bbox, pathsWithinBbox);
	PrintPathsWithinBbox(pathsWithinBbox);

	cout << "line2" << endl;
	Contour line2;
	line2.push_back(Point(-0.1, 0.5));
	line2.push_back(Point(0.5, 1.1));

	AnalyseContour(line2, bbox, pathsWithinBbox);
	PrintPathsWithinBbox(pathsWithinBbox);

	cout << "line3" << endl;
	Contour line3;
	line3.push_back(Point(-0.1, 0.5));
	line3.push_back(Point(0.1, 0.5));
	line3.push_back(Point(0.9, 0.5));
	line3.push_back(Point(1.1, 0.5));

	AnalyseContour(line3, bbox, pathsWithinBbox);
	PrintPathsWithinBbox(pathsWithinBbox);
}

