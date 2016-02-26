#include "cppo5m/OsmData.h"
#include "TagPreprocessor.h"
#include "Regrouper.h"
#include "LayerDrawManager.h"
#include <fstream>
#include <iostream>
using namespace std;



int main()
{
	std::ifstream fi("1374.o5m");

	class Regrouper regrouper;

	class TagPreprocessor tagPreprocessor;
	tagPreprocessor.output = &regrouper;	

	class O5mDecode dec(fi);
	dec.output = &tagPreprocessor;
	dec.DecodeHeader();

	while (!fi.eof())
		dec.DecodeNext();
	
	regrouper.UpdateIdMappings();

	FeatureStore featureStore;
	regrouper.FindAreas(&featureStore);
	regrouper.FindLines(&featureStore);
	regrouper.FindPois(&featureStore);
	
	class LayerDrawManager layerDrawManager;
	std::set<int> layerNums = layerDrawManager.FindLayers(featureStore);

	for(std::set<int>::iterator it = layerNums.begin(); it != layerNums.end(); it++)
	{
		cout << *it << endl;
	}
}

