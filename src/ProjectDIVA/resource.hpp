#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <typeinfo>
#include <d3dx9.h>


using namespace std;

#ifdef RECT
struct RECT{
	int	left, top, right, bottom;
};
struct fRECT{
	float left, top, right, bottom;
};
#endif

static void ErrorMsg(string error_text, bool fatal=true)
{
	MessageBox(NULL, error_text.c_str(), NULL, MB_OK|MB_ICONEXCLAMATION);
	if(fatal) exit(1);
}

static int GetArraySize(string &res_name)
{
	int posi = res_name.find_first_of('[');
	if( posi == string::npos ) return 1;
	sscanf(&res_name.c_str()[posi+1],"%d",&posi);
	return	posi;
}

class Resource{
public:
	void	*res_ptr;
	string	res_name;

	virtual string res_type() = NULL;
	virtual Resource* LoadNew(string &line) = NULL;
};

class ImageResource : public Resource {
public:
	static	IDirect3DDevice9 *pD3D;
	static	map <string,LPDIRECT3DTEXTURE9> texture_pool;
	string	res_type(){ return string("Image"); };
	string	filename;
	RECT	srcRect;
	fRECT	dstRect;
	int		Width, Height;
	D3DCOLOR color;

	LPDIRECT3DTEXTURE9 GetTexture()
	{
		return (LPDIRECT3DTEXTURE9)res_ptr;
	}

	Resource* LoadNew(string &line)
	{
		istringstream iss(line);
		string w;
		if(!(iss>>w)) return NULL;
		if(w.compare(res_type())) return NULL;
		if(!(iss>>res_name)) return NULL;

		// Load D3D texture
		if(!(iss>>filename)) return NULL;
		LPDIRECT3DTEXTURE9 pTexture = GetTexture(filename);
		res_ptr = pTexture;

		{// load src rect
			int X, Y;
			if(!(iss>>X>>Y>>Width>>Height)) goto err;
			if(!X && !Y && !Width && !Height)
				GetTextureSize(pTexture,Width,Height);
			RECT rect = {X,Y,X+Width,Y+Height};
			srcRect = rect;
		}

		{// load dest rect
			float X, Y, width, height;
			if(!(iss>>X>>Y>>width>>height)) X=Y=width=height=0;
			fRECT rect = {X,Y,X+width,Y+height};
			dstRect = rect;
		}

		{// load default color
			int r, g, b, a;
			if(!(iss>>r>>g>>b>>a)) r=g=b=a=255;
			color = D3DCOLOR_RGBA(r,g,b,a);
		}

		int size=GetArraySize(res_name), xstep=0, ystep=0;
		if(size>1) iss>>xstep>>ystep;

		ImageResource *array = new ImageResource [size];
		array[0] = *this;
		if(size>1)
		{
			int width, height, r, g, b, a;
			int initLeft = srcRect.left, initRight = srcRect.right;
			GetTextureSize(pTexture, width, height);
			for(int x=1; x<size; ++x)
			{
				srcRect.left += xstep, srcRect.right += xstep;
				if(srcRect.left>=width || srcRect.right>width)
				{
					srcRect.left = initLeft, srcRect.right = initRight;
					srcRect.top += ystep, srcRect.bottom += ystep;
				}
				array[x] = *this;
			}
			for(int x=0; x<size; ++x)
				if(iss>>r>>g>>b>>a)
					array[x].color = D3DCOLOR_RGBA(r,g,b,a);
		}
		return	array;

err:
		ErrorMsg("Malformed line for resource \""+res_name+"\"");
		return NULL;
	}

	static LPDIRECT3DTEXTURE9 GetTexture(string &filename)
	{
		LPDIRECT3DTEXTURE9 pTexture = texture_pool[filename];
		if(!pTexture)
		{
			if(FAILED(D3DXCreateTextureFromFile(pD3D,filename.c_str(),&pTexture)))
				ErrorMsg("D3DXCreateTextureFromFile failed to load \""+filename+"\"");
			texture_pool[filename] = pTexture;
		}
		return	pTexture;
	}

private:
	void GetTextureSize(LPDIRECT3DTEXTURE9 pTexture, int &width, int &height)
	{
		D3DSURFACE_DESC desc;
		if(FAILED(pTexture->GetLevelDesc(0,&desc)))
		{
			ErrorMsg("GetLevelDesc failed to load \""+res_name+"\"");
		}
		width = desc.Width;
		height = desc.Height;
	}
};

class SoundResource : public Resource {
public:
	string	res_type(){ return string("Sound"); };
	string	filename;
	Resource* LoadNew(string &line)
	{
		istringstream iss(line);
		string w;
		if(!(iss>>w)) return NULL;
		if(w.compare(res_type())) return NULL;
		if(!(iss>>res_name)) return NULL;
	}
};

class FloatResource : public Resource {
public:
	float	*value;
	string	res_type(){ return string("Float"); };
	Resource* LoadNew(string &line)
	{
		istringstream iss(line);
		string w;
		if(!(iss>>w)) return NULL;
		if(w.compare(res_type())) return NULL;
		if(!(iss>>res_name)) return NULL;
		int size = GetArraySize(res_name);
		value = new float [size];
		for(int x=0; x<size; x++)
		{
			if(!(iss>>value[x])) ErrorMsg("Malformed line for resource \""+res_name+"\"");
		}
		FloatResource *array = new FloatResource();
		*array = *this;
		return array;
	}
};

class IntResource : public Resource {
public:
	int	*value;
	string	res_type(){ return string("Int"); };
	Resource* LoadNew(string &line)
	{
		istringstream iss(line);
		string w;
		if(!(iss>>w)) return NULL;
		if(w.compare(res_type())) return NULL;
		if(!(iss>>res_name)) return NULL;
		int size = GetArraySize(res_name);
		value = new int [size];
		for(int x=0; x<size; x++)
		{
			if(!(iss>>value[x])) ErrorMsg("Malformed line for resource \""+res_name+"\"");
		}
		IntResource *array = new IntResource();
		*array = *this;
		return array;
	}
};

class ResourceLoader{
	map <string, Resource*> pool;

public:
	Resource * getResource(string res_name){ return pool[res_name]; }
	ResourceLoader(){}
	ResourceLoader(const char *filename){ LoadFile(filename); }
	int LoadFile(const char *filename)
	{
		// register resource types
		static const int registryN = 4;
		static Resource* registry[registryN] = {
			(Resource*) new ImageResource(),
			(Resource*) new SoundResource(),
			(Resource*) new FloatResource(),
			(Resource*) new IntResource(),
		};

		// Load resource file
		ifstream fin(filename);
		if(!fin) return -1;
		string line;
		int N_loaded = 0;
		while(getline(fin,line))
		{
			if(line.empty()) continue;
			if(line[0]=='#') continue;
			Resource *pRes;
			for(int x=0; x<registryN; x++)
			{
				pRes = registry[x]->LoadNew(line);
				if(pRes)
				{
					pool[pRes->res_name] = pRes;
					N_loaded++;
					break;
				}
			}
		}

		return	N_loaded;
	};
};

