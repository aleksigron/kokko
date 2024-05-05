#include "Resources/MeshId.hpp"

#include <cassert>

#include "doctest/doctest.h"

#include "Core/StringView.hpp"
#include "Core/StringUtil.hpp"

#include "Math/Random.hpp"

namespace kokko
{

Optional<MeshUid> MeshUid::FromString(ArrayView<const char> str)
{
	if (str.GetCount() < MeshUid::StringLength)
		return Optional<MeshUid>();

	ConstStringView strRef(str.GetData(), str.GetCount());
	intptr_t split = strRef.FindFirst(ConstStringView(":"));

	if (split != Uid::StringLength)
		return Optional<MeshUid>();

	auto uidOpt = Uid::FromString(str.GetSubView(0, static_cast<size_t>(split)));

	size_t indexPos = static_cast<size_t>(split) + 1;
	auto indexOpt = HexadecimalToInteger<uint32_t>(str.GetSubView(indexPos, indexPos + sizeof(uint32_t) * 2));
	
	if (uidOpt.HasValue() == false || indexOpt.HasValue() == false)
		return Optional<MeshUid>();

	MeshUid result;
	result.modelUid = uidOpt.GetValue();
	result.meshIndex = indexOpt.GetValue();
	return result;
}

void MeshUid::WriteTo(ArrayView<char> out) const
{
	assert(out.GetCount() >= MeshUid::StringLength);
	modelUid.WriteTo(out.GetSubView(0, Uid::StringLength));
	out[Uid::StringLength] = ':';
	IntegerToHexadecimal(meshIndex, out.GetSubView(Uid::StringLength + 1, MeshUid::StringLength));
}

TEST_CASE("MeshUid.Serialization")
{
	// Check serialization and deserialization with 20 random generated values
	for (int i = 0; i < 20; ++i)
	{
		char buffer[MeshUid::StringLength];
		MeshUid original = MeshUid{ Uid::Create(), Random::Uint(0u, 1u << 31u) };
		original.WriteTo(buffer);
		auto result = MeshUid::FromString(ArrayView<const char>(buffer, sizeof(buffer)));
		CHECK(result.HasValue() == true);
		CHECK(result.GetValue() == original);
	}

	const char validString[] = "0123456789abcdef0123456789abcdef:0123abcd";
	auto result0 = MeshUid::FromString(ArrayView(validString, sizeof(validString) - 2));
	CHECK(result0.HasValue() == false);

	auto result1 = MeshUid::FromString(ArrayView(validString, 0));
	CHECK(result1.HasValue() == false);

	const char invalidString[] = "0123456789abcdef123456789abcde:0123abcd";
	auto result2 = MeshUid::FromString(ArrayView(invalidString));
	CHECK(result2.HasValue() == false);
}

}
