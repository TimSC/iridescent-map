#include "Regrouper.h"
#include <iostream>
using namespace std;

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

// ******************************

Regrouper::Regrouper() : OsmData()
{

}

Regrouper::~Regrouper()
{

}

bool Regrouper::DetectIfArea(const std::vector<int64_t> &wayNodes, const TagMap &tags)
{
	//Closed ways are probably areas
	bool isArea = false;
	if(wayNodes.size() >=2 && wayNodes[0] == wayNodes[wayNodes.size()-1])
		isArea = true;
	
	//Highways are probably not areas
	TagMap::const_iterator chkTag = tags.find("highway");
	if(chkTag != tags.end())
		isArea = false;

	chkTag = tags.find("landuse");
	if(chkTag != tags.end())
		isArea = true;

	//Manually overridden areas should follow tags
	TagMap::const_iterator typeTag = tags.find("area");
	if(typeTag != tags.end())
	{ 
		if(typeTag->second == "yes") isArea = true;
		if(typeTag->second == "no") isArea = false;
	}
	return isArea;
}

void Regrouper::FindAreas(class IRegroupResultHandler *output)
{
	//Find and process mutlipolygon areas
	for(size_t i=0; i < this->relations.size();i++)
	{
		class OsmRelation &relation = this->relations[i];
		TagMap::iterator typeTag = relation.tags.find("type");
		if(typeTag == relation.tags.end() || typeTag->second != "multipolygon")
			continue;
		//cout << relation.objId << endl;

		std::vector<IdLatLonList> innerShapes, outerShapes;
		for(size_t j=0; j < relation.refIds.size(); j++)
		{
			std::string &refType = relation.refTypeStrs[j];
			int64_t refId = relation.refIds[j];
			std::string &refRole = relation.refRoles[j];

			//cout << refType << "," << refId << "," << refRole << endl;
			
			if(refType == "way")
			{
				IdLatLonList latLonList;
				class OsmWay *refWay = this->wayIdMap[refId];
				if(refWay == NULL) continue;

				std::vector<int64_t> &wayNodes = refWay->refs;
				for(size_t k =0; k < wayNodes.size(); k++)
				{
					if(k == wayNodes.size()-1 && wayNodes[k] == wayNodes[0])
						continue; //Don't include node twice since this is an area and assumed to be closed.

					class OsmNode *nd = this->nodeIdMap[wayNodes[k]];
					if(nd == NULL) continue;
					
					latLonList.push_back(IdLatLon(nd->lat, nd->lon, nd->objId));
				}
				if(refRole == "inner") innerShapes.push_back(latLonList);
				if(refRole == "outer") outerShapes.push_back(latLonList);

			}
		}

		//TODO What about outer shapes that are split into multiple ways?

		//TODO distinguish between closed and unclosed shapes

		//Output the overall shape
		if(output != NULL)
			output->FoundArea(relation.tags, outerShapes, innerShapes);
	}

	//Output ways that are areas
	for(size_t i=0; i < this->ways.size();i++)
	{
		class OsmWay &way = this->ways[i];
		std::vector<int64_t> &wayNodes = way.refs;

		bool isArea = DetectIfArea(wayNodes, way.tags);
		if(!isArea) continue;

		//Get shape
		std::vector<IdLatLonList> innerShapes, outerShapes;
		IdLatLonList outerShape;
		for(size_t k =0; k < wayNodes.size(); k++)
		{
			if(k == wayNodes.size()-1 && wayNodes[k] == wayNodes[0])
				continue; //Don't include node twice since this is an area and assumed to be closed.

			class OsmNode *nd = this->nodeIdMap[wayNodes[k]];
			if(nd == NULL) continue;
			outerShape.push_back(IdLatLon(nd->lat, nd->lon, nd->objId));
		}
		outerShapes.push_back(outerShape);

		if(output != NULL)
			output->FoundArea(way.tags, outerShapes, innerShapes);
	}
}

void Regrouper::FindLines(class IRegroupResultHandler *output)
{
	//Output ways that are lines
	for(size_t i=0; i < this->ways.size();i++)
	{
		class OsmWay &way = this->ways[i];
		std::vector<int64_t> &wayNodes = way.refs;

		bool isArea = DetectIfArea(wayNodes, way.tags);
		if(isArea) continue;

		//Get shape
		IdLatLonList shape;
		for(size_t k =0; k < wayNodes.size(); k++)
		{
			class OsmNode *nd = this->nodeIdMap[wayNodes[k]];
			if(nd == NULL) continue;
			shape.push_back(IdLatLon(nd->lat, nd->lon, nd->objId));
		}

		if(output != NULL)
			output->FoundLine(way.tags, shape);
	}
}

void Regrouper::FindPois(class IRegroupResultHandler *output)
{
	//Output nodes that are POIs
	for(size_t i=0; i < this->nodes.size();i++)
	{
		class OsmNode &node = this->nodes[i];
		
		if(node.tags.size() == 0)
			continue; //Empty tags indicates this is not a POI

		if(output != NULL)
			output->FoundPoi(node.tags, node.objId, node.lat, node.lon);
	}
}

void Regrouper::UpdateIdMappings()
{
	this->nodeIdMap.clear();
	this->wayIdMap.clear();
	this->relationIdMap.clear();

	for(size_t i=0; i < this->nodes.size();i++)
	{
		class OsmNode &node = this->nodes[i];
		this->nodeIdMap[node.objId] = &node;
	}

	for(size_t i=0; i < this->ways.size();i++)
	{
		class OsmWay &way = this->ways[i];
		this->wayIdMap[way.objId] = &way;
	}

	for(size_t i=0; i < this->relations.size();i++)
	{
		class OsmRelation &relation = this->relations[i];
		this->relationIdMap[relation.objId] = &relation;
	}
}

