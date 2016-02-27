
#include <cmath>
#include "Transform.h"
#include <iostream>
using namespace std;

SlippyTilesTransform::SlippyTilesTransform(unsigned int zoom, unsigned int xtile, unsigned int ytile):
	zoom(zoom), xtile(xtile), ytile(ytile)
{

}

SlippyTilesTransform::~SlippyTilesTransform()
{

}

double SlippyTilesTransform::Long2tilex(double lon, int z) 
{ 
	return ((lon + 180.0) / 360.0 * pow(2.0, z)); 
}

double SlippyTilesTransform::Lat2tiley(double lat, int z)
{ 
	return (1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z); 
}

double SlippyTilesTransform::Tilex2long(int x, int z) 
{
	return x / pow(2.0, z) * 360.0 - 180;
}

double SlippyTilesTransform::Tiley2lat(int y, int z) 
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

void SlippyTilesTransform::LatLong2Screen(double lat, double lon, double &sxOut, double &syOut)
{
	double tx = Long2tilex(lon, this->zoom);
	tx -= xtile;
	double ty = Lat2tiley(lat, this->zoom);
	ty -= ytile + 1;
	sxOut = tx;
	syOut = - ty;
}

