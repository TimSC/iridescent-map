#ifndef _REGROUPER_H
#define _REGROUPER_H
#include "cppo5m/OsmData.h"
#include <map>

typedef std::vector<std::pair<double, double> > LatLonList;

class IRegroupResultHandler
{
public:
	virtual void FoundArea(const TagMap &tags, 
		const std::vector<LatLonList> &outerShapes, 
		const std::vector<LatLonList> &innerShapes) {};
	virtual void FoundLine(const TagMap &tags, 
		const LatLonList &shape) {};
	virtual void FoundPoi(const TagMap &tags, 
		double lat, double lon) {};
};

class FeatureStore:IRegroupResultHandler
{
public:
	int test;

};

class Regrouper : public OsmData
{
public:
	std::map<int64_t, class OsmNode *> nodeIdMap;
	std::map<int64_t, class OsmWay *> wayIdMap;
	std::map<int64_t, class OsmRelation *> relationIdMap;

	Regrouper();
	virtual ~Regrouper();

	void FindAreas(class IRegroupResultHandler *output);
	void FindLines(class IRegroupResultHandler *output);
	void FindPois(class IRegroupResultHandler *output);

	void UpdateIdMappings();
	bool DetectIfArea(const std::vector<int64_t> &wayNodes, const TagMap &tags);
};

#endif //_REGROUPER_H
