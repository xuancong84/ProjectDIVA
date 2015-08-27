#pragma once

template <class T>
class CircularQueue {
private:
	T*	m_data;
	int p_head;
	int p_tail;
	int m_size;
	int m_max_size;
	int m_mask;

public:
	CircularQueue( int _size = 255 )
	{
		for(m_max_size=1; m_max_size<=_size; m_max_size<<=1);
		m_mask = m_max_size-1;
		m_data = new T [m_max_size];
		p_head = p_tail = m_size = 0;
	}
	~CircularQueue()
	{
		delete [] m_data;
	}

	void push_back(T x)
	{
		if( m_size >= m_max_size-1 )
		{	//expand the queue
			T* new_data = new T [m_max_size<<1];
			memcpy(new_data,m_data,sizeof(T)*m_max_size);
			for(int x=p_head; x!=p_tail; inc(x))
				if(x>p_tail) new_data[x+m_max_size] = new_data[x];
			delete [] m_data;
			m_data = new_data;
			if(p_head>p_tail) p_head += m_max_size;
			m_max_size<<=1;
			m_mask = m_max_size-1;
		}
		m_data[p_tail] = x;
		inc(p_tail);
		++m_size;
	}
	T& pop()
	{
		int old_head = p_head;
		if(m_size>0)
		{
			//m_data[p_head].~T();
			inc(p_head);
			--m_size;
		}
		return m_data[old_head];
	}

	struct iterator{
		int posi;
		CircularQueue <T> *q;
		iterator& operator ++ ()
		{
			//posi = (posi+1)&q->m_mask;
			q->inc(posi);
			return *this;
		}
		bool operator != (const iterator rhs)
		{
			return posi!=rhs.posi;
		}
		bool operator == (const iterator rhs)
		{
			return posi==rhs.posi;
		}
		T& operator *(){return q->m_data[posi];}
		T* operator ->(){ return &q->m_data[posi];}
	};
	iterator begin()
	{
		iterator i={p_head,this};
		return i;
	}
	iterator end()
	{
		iterator i={p_tail,this};
		return i;
	}
	bool empty() const { return !m_size; }
	int size() const { return m_size; }
	void clear(){ while(m_size>0)pop(); }
	inline void inc(int &var){ var=(var+1)&m_mask; }
	void erase(iterator &it)
	{
		if(!m_size) return;
		int x;
		for(x=p_head; x!=p_tail; inc(x))
			if(x==it.posi) break;
		if(x==p_tail) return;
		for(int oldx=(x-1+m_max_size)&m_mask; x!=p_head; x=oldx, oldx=(x-1+m_max_size)&m_mask)
			m_data[x] = m_data[oldx];
		inc(p_head);
		--m_size;
	}
};
