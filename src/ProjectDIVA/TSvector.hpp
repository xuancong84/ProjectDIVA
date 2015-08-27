#pragma once

// Thread-safe unordered vector
template <class T>
class TSvector {
private:
	T*	m_data;
	int	m_size;
	int	m_max_size;
	int m_lock;
	DWORD	threadID;
	HANDLE	hEvent;

	void AcquireLock()
	{
		DWORD currentID = GetCurrentThreadId();
		if(currentID!=threadID)
		{
			WaitForSingleObject(hEvent,INFINITE);
			threadID = currentID;
		}
		m_lock++;
	}
	void ReleaseLock()
	{
		if(--m_lock == 0)
		{
			threadID = 0;
			SetEvent(hEvent);
		}
	}

	struct iterator{	// must be private
		int posi;
		TSvector <T> *q;

		iterator& operator ++ ()
		{
			++ posi;
			return *this;
		}
		iterator& operator -- ()
		{
			-- posi;
			return *this;
		}
		iterator& operator + (int rhs)
		{
			posi += rhs;
			return *this;
		}
		iterator& operator += (int rhs)
		{
			posi += rhs;
			return *this;
		}
		bool operator != (int rhs){ return posi!=rhs; }
		bool operator == (int rhs){	return posi==rhs; }
		void operator = (int rhs) { posi = rhs; }
		T& operator *()	{ return q->m_data[posi]; }
		T* operator ->(){ return &q->m_data[posi]; }
		iterator(int _posi, TSvector <T> *_q):posi(_posi),q(_q)
		{	// acquire lock
			q->AcquireLock();
		}
		~iterator()
		{	// release lock
			q->ReleaseLock();
		}
	};

public:
	TSvector(int _size = 16)
	{
		if(_size<=0) _size=4;
		m_lock = m_size = 0;
		m_max_size = _size;
		m_data = new T [m_max_size];
		hEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
	}
	~TSvector()
	{
		AcquireLock();
		CloseHandle(hEvent);
		delete [] m_data;
	}

	bool empty() const { return !m_size; }
	int size() const { return m_size; }
	void push_back(T& x)
	{
		AcquireLock();
		if(m_size >= m_max_size)	//expand if full
		{
			m_max_size <<= 1;
			T* new_data = new T [m_max_size];
			for(int x=0; x<m_size; x++)
				new_data[x] = m_data[x];
			delete [] m_data;
			m_data = new_data;
		}
		m_data[m_size++] = x;
		ReleaseLock();
	}
	void push_back(T x)
	{
		AcquireLock();
		if(m_size >= m_max_size)	//expand if full
		{
			m_max_size <<= 1;
			T* new_data = new T [m_max_size];
			for(int x=0; x<m_size; x++)
				new_data[x] = m_data[x];
			delete [] m_data;
			m_data = new_data;
		}
		m_data[m_size++] = x;
		ReleaseLock();
	}
	T pop_back()
	{
		AcquireLock();
		if(m_size>0)
		{
			--m_size;
			ReleaseLock();
			return m_data[m_size];
		}
		ReleaseLock();
	}
	void erase(iterator &it)
	{
		if(m_size>0)
		{
			m_data[it.posi] = m_data[--m_size];
			m_data[m_size] = T();
		}
	}
	void clear()
	{
		AcquireLock();
		delete [] m_data;
		m_data = new T [m_max_size];
		m_size = 0;
		ReleaseLock();
	}

	iterator begin(){ return iterator(0,this); }
	int end(){ return m_size; }
};
