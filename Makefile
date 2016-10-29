
all: test
test: cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInputO5m.cpp ReadInputMbtiles.cpp cppGzip/DecodeGzip.cpp TagPreprocessor.cpp Regrouper.cpp drawlib/drawlibcairo.cpp drawlib/drawlib.cpp drawlib/cairotwisted.cpp drawlib/RdpSimplify.cpp drawlib/LineLineIntersect.cpp MapRender.cpp Transform.cpp Style.cpp LabelEngine.cpp TriTri2d.cpp CompletePoly.cpp Coast.cpp mbtiles-cpp/VectorTile.cpp mbtiles-cpp/vector_tile20/vector_tile.pb.cc test.cpp
	g++ cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInputO5m.cpp ReadInputMbtiles.cpp cppGzip/DecodeGzip.cpp TagPreprocessor.cpp Regrouper.cpp drawlib/drawlibcairo.cpp drawlib/drawlib.cpp drawlib/cairotwisted.cpp drawlib/RdpSimplify.cpp drawlib/LineLineIntersect.cpp MapRender.cpp Transform.cpp Style.cpp  LabelEngine.cpp TriTri2d.cpp CompletePoly.cpp Coast.cpp mbtiles-cpp/VectorTile.cpp mbtiles-cpp/vector_tile20/vector_tile.pb.cc test.cpp `pkg-config --cflags --libs gtk+-3.0` -Wall -lz -g -lprotobuf-lite -o test

