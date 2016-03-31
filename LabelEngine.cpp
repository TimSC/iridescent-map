#include "LabelEngine.h"
#include <iostream>
#include <stdlib.h>
#include <stdexcept>
#include "TriTri2d.h"
using namespace std;

LabelDef::LabelDef(const class LabelBounds &labelBounds,
		const class TextProperties &foregroundProp,
		const class TextProperties &backgroundProp,
		const std::vector<class TextLabel> &labels) : labelBounds(labelBounds),
		foregroundProp(foregroundProp),
		backgroundProp(backgroundProp),
		labels(labels)
{

}

LabelDef::LabelDef(const class LabelBounds &labelBounds,
		const class TextProperties &foregroundProp,
		const class TextProperties &backgroundProp,
		const std::vector<class TwistedTextLabel> &twistedLabels) : labelBounds(labelBounds),
		foregroundProp(foregroundProp),
		backgroundProp(backgroundProp),
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
	foregroundProp = arg.foregroundProp;
	backgroundProp = arg.backgroundProp;
	labels = arg.labels;
	twistedLabels = arg.twistedLabels;
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

}

LabelEngine::~LabelEngine()
{

}

void LabelEngine::LabelPoisToStyledLabel(std::vector<class PoiLabel> &poiLabels, LabelsByImportance &organisedLabelsOut)
{
	organisedLabelsOut.clear();
	for(size_t i=0;i < poiLabels.size(); i++)
	{
		class PoiLabel &label = poiLabels[i];

		//Extract final draw parameters from tags and style
		std::string &textName = label.textName;
		std::string outString;
		if(textName[0] == '[' && textName[textName.size()-1] == ']') 
		{
			std::string keyName(&textName[1], textName.size()-2);
			TagMap::const_iterator it = label.tags.find(keyName);
			if(it == label.tags.end()) continue;
			outString = it->second;
		}
		else
			outString = textName;

		int textSize = 9;
		StyleAttributes::const_iterator paramIt = label.styleAttributes.find("text-size");
		if(paramIt != label.styleAttributes.end())
			textSize = atoi(paramIt->second.c_str());

		int importance = 0;
		paramIt = label.styleAttributes.find("text-importance");
		if(paramIt != label.styleAttributes.end())
			importance = atoi(paramIt->second.c_str());

		double fillR=1.0, fillG=1.0, fillB=1.0, fillA = 1.0;
		paramIt = label.styleAttributes.find("text-fill");
		if(paramIt != label.styleAttributes.end())
			ColourStringToRgba(paramIt->second.c_str(), fillR, fillG, fillB, fillA);

		double haloR=0.0, haloG=0.0, haloB=0.0, haloA=1.0;
		paramIt = label.styleAttributes.find("text-halo-fill");
		if(paramIt != label.styleAttributes.end())
			ColourStringToRgba(paramIt->second.c_str(), haloR, haloG, haloB, haloA);

		double haloWidth=2.0;
		paramIt = label.styleAttributes.find("text-halo-radius");
		if(paramIt != label.styleAttributes.end())
			haloWidth = atof(paramIt->second.c_str());

		std::string placement = "point";
		paramIt = label.styleAttributes.find("text-placement");
		if(paramIt != label.styleAttributes.end())
			placement = paramIt->second;

		//A shape is required for any drawing to happen
		if(label.shape.size() == 0) continue;
		
		//Define draw styles
		class TextProperties foregroundProp(fillR, fillG, fillB);
		foregroundProp.fontSize = textSize;
		foregroundProp.halign = 0.5;

		class TextProperties backgroundProp(haloR, haloG, haloB);
		backgroundProp.fontSize = textSize;
		backgroundProp.la = haloA;
		backgroundProp.outline = true;
		backgroundProp.fill = false;
		backgroundProp.lineWidth=haloWidth;
		backgroundProp.halign = 0.5;

		if(placement == "point")
		{
			//Get bounds
			double lx = label.shape[0].first;
			double ly = label.shape[0].second;
			TextLabel outLabel(outString, lx, ly);
			TwistedTriangles bounds;
			if(this->output != NULL)
			{
				this->output->GetTriangleBoundsText(outLabel, foregroundProp, 
					bounds);
			}
	
			std::vector<class TextLabel> textStrs;
			textStrs.push_back(outLabel);

			class LabelBounds labelBounds(bounds);

			//Add label definition to list
			LabelsByImportance::iterator it = organisedLabelsOut.find(importance);
			if(it == organisedLabelsOut.end())
				organisedLabelsOut[importance] = vector<class LabelDef>();
			organisedLabelsOut[importance].push_back(LabelDef(labelBounds, foregroundProp, backgroundProp, textStrs));
		}
		
		if(placement == "line")
		{
			//Get bounds
			std::vector<TwistedCurveCmd> path;
			SmoothContour(label.shape, path);
			TwistedTextLabel outLabel(outString, path);
			TwistedTriangles bounds;
			if(this->output != NULL)
			{
				this->output->GetTriangleBoundsTwistedText(outLabel, foregroundProp, 
					bounds);
			}
	
			std::vector<class TwistedTextLabel> textStrs;
			textStrs.push_back(outLabel);

			class LabelBounds labelBounds(bounds);

			//Add label definition to list
			LabelsByImportance::iterator it = organisedLabelsOut.find(importance);
			if(it == organisedLabelsOut.end())
				organisedLabelsOut[importance] = vector<class LabelDef>();
			organisedLabelsOut[importance].push_back(LabelDef(labelBounds, foregroundProp, backgroundProp, textStrs));
		}
	}
}

bool LabelOverlaps(const class LabelDef &label, const LabelsByImportance existingLabels)
{
	for(LabelsByImportance::const_iterator itr = existingLabels.begin(); itr != existingLabels.end(); itr++)
	{
		const vector<class LabelDef> &lbs = itr->second;
		for(size_t j=0; j<lbs.size(); j++)
		{
			if(label.labelBounds.Overlaps(lbs[j].labelBounds))
				return true;
		}
	}
	return false;
}

void LabelEngine::RemoveOverlapping(const LabelsByImportance &organisedLabelsTmp, LabelsByImportance &organisedLabelsOut)
{
	organisedLabelsOut.clear();
	
	//Starting with high imporance labels, check for overlaps
	if(organisedLabelsTmp.size() == 0)
		return;
	LabelsByImportance::const_iterator itr = organisedLabelsTmp.end();
	itr --;
	bool looping = true;
	while(looping)
	{
		if(itr == organisedLabelsTmp.begin())
			looping = false;
		int importance = itr->first;
		const vector<class LabelDef> &lbs = itr->second;
		for(size_t j=0; j<lbs.size(); j++)
		{
			const class LabelDef &label = lbs[j];
			bool overlaps = LabelOverlaps(label, organisedLabelsOut);
			if(!overlaps)
			{
				LabelsByImportance::iterator chkIt = organisedLabelsOut.find(importance);
				if(chkIt == organisedLabelsOut.end())
					organisedLabelsOut[importance] = vector<class LabelDef>();
				organisedLabelsOut[importance].push_back(label);
			}
		}
		if(looping)
			itr --;
	}
}

void LabelEngine::WriteDrawCommands(const LabelsByImportance &organisedLabels)
{

	for(LabelsByImportance::const_iterator itr = organisedLabels.begin(); itr != organisedLabels.end(); itr++)
	{
		const vector<class LabelDef> &lbs = itr->second;
		for(size_t j=0; j<lbs.size(); j++)
		{
			const class LabelDef &labelDef = lbs[j];

			if(labelDef.labels.size() > 0)
			{
				//Ghost background
				if(this->output != NULL)
					this->output->AddDrawTextCmd(labelDef.labels, labelDef.backgroundProp);

				//Foreground text
				if(this->output != NULL)
					this->output->AddDrawTextCmd(labelDef.labels, labelDef.foregroundProp);
			}

			if(labelDef.twistedLabels.size() > 0)
			{
				//Ghost background
				if(this->output != NULL)
					this->output->AddDrawTwistedTextCmd(labelDef.twistedLabels, labelDef.backgroundProp);

				//Foreground text
				if(this->output != NULL)
					this->output->AddDrawTwistedTextCmd(labelDef.twistedLabels, labelDef.foregroundProp);
			}

		}
	}
}


