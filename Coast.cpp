#include "Coast.h"
#include <arpa/inet.h>
#include <iostream>
#include <stdexcept>
using namespace std;

CoastMap::CoastMap(const char* filename, int zoom) : coastBinFi(filename)
{
	this->coastBinFi.seekg(0);
	char sizeValsRaw[8];
	this->coastBinFi.read(sizeValsRaw, 8);

	this->width = ntohl(*(uint32_t *)&sizeValsRaw[0]);
	this->height = ntohl(*(uint32_t *)&sizeValsRaw[4]);
	this->zoom = zoom;
}

CoastMap::~CoastMap()
{
}

bool CoastMap::GetVal(int y, int x)
{
	if (x >= this->width || y >= this->height)
		throw runtime_error("Out of bounds");

	streampos targetPos = 8 + (y * this->width / 8) + x / 8;

	//Check cache
	for(CoastMapCacheList::iterator it = cache.begin(); it != cache.end(); it++)
	{
		if(it->first == targetPos)
		{
			return (it->second & (1 << (7-(x % 8)))) != 0;
		}
	}

	//If not found, read from file
	this->coastBinFi.seekg(targetPos);

	unsigned char byte;
	this->coastBinFi.read((char *)&byte, 1);
	cache.insert(cache.end(), std::pair<streampos, unsigned char>(targetPos, byte));
	if(cache.size() > COAST_MAP_CACHE_SIZE)
		cache.erase(cache.begin());
	return (byte & (1 << (7-(x % 8)))) != 0;
}

