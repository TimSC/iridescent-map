#include "LabelEngine.h"
#include <iostream>
using namespace std;

PoiLabel::PoiLabel()
{
	sx = 0.0;
	sy = 0.0;
}

PoiLabel::PoiLabel(double sx, double sy, const std::string &textName, const TagMap &tags):
	sx(sx), sy(sy), textName(textName), tags(tags)
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
	sx = arg.sx;
	sy = arg.sy;
	textName = arg.textName;
	tags = arg.tags;
}

// *******************************************************

LabelEngine::LabelEngine(class IDrawLib *output):
	output(output)
{

}

LabelEngine::~LabelEngine()
{

}

void LabelEngine::WriteDrawCommands()
{
	for(size_t i=0;i < this->poiLabels.size(); i++)
	{
		class PoiLabel &label = this->poiLabels[i];

		std::string &textName = label.textName;
		std::string outString;
		if(textName[0] == '[' && textName[textName.size()-1] == ']') {
			std::string keyName(&textName[1], textName.size()-2);
			TagMap::const_iterator it = label.tags.find(keyName);
			if(it == label.tags.end()) continue;
			outString = it->second;
		}
		else
			outString = textName;

		class TextProperties textProp(1.0,1.0,1.0);
		std::vector<class TextLabel> textStrs;
		textStrs.push_back(TextLabel(outString, label.sx, label.sy));
		if(this->output != NULL)
			this->output->AddDrawTextCmd(textStrs, textProp);
	}
}

void LabelEngine::AddPolygonLabel(const std::vector<Polygon> &polygons, std::string &textName, TagMap &tags)
{

}

void LabelEngine::AddLineLabel(const Contour &line, std::string &textName, TagMap &tags)
{

}

void LabelEngine::AddPoiLabel(double sx, double sy, std::string &textName, TagMap &tags)
{
	this->poiLabels.push_back(PoiLabel(sx, sy, textName, tags));
}

