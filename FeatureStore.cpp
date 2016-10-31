#include "FeatureStore.h"

IdLatLon::IdLatLon()
{
	lat = 0.0;
	lon = 0.0;
	objId = 0;
}

IdLatLon::IdLatLon(const IdLatLon &a)
{
	*this = a;
}

IdLatLon::IdLatLon(double lat, double lon, int64_t objId) : lat(lat),
	lon(lon),
	objId(objId)
{
	
}

IdLatLon::~IdLatLon()
{
	
}

IdLatLon& IdLatLon::operator=(const IdLatLon &arg)
{
	lat = arg.lat;
	lon = arg.lon;
	objId = arg.objId;
	return *this;
}

// ******************************

FeatureArea::FeatureArea()
{

}

FeatureArea::FeatureArea(const FeatureArea &a)
{
	*this = a;
}

FeatureArea::FeatureArea(const TagMap &tags,
	const std::vector<IdLatLonList> &outerShapes, 
	const std::vector<IdLatLonList> &innerShapes):
	tags(tags),
	outerShapes(outerShapes),
	innerShapes(innerShapes)
{
	
}

FeatureArea::~FeatureArea()
{

}

FeatureArea& FeatureArea::operator=(const FeatureArea &arg)
{
	tags = arg.tags;
	outerShapes = arg.outerShapes;
	innerShapes = arg.innerShapes;
	return *this;
}

FeatureLine::FeatureLine()
{

}

FeatureLine::FeatureLine(const FeatureLine &a)
{
	*this = a;
}

FeatureLine::FeatureLine(const TagMap &tags, const IdLatLonList &shape):
	tags(tags), shape(shape)
{

}

FeatureLine::~FeatureLine()
{

}

FeatureLine& FeatureLine::operator=(const FeatureLine &arg)
{
	tags = arg.tags;
	shape = arg.shape;
	return *this;
}

FeaturePoi::FeaturePoi()
{

}

FeaturePoi::FeaturePoi(const FeaturePoi &a)
{
	*this = a;
}

FeaturePoi::FeaturePoi(const TagMap &tags, int64_t nid, double lat, double lon) :
	tags(tags), nid(nid), lat(lat), lon(lon)
{

}

FeaturePoi::~FeaturePoi()
{

}

FeaturePoi& FeaturePoi::operator=(const FeaturePoi &arg)
{
	tags = arg.tags;
	nid = arg.nid;
	lat = arg.lat;
	lon = arg.lon;
	return *this;
}

// ********************************

FeatureStore::FeatureStore()
{

}

FeatureStore::FeatureStore(const FeaturePoi &a)
{
	*this = a;
}

FeatureStore::~FeatureStore()
{

}

FeatureStore& FeatureStore::operator=(const FeatureStore &arg)
{
	areas = arg.areas;
	lines = arg.lines;
	pois = arg.pois;
	return *this;
}

void FeatureStore::FoundArea(const TagMap &tags, 
	const std::vector<IdLatLonList> &outerShapes, 
	const std::vector<IdLatLonList> &innerShapes)
{
	this->areas.push_back(FeatureArea(tags, outerShapes, innerShapes));
}

void FeatureStore::FoundLine(const TagMap &tags, 
	const IdLatLonList &shape)
{
	this->lines.push_back(FeatureLine(tags, shape));
}

void FeatureStore::FoundPoi(const TagMap &tags, 
	int64_t nid,
	double lat, double lon)
{
	this->pois.push_back(FeaturePoi(tags, nid, lat, lon));
}

void FeatureStore::Clear()
{
	this->areas.clear();
	this->lines.clear();
	this->pois.clear();
}
