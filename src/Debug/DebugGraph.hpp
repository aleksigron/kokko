#pragma once

#include "Core/Queue.hpp"
#include "Math/Rectangle.hpp"

class DebugVectorRenderer;

class Allocator;

class DebugGraph
{
private:
	struct DataPoint
	{
		double data;
		double time;
	};

	DebugVectorRenderer* vectorRenderer;
	Rectanglef drawArea;
	Queue<DataPoint> data;

	double timeRange;

public:
	DebugGraph(Allocator* allocator, DebugVectorRenderer* vr);
	~DebugGraph();

	void SetDrawArea(const Rectanglef& area) { drawArea = area; }

	void Update();
	void DrawToVectorRenderer();

	void AddDataPoint(double data);

	double GetAverageOverLastSeconds(double seconds);
};
