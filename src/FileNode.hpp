#pragma once

class FileNode
{
public:
	static const FileNode NullNode;

public:
	enum Type
	{
		Null,
		Bool,
		Int,
		UInt,
		Float,
		String,
		Array,
		Object,
		RawData
	};

public:
	struct Pair
	{
		const char* k;
		const FileNode* v;
	};

private:
	union Value
	{
		uint32_t i;
		const void* p;
	};

private:
	Type m_type;
	uint32_t m_size;
	Value m_val;

public:
	class Iterator
	{
	private:
		const void* m_cur;
		bool m_isObj;

	public:
		Iterator()
			: m_isObj(false), m_cur(nullptr) {
		}
		Iterator(const Pair* pair)
			: m_isObj(true), m_cur(pair) {
		}
		Iterator(const FileNode* node)
			: m_isObj(false), m_cur(node) {
		}

		bool operator!=(const Iterator& it) const {
			return m_cur != it.m_cur;
		}
		const Iterator& operator++() {
			m_cur = ((const char*)m_cur) + (m_isObj ? sizeof(Pair) : sizeof(FileNode));
			return *this;
		}
		Iterator operator++(int) {
			Iterator result = *this;
			++(*this);
			return result;
		}
		const FileNode& operator*() const {
			return *(const FileNode*)m_cur;
		}
		const char* key() const {
			return ((const Pair*)m_cur)->k;
		}
		const FileNode& value() const {
			return *(const FileNode*)((const Pair*)m_cur)->v;
		}
	};

public:
	FileNode()
		: m_type(Null), m_size(0) {
		m_val.p = nullptr;
	}
	FileNode(Type type, uint32_t intVal)
		: m_type(type), m_size(0) {
		m_val.i = intVal;
	}
	FileNode(Type type, uint32_t size, const void* ptrVal)
		: m_type(type), m_size(size) {
		m_val.p = ptrVal;
	}

	Type type() const {
		return m_type;
	}
	uint32_t size() const {
		return m_size;
	}
	Iterator begin() const {
		if (m_type == Object)
			return Iterator((const Pair*)m_val.p);
		else
			return Iterator((const FileNode*)m_val.p);
	}
	Iterator end() const {
		if (m_type == Object)
			return Iterator((const Pair*)m_val.p + m_size);
		else
			return Iterator((const FileNode*)m_val.p + m_size);
	}
	const FileNode& operator[](const char* key) const {
		for (uint32_t i = 0; i < m_size; i++)
			if (strcmp((((const Pair*)m_val.p) + i)->k, key) == 0)
				return *(((const Pair*)m_val.p) + i)->v;
		return NullNode;
	}
	operator bool() const {
		return m_type != Null;
	}
};