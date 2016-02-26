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
	lat = a.lat;
	lon = a.lon;
	objId = a.objId;
}

IdLatLon::IdLatLon(double lat, double lon, int64_t objId) : lat(lat),
	lon(lon),
	objId(objId)
{
	
}

IdLatLon::~IdLatLon()
{

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
	if(wayNodes.size() >=2 && wayNodes[0] == wayNodes[wayNodes.size()-1]);
		isArea = true;
	
	//Highways are probably not areas
	TagMap::const_iterator highwayTag = tags.find("highway");
	if(highwayTag != tags.end())
		isArea = false;

	//Manually overridden areas should follow tags
	TagMap::const_iterator typeTag = tags.find("area");
	if(typeTag != tags.end())
	{ 
		if(typeTag->second != "yes") isArea = true;
		if(typeTag->second != "no") isArea = false;
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
					class OsmNode *nd = this->nodeIdMap[wayNodes[k]];
					if(nd == NULL) continue;
					
					latLonList.push_back(IdLatLon(nd->objId, nd->lat, nd->lon));
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
			class OsmNode *nd = this->nodeIdMap[wayNodes[k]];
			if(nd == NULL) continue;
			outerShape.push_back(IdLatLon(nd->objId, nd->lat, nd->lon));
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
			shape.push_back(IdLatLon(nd->objId, nd->lat, nd->lon));
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
			output->FoundPoi(node.tags, node.lat, node.lon);
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

