///\file
#include <fstream>
#include <iostream>
#include <sstream>
#include "ReadInputMbtiles.h"
#include "cppGzip/DecodeGzip.h"

using namespace std;

int ReadFileContents(const char *filename, int binaryMode, std::string &contentOut)
{
	contentOut = "";
	std::ios_base::openmode mode = (std::ios_base::openmode)0;
	if(binaryMode)
		mode ^= std::ios::binary;
	std::ifstream file(filename, mode);
	if(!file) return 0;

	file.seekg(0,std::ios::end);
	std::streampos length = file.tellg();
	file.seekg(0,std::ios::beg);

	std::vector<char> buffer(length);
	file.read(&buffer[0],length);

	contentOut.assign(&buffer[0], length);
	return 1;
}

FeatureDataStore::FeatureDataStore(FeatureStore &featureStoreIn) : DecodeVectorTileResults(), featureStore(featureStoreIn)
{

}

void FeatureDataStore::NumLayers(int numLayers)
{

}

void FeatureDataStore::LayerStart(const char *name, int version)
{

}

void FeatureDataStore::LayerEnd()
{

}

void FeatureDataStore::Feature(int typeEnum, bool hasId, unsigned long long id, 
	const std::map<std::string, std::string> &tagMap,
	std::vector<Point2D> &points, 
	std::vector<std::vector<Point2D> > &lines,
	std::vector<Polygon2D> &polygons)
{
	string typeStr = FeatureTypeToStr(typeEnum);

	if(typeStr == "Point")
		for(size_t i =0; i < points.size(); i++)
		{
			FeaturePoi poi(tagMap, 0, points[i].first, points[i].second);
			this->featureStore.pois.push_back(poi);
		}
	if(typeStr == "LineString")
		for(size_t i =0; i < lines.size(); i++)
		{
			IdLatLonList line;
			vector<Point2D> &linePts = lines[i];
			for(size_t j =0; j < linePts.size(); j++)
			{
				IdLatLon pt(linePts[j].first, linePts[j].second, 0);
				line.push_back(pt);
			}

			FeatureLine lineFeat(tagMap, line);
			this->featureStore.lines.push_back(lineFeat);
		}
	if(typeStr == "Polygon")
		for(size_t i =0; i < polygons.size(); i++)
		{
			Polygon2D &polygon = polygons[i];
			std::vector<IdLatLonList> outerShapes;
			std::vector<IdLatLonList> innerShapes;

			IdLatLonList outerLine;
			LineLoop2D &linePts = polygon.first; //Outer shape
			for(size_t j =0; j < linePts.size(); j++)
			{
				IdLatLon pt(linePts[j].first, linePts[j].second, 0);
				outerLine.push_back(pt);
			}
			outerShapes.push_back(outerLine);

			if(polygon.second.size() > 0)
			{
				//Inner shape
				for(size_t k =0; k < polygon.second.size(); k++)
				{
					IdLatLonList innerLine;
					LineLoop2D &linePts2 = polygon.second[k];
					for(size_t j =0; j < linePts2.size(); j++)
					{
						IdLatLon pt(linePts2[j].first, linePts2[j].second, 0);
						innerLine.push_back(pt);
					}
					innerShapes.push_back(innerLine);
				}
			}

			FeatureArea area(tagMap, outerShapes, innerShapes);
			this->featureStore.areas.push_back(area);
		}
}

///ReadInput reads a file, preprocesses it and outputs a FeatureStore representation of the data.
void ReadInputMbtiles(int zoom, const char *basePath, int xtile, int ytile, FeatureStore &featureStore)
{
	// ** Read input file and store in memory using node/way/relation **
	stringstream finaStr;
	finaStr << basePath << "/";
	finaStr << zoom << "/";
	finaStr << xtile << "/" << ytile << ".vector.pbf";
	string fina = finaStr.str();
	cout << fina << endl;
	std::filebuf fi;
	std::filebuf* ret = fi.open(fina.c_str(), std::ios::in | std::ios::binary);
	if(ret == NULL)
		throw std::runtime_error("Error opening input file");

	//Ungzip the data
	DecodeGzip dec(fi);
	string tileData;

	char tmp[1024];
	while(dec.in_avail())
	{
		streamsize bytes = dec.sgetn(tmp, 1024);
		tileData.append(tmp, bytes);
	}

	//Decode vector data
	class FeatureDataStore results(featureStore);
	class DecodeVectorTile vectorDec(results);
	vectorDec.DecodeTileData(tileData, zoom, xtile, ytile);

	featureStore.Clear();
}

