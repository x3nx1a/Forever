#pragma once

class BinaryReader
{
public:
	explicit BinaryReader()
		: m_buffer(nullptr),
		m_cur(nullptr),
		m_size(0)
	{
	}

	explicit BinaryReader(const void* buffer, int size)
		: m_buffer((const char*)buffer),
		m_cur((const char*)buffer),
		m_size(size)
	{
	}

	int size() const
	{
		return m_size;
	}

	template<typename T>
	void read(T* data, int count) const
	{
		const int size = count * sizeof(T);
		memcpy(data, m_cur, size);
		m_cur += size;
	}

	template<typename T>
	T read() const
	{
		T val;
		read(&val, 1);
		return val;
	}

	template<typename T>
	const BinaryReader& operator>>(T& val) const
	{
		read(&val, 1);
		return *this;
	}

	void skip(int count) const
	{
		m_cur += count;
	}

private:
	const char* m_buffer;
	mutable const char* m_cur;
	int m_size;
};