#include <utility>
#include <vector>
#include <iostream>
#include <cmath>
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
};

void DetectLineBboxEntryExit(const Point &pt1, const Point &pt2, const std::vector<double> &bbox)
{
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
			if(ok && iy >= bbox[1] && iy < bbox[3])
			{
				class Crossing crossing;
				crossing.ix = ix;
				crossing.iy = iy;
				crossing.edgeIndex = edgeIndex;
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
			if(ok && ix >= bbox[0] && ix < bbox[2])
			{
				class Crossing crossing;
				crossing.ix = ix;
				crossing.iy = iy;
				crossing.edgeIndex = edgeIndex;
				crossings.push_back(crossing);
			}
		}
	}

	//Find nearest collision point from line start (pt1)
	bool minSet = false;
	double minDistSq = -1.0;
	size_t minIndex = 0;
	for(size_t i=0; i < crossings.size(); i++)
	{
		class Crossing &crossing = crossings[i];
		double distSq = pow(crossing.ix - pt1.first, 2.0) + pow(crossing.iy - pt1.second, 2.0);
		if(!minSet || distSq < minDistSq)
		{
			minDistSq = distSq;
			minSet = true;
			minIndex = i;
		}
	}

	if(minSet)
	{
		class Crossing &crossing = crossings[minIndex];
		cout << "crossing " << crossing.edgeIndex << "," << crossing.ix << "," << crossing.iy << endl;
	}
}

void AnalyseContour(const Contour &contour, const std::vector<double> &bbox)
{
	unsigned int numInside = 0;
	unsigned int numOutside = 0;
	if(contour.size() < 2) return;

	const Point &firstPt = contour[0];
	bool firstPtInBbox = IsPointInBbox(firstPt, bbox);
	const Point &lastPt = contour[contour.size()-1];
	bool lastPtInBbox = IsPointInBbox(lastPt, bbox);

	if(!firstPtInBbox && !lastPtInBbox)
	{
		//Contour starts and ends outside, so check if it passes through at any stage
		const Point *prevPt = &contour[0];
		for(size_t i=1; i < contour.size(); i++)
		{
			const Point *pt = &contour[i];
			cout << prevPt->first<<","<<prevPt->second << "\t" << pt->first<<","<<pt->second << endl;
			DetectLineBboxEntryExit(*prevPt, *pt, bbox);

			prevPt = pt;
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

	Contour line1;
	line1.push_back(Point(0.5, -0.1));
	line1.push_back(Point(0.5, 0.4));
	line1.push_back(Point(0.5, 1.1));

	AnalyseContour(line1, bbox);
}

