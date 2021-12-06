#include "System/FilesystemVirtual.hpp"

#include <cstring>

FilesystemVirtual::FilesystemVirtual(Allocator* allocator) :
	allocator(allocator),
	mountPointCount(0),
	mountStringBuffer(nullptr),
	pathStore(allocator)
{
	pathStore.Reserve(512);
}

void FilesystemVirtual::SetMountPoints(ArrayView<MountPoint> mounts)
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
				pointIndices[mountPointCount] = mountPointCount;
				mountPointCount += 1;
				stringDataUsed += mount.mountName.len + mount.mountedPath.len;
			}
		}

		// TODO: We should make sure that trailing slashes are correct
	}

	if (mountPointCount > 0)
	{
		mountStringBuffer = static_cast<char*>(allocator->Allocate(stringDataUsed));
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

bool FilesystemVirtual::ReadBinary(const char* path, Array<uint8_t>& output)
{
	if (FindMountedPath(path))
	{
		bool result = defaultFs.ReadBinary(pathStore.GetCStr(), output);
		pathStore.Clear();
		return result;
	}
	else
	{
		return defaultFs.ReadBinary(path, output);
	}
}

bool FilesystemVirtual::ReadText(const char* path, String& output)
{
	if (FindMountedPath(path))
	{
		bool result = defaultFs.ReadText(pathStore.GetCStr(), output);
		pathStore.Clear();
		return result;
	}
	else
	{
		return defaultFs.ReadText(path, output);
	}
}

bool FilesystemVirtual::WriteText(const char* path, ArrayView<const char> content, bool append)
{
	if (FindMountedPath(path))
	{
		bool result = defaultFs.WriteText(pathStore.GetCStr(), content, append);
		pathStore.Clear();
		return result;
	}
	else
	{
		return defaultFs.WriteText(path, content, append);
	}
}

bool FilesystemVirtual::FindMountedPath(const char* path)
{
	size_t firstSlash = 0;

	for (size_t i = 0; ; ++i)
	{
		char c = path[i];

		if (c == '\0')
			return false;

		if (c == '/' || c == '\\')
		{
			firstSlash = i;
			break;
		}
	}

	if (firstSlash == 0)
		return false;

	for (size_t i = 0; i < mountPointCount; ++i)
	{
		const StringRef& mountName = mountPoints[i].mountName;
		if (mountName.len == firstSlash)
		{
			bool match = true;

			for (size_t j = 0; j < firstSlash; ++j)
			{
				if (path[j] != mountName.str[j])
				{
					match = false;
					break;
				}
			}

			if (match)
			{
				const StringRef& mountPath = mountPoints[i].mountedPath;

				const char* relativePath = &path[firstSlash + 1];
				
				pathStore.Assign(mountPath);
				pathStore.Append('/');
				pathStore.Append(relativePath);

				return true;
			}
		}
	}

	return false;
}
