//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct RegKeyTraits
{
	static void Close(HKEY key) { RegCloseKey(key); }
	static HKEY InvalidValue() { return (HKEY)null; }
};

//////////////////////////////////////////////////////////////////////

struct HandleTraits
{
	static void Close(HANDLE handle) { CloseHandle(handle); }
	static HANDLE InvalidValue() { return INVALID_HANDLE_VALUE; }
};

//////////////////////////////////////////////////////////////////////

struct IUnknownTraits
{
	static void Close(IUnknown *comptr) { comptr->Release(); }
	static IUnknown *InvalidValue() { return null; }
};

//////////////////////////////////////////////////////////////////////

template<typename T, typename traits> struct ObjHandle
{
	T obj;

	ObjHandle(T p) : obj(p)
	{
	}

	ObjHandle() : obj(traits::InvalidValue())
	{
	}

	operator T()
	{
		return obj;
	}

	T &operator = (T p)
	{
		obj = p;
		return obj;
	}

	T *operator &()
	{
		return &obj;
	}

	bool IsValid() const
	{
		return obj != traits::InvalidValue();
	}

	void Close()
	{
		if (IsValid())
		{
			traits::Close(obj);
		}
		obj = traits::InvalidValue();
	}

	void Release()
	{
		Close();
	}

	~ObjHandle()
	{
		Close();
	}
};

//////////////////////////////////////////////////////////////////////

template<typename T> using DXPtr2 = ObjHandle < T, IUnknownTraits >;
using Handle = ObjHandle < HANDLE, HandleTraits >;
using RegKey = ObjHandle < HKEY, RegKeyTraits >;

