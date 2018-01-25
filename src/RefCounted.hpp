#pragma once

class RefCounted
{
protected:
	RefCounted()
		: m_refCount(1)
	{
	}

	virtual ~RefCounted()
	{
	}

public:
	void addRef()
	{
		m_refCount++;
	}

	void release()
	{
		m_refCount--;
		if (!m_refCount)
			delete this;
	}

	uint32_t refCount() const {
		return m_refCount;
	}

private:
	uint32_t m_refCount;

private:
	RefCounted(const RefCounted&) = delete;
	RefCounted& operator=(const RefCounted&) = delete;
};

template<typename T>
class RefCountedPtr
{
private:
	explicit RefCountedPtr(T* ptr)
		: m_ptr(ptr)
	{
	}

public:
	template<typename SubClass = T, typename... Args>
	static RefCountedPtr create(Args... args)
	{
		return RefCountedPtr((T*)(new SubClass(args...)));
	}

	RefCountedPtr()
		: m_ptr(nullptr)
	{
	}

	RefCountedPtr(std::nullptr_t)
		: m_ptr(nullptr)
	{
	}

	~RefCountedPtr()
	{
		if (m_ptr)
			m_ptr->release();
	}

	RefCountedPtr(const RefCountedPtr& ptr)
		: m_ptr(ptr.m_ptr)
	{
		if (m_ptr)
			m_ptr->addRef();
	}

	RefCountedPtr& operator=(const RefCountedPtr& ptr)
	{
		if (m_ptr)
			m_ptr->release();

		m_ptr = ptr.m_ptr;
		if (m_ptr)
			m_ptr->addRef();

		return *this;
	}

	const T* operator->() const {
		return m_ptr;
	}

	T* operator->() {
		return m_ptr;
	}

	operator bool() const {
		return m_ptr != nullptr;
	}

	T* get() {
		return m_ptr;
	}

	const T* get() const {
		return m_ptr;
	}

private:
	T* m_ptr;
};