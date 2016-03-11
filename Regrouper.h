#ifndef _REGROUPER_H
#define _REGROUPER_H
#include "cppo5m/OsmData.h"
#include <map>

class IdLatLon
{
public:
	double lat;
	double lon;
	int64_t objId;

	IdLatLon();
	IdLatLon(const IdLatLon &a);
	IdLatLon(double lat, double lon, int64_t objId);
	virtual ~IdLatLon();
	IdLatLon& operator=(const IdLatLon &arg);
};

typedef std::vector<class IdLatLon> IdLatLonList;

class FeatureArea
{
public:
	TagMap tags;
	std::vector<IdLatLonList> outerShapes; 
	std::vector<IdLatLonList> innerShapes;

	FeatureArea();
	FeatureArea(const FeatureArea &a);
	FeatureArea(const TagMap &tags,
		const std::vector<IdLatLonList> &outerShapes, 
		const std::vector<IdLatLonList> &innerShapes);
	virtual ~FeatureArea();
	FeatureArea& operator=(const FeatureArea &arg);
};

class FeatureLine
{
public:
	TagMap tags;
	IdLatLonList shape;

	FeatureLine();
	FeatureLine(const FeatureLine &a);
	FeatureLine(const TagMap &tags, const IdLatLonList &shape);
	virtual ~FeatureLine();
	FeatureLine& operator=(const FeatureLine &arg);
};

class FeaturePoi
{
public:
	TagMap tags;
	double lat, lon;

	FeaturePoi();
	FeaturePoi(const FeaturePoi &a);
	FeaturePoi(const TagMap &tags, double lat, double lon);
	virtual ~FeaturePoi();
	FeaturePoi& operator=(const FeaturePoi &arg);
};

class IRegroupResultHandler
{
public:
	virtual void FoundArea(const TagMap &tags, 
		const std::vector<IdLatLonList> &outerShapes, 
		const std::vector<IdLatLonList> &innerShapes) {};
	virtual void FoundLine(const TagMap &tags, 
		const IdLatLonList &shape) {};
	virtual void FoundPoi(const TagMap &tags, 
		double lat, double lon) {};
};

class FeatureStore: public IRegroupResultHandler
{
public:
	std::vector<class FeatureArea> areas;
	std::vector<class FeatureLine> lines;
	std::vector<class FeaturePoi> pois;

	FeatureStore();
	FeatureStore(const FeaturePoi &a);
	virtual ~FeatureStore();
	FeatureStore& operator=(const FeatureStore &arg);

	virtual void FoundArea(const TagMap &tags, 
		const std::vector<IdLatLonList> &outerShapes, 
		const std::vector<IdLatLonList> &innerShapes);
	virtual void FoundLine(const TagMap &tags, 
		const IdLatLonList &shape);
	virtual void FoundPoi(const TagMap &tags, 
		double lat, double lon);
	virtual void Clear();
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

	///Iterates over object lists and regenerates internal ID indexing map structures.
	void UpdateIdMappings();
	bool DetectIfArea(const std::vector<int64_t> &wayNodes, const TagMap &tags);
};

#endif //_REGROUPER_H
