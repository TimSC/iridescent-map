#include "LabelEngine.h"
#include <iostream>
#include <stdlib.h>
#include <stdexcept>
#include "TriTri2d.h"
#include "drawlib/RdpSimplify.h"
using namespace std;

bool LabelOverlaps(const class LabelBounds &labelBounds, const LabelsByImportance &existingLabels);

// *************************************

LabelDef::LabelDef()
{

}

LabelDef::LabelDef(const class LabelBounds &labelBounds,
		const class TextProperties &properties,
		const std::vector<class TextLabel> &labels) : labelBounds(labelBounds),
		properties(properties),
		labels(labels)
{

}

LabelDef::LabelDef(const class LabelBounds &labelBounds,
		const class TextProperties &properties,
		const std::vector<class TwistedTextLabel> &twistedLabels) : labelBounds(labelBounds),
		properties(properties),
		twistedLabels(twistedLabels)
{

}

LabelDef::LabelDef(const class LabelDef &a) : labelBounds(a.labelBounds)
{
	*this = a;
}

LabelDef::~LabelDef()
{

}

LabelDef& LabelDef::operator=(const LabelDef &arg)
{
	labelBounds = arg.labelBounds;
	properties = arg.properties;
	labels = arg.labels;
	twistedLabels = arg.twistedLabels;
	icons = arg.icons;
}

void LabelDef::Translate(double tx, double ty)
{
	labelBounds.Translate(tx, ty);
	for(size_t i=0; i<labels.size();i++)
	{
		class TextLabel &tl = labels[i];
		tl.Translate(tx, ty);
	}
	for(size_t i=0; i<twistedLabels.size();i++)
	{
		class TwistedTextLabel &tl = twistedLabels[i];
		tl.Translate(tx, ty);
	}
	for(size_t i=0; i<icons.size();i++)
	{
		class LabelIcon &ic = icons[i];
		ic.Translate(tx, ty);
	}
}

// **********************************************

void MergeLabelsByImportance(LabelsByImportance &mergeIntoThis, const LabelsByImportance &labelsToMerge)
{
	for(LabelsByImportance::const_iterator itr = labelsToMerge.begin(); itr != labelsToMerge.end(); itr++)
	{
		int importance = itr->first;
		const vector<class LabelDef> &lbs = itr->second;
		LabelsByImportance::iterator mergeIt = mergeIntoThis.find(importance);
		if(mergeIt == mergeIntoThis.end())
			mergeIntoThis[importance] = vector<class LabelDef>();
		vector<class LabelDef> &layer = mergeIntoThis[importance];
		for(size_t i=0; i<lbs.size(); i++)
			layer.push_back(lbs[i]);
	}
}

void TranslateLabelsByImportance(const LabelsByImportance &labelsIn, double tx, double ty, LabelsByImportance &labelsOut)
{
	labelsOut.clear();
	for(LabelsByImportance::const_iterator itr = labelsIn.begin(); itr != labelsIn.end(); itr++)
	{
		int importance = itr->first;
		const vector<class LabelDef> &lbs = itr->second;
		vector<class LabelDef> layer;
		for(size_t i=0; i<lbs.size(); i++)
		{
			class LabelDef labelCpy = lbs[i];
			labelCpy.Translate(tx, ty);
			layer.push_back(labelCpy);
		}
		labelsOut[importance] = layer;
	}
}

// **********************************************

void ValidateTriangles(const TwistedTriangles &bounds)
{
	//Validate triangles
	for(size_t i = 0; i < bounds.size(); i++)
	{
		const std::vector<Point> &tri = bounds[i];
		if(tri.size() != 3)
			throw std::runtime_error("triange must have three sides");
		double det = Det2D(tri[0], tri[1], tri[2]);
		if(det < 0.0)
			throw std::runtime_error("triangle has wrong winding direction");
	}
}

void UrlToLocalFile(const string &inStr, string &outStr)
{
	//Input in the form url('symbols/pub.png')
	if(inStr.substr(0, 5) == "url('")
	{
		outStr = inStr.substr(5, inStr.size()-7);
	}
}

// **********************************************

LabelBounds::LabelBounds()
{
	this->minX = 0.0; this->minY = 0.0; this->maxX = 0.0; this->maxY = 0.0;
}

LabelBounds::LabelBounds(const TwistedTriangles &bounds) : bounds(bounds)
{
#ifdef ENABLE_TRI_VALIDATION
	ValidateTriangles(this->bounds);
#endif //ENABLE_TRI_VALIDATION
	this->UpdateRectBbox();
}

LabelBounds::LabelBounds(const class LabelBounds &a)
{
	*this = a;
}

LabelBounds::~LabelBounds()
{

}

void LabelBounds::UpdateRectBbox()
{
	bool boundsSet = false;
	for(size_t i = 0; i < this->bounds.size(); i++)
	{
		const std::vector<Point> &tri = this->bounds[i];
		for(size_t j=0; j < tri.size(); j++)
		{
			const Point &pt = tri[j];
			if(boundsSet)
			{
				if(pt.first > this->maxX)
					this->maxX = pt.first;
				if(pt.first < this->minX)
					this->minX = pt.first;
				if(pt.second > this->maxY)
					this->maxY = pt.second;
				if(pt.second < this->minY)
					this->minY = pt.second;
			}
			else
			{
				this->minX = pt.first;
				this->minY = pt.second;
				this->maxX = pt.first;
				this->maxY = pt.second;
				boundsSet = true;
			}
		}
	}
	if(!boundsSet)
	{
		this->minX = 0.0; this->minY = 0.0; this->maxX = 0.0; this->maxY = 0.0;
	}
}

LabelBounds& LabelBounds::operator=(const LabelBounds &arg)
{
	this->bounds = arg.bounds;
#ifdef ENABLE_TRI_VALIDATION
	ValidateTriangles(this->bounds);
#endif //ENABLE_TRI_VALIDATION
	this->UpdateRectBbox();
	return *this;
}

bool LabelBounds::Overlaps(const LabelBounds &arg) const
{
	//Rough rectangle collision code (fast but inexact)
	if (maxX < arg.minX) return false;
	if (minX >= arg.maxX) return false;
	if (maxY < arg.minY) return false;
	if (minY >= arg.maxY) return false;

	//Exact triangle collision code
	for(size_t i = 0; i < this->bounds.size(); i++)
	{
		const std::vector<Point> &tri1 = this->bounds[i];
		for(size_t j = 0; j < arg.bounds.size(); j++)
		{
			const std::vector<Point> &tri2 = arg.bounds[j];
			if(TriTri2D(&tri1[0], &tri2[0])) return true;
		}
	}

	return false;
}

void LabelBounds::Translate(double tx, double ty)
{
	for(size_t i = 0; i < this->bounds.size(); i++)
	{
		std::vector<Point> &tri = this->bounds[i];
		for(size_t j = 0; j < tri.size(); j++)
		{
			Point &pt = tri[j];
			pt.first += tx;
			pt.second += ty;
		}
	}
}

// ***************************************************

LabelIcon::LabelIcon()
{
	x = 0.0; y = 0.0;
	iconFile = "";
}

LabelIcon::LabelIcon(const class LabelIcon &a)
{
	*this = a;
}

LabelIcon::~LabelIcon()
{

}

LabelIcon& LabelIcon::operator=(const LabelIcon &arg)
{
	x=arg.x;
	y=arg.y;
	iconFile=arg.iconFile;
}

void LabelIcon::Translate(double tx, double ty)
{
	x += tx;
	y += ty;
}

// ***************************************************

PoiLabel::PoiLabel()
{

}

PoiLabel::PoiLabel(const Contour &shape, const std::string &textName, const TagMap &tags, const StyleAttributes &styleAttributes):
	shape(shape), textName(textName), tags(tags), styleAttributes(styleAttributes)
{
	
}

PoiLabel::PoiLabel(const class PoiLabel &a)
{
	*this = a;
}

PoiLabel::~PoiLabel()
{

}

PoiLabel& PoiLabel::operator=(const PoiLabel &arg)
{
	shape = arg.shape;
	textName = arg.textName;
	tags = arg.tags;
	styleAttributes = arg.styleAttributes;
}

// *******************************************************

LabelEngine::LabelEngine(class IDrawLib *output):
	output(output)
{
	outString = "";
	textSize = 9;
	fillR=1.0; fillG=1.0; fillB=1.0; fillA = 1.0;
	haloR=0.0; haloG=0.0; haloB=0.0; haloA=0.0;
	haloWidth=2.0;
	placement = "point";
	markerFile = "";
}

LabelEngine::~LabelEngine()
{

}

void LabelEngine::GetResourceSizeCached(const std::string &filename, unsigned &widthOut, unsigned &heightOut)
{
	ResourceSizeCache::iterator it = this->resourceSizeCache.find(filename);
	if(it == resourceSizeCache.end())
	{
		this->output->GetResourceDimensionsFromFilename(filename, widthOut, heightOut);
		this->resourceSizeCache[filename] = std::pair<unsigned, unsigned>(widthOut, heightOut);
	}
	else
	{
		const std::pair<unsigned, unsigned> &resSize = it->second;
		widthOut = resSize.first;
		heightOut = resSize.second;
	}
}

bool LabelEngine::LayoutIconAndText(const class PoiLabel &label, bool enableIcon, bool enableText, LabelsByImportance &organisedLabelsOut, LabelDef &labelDefOut)
{
	unsigned resWidth=0, resHeight=0;
	if(this->markerFile.size() > 0 && enableIcon)
		this->GetResourceSizeCached(this->markerFile, resWidth, resHeight);
	TwistedTriangles bounds;
	std::vector<class TextLabel> textStrs;
	class TextProperties labelProperties(fillR, fillG, fillB);
	if(enableText)
	{
		//Define draw styles
		labelProperties.fontSize = textSize;
		labelProperties.halign = 0.5;
		labelProperties.fa = fillA;
		labelProperties.lr = haloR;
		labelProperties.lg = haloG;
		labelProperties.lb = haloB;
		labelProperties.la = haloA;
		if(haloA > 0.0)
			labelProperties.outline = true;
		else 
			labelProperties.outline = false;
		labelProperties.fill = true;
		labelProperties.lineWidth=haloWidth;

		//Get bounds for text
		double lx = label.shape[0].first;
		double ly = label.shape[0].second;
		//if(this->markerFile.size() > 0 && enableIcon)
		//	ly -= resHeight / 2.0;
		TextLabel outLabel(outString, lx, ly + resHeight / 2.0);
		if(this->output != NULL)
		{
			this->output->GetTriangleBoundsText(outLabel, labelProperties, 
				bounds);
		}

		textStrs.push_back(outLabel);
	}
	
	//Icon bounds
	LabelIcon labelIcon;
	if(this->markerFile.size() > 0 && enableIcon)
	{
		double lx = label.shape[0].first;
		double ly = label.shape[0].second;
		labelIcon.x = lx - resWidth / 2.0;
		labelIcon.y = ly - resHeight / 2.0;
		labelIcon.iconFile = markerFile;
		std::vector<Point> iconTri1;
		iconTri1.push_back(Point(labelIcon.x, labelIcon.y));
		iconTri1.push_back(Point(labelIcon.x+resWidth, labelIcon.y));
		iconTri1.push_back(Point(labelIcon.x+resWidth, labelIcon.y+resHeight));
		bounds.push_back(iconTri1);
		std::vector<Point> iconTri2;
		iconTri2.push_back(Point(labelIcon.x, labelIcon.y));
		iconTri2.push_back(Point(labelIcon.x+resWidth, labelIcon.y+resHeight));
		iconTri2.push_back(Point(labelIcon.x, labelIcon.y+resHeight));
		bounds.push_back(iconTri2);
	}

	//Check label does not collide with any existing labels
	class LabelBounds labelBounds(bounds);
	if (LabelOverlaps(labelBounds, organisedLabelsOut)) return false;

	LabelDef labelDef(labelBounds, labelProperties, textStrs);
	if(this->markerFile.size() > 0 && enableIcon)
		labelDef.icons.push_back(labelIcon);
	labelDefOut = labelDef;
	return true;
}

void LabelEngine::LabelPoisToStyledLabel(const std::vector<class PoiLabel> &poiLabels, LabelsByImportance &organisedLabelsOut)
{
	organisedLabelsOut.clear();

	map<int, vector<const class PoiLabel *> > sortedByImportance;

	//Sort labels by importance
	for(size_t i=0;i < poiLabels.size(); i++)
	{
		const class PoiLabel &label = poiLabels[i];

		int importance = 0;
		StyleAttributes::const_iterator paramIt = label.styleAttributes.find("text-importance");
		if(paramIt != label.styleAttributes.end())
			importance = atoi(paramIt->second.c_str());

		map<int, vector<const class PoiLabel *> >::iterator importanceGroup = sortedByImportance.find(importance);
		if(importanceGroup == sortedByImportance.end())
		{
			sortedByImportance[importance] = vector<const class PoiLabel *>();
			importanceGroup = sortedByImportance.find(importance);
		}
		importanceGroup->second.push_back(&label);
		
	}

	//Created styled labels in importance order, check for collisions and try to fit in available space
	for(map<int, vector<const class PoiLabel *> >::reverse_iterator it = sortedByImportance.rbegin();
		it != sortedByImportance.rend(); it++)
	{
		int importance = it->first ;
		vector<const class PoiLabel *> &groupLabels = it->second;

		for(size_t i=0;i < groupLabels.size(); i++)
		{
			const class PoiLabel &label = *groupLabels[i];

			//Extract final draw parameters from tags and style
			const std::string &textName = label.textName;
			this->outString = "";
			if(textName[0] == '[' && textName[textName.size()-1] == ']') 
			{
				std::string keyName(&textName[1], textName.size()-2);
				TagMap::const_iterator it = label.tags.find(keyName);
				if(it == label.tags.end()) continue;
				this->outString = it->second;
			}
			else
				this->outString = textName;

			this->textSize = 9;
			StyleAttributes::const_iterator paramIt = label.styleAttributes.find("text-size");
			if(paramIt != label.styleAttributes.end())
				this->textSize = atoi(paramIt->second.c_str());

			this->fillR=1.0; this->fillG=1.0; this->fillB=1.0; this->fillA = 1.0;
			paramIt = label.styleAttributes.find("text-fill");
			if(paramIt != label.styleAttributes.end())
				ColourStringToRgba(paramIt->second.c_str(), this->fillR, this->fillG, this->fillB, this->fillA);

			this->haloR=0.0; this->haloG=0.0; this->haloB=0.0; this->haloA=0.0;
			paramIt = label.styleAttributes.find("text-halo-fill");
			if(paramIt != label.styleAttributes.end())
				ColourStringToRgba(paramIt->second.c_str(), this->haloR, this->haloG, this->haloB, this->haloA);

			this->haloWidth=2.0;
			paramIt = label.styleAttributes.find("text-halo-radius");
			if(paramIt != label.styleAttributes.end())
				this->haloWidth = atof(paramIt->second.c_str());

			this->placement = "point";
			paramIt = label.styleAttributes.find("text-placement");
			if(paramIt != label.styleAttributes.end())
				this->placement = paramIt->second;

			this->markerFile = "";
			paramIt = label.styleAttributes.find("marker-file");
			if(paramIt != label.styleAttributes.end())
				UrlToLocalFile(paramIt->second, this->markerFile);

			//A shape is required for any drawing to happen
			if(label.shape.size() == 0) continue;

			if(placement == "point")
			{
				//Try layouts with and without icon and text to make it visible
				LabelDef labelDef;
				bool labelOk = LayoutIconAndText(label, true, true, organisedLabelsOut, labelDef);
				if(!labelOk) labelOk = LayoutIconAndText(label, false, true, organisedLabelsOut, labelDef);
				if(!labelOk) labelOk = LayoutIconAndText(label, true, false, organisedLabelsOut, labelDef);
				if(!labelOk) continue;	

				//Add label definition to list
				LabelsByImportance::iterator it = organisedLabelsOut.find(importance);
				if(it == organisedLabelsOut.end())
					organisedLabelsOut[importance] = vector<class LabelDef>();
				organisedLabelsOut[importance].push_back(labelDef);
			}
		
			if(placement == "line")
			{
				//Define draw styles
				class TextProperties labelProperties(fillR, fillG, fillB);
				labelProperties.fontSize = textSize;
				labelProperties.valign = 0.7;
				labelProperties.fa = fillA;
				labelProperties.lr = haloR;
				labelProperties.lg = haloG;
				labelProperties.lb = haloB;
				labelProperties.la = haloA;
				if(haloA > 0.0)
					labelProperties.outline = true;
				else 
					labelProperties.outline = false;
				labelProperties.fill = true;

				labelProperties.lineWidth=haloWidth;

				//Get bounds
				std::vector<TwistedCurveCmd> path;
				FixBezierToPoints(label.shape, path);
				TwistedTextLabel outLabel(outString, path);
				TwistedTriangles bounds;
				double pathLen = -1.0, textLen = -1.0;
				if(this->output != NULL)
				{
					this->output->GetTriangleBoundsTwistedText(outLabel, labelProperties, 
						bounds, pathLen, textLen);
				}

				//Don't draw if string is too long for path
				if(pathLen >= 0.0 && textLen >= 0.0 && textLen > pathLen)
					continue;
	
				std::vector<class TwistedTextLabel> textStrs;
				textStrs.push_back(outLabel);

				class LabelBounds labelBounds(bounds);

				//Check label does not collide with any existing labels
				if (LabelOverlaps(labelBounds, organisedLabelsOut)) continue;

				//Add label definition to list
				LabelsByImportance::iterator it = organisedLabelsOut.find(importance);
				if(it == organisedLabelsOut.end())
					organisedLabelsOut[importance] = vector<class LabelDef>();
				organisedLabelsOut[importance].push_back(LabelDef(labelBounds, labelProperties, textStrs));
			}
		}
	}
}

bool LabelOverlaps(const class LabelBounds &labelBounds, const LabelsByImportance &existingLabels)
{
	for(LabelsByImportance::const_iterator itr = existingLabels.begin(); itr != existingLabels.end(); itr++)
	{
		const vector<class LabelDef> &lbs = itr->second;
		for(size_t j=0; j<lbs.size(); j++)
		{
			if(labelBounds.Overlaps(lbs[j].labelBounds))
				return true;
		}
	}
	return false;
}

void LabelEngine::RemoveOverlapping(const LabelsByImportance &organisedLabelsTmp, LabelsByImportance &organisedLabelsOut)
{
	organisedLabelsOut.clear();
	
	//Starting with high imporance labels, check for overlaps
	for(LabelsByImportance::const_reverse_iterator itr = organisedLabelsTmp.rbegin(); itr != organisedLabelsTmp.rend(); itr++)
	{
		int importance = itr->first;
		const vector<class LabelDef> &lbs = itr->second;
		for(size_t j=0; j<lbs.size(); j++)
		{
			const class LabelDef &label = lbs[j];
			bool overlaps = LabelOverlaps(label.labelBounds, organisedLabelsOut);
			if(!overlaps)
			{
				LabelsByImportance::iterator chkIt = organisedLabelsOut.find(importance);
				if(chkIt == organisedLabelsOut.end())
					organisedLabelsOut[importance] = vector<class LabelDef>();
				organisedLabelsOut[importance].push_back(label);
			}
		}
	}
}

void LabelEngine::WriteDrawCommands(const LabelsByImportance &organisedLabels)
{
	//Load resources
	std::map<std::string, std::string> loadIdToFilenameMapping;
	for(LabelsByImportance::const_iterator itr = organisedLabels.begin(); itr != organisedLabels.end(); itr++)
	{
		const vector<class LabelDef> &lbs = itr->second;
		for(size_t j=0; j<lbs.size(); j++)
		{
			const class LabelDef &labelDef = lbs[j];
			for(size_t k=0; k < labelDef.icons.size(); k++)
			{
				const class LabelIcon &icon = labelDef.icons[k];
				loadIdToFilenameMapping[icon.iconFile] = icon.iconFile;
			}
		}
	}

	this->output->AddLoadImageResourcesCmd(loadIdToFilenameMapping);

	//Do drawing
	for(LabelsByImportance::const_iterator itr = organisedLabels.begin(); itr != organisedLabels.end(); itr++)
	{
		const vector<class LabelDef> &lbs = itr->second;
		for(size_t j=0; j<lbs.size(); j++)
		{
			const class LabelDef &labelDef = lbs[j];

			if(this->output == NULL) continue;

			if(labelDef.labels.size() > 0)
			{
				//Foreground text
				this->output->AddDrawTextCmd(labelDef.labels, labelDef.properties);
			}

			if(labelDef.twistedLabels.size() > 0)
			{
				this->output->AddDrawTwistedTextCmd(labelDef.twistedLabels, labelDef.properties);
			}

			for(size_t i =0; i < labelDef.icons.size(); i++) 
			{
				const class LabelIcon &labelIcon = labelDef.icons[i];
	
				unsigned resWidth=0, resHeight=0;
				this->GetResourceSizeCached(labelIcon.iconFile, resWidth, resHeight);

				std::vector<Polygon> iconPolys;
				ShapeProperties iconProperties;
				Contour iconOuter;
				iconOuter.push_back(Point(labelIcon.x, labelIcon.y));
				iconOuter.push_back(Point(labelIcon.x+resWidth, labelIcon.y));
				iconOuter.push_back(Point(labelIcon.x+resWidth, labelIcon.y+resHeight));
				iconOuter.push_back(Point(labelIcon.x, labelIcon.y+resHeight));
				Polygon poly(iconOuter, Contours());
				iconPolys.push_back(poly);

				iconProperties.imageId=labelIcon.iconFile;
				iconProperties.texx = -labelIcon.x;
				iconProperties.texy = -labelIcon.y;

				this->output->AddDrawPolygonsCmd(iconPolys, iconProperties);
			}

		}
	}

	//Release resources
	std::vector<std::string> unloadIds;
	for(std::map<std::string, std::string>::iterator it = loadIdToFilenameMapping.begin();
		it != loadIdToFilenameMapping.end(); it++)
	{
		unloadIds.push_back(it->first);
	}
	this->output->AddUnloadImageResourcesCmd(unloadIds);
}

// **********************************************

void SmoothLabelPaths(std::vector<class PoiLabel> &poiLabels, double tolerance)
{
	for(size_t i=0;i < poiLabels.size(); i++)
	{
		class PoiLabel &label = poiLabels[i];
		Contour &shape = label.shape;
		if(shape.size() <= 2) continue; //Can't be simplified
		
		Contour simpified;
		RamerDouglasPeucker(shape, 5.0, simpified);
		label.shape = simpified;
	}
}

