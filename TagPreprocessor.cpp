
#include "TagPreprocessor.h"

TagPreprocessor::TagPreprocessor()
{
	output = NULL;
}

TagPreprocessor::~TagPreprocessor()
{

}

void TagPreprocessor::StoreIsDiff(bool isDiff)
{
	if(output != NULL)
		output->StoreIsDiff(isDiff);
}

void TagPreprocessor::StoreBounds(double x1, double y1, double x2, double y2)
{
	if(output != NULL)
		output->StoreBounds(x1, y1, x2, y2);
}

void TagPreprocessor::StoreNode(int64_t objId, const class MetaData &metaData, 
	const TagMap &tags, double lat, double lon)
{
	TagMap localCopy = tags;
	FilterTags(localCopy);
	if(output != NULL)
		output->StoreNode(objId, metaData, localCopy, lat, lon);
}

void TagPreprocessor::StoreWay(int64_t objId, const class MetaData &metaDta, 
	const TagMap &tags, const std::vector<int64_t> &refs)
{
	TagMap localCopy = tags;
	FilterTags(localCopy);
	if(output != NULL)
		output->StoreWay(objId, metaDta, localCopy, refs);
}

void TagPreprocessor::StoreRelation(int64_t objId, const class MetaData &metaData, const TagMap &tags, 
	const std::vector<std::string> &refTypeStrs, const std::vector<int64_t> &refIds, 
	const std::vector<std::string> &refRoles)
{
	TagMap localCopy = tags;
	FilterTags(localCopy);
	if(output != NULL)
		output->StoreRelation(objId, metaData, localCopy, 
			refTypeStrs, refIds, refRoles);
}

void TagPreprocessor::SetTagIfEmpty(TagMap &tags, const char* key, const char* value)
{
	TagMap::iterator keyChk = tags.find(key);
	if(keyChk == tags.end())
		tags[key] = value;
}

void TagPreprocessor::FilterTags(TagMap &tags)
{
	TagMap::iterator desigTag = tags.find("designation");
	if(desigTag != tags.end())
	{
		if(desigTag->second == "public_footpath")
		{
			SetTagIfEmpty(tags, "foot", "designated");
		}
		if(desigTag->second == "public_bridleway")
		{
			SetTagIfEmpty(tags, "foot", "designated");
			SetTagIfEmpty(tags, "bicycle", "designated");
			SetTagIfEmpty(tags, "horse", "designated");
		}
		if(desigTag->second == "restricted_byway")
		{
			SetTagIfEmpty(tags, "foot", "designated");
			SetTagIfEmpty(tags, "bicycle", "designated");
			SetTagIfEmpty(tags, "horse", "designated");
		}
		if(desigTag->second == "byway_open_to_all_traffic")
		{
			SetTagIfEmpty(tags, "foot", "designated");
			SetTagIfEmpty(tags, "bicycle", "designated");
			SetTagIfEmpty(tags, "horse", "designated");
			SetTagIfEmpty(tags, "vehicle", "designated");
		}
	}
}

