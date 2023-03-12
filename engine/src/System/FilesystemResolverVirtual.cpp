#include "System/FilesystemResolverVirtual.hpp"

#include "Core/String.hpp"

#include "Memory/Allocator.hpp"

namespace kokko
{

FilesystemResolverVirtual::FilesystemResolverVirtual(Allocator* allocator) :
	allocator(allocator),
	mountPointCount(0),
	mountStringBuffer(nullptr)
{
}

FilesystemResolverVirtual::~FilesystemResolverVirtual()
{
	allocator->Deallocate(mountStringBuffer);
}

void FilesystemResolverVirtual::SetMountPoints(ArrayView<MountPoint> mounts)
{
	if (mountStringBuffer != nullptr)
	{
		allocator->Deallocate(mountStringBuffer);
		mountStringBuffer = nullptr;
	}

	size_t pointIndices[MaxMountPoints];
	mountPointCount = 0;
	size_t stringDataUsed = 0;

	for (size_t i = 0, count = mounts.GetCount(); i < count; ++i)
	{
		const MountPoint& mount = mounts[i];
		size_t mountNameLen = mount.mountName.len;
		size_t mountedPathLen = mount.mountedPath.len;

		if (mountNameLen > 0 && mountedPathLen > 0)
		{
			bool valid = true;

			// Make sure no slashes in the mount name
			for (size_t j = 0; j < mountNameLen; ++j)
			{
				char c = mount.mountName.str[j];
				if (c == '/' || c == '\\')
				{
					valid = false;
					break;
				}
			}

			if (valid)
			{
				char lastChar = mount.mountedPath[mountedPathLen - 1];
				if (lastChar == '/' || lastChar == '\\')
					valid = false;
			}

			if (valid)
			{
				pointIndices[mountPointCount] = mountPointCount;
				mountPointCount += 1;
				stringDataUsed += mount.mountName.len + mount.mountedPath.len;
			}
		}
	}

	if (mountPointCount > 0)
	{
		mountStringBuffer = static_cast<char*>(allocator->Allocate(stringDataUsed, "FilesystemResolverVirtual.mountStringBuffer"));
		char* strBufItr = mountStringBuffer;

		for (size_t i = 0; i < mountPointCount; ++i)
		{
			const MountPoint& mount = mounts[pointIndices[i]];

			std::memcpy(strBufItr, mount.mountName.str, mount.mountName.len);
			mountPoints[i].mountName.str = strBufItr;
			mountPoints[i].mountName.len = mount.mountName.len;
			strBufItr += mount.mountName.len;

			std::memcpy(strBufItr, mount.mountedPath.str, mount.mountedPath.len);
			mountPoints[i].mountedPath.str = strBufItr;
			mountPoints[i].mountedPath.len = mount.mountedPath.len;
			strBufItr += mount.mountedPath.len;
		}
	}
}
bool FilesystemResolverVirtual::ResolvePath(const char* path, String& resultOut)
{
	size_t firstSlash = 0;
	size_t length = 0;

	for (size_t i = 0; ; ++i)
	{
		char c = path[i];

		if (c == '\0')
		{
			length = i;
			break;
		}
		else if (c == '/' || c == '\\')
		{
			firstSlash = i;
			length = i;
			break;
		}
	}

	if (length == 0)
		return false;

	bool addTail = firstSlash != 0;

	for (size_t i = 0; i < mountPointCount; ++i)
	{
		const ConstStringView& mountName = mountPoints[i].mountName;
		if (mountName.len == length)
		{
			bool match = true;

			for (size_t j = 0; j < length; ++j)
			{
				if (path[j] != mountName.str[j])
				{
					match = false;
					break;
				}
			}

			if (match)
			{
				resultOut.Assign(mountPoints[i].mountedPath);

				if (addTail)
				{
					const char* relativePath = &path[firstSlash + 1];
					resultOut.Append('/');
					resultOut.Append(relativePath);
				}

				return true;
			}
		}
	}

	return false;
}

} // namespace kokko
