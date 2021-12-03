#include "Core/Uid.hpp"

#include <limits>

#include "Math/Random.hpp"

namespace kokko
{

Uid Uid::Create()
{
	Uid result;

	result.data[0] = Random::Uint64(0, std::numeric_limits<uint64_t>::max());
	result.data[1] = Random::Uint64(0, std::numeric_limits<uint64_t>::max());

	return result;
}

}
