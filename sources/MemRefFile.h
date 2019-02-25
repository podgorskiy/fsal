#pragma once
#include "fsal_common.h"
#include "FileInterface.h"

#include <cstdio>
#include <memory>

namespace fsal
{
	class MemRefFile : public FileInterface
	{
	public:
		MemRefFile();

		MemRefFile(uint8_t* data, size_t size, bool copy);

		MemRefFile(std::shared_ptr<uint8_t> data, size_t size);

		virtual ~MemRefFile() override;

		virtual bool ok() const override;

		virtual path GetPath() const override;

		virtual Status Open(path filepath, Mode mode) override;

		virtual Status ReadData(uint8_t* dst, size_t size, size_t* bytesRead) override;

		virtual Status WriteData(const uint8_t* src, size_t size)  override;

		virtual Status SetPosition(size_t position) const  override;

		virtual size_t GetPosition() const  override;

		virtual size_t GetSize() const  override;

		virtual Status FlushBuffer() const override;

		virtual const uint8_t* GetDataPointer() const  override;

		virtual uint8_t* GetDataPointer()  override;

	private:
		bool Resize(size_t newSize);

		uint8_t* m_data;
		std::shared_ptr<uint8_t> m_sharedData;

		bool m_hasOwnership;

		size_t m_size;
		size_t m_reserved;

		mutable size_t m_offset;
	};
}
