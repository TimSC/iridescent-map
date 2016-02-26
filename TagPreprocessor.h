#ifndef _TAG_PREPROCESSOR_H
#define _TAG_PREPROCESSOR_H
#include "cppo5m/OsmData.h"

class TagPreprocessor : public IDataStreamHandler
{
public:
	class IDataStreamHandler *output;

	TagPreprocessor();
	virtual ~TagPreprocessor();

	void StoreIsDiff(bool);
	void StoreBounds(double x1, double y1, double x2, double y2);
	void StoreNode(int64_t objId, const class MetaData &metaData, 
		const TagMap &tags, double lat, double lon);
	void StoreWay(int64_t objId, const class MetaData &metaDta, 
		const TagMap &tags, const std::vector<int64_t> &refs);
	void StoreRelation(int64_t objId, const class MetaData &metaData, const TagMap &tags, 
		const std::vector<std::string> &refTypeStrs, const std::vector<int64_t> &refIds, 
		const std::vector<std::string> &refRoles);

	void FilterTags(TagMap &tags);
	void SetTagIfEmpty(TagMap &tags, const char* key, const char* value);
};

#endif //_TAG_PREPROCESSOR_H
