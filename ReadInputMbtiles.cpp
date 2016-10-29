///\file
#include <fstream>
#include <iostream>
#include <sstream>
#include "FeatureStore.h"
#include "mbtiles-cpp/VectorTile.h"
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

class ExampleDataStore : public DecodeVectorTileResults
{
public:
	ExampleDataStore() : DecodeVectorTileResults()
	{
		cout << "Create custom data store..." << endl;
	}

	void Feature(int typeEnum, bool hasId, unsigned long long id, 
		const std::map<std::string, std::string> &tagMap,
		std::vector<Point2D> &pointsOut, 
		std::vector<std::vector<Point2D> > &linesOut,
		std::vector<Polygon2D> &polygonsOut)
	{
		//In real use, delete this function call and add your own functionality.
		DecodeVectorTileResults::Feature(typeEnum, hasId, id, 
			tagMap, pointsOut, linesOut, polygonsOut);
	}
};

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
	class ExampleDataStore results;
	class DecodeVectorTile vectorDec(results);
	vectorDec.DecodeTileData(tileData, zoom, xtile, ytile);

	featureStore.Clear();
}

