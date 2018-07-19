#pragma once
#include "fsal_common.h"
#include "Status.h"
#include "Location.h"

#include <memory>

namespace fsal
{
	class FileInterface;
	class File;

	class File
	{
	public:
		enum Origin
		{
			Beginning,
			CurrentPosition,
			End
		};

		File();
		
		File(FileInterface* file);

		bool ok() const;
		
		Status Read(uint8_t* destanation, size_t size, size_t* readBytes = nullptr);
		
		Status Write(const uint8_t* source, size_t size);
		
		Status Seek(ptrdiff_t offset, Origin origin);

		size_t Tell() const;
		
		size_t GetSize() const;
		
		path GetPath() const;
		
		Status Flush();

		template<typename T>
		Status Read(T& data)
		{
			return Read(reinterpret_cast<uint8_t*>(&data), sizeof(T));
		}

		template<typename T>
		Status Write(const T& data)
		{
			return Write(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
		}

		operator std::string()
		{
			std::string buff;
			buff.resize(GetSize());
			Read(const_cast<uint8_t*>((const uint8_t*)(buff.data())), buff.size());
			return buff;
		}

	private:
		std::shared_ptr<FileInterface> m_file;
	};
}
