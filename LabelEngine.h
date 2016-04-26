#ifndef _LABEL_ENGINE_H
#define _LABEL_ENGINE_H

#include "drawlib/drawlib.h"
#include "cppo5m/o5m.h"
#include "Style.h"

typedef std::map<std::string, std::pair<unsigned, unsigned> > ResourceSizeCache;

class LabelBounds
{
protected:
	TwistedTriangles bounds;
	double minX, minY, maxX, maxY;
	void UpdateRectBbox();

public:
	LabelBounds();
	LabelBounds(const TwistedTriangles &bounds);
	LabelBounds(const class LabelBounds &a);
	virtual ~LabelBounds();
	LabelBounds& operator=(const LabelBounds &arg);

	bool Overlaps(const LabelBounds &arg) const;
	void Translate(double tx, double ty);
};

class LabelIcon
{
public:
	double x, y;
	std::string iconFile;

	LabelIcon();
	LabelIcon(const class LabelIcon &a);
	virtual ~LabelIcon();
	LabelIcon& operator=(const LabelIcon &arg);

	void Translate(double tx, double ty);
};

///A label/icon group with low level drawing API information, with bounding info.
class LabelDef
{
public:
	class LabelBounds labelBounds;
	class TextProperties properties;
	std::vector<class TextLabel> labels;
	std::vector<class TwistedTextLabel> twistedLabels;
	std::vector<class LabelIcon> icons;

	LabelDef();
	LabelDef(const class LabelBounds &labelBounds,
		const class TextProperties &properties,
		const std::vector<class TextLabel> &labels);
	LabelDef(const class LabelBounds &labelBounds,
		const class TextProperties &properties,
		const std::vector<class TwistedTextLabel> &twistedLabels);
	LabelDef(const class LabelDef &a);
	virtual ~LabelDef();
	LabelDef& operator=(const LabelDef &arg);

	void Translate(double tx, double ty);
};

typedef std::map<int, std::vector<class LabelDef> > LabelsByImportance;
void MergeLabelsByImportance(LabelsByImportance &mergeIntoThis, const LabelsByImportance &labelsToMerge);
void TranslateLabelsByImportance(const LabelsByImportance &labelsIn, double tx, double ty, LabelsByImportance &labelsOut);
void SmoothLabelPaths(std::vector<class PoiLabel> &poiLabels, double tolerance);

///An object to label in draw space, as well as tags and map style definition.
class PoiLabel
{
public:
	Contour shape;
	std::string textName;
	TagMap tags;
	StyleAttributes styleAttributes;

	PoiLabel();
	PoiLabel(const Contour &shape, const std::string &textName, const TagMap &tags, const StyleAttributes &styleAttributes);
	PoiLabel(const class PoiLabel &a);
	virtual ~PoiLabel();
	PoiLabel& operator=(const PoiLabel &arg);
};

class LabelEngine
{
protected:
	class IDrawLib *output; //Borrowed reference
	std::string outString;
	int textSize;
	double fillR, fillG, fillB, fillA;
	double haloR, haloG, haloB, haloA;
	double haloWidth;
	std::string placement;
	std::string markerFile;
	ResourceSizeCache resourceSizeCache;

	bool LayoutIconAndText(const class PoiLabel &label, bool enableIcon, bool enableText, LabelsByImportance &organisedLabelsOut, LabelDef &labelDefOut);
	void GetResourceSizeCached(const std::string &filename, unsigned &widthOut, unsigned &heightOut);

public:
	LabelEngine(class IDrawLib *output);
	virtual ~LabelEngine();

	void LabelPoisToStyledLabel(const std::vector<class PoiLabel> &poiLabels, LabelsByImportance &organisedLabelsOut);
	void WriteDrawCommands(const LabelsByImportance &organisedLabels);
	void RemoveOverlapping(const LabelsByImportance &organisedLabelsTmp, LabelsByImportance &organisedLabelsOut);
};

#endif //_LABEL_ENGINE_H
