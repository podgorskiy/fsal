#pragma once
#include "fsal_common.h"
#include "FastPathNormalization.h"
#include <vector>
#include <string>
#include <algorithm>
#include <mutex>

namespace fsal
{
	template<typename UserData>
	struct FileEntry
	{
		FileEntry(): depth(0), size(0), offset(-1){};
		FileEntry(const std::string& str) : depth(0), data(UserData())
		{
			NormalizePath(str, path, filenamePos, depth);
		}
		FileEntry(const std::string& str, const UserData& data) : depth(0), data(data)
		{
			NormalizePath(str, path, filenamePos, depth);
		}

		// Full path in archive.
		std::string path;

		// Name of the entry. Ends with slash for directories
		int filenamePos;

		// Depth in file tree. Files in the root dir have zero depth
		int depth;

		UserData data;
	};


	// Compare operator. Deeper path is always greater
	template<typename UserData>
	inline bool operator <(const FileEntry<UserData>& a, const FileEntry<UserData>& b)
	{
		if (a.depth == b.depth)
		{
			return a.path < b.path;
		}
		else
		{
			return a.depth < b.depth;
		}
	}

	template<typename UserData>
	class FileList
	{
	public:
		UserData FindEntry(const fs::path& path)
		{
			FileEntry<UserData> key(path.string());

			int index = GetIndex(key);
			if (index != -1)
			{
				return m_fileList[index].data;
			}
			return UserData();
		}

		int GetIndex(const FileEntry<UserData>& key, bool getLowerBound = false)
		{
			if (!sorted)
			{
				std::lock_guard<std::mutex> lock(m_table_modification);
				std::sort(m_fileList.begin(), m_fileList.end());
				int depth = 0;
				depthTable.push_back(0);
				for (int i = 0, l = (int)m_fileList.size(); i != l; ++i)
				{
					if (depth != m_fileList[i].depth)
					{
						int newDepth = m_fileList[i].depth;
						depthTable.resize(newDepth + 1, depthTable[depth]);
						depthTable[newDepth] = i;
						depth = newDepth;
					}
				}
				depthTable.push_back((int)m_fileList.size());
				sorted = true;
			}

			if (!(key.depth + 1 < (int)depthTable.size()))
			{
				return -1;
			}

			size_t right = depthTable[key.depth + 1];
			size_t left = depthTable[key.depth];
			size_t it = 0;

			// Searching for lower bound
			size_t count = right - left;

			while (count > 0)
			{
				it = left;
				size_t step = count / 2;
				it += step;

				bool less = m_fileList[it].path < key.path;
				if (less)
				{
					left = ++it;
					count -= step + 1;
				}
				else
					count = step;
			}

			if (getLowerBound)
			{
				return (int)left;
			}
			else
			{
				bool less = key.path < m_fileList[it].path;
				if (!(left == right) && !less)
				{
					return (int)left;
				}
			}

			return -1;
		}

		void Add(const UserData& data, const std::string& path)
		{
			FileEntry<UserData> entry(path, data);
			std::lock_guard<std::mutex> lock(m_table_modification);
			m_fileList.push_back(entry);
			sorted = false;
		}

		std::vector<std::string> ListDirectory(const fs::path& path)
		{
			std::vector<std::string> result;

			int index = GetIndex((path / "a").string(), true);

			int lastIndex = m_fileList[index].depth + 1 < (int)depthTable.size() ? depthTable[m_fileList[index].depth + 1] : (int)m_fileList.size();

			FileEntry<UserData> key(path.string());

			size_t key_size = key.path.length();

			// Starting from the obtained low bound, we are going to grab all paths that start with given path.
			while(index != -1 && index < lastIndex && m_fileList[index].path.compare(0, key_size, key.path) == 0)
			{
				const FileEntry<UserData>& entry = m_fileList[index];
				std::string filename(entry.path.begin() + entry.filenamePos, entry.path.end());
				result.push_back(filename);
				++index;
			}

			return result;
		}

	private:
		std::vector<int> depthTable;
		std::vector<FileEntry<UserData> > m_fileList;
		std::mutex m_table_modification;
		bool sorted = false;
	};
}
