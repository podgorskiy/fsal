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

		operator bool() const;

		operator std::string() const;

		void operator =(const std::string& x);

		Status Read(uint8_t* destanation, size_t size, size_t* readBytes = nullptr) const;
		
		Status Write(const uint8_t* source, size_t size);
		
		Status Seek(ptrdiff_t offset, Origin origin = Beginning) const;

		size_t Tell() const;
		
		size_t GetSize() const;
		
		path GetPath() const;
		
		Status Flush() const;

		const uint8_t* GetDataPointer() const;

		uint8_t* GetDataPointer();

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

	private:
		std::shared_ptr<FileInterface> m_file;
	};
}
