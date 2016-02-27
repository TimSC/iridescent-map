#ifndef _TRANSFORM_H
#define _TRANSFORM_H

class ITransform
{
public:
	virtual void LatLong2Screen(double lat, double lon, double &sxOut, double &syOut) = 0;
};

class SlippyTilesTransform : public ITransform
{
public:
	unsigned int zoom;
	unsigned int xtile; 
	unsigned int ytile;

	SlippyTilesTransform(unsigned int zoom, unsigned int xtile, unsigned int ytile);
	virtual ~SlippyTilesTransform();

	double Long2tilex(double lon, int z);
	double Lat2tiley(double lat, int z);
	double Tilex2long(int x, int z);
	double Tiley2lat(int y, int z);
	void LatLong2Screen(double lat, double lon, double &sxOut, double &syOut);
};

#endif //_TRANSFORM_H

