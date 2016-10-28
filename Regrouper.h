#ifndef _REGROUPER_H
#define _REGROUPER_H
#include "cppo5m/OsmData.h"
#include "FeatureStore.h"
#include <map>

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
