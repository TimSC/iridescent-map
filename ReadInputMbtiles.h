#ifndef _READ_INPUT_MBTILES_H
#define _READ_INPUT_MBTILES_H

#include <vector>
#include <map>
#include "Regrouper.h"
#include "mbtiles-cpp/VectorTile.h"
#include "FeatureStore.h"

void ReadInputMbtiles(int zoom, const char *basePath, int xtile, int ytile, FeatureStore &featureStore);
int ReadFileContents(const char *filename, int binaryMode, std::string &contentOut);

class FeatureDataStore : public DecodeVectorTileResults
{
public:
	FeatureStore &featureStore;

	FeatureDataStore(FeatureStore &featureStoreIn);

	void NumLayers(int numLayers);
	void LayerStart(const char *name, int version);
	void LayerEnd();

	void Feature(int typeEnum, bool hasId, unsigned long long id, 
		const std::map<std::string, std::string> &tagMap,
		std::vector<Point2D> &pointsOut, 
		std::vector<std::vector<Point2D> > &linesOut,
		std::vector<Polygon2D> &polygonsOut);
};

#endif //_READ_INPUT_MBTILES_H

