#include "GameIni.h"

GameIni gameini;

const double GameIni::NOTE_BLOWUP = 1.8;
/*
	delay = base::SECOND*0.25;
	COOL = base::SECOND*0.025;
	FINE = base::SECOND*0.075;
	SAFE = base::SECOND*0.12;
	SAD = base::SECOND*0.18;
	eps = base::SECOND*0.01;
*/
void GameIni::Init()
{
	playstate_delay = base::SECOND*0.5;
	uistate_delay = base::SECOND*0.5;
	delay = base::SECOND*0.35;
	COOL = base::SECOND*0.05;
	FINE = base::SECOND*0.10;
	SAFE = base::SECOND*0.20;
	SAD = base::SECOND*0.30;
	eps = base::SECOND*0.01;
}

const int GameIni::HP_CHANGE[GAME_LEVEL_MAX][PRESS_STATE_MAX]={
	{2, 1, 0, -1, -2},
	{2, 1, 0, -1, -2},
	{1, 0, -1, -2, -4},
	{1, 0, -25, -50, -100},
};

/*
template <class T>
class TSvector {
private:
	T*	m_data;
	int	m_size;
	int	m_max_size;
	HANDLE hEvent;

	struct iterator{	// must be private
		int posi;
		TSvector <T> *q;

		iterator& operator ++ ()
		{
			++ posi;
			return *this;
		}
		bool operator != (int rhs){ return posi!=rhs; }
		bool operator == (int rhs){	return posi==rhs; }
		void operator = (int rhs) { posi = rhs; }
		T& operator *()	{ return q->m_data[posi]; }
		T* operator ->(){ return &q->m_data[posi]; }
		iterator(int _posi, TSvector <T> *_q):posi(_posi),q(_q)
		{	// acquire lock
			WaitForSingleObject(q->hEvent,INFINITE);
		}
		~iterator()
		{	// release lock
			SetEvent(q->hEvent);
		}
	};

public:
	TSvector(int _size = 16)
	{
		if(_size<=0) _size=4;
		m_size = 0;
		m_max_size = _size;
		m_data = new T [m_max_size];
		hEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
	}
	~TSvector()
	{
		WaitForSingleObject(hEvent,INFINITE);
		CloseHandle(hEvent);
		delete [] m_data;
	}

	bool empty() const { return !m_size; }
	int size() const { return m_size; }
	void push_back(T& x)
	{
		WaitForSingleObject(hEvent,INFINITE);
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
		SetEvent(hEvent);
	}
	T pop_back()
	{
		WaitForSingleObject(hEvent,INFINITE);
		if(m_size>0)
		{
			--m_size;
			SetEvent(hEvent);
			return m_data[m_size];
		}
		SetEvent(hEvent);
	}
	void erase(iterator &it)
	{
		if((--m_size)>0)
		{
			m_data[it.posi] = m_data[m_size];
			m_data[m_size] = T();
		}
	}
	void clear()
	{
		WaitForSingleObject(hEvent,INFINITE);
		delete [] m_data;
		m_data = new T [m_max_size];
		m_size = 0;
		SetEvent(hEvent);
	}

	iterator begin(){ return iterator(0,this); }
	int end(){ return m_size; }
};

class Test{
public:
	int _x,_y;
	char *buf;
	Test(){_x=_y=0;buf=new char[100];}
	Test(int x,int y)
	{
		_x=x; _y=y;
		buf = new char [100];
	}
	~Test()
	{
		delete [] buf;
	}
	void operator = (Test& rhs)
	{
		buf = new char [100]; memcpy(buf,rhs.buf,100);
		_x=rhs._x; _y=rhs._y;
	}
};

void test(){
	TSvector <string*> cs(3);
	string *p=new string("aaa");
	cs.push_back(p);
	cs.push_back(p=new string("bbb"));
	cs.push_back(p=new string("ccc"));
	cs.push_back(p=new string("ddd"));
	for(auto it=cs.begin();it!=cs.end();it++){
		//it->append("K");
		(*it)->append("L");
	}
	for(auto it=cs.begin();it!=2;it++){
		cs.erase(it);
	}
	if(!cs.empty()) cs.pop_back();
	cs.push_back(p=new string("ddd"));
	cs.pop_back();
	cs.pop_back();
	cs.push_back(p=new string("eee"));
	cs.push_back(p=new string("fff"));
	int x=cs.size();
	cs.clear();

	TSvector <string> ca(3);
	ca.push_back(string("aa"));
	ca.push_back(string("bb"));
	ca.push_back(string("cc"));
	{
		auto it=ca.begin();
		it->append("OK");
		ca.erase(it);
	}

	TSvector <int> ci(3);
	int i;
	ci.push_back(i=1);
	ci.push_back(i=2);
	ci.push_back(i=3);

	TSvector <Test> cc(3);
	cc.push_back(Test(1,2));
	cc.push_back(Test(1,2));
	cc.push_back(Test(1,2));
	{
		auto it=cc.begin();
		cc.erase(it);
	}

	string *oldptr = new string [3];
	oldptr[0]="a";oldptr[0]="b";oldptr[0]="c";

	char *newptr = new char [sizeof(string)*3];
	memcpy(newptr,oldptr,sizeof(string)*3);

};
*/