#include "cppo5m/OsmData.h"
#include <fstream>
#include <iostream>
using namespace std;

int main()
{
	class OsmData osmData;
	std::ifstream infi("1374.o5m");



	osmData.LoadFromO5m(infi);
	cout << "nodes " << osmData.nodes.size() << endl;
	cout << "ways " << osmData.ways.size() << endl;
	cout << "relations " << osmData.relations.size() << endl;

}

