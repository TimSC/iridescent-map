
all: test
test: cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp TagPreprocessor.cpp
	g++ cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp TagPreprocessor.cpp -o test

