///\file
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

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
	std::filebuf* ret = fi.open(fina.c_str(), std::ios::in);
	if(ret == NULL)
		throw std::runtime_error("Error opening input file");


	



	featureStore.Clear();
}

