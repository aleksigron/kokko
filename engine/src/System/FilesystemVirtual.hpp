#pragma once

#include <cstddef>

#include "Core/Array.hpp"
#include "Core/Pair.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "System/Filesystem.hpp"
#include "System/FilesystemDefault.hpp"

class FilesystemVirtual : public Filesystem
{
public:
	struct MountPoint
	{
		StringRef mountName;
		StringRef mountedPath;
	};

	FilesystemVirtual(Allocator* allocator);
	virtual ~FilesystemVirtual() = default;

	void SetMountPoints(ArrayView<MountPoint> mounts);

	virtual bool ReadBinary(const char* path, Array<uint8_t>& output) override;
	virtual bool ReadText(const char* path, kokko::String& output) override;
	virtual bool WriteText(const char* path, ArrayView<const char> content, bool append) override;

private:
	bool FindMountedPath(const char* path);

	static const size_t MaxMountPoints = 16;

	Allocator* allocator;

	MountPoint mountPoints[MaxMountPoints];
	size_t mountPointCount;
	char* mountStringBuffer;

	kokko::String pathStore;
	FilesystemDefault defaultFs;
};
