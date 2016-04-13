#include <utility>
#include <vector>
#include <iostream>
#include <cmath>
#include <map>
#include <stdexcept>
#include <stdint.h>
#include <assert.h>
#include "drawlib/LineLineIntersect.h"
using namespace std;
typedef std::pair<double, double> Point;
typedef std::pair<int64_t, Point> PointWithId;
typedef std::vector<Point> Contour;
typedef std::vector<PointWithId> ContourWithIds;
typedef std::vector<ContourWithIds> ContoursWithIds;
typedef map<int, map<double, int> > EdgeMap;

bool IsPointInBbox(const Point &pt, const std::vector<double> &bbox)
{
	//bbox defined as left,bottom,right,top
	if(pt.first < bbox[0]) return false;
	if(pt.first >= bbox[2]) return false;
	if(pt.second > bbox[1]) return false;
	if(pt.second <= bbox[3]) return false;
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
			if(ok && iy <= bbox[1] && iy >= bbox[3])
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

bool CheckWinding(const std::vector<class PointInfo> &path)
{
	//http://stackoverflow.com/a/1165943/4288232
	double tot = 0.0;
	const class PointInfo *prevPt = &path[path.size()-1];
	for(size_t i=0; i<path.size(); i++)
	{
		const class PointInfo *pt = &path[i];
		tot += (pt->x - prevPt->x)*(pt->y + prevPt->y);
		prevPt = pt;
	}
	return tot > 0.0;
}

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
		if(pt->first == 0)
			throw invalid_argument("Input contours cannot have zero ID");
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
		cout << "loop: ";
		const std::vector<class PointInfo> &line = pathsWithinBbox[i];
		for(size_t j=0; j< line.size(); j++)
		{
			const class PointInfo &pt = line[j];
			cout << pt.x << "," << pt.y << "," << pt.edgeIndex << "," << pt.nid << endl;
		}
	}
}

void TestContourAnalyse()
{
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

bool SearchForConnection(int edgeIndex, double cursor, EdgeMap &startOnEdgeMap, const vector<bool> &pathSentToOutput, int &foundPathIndexOut)
{
	foundPathIndexOut = -1;
	int direction = 0;
	if(edgeIndex >= 0 && edgeIndex <= 1)
		direction = 1;
	if(edgeIndex >= 2 && edgeIndex <= 3)
		direction = -1;
	//cout << cursor <<"," << direction << endl;	
	
	//Check first side
	const map<double, int> &thisEdge = startOnEdgeMap[edgeIndex];

	if(direction == 1)
	{
		for(map<double, int>::const_iterator it = thisEdge.begin(); it != thisEdge.end(); it++)
		{
			if(pathSentToOutput[it->second]) continue;
			if(it->first < cursor) continue;
			foundPathIndexOut = it->second;
			return true;
		}
	}
	else
	{
		for(map<double, int>::const_reverse_iterator it = thisEdge.rbegin(); it != thisEdge.rend(); it++)
		{
			if(pathSentToOutput[it->second]) continue;
			if(it->first > cursor) continue;
			foundPathIndexOut = it->second;
			return true;
		}
	}

	//Try the other edges, then 
	for(int i=0; i<4; i++)
	{
		edgeIndex = (edgeIndex+1) % 4;
		const map<double, int> &currentEdge = startOnEdgeMap[edgeIndex];
		if(direction == 1)
		{
			for(map<double, int>::const_iterator it = currentEdge.begin(); it != currentEdge.end(); it++)
			{
				if(pathSentToOutput[it->second]) continue;
				foundPathIndexOut = it->second;
				return true;
			}
		}
		else
		{
			for(map<double, int>::const_reverse_iterator it = currentEdge.rbegin(); it != currentEdge.rend(); it++)
			{
				if(pathSentToOutput[it->second]) continue;
				foundPathIndexOut = it->second;
				return true;
			}
		}
	}

	//This should not usually happen because the original path should have been returned
	return false;
}

void TraverseCorners(int prevEdgeIndex, int edgeIndex, const std::vector<double> &bbox, 
	int direction,
	std::vector<class PointInfo> &appendToOut)
{
	//bbox defined as left,bottom,right,top
	if(prevEdgeIndex == -1) return;
	if(prevEdgeIndex == edgeIndex) return;
	int currentEdge = prevEdgeIndex;

	if(direction == -1) while(currentEdge != edgeIndex)
	{
		currentEdge --;
		if(currentEdge < 0) currentEdge += 4;
		//cout << "Change to edge " << currentEdge << " from " << (currentEdge+1)%4 << endl;
		switch(currentEdge)
		{
		case 0:
			//Bottom left
			appendToOut.push_back(PointInfo(bbox[0], bbox[1], 0, 0));
			break;
		case 1:
			//Bottom right
			appendToOut.push_back(PointInfo(bbox[2], bbox[1], 1, 0));
			break;
		case 2:
			//Top right
			appendToOut.push_back(PointInfo(bbox[2], bbox[3], 2, 0));
			break;
		case 3:
			//Top left
			appendToOut.push_back(PointInfo(bbox[0], bbox[3], 3, 0));
			break;
		}
	}

	if(direction == 1) while(currentEdge != edgeIndex)
	{
		currentEdge ++;
		if(currentEdge >= 4) currentEdge -= 4;
		//cout << "Change to edge " << currentEdge << " from " << (currentEdge+1)%4 << endl;
		switch(currentEdge)
		{
		case 0:
			//Top left
			appendToOut.push_back(PointInfo(bbox[0], bbox[3], 3, 0));
			break;
		case 1:
			//Bottom left
			appendToOut.push_back(PointInfo(bbox[0], bbox[1], 0, 0));
			break;
		case 2:
			//Bottom right
			appendToOut.push_back(PointInfo(bbox[2], bbox[1], 1, 0));
			break;
		case 3:
			//Top right
			appendToOut.push_back(PointInfo(bbox[2], bbox[3], 2, 0));
			break;
		}
	}

}

void AssignContoursToEdgeMap(const ContoursWithIds &contours, 
	const std::vector<double> &bbox, 
	double eps,
	std::vector<std::vector<class PointInfo> > &collectedLoopsOut,
	std::vector<std::vector<class PointInfo> > &internalLoopsOut,
	std::vector<std::vector<class PointInfo> > &reverseInternalLoopsOut)
{	
	collectedLoopsOut.clear();
	internalLoopsOut.clear();
	reverseInternalLoopsOut.clear();

	//Find sections of contours that are within the bbox
	std::vector<std::vector<class PointInfo> > pathsWithinBbox;
	for(size_t i=0; i<contours.size(); i++)
	{
		const ContourWithIds &contourWithIds = contours[i];	
		std::vector<std::vector<class PointInfo> > pathsTmp;
		AnalyseContour(contourWithIds, 
			bbox, 
			eps,
			pathsTmp);
		pathsWithinBbox.insert(pathsWithinBbox.end(), pathsTmp.begin(), pathsTmp.end());	
	}
	
	//PrintPathsWithinBbox(pathsWithinBbox);

	//Assign paths to edge maps
	EdgeMap startOnEdgeMap, endOfEdgeMap;
	for(size_t i=0; i<4; i++)
	{
		startOnEdgeMap[i] = map<double, int>();
		endOfEdgeMap[i] = map<double, int>();
	}
	for(size_t i=0; i<pathsWithinBbox.size(); i++)
	{
		std::vector<class PointInfo> &path = pathsWithinBbox[i];
		class PointInfo &startPt = path[0];
		class PointInfo &endPt = path[path.size()-1];
		if(startPt.edgeIndex == 0 || startPt.edgeIndex == 2) //Left or right edge
			startOnEdgeMap[startPt.edgeIndex][startPt.y] = i;
		if(startPt.edgeIndex == 1 || startPt.edgeIndex == 3) //Top or bottom edge
			startOnEdgeMap[startPt.edgeIndex][startPt.x] = i;
		if(endPt.edgeIndex == 0 || endPt.edgeIndex == 2) //Left or right edge
			endOfEdgeMap[endPt.edgeIndex][endPt.y] = i;
		if(endPt.edgeIndex == 1 || endPt.edgeIndex == 3) //Top or bottom edge
			endOfEdgeMap[endPt.edgeIndex][endPt.x] = i;
	}

	//Track which paths have been processed
	vector<bool> pathSentToOutput;
	pathSentToOutput.resize(pathsWithinBbox.size());
	for(size_t i=0; i<pathSentToOutput.size(); i++)
		pathSentToOutput[i] = false;
	
	vector<vector<int> > collectedLoops;
	bool running = true;
	while(running)
	{
		//Pick any way that starts on an edge
		int initialPathIndex = -1;
		for(size_t i=0; i<startOnEdgeMap.size(); i++)
		{
			map<double, int> &thisEdge = startOnEdgeMap[i];
			for(map<double, int>::const_iterator it=thisEdge.begin(); it != thisEdge.end(); it++)
			{
				int pathIndex = it->second;
				if(pathSentToOutput[pathIndex]) continue;
				initialPathIndex = pathIndex;
			}
		}
		if(initialPathIndex == -1)
		{
			running = false;
			continue;
		}

		int currentPathIndex = initialPathIndex;
		vector<int> loopPaths;
		loopPaths.push_back(initialPathIndex);
		bool completingLoop = true;
		while(completingLoop)
		{
			std::vector<class PointInfo> &currentPath = pathsWithinBbox[currentPathIndex];
			class PointInfo &startPt = currentPath[0];
			class PointInfo &endPt = currentPath[currentPath.size()-1];
			//cout << "currentPathIndex " << currentPathIndex << endl;
			//cout << startPt.x << "," << startPt.y << "," << startPt.edgeIndex << "," << startPt.nid << endl;
			//cout << endPt.x << "," << endPt.y << "," << endPt.edgeIndex << "," << endPt.nid << endl;
			if(endPt.edgeIndex == -1)
			{
				cout << "loop ended without completion" << endl;
				//Discard these paths as just too confusing
				for(size_t i =0;i<loopPaths.size();i++)
					pathSentToOutput[loopPaths[i]] = true;
				completingLoop = false;
				continue; //This indicates the path ended without hitting the edge, which is not good.
			}

			//Search for continuity of path
			double cursor = -1.0;
			if(endPt.edgeIndex == 0 || endPt.edgeIndex == 2)
				cursor = endPt.y;
			if(endPt.edgeIndex == 1 || endPt.edgeIndex == 3)
				cursor = endPt.x;

			int foundPathIndex = -1;
			bool found = SearchForConnection(endPt.edgeIndex, cursor, startOnEdgeMap, pathSentToOutput, foundPathIndex);
			if(!found)
			{
				//Strange error has occurred.
				//cout << "strange error" << endl;
				for(size_t i =0;i<loopPaths.size();i++)
					pathSentToOutput[loopPaths[i]] = true;
			}

			bool loopComplete = false;
			for(size_t i =0;i<loopPaths.size();i++)
			{
				if(loopPaths[i] == foundPathIndex)
				{
					loopComplete = true;
					break;
				}
			}		

			if(loopComplete)
			{
				//cout << "loop complete" << endl;
				completingLoop = false;
				for(size_t i =0;i<loopPaths.size();i++)
					pathSentToOutput[loopPaths[i]] = true;
				collectedLoops.push_back(loopPaths);
			}
			else
			{
				currentPathIndex = foundPathIndex;
				loopPaths.push_back(foundPathIndex);
			}
		}
	}

	//Write loops to output with appropriate corner information
	for(size_t i=0;i<collectedLoops.size();i++)
	{
		//cout << "loop: ";
		vector<int> &loop = collectedLoops[i];

		const std::vector<class PointInfo> &firstPath = pathsWithinBbox[loop[0]];
		const class PointInfo &firstPt = firstPath[0];
		std::vector<class PointInfo> loopOut;

		int prevEdgeIndex = -1;
		int direction = 1;
		for(size_t j=0;j<loop.size();j++)
		{
			const std::vector<class PointInfo> &path = pathsWithinBbox[loop[j]];
			//cout << "prevEdgeIndex: " << prevEdgeIndex << endl;
			const class PointInfo &pathFirstPt = path[0];
			TraverseCorners(prevEdgeIndex, pathFirstPt.edgeIndex, bbox, direction, loopOut);

			//cout << loop[j] << ": ";
			for(size_t k=0; k< path.size(); k++)
			{
				const class PointInfo &pt = path[k];
				//cout << pt.x << "," << pt.y << "," << pt.edgeIndex << "," << pt.nid << endl;
				loopOut.push_back(PointInfo(pt.x, pt.y, pt.edgeIndex, pt.nid));
				prevEdgeIndex = pt.edgeIndex;
			}
		}

		//cout << "prevEdgeIndex: " << prevEdgeIndex << endl;
		//cout << "first: " << firstPt.x << "," << firstPt.y << "," << firstPt.edgeIndex << "," << firstPt.nid << endl;
		TraverseCorners(prevEdgeIndex, firstPt.edgeIndex, bbox, direction, loopOut);
		collectedLoopsOut.push_back(loopOut);
	}

	//Output closed loop islands
	for(size_t i=0; i<pathsWithinBbox.size(); i++)
	{
		if(pathSentToOutput[i]) continue;
		//Check if closed
		std::vector<class PointInfo> &path = pathsWithinBbox[i];
		class PointInfo &startPt = path[0];
		class PointInfo &endPt = path[path.size()-1];

		if(startPt.nid != endPt.nid || startPt.nid == 0)
			continue; //Not closed

		//Output while omitting last point
		std::vector<class PointInfo> pathCopy = path;
		pathCopy.pop_back();
		bool windingDirection = CheckWinding(pathCopy);
		if(windingDirection)
			internalLoopsOut.push_back(pathCopy);
		else
			reverseInternalLoopsOut.push_back(pathCopy);

		pathSentToOutput[i] = true;
	}
}

int main()
{
	//Coastlines have land on the left, sea on the right
	//y axis is down the screen, like cairo
	std::vector<std::vector<class PointInfo> > collectedLoops, internalLoops, reverseInternalLoops;

	//left,bottom,right,top
	std::vector<double> bbox;
	bbox.push_back(0.0);
	bbox.push_back(1.0);
	bbox.push_back(1.0);
	bbox.push_back(0.0);

	cout << "example1, land as right of image, sea on left" << endl;
	ContoursWithIds example1Contours;
	ContourWithIds line1;
	line1.push_back(PointWithId(1, Point(0.5, -0.1)));
	line1.push_back(PointWithId(2, Point(0.5, 0.4)));
	line1.push_back(PointWithId(3, Point(0.5, 1.1)));
	example1Contours.push_back(line1);

	AssignContoursToEdgeMap(example1Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	PrintPathsWithinBbox(collectedLoops);

	cout << "example2, land as vertical bar between vertical sea strips" << endl;
	ContoursWithIds example2Contours;
	ContourWithIds line2;
	line2.push_back(PointWithId(3, Point(0.6, 1.1)));
	line2.push_back(PointWithId(2, Point(0.6, 0.4)));
	line2.push_back(PointWithId(1, Point(0.6, -0.1)));
	example2Contours.push_back(line1);
	example2Contours.push_back(line2);

	AssignContoursToEdgeMap(example2Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	PrintPathsWithinBbox(collectedLoops);

	cout << "example3, land in bottom right" << endl;
	ContoursWithIds example3Contours;
	ContourWithIds line3;
	line3.push_back(PointWithId(1, Point(1.1, 0.5)));
	line3.push_back(PointWithId(2, Point(0.5, 0.5)));
	line3.push_back(PointWithId(3, Point(0.5, 1.1)));
	example3Contours.push_back(line3);

	AssignContoursToEdgeMap(example3Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	PrintPathsWithinBbox(collectedLoops);

	cout << "example4, sea in top right" << endl;
	ContoursWithIds example4Contours;
	ContourWithIds line4;
	line4.push_back(PointWithId(1, Point(1.1, 0.5)));
	line4.push_back(PointWithId(2, Point(0.5, 0.5)));
	line4.push_back(PointWithId(3, Point(0.5, -0.1)));
	example4Contours.push_back(line4);

	AssignContoursToEdgeMap(example4Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	PrintPathsWithinBbox(collectedLoops);

	cout << "example5, land in bottom right and top left" << endl;
	ContoursWithIds example5Contours;
	ContourWithIds line5;
	line5.push_back(PointWithId(1, Point(1.1, 0.7)));
	line5.push_back(PointWithId(2, Point(0.7, 0.7)));
	line5.push_back(PointWithId(3, Point(0.7, 1.1)));
	example5Contours.push_back(line5);
	ContourWithIds line6;
	line6.push_back(PointWithId(4, Point(-0.1, 0.4)));
	line6.push_back(PointWithId(5, Point(0.4, 0.4)));
	line6.push_back(PointWithId(6, Point(0.4, -0.1)));
	example5Contours.push_back(line6);

	AssignContoursToEdgeMap(example5Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	PrintPathsWithinBbox(collectedLoops);

	cout << "example6, sea in bottom right and top left" << endl;
	ContoursWithIds example6Contours;
	ContourWithIds line7;
	line7.push_back(PointWithId(1, Point(0.7, 1.1)));
	line7.push_back(PointWithId(2, Point(0.7, 0.7)));
	line7.push_back(PointWithId(3, Point(1.1, 0.7)));
	example6Contours.push_back(line7);
	ContourWithIds line8;
	line8.push_back(PointWithId(4, Point(0.4, -0.1)));
	line8.push_back(PointWithId(5, Point(0.4, 0.4)));
	line8.push_back(PointWithId(6, Point(-0.1, 0.4)));
	example6Contours.push_back(line8);

	AssignContoursToEdgeMap(example6Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	PrintPathsWithinBbox(collectedLoops);

	cout << "example7, land island" << endl;
	ContoursWithIds example7Contours;
	ContourWithIds line9;
	line9.push_back(PointWithId(1, Point(0.3, 0.3)));
	line9.push_back(PointWithId(2, Point(0.3, 0.7)));
	line9.push_back(PointWithId(3, Point(0.7, 0.7)));
	line9.push_back(PointWithId(4, Point(0.7, 0.3)));
	line9.push_back(PointWithId(1, Point(0.3, 0.3)));
	example7Contours.push_back(line9);

	AssignContoursToEdgeMap(example7Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	PrintPathsWithinBbox(collectedLoops);
	PrintPathsWithinBbox(internalLoops);
	assert(reverseInternalLoops.size() == 0);

	cout << "example8, inland sea" << endl;
	ContoursWithIds example8Contours;
	ContourWithIds line10;
	line10.push_back(PointWithId(1, Point(0.3, 0.3)));
	line10.push_back(PointWithId(2, Point(0.7, 0.3)));
	line10.push_back(PointWithId(3, Point(0.7, 0.7)));
	line10.push_back(PointWithId(4, Point(0.3, 0.7)));
	line10.push_back(PointWithId(1, Point(0.3, 0.3)));
	example8Contours.push_back(line10);

	AssignContoursToEdgeMap(example8Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	assert(collectedLoops.size() == 0);
	assert(internalLoops.size() == 0);
	PrintPathsWithinBbox(reverseInternalLoops);

	cout << "example9, half a doughnut" << endl;
	ContoursWithIds example9Contours;
	ContourWithIds line11;
	line11.push_back(PointWithId(1, Point(0.1, -0.1)));
	line11.push_back(PointWithId(2, Point(0.1, 0.5)));
	line11.push_back(PointWithId(3, Point(0.9, 0.5)));
	line11.push_back(PointWithId(4, Point(0.9, -0.1)));
	example9Contours.push_back(line11);
	ContourWithIds line12;
	line12.push_back(PointWithId(5, Point(0.8, -0.1)));
	line12.push_back(PointWithId(6, Point(0.8, 0.5)));
	line12.push_back(PointWithId(7, Point(0.2, 0.5)));
	line12.push_back(PointWithId(8, Point(0.2, -0.1)));
	example9Contours.push_back(line12);

	AssignContoursToEdgeMap(example9Contours, bbox, 1e-6, collectedLoops, internalLoops, reverseInternalLoops);
	PrintPathsWithinBbox(collectedLoops);
}

