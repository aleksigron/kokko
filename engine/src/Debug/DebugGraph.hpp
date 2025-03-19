#pragma once

#include "Core/Array.hpp"
#include "Core/Queue.hpp"

#include "Math/Vec3.hpp"
#include "Math/Rectangle.hpp"

namespace kokko
{
class Allocator;
class DebugVectorRenderer;

class DebugGraph
{
private:
	struct DataPoint
	{
		double data;
		double time;
	};

	kokko::DebugVectorRenderer* vectorRenderer;
	Rectanglef drawArea;
	kokko::Queue<DataPoint> data;
	kokko::Array<Vec3f> pointStorage;

	double timeRange;

public:
	DebugGraph(Allocator* allocator, kokko::DebugVectorRenderer* vr);
	~DebugGraph();

	void SetDrawArea(const Rectanglef& area) { drawArea = area; }

	void Update();
	void DrawToVectorRenderer();

	void AddDataPoint(double data);

	double GetAverageOverLastSeconds(double seconds);
};

} // namespace kokko
