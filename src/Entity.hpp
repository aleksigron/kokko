#pragma once

struct Entity
{
private:
	static const unsigned int IndexBits = 22;
	static const unsigned int IndexMask = (1 << IndexBits) - 1;

	static const unsigned int GenerationBits = 8;
	static const unsigned int GenerationMask = (1 << GenerationBits) - 1;

public:
	static Entity Make(unsigned int index, unsigned int generation)
	{
		Entity e;
		e.id = (index & IndexMask) | ((generation & GenerationMask) << IndexBits);
		return e;
	}

	bool IsNull()
	{
		return id == 0;
	}
	
	unsigned int id;

	unsigned int Index() const { return id & IndexMask; }
	unsigned int Generation() const { return (id >> IndexBits) & GenerationMask; }
};
