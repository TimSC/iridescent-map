#ifndef _TRANSFORM_H
#define _TRANSFORM_H

class ITransform
{
public:
	virtual double Long2tilex(double lon, int z) = 0;
	virtual double Lat2tiley(double lat, int z) = 0;
	virtual double Tilex2long(int x, int z) = 0;
	virtual double Tiley2lat(int y, int z) = 0;
};

class SlippyTilesTransform : public ITransform
{
public:
	double Long2tilex(double lon, int z);
	double Lat2tiley(double lat, int z);
	double Tilex2long(int x, int z);
	double Tiley2lat(int y, int z);
};

#endif //_TRANSFORM_H

