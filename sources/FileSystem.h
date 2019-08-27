#pragma once
#include "fsal_common.h"
#include "Status.h"
#include "Location.h"
#include "File.h"
#include "Archive.h"

namespace fsal
{
	struct FsalImplementation;

	class FileSystem
	{
	public:
		FileSystem();

		File Open(const Location& location, Mode mode = kRead);

		bool Exists(const Location& location);

		Status Rename(const Location& srcLocation, const Location& dstLocation);

		Status Remove(const Location& location);

		Status CreateDirectory(const Location& location);

		void PushSearchPath(const Location& location);

		void PopSearchPath();

		void ClearSearchPaths();

		Status MountArchive(const File& archive);

	private:
		Status Find(const Location& location, path& absolutePath, PathType& type, Archive& archive);
		std::shared_ptr<FsalImplementation> m_impl;
	};
}