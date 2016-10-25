#ifndef _COAST_H
#define _COAST_H

#include <sstream>
#include <stdint.h>
#include <fstream>
#include <list>
#include "cppGzip/DecodeGzip.h"

typedef std::list<std::pair<std::streampos, unsigned char> > CoastMapCacheList;
const int COAST_MAP_CACHE_SIZE = 10;

class CoastMap
{
public:
	CoastMap(const char* filename);
	virtual ~CoastMap();
	
	bool GetVal(int y, int x);

	uint32_t width, height;
	int zoom;
	std::ifstream coastBinFi;
	CoastMapCacheList cache;
};

#endif //_COAST_H

