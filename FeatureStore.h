#ifndef _FEATURE_STORE_H
#define _FEATURE_STORE_H

#include "cppo5m/OsmData.h"

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
	int64_t nid;
	double lat, lon;

	FeaturePoi();
	FeaturePoi(const FeaturePoi &a);
	FeaturePoi(const TagMap &tags, int64_t nid, double lat, double lon);
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
		int64_t nid,
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
		int64_t nid,
		double lat, double lon);
	virtual void Clear();
};

#endif //_FEATURE_STORE_H

