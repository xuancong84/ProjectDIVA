#include "notemap.h"
#include "VideoEngine.h"
#include "SoundEngine.h"
#include "GraphEngine.h"
#include "GameMana.h"
#include <iostream>

using namespace std;
NoteMap _notemap;

bool bHasAudio;
/*
int UNIT_COLOR[][3]={
	{246,  127,  167},	//UNIT_CIRCLE
	{240,  135,  230},	//UNIT_RECT
	{98,  191,  225},	//UNIT_CROSS
	{234, 254,  183},	//UNIT_TRIANGLE
	{240,  135,  230},	//UNIT_RIGHT
	{246,  127,  167},	//UNIT_LEFT
	{234, 254,  183},	//UNIT_DOWN
	{98,  191,  225},	//UNIT_UP
};

int UNIT_COLOR[][3]={
	{126,  240,  142},	//UNIT_CIRCLE
	{255,  141,  166},	//UNIT_RECT
	{96,  203,  255},	//UNIT_CROSS
	{234, 51,  255},	//UNIT_TRIANGLE
	{126,  240,  142},	//UNIT_RIGHT
	{255,  141,  166},	//UNIT_LEFT
	{96,  203,  255},	//UNIT_DOWN
	{234, 51,  255},	//UNIT_UP
};*/

void trimLeft(string &target)
{
	if (target.size() != 0)
	{
		for (string::iterator pos = target.begin();  pos < target.end(); )
		{
			if (*pos == ' ')
			{
				target.erase(pos);
				pos = target.begin();
			}
			else
			{
				return;
			}
		}
	}
}

void trimRight(string &target)
{
	if (target.size() != 0)
	{
		for (string::iterator pos = target.end()-1; pos != target.begin(); )
		{
			if (*pos == ' ')
			{
				target.erase(pos);
				pos = target.end();
			}
			else
			{
				return;
			}
		}
	}
}

int CheckResource(string filename)
{
	if(!isImage(filename))
		return VIDEO;
	else
		return GRAPH;
}

int	NoteMap::SetTimePos(float v)
{
	int posi = (int)(_timeTable._frameNum*v+0.5f);
	_timeTable.SetFramePos(posi);
	_timeTable.SetNotesPos(posi);
	double cur_note_time = _timeTable.GetNotes()->getTimePosition();
	while(true){
		_timeTable.NextNotes();
		LpFrame note = _timeTable.GetNotes();
		if(note==NULL)
			break;
		if(note->getTimePosition()-cur_note_time >= gameini.note_standing/core._notemana.tail_speed_factor*core._notemana.singleTime)
			break;
	}
	return posi;
}

//NoteMap
void NoteMap::SetNew(string filename,string baseDirec)
{
	int pos;
	
	Clear();

	FILE *fin = fopen(filename.c_str(),"r");
	//ifstream fin(filename.c_str());
	char buffer[300];
	if(!fin)
		return;

	//Map General Info
	fgets(buffer,300,fin);
	checkTail0(buffer,300);
	EditorVer = buffer;
	//getline(fin,EditorVer);
	fgets(buffer,300,fin);
	checkTail0(buffer,300);
	_mapName = Ansi2UTF8(buffer);
	//getline(fin,_mapName);
	fgets(buffer,300,fin);
	checkTail0(buffer,300);
	_noterName = Ansi2UTF8(buffer);
	//getline(fin,_noterName);
	fgets(buffer,300,fin);
	checkTail0(buffer,300);
	_authorName = Ansi2UTF8(buffer);
	//getline(fin,_authorName);
	fgets(buffer,300,fin);
	checkTail0(buffer,300);
	_musicStyle = Ansi2UTF8(buffer);
	//getline(fin,_musicStyle);
	fgets(buffer,300,fin);
	checkTail0(buffer,300);
	//OverViewPic
	fscanf(fin,"%d\n%d\n%lf\n%d\n",&_level,&_hard,&_timeTable._BPM,&_timeTable._frameNum);
	//fin>>_level>>_hard;

    //TimeTable General Info
	//fin>>_timeTable._BPM;//Total BPM
	//fin>>_timeTable._frameNum;//Period Num
	_timeTable._frameNum *= notePerPeriod;
	_timeTable._frame = new Frame[_timeTable._frameNum];
	for(int i = 0; i < _timeTable._frameNum; i++)
	{
		_timeTable._frame[i].Clear();
		_timeTable._frame[i].notePos = i;
		for(int j = 0; j < NOTENum; j++)
			_timeTable._frame[i]._note[j].notePos = i;
	}

	//Period Info
			 
	//BPM
	while(true)
	{
		fscanf(fin,"%d",&pos);
		//fin >> pos;
		if(pos==-1)
			break;
		fscanf(fin,"%lf\n",&_timeTable._frame[pos]._BPM);
		if(EditorVer<="1.0.4.7")
		{
			double digit = _timeTable._frame[pos]._BPM-floor(_timeTable._frame[pos]._BPM);
			if(digit>0.8)
				_timeTable._frame[pos]._BPM = ceil(_timeTable._frame[pos]._BPM);
			else if(digit<0.2)
				_timeTable._frame[pos]._BPM = floor(_timeTable._frame[pos]._BPM);
		}
		//fin >> _timeTable._frame[pos]._BPM;
	    _timeTable._frame[pos]._IsSetBPM = true;
	}

	double lastTime = 0,singleTime = 1;
	int lastBPMIndex = 0;
	for(int i = 0; i < _timeTable._frameNum; i++)
	{
		_timeTable._frame[i].setTimePosition(lastTime+(i-lastBPMIndex)*singleTime);
		if(_timeTable._frame[i]._BPM>0)
		{
			//	singleTime = _timeTable._frame[i]._BPM;
			lastTime += (i-lastBPMIndex)*singleTime,lastBPMIndex = i;
			singleTime = 60.0*base::SECOND/(_timeTable._frame[i]._BPM*((notePerPeriod/timePerPeriod)));
		}
		for(int j = 0; j < NOTENum; j++)
			_timeTable._frame[i]._note[j].notePos = i;
	}

	//Resource
	while(true)
	{
		fscanf(fin,"%d",&pos);
		//fin >> pos;
		if(pos==-1)
			break;
		if(pos<0) videoEngine.m_pTime = -pos*0.001, pos=0;
		else videoEngine.m_pTime = 0;
		fscanf(fin,"%d\n",&_timeTable._frame[pos]._resource);
		//fin >> _timeTable._frame[pos]._resource;
	}

	//BGS
	while(true)
	{
		fscanf(fin,"%d",&pos);
		//fin >> pos;
		if(pos==-1)
			break;
		int bgsNum, resNum;
		fscanf(fin," %d %d\n",&bgsNum,&resNum);
		if(pos<0) soundEngine->playOffset[resNum]=-pos, pos=0;
		else soundEngine->playOffset[resNum]=0;
		//fin >> bgsNum;
		_timeTable._frame[pos]._BGM[bgsNum]=resNum;
		//fin >> _timeTable._frame[pos]._BGM[bgsNum];
	}
	
	//note
	while(true)
	{
		fscanf(fin,"%d",&pos);
		//fin >> pos;
		if(pos==-1)
			break;
	    int noteNum =  _timeTable._frame[pos]._noteNum;
		Note &tnote = _timeTable._frame[pos]._note[noteNum];
		fscanf(fin," %d %d %d %d %d %d",&tnote._type,&tnote._x,&tnote._y,&tnote._tailx,&tnote._taily,&tnote._key);
		if(tnote._type>=NOTENum)
			fscanf(fin,"%lf\n",&tnote.duration);
		else
			fscanf(fin,"\n");
		//fin >> _timeTable._frame[pos]._note[noteNum]._type >> _timeTable._frame[pos]._note[noteNum]._x >> 
			//_timeTable._frame[pos]._note[noteNum]._y >> _timeTable._frame[pos]._note[noteNum]._tailx >> 
			//_timeTable._frame[pos]._note[noteNum]._taily >> _timeTable._frame[pos]._note[noteNum]._key;
		_timeTable._frame[pos]._noteNum++;
	}
	
	wav.clear();
	resource.clear();

	//wav
	while(true)
	{
		fscanf(fin,"%d",&pos);
		//fin >> pos;
		if(pos==-1)
			break;
		string filename;
		fgets(buffer,300,fin);
		checkTail0(buffer,300);
		filename=buffer;
		//getline(fin,filename);
		trimLeft(filename);
		trimRight(filename);
		filename = baseDirec + filename;
		wav[pos] = filename;
	}

	//resource
	while(true)
	{
		fscanf(fin,"%d",&pos);
		//fin >> pos;
		if(pos==-1)
		    break;
		string filename;
		fgets(buffer,300,fin);
		checkTail0(buffer,300);
		filename=buffer;
		//getline(fin,filename);
		trimLeft(filename);
		trimRight(filename);
		filename = baseDirec + filename;
		resource[pos] = filename;
	}
	
	if(EditorVer>="1.0.1.0")
		fscanf(fin,"%d %d",&_chanceTimeStart,&_chanceTimeEnd);
	fclose(fin);
}

void NoteMap::Resource_read()
{
	string filename;
	bHasAudio = false;

	// scan through BGS resource
	for(auto ptr=_notemap.wav.begin(); ptr!=_notemap.wav.end(); ++ptr)
	{
		auto new_sound = soundEngine->ReadMusic(ptr->second);
		soundEngine->playMusic[ptr->first] = new_sound;
		bHasAudio = true;
	}

	// scan through general resource
	for(auto ptr = resource.begin(); ptr != resource.end(); ptr++)
	{
		filename = ptr->second;
		if(CheckResource(filename)==GRAPH)
			graphEngine.AddTexture(filename,GameCoreRes);
		else if(CheckResource(filename)==VIDEO)
		{
			// load the video file and create DirectShow textures
			videoEngine.Read(filename);
		}
	}
	Sleep(250);	// give plenty time for audio/video caching buffering
}

bool TimeTable::NextFrame()
{
	if(framePos==_frameNum)
		return true;
	return ((++framePos)==_frameNum);
}

bool TimeTable::NextNotes()
{
	if(notesPos==_frameNum)
		return true;
	return ((++notesPos)==_frameNum);
}
/*//Period
bool Period::NextFrame()
{
	if(framePos==notePerPeriod)
		return true;
	return ((++framePos)==notePerPeriod);
}*/

DWORD HSL2RGB(float h, float sl, float l)
{
	float v;
	float r,g,b;

	r = l;   // default to gray
	g = l;
	b = l; 
	v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
	if (v > 0) 
	{
		float m;
		float sv;
		int sextant;
		float fract, vsf, mid1, mid2;

		m = l + l - v;
		sv = (v - m ) / v;
		h *= 6.0;
		sextant = (int)h; 
		fract = h - sextant;
		vsf = v * sv * fract;
		mid1 = m + vsf;
		mid2 = v - vsf;
		switch (sextant) 
		{
		case 0: 
			r = v; 
			g = mid1; 
			b = m; 
			break;
		case 1: 
			r = mid2; 
			g = v; 
			b = m; 
			break;
		case 2: 
			r = m; 
			g = v; 
			b = mid1; 
			break;
		case 3: 
			r = m; 
			g = mid2; 
			b = v; 
			break;
		case 4: 
			r = mid1; 
			g = m; 
			b = v; 
			break;
		case 5: 
			r = v; 
			g = m; 
			b = mid2; 
			break;
		}
	}
	DWORD rgb[4];
	rgb[3] = 0xff;
	rgb[2] = r * 255.0f;
	rgb[1] = g * 255.0f;
	rgb[0] = b * 255.0f;
	return *(DWORD*)rgb;
}

double Frame::getTimePosition() const
{
	return timePosition + core.timeoffset;
}

void Frame::setTimePosition(double val)
{
	timePosition = val;
}

