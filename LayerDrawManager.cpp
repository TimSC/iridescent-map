
#include "LayerDrawManager.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

LayerDrawManager::LayerDrawManager()
{

}

LayerDrawManager::~LayerDrawManager()
{

}

void LayerDrawManager::ExtractLayerFromTags(const TagMap &tags, std::set<int> &layerSet)
{
	TagMap::const_iterator it = tags.find("layer");
	int layer = 0;
	if(it != tags.end())
	{
		layer = atoi(it->second.c_str());
	}
	layerSet.insert(layer);
}

std::set<int> LayerDrawManager::FindLayers(class FeatureStore &store)
{
	std::set<int> layerSet;
	for(size_t i =0; i < store.areas.size(); i++)
		this->ExtractLayerFromTags(store.areas[i].tags, layerSet);
	for(size_t i =0; i < store.lines.size(); i++)
		this->ExtractLayerFromTags(store.lines[i].tags, layerSet);
	for(size_t i =0; i < store.pois.size(); i++)
		this->ExtractLayerFromTags(store.pois[i].tags, layerSet);
	return layerSet;
}

/*
void LayerDrawManager::DrawLayers(class FeatureStore &store)
{
	for(std::set<int>::iterator it = layerSet.begin(); it != layerSet.end(); it++)
	{
		cout << *it << endl;
	}
}
*/

