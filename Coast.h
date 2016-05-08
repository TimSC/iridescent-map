#ifndef _COAST_H
#define _COAST_H

#include <sstream>
#include <stdint.h>
#include <fstream>
#include <list>
#include "ReadGzip.h"

typedef std::list<std::pair<std::streampos, unsigned char> > CoastMapCacheList;
const int COAST_MAP_CACHE_SIZE = 10;

class CoastMap
{
public:
	CoastMap(const char* filename, int zoom);
	virtual ~CoastMap();
	

	bool GetVal(int x, int y);

	uint32_t width, height;
	int zoom;
	std::ifstream coastBinFi;
	CoastMapCacheList cache;
};

#endif //_COAST_H

