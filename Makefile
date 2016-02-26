
all: test
test: cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp TagPreprocessor.cpp Regrouper.cpp LayerDrawManager.cpp drawlib/drawlibcairo.cpp drawlib/drawlib.cpp MapRender.cpp Transform.cpp
	g++ cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp TagPreprocessor.cpp Regrouper.cpp LayerDrawManager.cpp drawlib/drawlibcairo.cpp drawlib/drawlib.cpp MapRender.cpp Transform.cpp `pkg-config --cflags --libs gtk+-2.0` -o test

