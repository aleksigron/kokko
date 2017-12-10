#pragma once

#include "Queue.hpp"
#include "Rectangle.hpp"

class DebugVectorRenderer;

class DebugGraph
{
private:
	struct DataPoint
	{
		double data;
		double time;
	};

	DebugVectorRenderer* vectorRenderer;
	Rectangle drawArea;
	Queue<DataPoint> data;

	double timeRange;

public:
	DebugGraph(DebugVectorRenderer* vr);
	~DebugGraph();

	void SetDrawArea(const Rectangle& area) { drawArea = area; }

	void Update();
	void DrawToVectorRenderer();

	void AddDataPoint(double data);
};
