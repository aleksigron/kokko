#pragma once

#include "Core/ArrayView.hpp"
#include "Core/StringView.hpp"

#include "System/FilesystemResolver.hpp"

class Allocator;

namespace kokko
{

class FilesystemResolverVirtual : public FilesystemResolver
{
public:
	struct MountPoint
	{
		ConstStringView mountName;
		ConstStringView mountedPath;
	};

	FilesystemResolverVirtual(Allocator* allocator);
	virtual ~FilesystemResolverVirtual() = default;

	void SetMountPoints(ArrayView<MountPoint> mounts);

	virtual bool ResolvePath(const char* path, String& resultOut) override;

private:
	static const size_t MaxMountPoints = 16;

	Allocator* allocator;

	MountPoint mountPoints[MaxMountPoints];
	size_t mountPointCount;
	char* mountStringBuffer;
};

} // namespace kokko
