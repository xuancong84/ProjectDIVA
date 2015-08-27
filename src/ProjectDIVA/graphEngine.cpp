//--------------------------------------------------------------------------------------
// File: GraphEngine.cpp
//
// Copyright (c) CK. All rights reserved.
//--------------------------------------------------------------------------------------

#include "GraphEngine.h"
#include "base.h"
#include <string>
#include <functional>
using namespace std;

GraphEngine graphEngine = GraphEngine();
//-------------------------Treap------------------------

Graph_Treap_Node::Graph_Treap_Node(string filename)
{
	_filename=filename;
	_p=rand();
	HRESULT hr;
	_l=_r=NULL;
	
	if(FAILED(hr=D3DXCreateTextureFromFile(base::Device,_filename.data(),&_tex)))
		ErrorExit("Error while read Texture \""+filename+"\"\nProgram will exit",0);
}

Graph_Treap_Node::~Graph_Treap_Node()
{
	SAFE_RELEASE(_tex);
}

Graph_Treap::Graph_Treap()
{
	v=NULL;
}

Graph_Treap::~Graph_Treap()
{
	clear();
}

void Graph_Treap::lr(Graph_Treap_Node *&p)
{
	Graph_Treap_Node *q=p->_l;p->_l=q->_r;q->_r=p;p=q;
}

void Graph_Treap::rr(Graph_Treap_Node *&p)
{
	Graph_Treap_Node *q=p->_r;p->_r=q->_l;q->_l=p;p=q;
}

bool Graph_Treap_cmp(Graph_Treap_Node *x, Graph_Treap_Node *y)
{
	return y==NULL?(x!=0):(x!=0&&x->_p<y->_p);
}

void Graph_Treap::del(string filename)
{
	del(filename,v);
}

void Graph_Treap::del(string filename, Graph_Treap_Node *&p)
{
	if(p==NULL) return;
	if(filename<p->_filename) del(filename,p->_l); 
	else if(filename>p->_filename) del(filename,p->_r);
	else
	{
		if(p->_l==NULL&&p->_r==NULL) {SAFE_DELETE(p);}
		else if(Graph_Treap_cmp(p->_l,p->_r)) {lr(p);del(filename,p->_r);}
		else {rr(p);del(filename,p->_l);}
	}
}

Texture* Graph_Treap::ins(string filename)
{
	return ins(filename,v);
}

Texture* Graph_Treap::ins(string filename,Graph_Treap_Node *&p)
{
	if(p==NULL) {p = new Graph_Treap_Node(filename);return p->_tex;}
	else if(filename<p->_filename) {Texture *ret=ins(filename,p->_l);if(Graph_Treap_cmp(p->_l,p)) lr(p);return ret;}
	else if(filename>p->_filename) {Texture *ret=ins(filename,p->_r);if(Graph_Treap_cmp(p->_r,p)) rr(p);return ret;}
	else return p->_tex;
}

Texture* Graph_Treap::find(string filename)
{
	return find(filename,v);
}

Texture* Graph_Treap::find(string filename,Graph_Treap_Node *p)
{
	if(p==NULL) return NULL;
	else if(filename<p->_filename) return find(filename,p->_l);
	else if(filename>p->_filename) return find(filename,p->_r);
	else return p->_tex;
}

void Graph_Treap::clear()
{
	clear(v);
}

void Graph_Treap::clear(Graph_Treap_Node *&p)
{
	if(p==NULL) return;
	else
	{
		clear(p->_l);
		clear(p->_r);
		SAFE_DELETE(p);
	}
}


//-----------------------End Treap----------------------




//------------------------------------Graph Engine Methods-------------------------------------------------------------


GraphEngine::GraphEngine(IDirect3DDevice9* pDevice,ID3DXSprite* pSprite)
{
	memset(_TexPool,0,sizeof(_TexPool));
	InitEngine(pDevice,pSprite);
}

void GraphEngine::InitEngine(IDirect3DDevice9* pDevice,ID3DXSprite* pSprite)
{
	ReleaseTex();
	_pDevice = pDevice;
	_pSprite = pSprite;
	_pFont = NULL;
	_Pooluse = 0;
	_ftDefaultColor = D3DCOLOR_ARGB(255,255,255,255);
	for(int i=0;i<GraphEngine::MaxPoolnum;i++)
		_TexPool[i] = new Graph_Treap();
}

GraphEngine::~GraphEngine()
{
	for(int i=0;i<GraphEngine::MaxPoolnum;i++)
		SAFE_DELETE(_TexPool[i]);
	for(map<string, map<int, map<bool, map<DWORD, ID3DXFont*>>>>::iterator ptr1 = fonts.begin();ptr1!=fonts.end();ptr1++)
	{
		for(map<int, map<bool, map<DWORD, ID3DXFont*>>>::iterator ptr2 = ptr1->second.begin();ptr2!=ptr1->second.end();ptr2++)
		{
			for(map<bool, map<DWORD, ID3DXFont*>>::iterator ptr3 = ptr2->second.begin();ptr3!=ptr2->second.end();ptr3++)
			{
				for(map<DWORD, ID3DXFont*>::iterator ptr4 = ptr3->second.begin();ptr4!=ptr3->second.end();ptr4++)
					ptr4->second->Release();
				ptr3->second.clear();
			}
			ptr2->second.clear();
		}
		ptr1->second.clear();
	}
	fonts.clear();
}

///-------------------------------------
///获取指定纹理的地址
///-------------------------------------
Texture* GraphEngine::findTexture(string filename,bool ForceAdd)
{
	Texture *ret = NULL;
	for(int i=0;i<MaxPoolnum;i++)
	{
		ret = _TexPool[i]->find(filename);
		if(ret) return ret;
	}
	if(ForceAdd)
		ret = AddTexture(filename);
	return ret;
}

///-------------------------------------
///将文件名为filename的纹理读入纹理池
///-------------------------------------
Texture* GraphEngine::AddTexture(string filename,int Poolnum)
{
	if(Poolnum==-1) Poolnum=_Pooluse;
	return _TexPool[Poolnum]->ins(filename);
}

///-------------------------------------
///将文件名为filename的纹理从纹理池中删除
///-------------------------------------
void GraphEngine::DeleteTexture(string filename,int Poolnum)
{
	if(Poolnum==-1)
	{
		for(int i=0;i<MaxPoolnum;i++)
			_TexPool[i]->del(filename);
	}
	else
		_TexPool[Poolnum]->del(filename);
}

///-------------------------------------
///获得纹理池中指定纹理的属性
///-------------------------------------
void GraphEngine::GetTextureDesc(string filename, D3DSURFACE_DESC *p_out)
{
	GetTextureDesc(findTexture(filename,false),p_out);
}

void GraphEngine::GetTextureDesc(Texture *tex, D3DSURFACE_DESC *p_out)
{
	if(tex) tex->GetLevelDesc(0,p_out);		
}

///-------------------------------------
///释放纹理资源
///-------------------------------------
void GraphEngine::ReleaseTex()
{
	for(int i=0;i<GraphEngine::MaxPoolnum;i++)
		if(_TexPool[i])
			_TexPool[i]->clear();
}

void GraphEngine::ReleaseTex(int Poolnum)
{
	_TexPool[Poolnum]->clear();
}

///-------------------------------------
///在点(dx,dy)处绘制整个纹理
///-------------------------------------
void GraphEngine::Draw(Texture *tex, float dx, float dy)
{
	Draw(tex,dx,dy,D3DCOLOR_ARGB( 255, 255, 255, 255 ));
}

///-------------------------------------
///在点(dx,dy)处绘制整个纹理，并用指定颜色渲染
///-------------------------------------
void GraphEngine::Draw(Texture *tex, float dx, float dy,D3DCOLOR color)
{
	// Obtain source rectangle
	D3DSURFACE_DESC desc;
	tex->GetLevelDesc(0,&desc);
	RECT srcRect = {0, 0, desc.Width, desc.Height};
	fRECT dstRect = {dx, dy, dx+desc.Width, dy+desc.Height};

	Draw(tex,srcRect,dstRect,color);
}

void GraphEngine::Draw(ImageResource *rc, float dx, float dy,D3DCOLOR color)
{
	fRECT dstRect = {dx, dy, dx+rc->dstRect.right-rc->dstRect.left, dy+rc->dstRect.bottom-rc->dstRect.top};
	Draw(rc->GetTexture(),rc->srcRect,dstRect,color);
}
void GraphEngine::Draw(ImageResource *rc, float dx, float dy)
{
	fRECT dstRect = {
		dx+rc->dstRect.left,
		dy+rc->dstRect.top,
		dx+rc->dstRect.right,
		dy+rc->dstRect.bottom
	};
	Draw(rc->GetTexture(),rc->srcRect,dstRect,rc->color);
}
void GraphEngine::Draw1(ImageResource *rc, float dx, float dy, D3DCOLOR color)
{
	fRECT dstRect = {
		dx+rc->dstRect.left,
		dy+rc->dstRect.top,
		dx+rc->dstRect.right,
		dy+rc->dstRect.bottom
	};
	Draw(rc->GetTexture(),rc->srcRect,dstRect,color);
}
void GraphEngine::DrawCenter(ImageResource *rc, float dx, float dy)
{
	fRECT dstRect = {
		rc->dstRect.left+dx,
		rc->dstRect.top+dy,
		rc->dstRect.right+dx,
		rc->dstRect.bottom+dy
	};
	Draw(rc->GetTexture(),rc->srcRect,dstRect,rc->color);
}
void GraphEngine::Draw(ImageResource *rc)
{
	Draw(rc->GetTexture(),rc->srcRect,rc->dstRect,rc->color);
}

///-------------------------------------
///在矩形drawRect内伸缩绘制整个纹理
///-------------------------------------
void GraphEngine::Draw(Texture* tex, const fRECT &drawRect)
{
	Draw(tex,drawRect,D3DCOLOR_ARGB( 255, 255, 255, 255 ));
}

///-------------------------------------
///在矩形drawRect内伸缩绘制整个纹理，并用制定颜色渲染
///-------------------------------------
void GraphEngine::Draw(Texture* tex, const fRECT &drawRect, D3DCOLOR color)
{
	// Obtain source rectangle
	D3DSURFACE_DESC desc;
	tex->GetLevelDesc(0,&desc);
	RECT srcRect = {0, 0, desc.Width, desc.Height};

	Draw(tex,srcRect,drawRect,color);
}

void GraphEngine::Draw(ImageResource *rc, const fRECT &drawRect, D3DCOLOR color)
{
	Draw( rc->GetTexture(), rc->srcRect, drawRect, color );
}

void GraphEngine::Draw(ImageResource *rc, const RECT &srcRect, const fRECT &drawRect)
{
	Draw( rc->GetTexture(), srcRect, drawRect, rc->color );
}

void GraphEngine::Draw(ImageResource *rc, const fRECT &drawRect)
{
	Draw( rc->GetTexture(), rc->srcRect, drawRect, rc->color );
}

///-------------------------------------
///在点(dx,dy)处绘制纹理的srcRect部分
///-------------------------------------
void GraphEngine::Draw(Texture *tex,const RECT &srcRect,float dx,float dy)
{
	fRECT dstRect = {dx, dy, dx+srcRect.right-srcRect.left, dy+srcRect.bottom-srcRect.top};
	Draw(tex,srcRect,dstRect,D3DCOLOR_ARGB( 255, 255, 255, 255 ));
}
///-------------------------------------
///在点(dx,dy)处绘制纹理的srcRect部分，并用制定颜色渲染
///-------------------------------------
void GraphEngine::Draw(Texture *tex,const RECT &srcRect,float dx,float dy,D3DCOLOR color)
{
	fRECT dstRect = {dx, dy, dx+srcRect.right-srcRect.left, dy+srcRect.bottom-srcRect.top};
	Draw(tex, srcRect, dstRect, color);
}

///-------------------------------------
///在矩形drawRect内伸缩绘制纹理的srcRect部分
///-------------------------------------
void GraphEngine::Draw(Texture *tex,const RECT &srcRect,const fRECT &drawRect)
{
	Draw(tex,srcRect,drawRect,D3DCOLOR_ARGB( 255, 255, 255, 255 ));
}

///-------------------------------------
///在矩形drawRect内伸缩绘制纹理的srcRect部分，并用制定颜色渲染
///-------------------------------------
void GraphEngine::Draw(	Texture *tex, const RECT &srcRect, const fRECT &drawRect, 
						D3DCOLOR color, const D3DXVECTOR3 *pCenter )
{

	//if(drawMode==2){
	// use sprite draw, faster
	float left, top, right, bottom;
	base::MakeDrawRect(drawRect, left, top, right, bottom);

	D3DXMATRIX transform,transform2,orialTrans;
	float fx = (right-left)/(srcRect.right-srcRect.left);
	float fy = (bottom-top)/(srcRect.bottom-srcRect.top);

	D3DXMatrixScaling(&transform, fx, fy, 1.0f);
	D3DXMatrixTranslation(&transform2,left,top,0.0f);
	transform = transform * transform2;
	_pSprite->GetTransform(&orialTrans);
	transform = transform * orialTrans;
	_pSprite->SetTransform(&transform);

	_pSprite->Draw(tex,&srcRect,pCenter,NULL,color);

	_pSprite->SetTransform(&orialTrans);

	/*
	}else if(drawMode==1){
		/*
		// use primitive draw, slower
		D3DXMATRIX transform,orialTrans,a,b;
		_pDevice->GetTransform(D3DTS_PROJECTION, &orialTrans);
		_pDevice->GetTransform(D3DTS_VIEW, &a);
		_pDevice->GetTransform(D3DTS_WORLD, &b);
		D3DXMatrixScaling(&transform, (float)WIDTH/base::window_rect.right,
			(float)HEIGHT/base::window_rect.bottom, 1.0f);
		transform = transform*orialTrans;
		//_pDevice->SetTransform(D3DTS_PROJECTION, &transform);
		*
		_pDevice->SetTexture(0, tex);

		DWORD FVF = D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1;
		struct VERTEX
		{
			float		x,y,z;		// The position
			D3DCOLOR	color;		// The color
			FLOAT		tu, tv;		// The texture coordinates
		};
		D3DSURFACE_DESC desc;
		tex->GetLevelDesc(0,&desc);
		VERTEX square[4]={
			{drawRect.left,drawRect.top,0, color,	 (float)srcRect.left/desc.Width,(float)srcRect.top/desc.Height},
			{drawRect.right,drawRect.top,0, color,	 (float)srcRect.right/desc.Width,(float)srcRect.top/desc.Height},
			{drawRect.left,drawRect.bottom,0, color, (float)srcRect.left/desc.Width,(float)srcRect.bottom/desc.Height},
			{drawRect.right,drawRect.bottom,0, color, (float)srcRect.right/desc.Width,(float)srcRect.bottom/desc.Height},
		};

		DrawPrimitiveUP( &square, FVF, sizeof(VERTEX), D3DPT_TRIANGLESTRIP, 2 );

		_pDevice->SetTexture(0, NULL);
		//_pDevice->SetTransform(D3DTS_PROJECTION, &orialTrans);
	}*/
}

void GraphEngine::DrawPrimitiveUP( void *pData, DWORD FVF, int stride,
								D3DPRIMITIVETYPE primitive_type, int primitive_count )
{
	// primitives can only be drawn in Mode 1
	SetDrawMode(1);
	_pDevice->SetFVF(FVF);
	_pDevice->DrawPrimitiveUP(primitive_type, primitive_count, pData, stride);
}

void GraphEngine::SetFont(string ftFaceName, int ftSize, bool ftItalic, DWORD ftWeight)
{
	// Normalize font size according to window size
	ftSize = (int)(ftSize*base::window_rect.right/480.0+0.5);
	ID3DXFont *& t_pFont = fonts[ftFaceName][ftSize][ftItalic][ftWeight];
	if(t_pFont==NULL)
	{
		D3DXCreateFont(_pDevice, ftSize, 0, ftWeight, 0, ftItalic, 
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			ftFaceName.c_str(),&t_pFont);
	}
	_pFont = t_pFont;
}

void GraphEngine::SetFontDefaultColor(int a,int r,int g,int b)
{
	_ftDefaultColor = D3DCOLOR_ARGB(a,r,g,b);
}

void GraphEngine::DrawText(const string &text, const string name, int dx, int dy, int nrows, D3DXCOLOR color){
	static map<string, pair<size_t,float> > time_elapse;		// <hash, time-elapse-in-seconds>
	hash<string> str_hash;
	size_t hashval = str_hash(text);
	if(hashval != time_elapse[name].first){
		time_elapse[name].first = hashval;
		time_elapse[name].second = 0;
	}
	float tm = time_elapse[name].second;
	auto its = split(text, "\n");
	// TODO
}

void GraphEngine::DrawText(const string &text, int dx, int dy)
{
	RECT drawRect = base::MakeDrawRect(dx,dy,0,0);
	//DrawText(text.c_str(), -1, &drawRect, DT_CALCRECT, 0);
	DrawText(text.c_str(), -1, &drawRect, DT_NOCLIP, _ftDefaultColor);
}

void GraphEngine::DrawText(const string &text, int dx, int dy, DWORD dwFlags)
{
	RECT drawRect = base::MakeDrawRect(dx,dy,0,0);
	//DrawText(text.c_str(), -1, &drawRect, DT_CALCRECT, 0);
	DrawText(text.c_str(), -1, &drawRect, dwFlags, _ftDefaultColor);
}

void GraphEngine::DrawText(const string &text, RECT drawRect)
{
	drawRect = base::MakeDrawRect(drawRect);
	DrawText(text.c_str(), -1, &drawRect, DT_NOCLIP, _ftDefaultColor);
}

void GraphEngine::DrawText(const string &text, RECT drawRect, DWORD dwFlags)
{
	drawRect = base::MakeDrawRect(drawRect);
	DrawText(text.c_str(), -1, &drawRect, dwFlags, _ftDefaultColor);
}

void GraphEngine::DrawText(const string &text, int dx, int dy, D3DXCOLOR color)
{
	RECT drawRect = base::MakeDrawRect(dx,dy,0,0);
	//DrawText(text.c_str(), -1, &drawRect, DT_CALCRECT, 0);
	DrawText(text.c_str(), -1, &drawRect, DT_NOCLIP, color);
}

void GraphEngine::DrawText(const string &text, int dx, int dy, DWORD dwFlags, D3DXCOLOR color)
{
	RECT drawRect = base::MakeDrawRect(dx,dy,0,0);
	//DrawText(text.c_str(), -1, &drawRect, DT_CALCRECT, 0);
	DrawText(text.c_str(), -1, &drawRect, dwFlags, color);
}

void GraphEngine::DrawText(const string &text, RECT drawRect, D3DXCOLOR color)
{
	drawRect = base::MakeDrawRect(drawRect);
	DrawText(text.c_str(), -1, &drawRect, DT_NOCLIP, color);
}

void GraphEngine::DrawText(const string &text, RECT drawRect, DWORD dwFlags, D3DXCOLOR color)
{
	drawRect = base::MakeDrawRect(drawRect);
	DrawText(text.c_str(), -1, &drawRect, dwFlags, color);
}

void GraphEngine::DrawText(LPCSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color)
{
	wstring ws = Ansi2WChar(pString, CP_UTF8);
	_pFont->DrawTextW((drawMode==2?_pSprite:NULL), ws.data(), Count, pRect, Format, Color);
}

HRESULT GraphEngine::Clear(){
	return _pDevice->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
}
void GraphEngine::SetDrawMode(int mode)
{
	if(drawMode==mode)
		return;
	if(drawMode==0){
		if(mode==1){
			_pDevice->BeginScene();
		}else if(mode==2){
			_pDevice->BeginScene();
			_pSprite->Begin( D3DXSPRITE_ALPHABLEND  );
			if(pMat)
				_pSprite->SetTransform(pMat);
		}
	}else if(drawMode==1){
		if(mode==0){
			_pDevice->EndScene();
		}else if(mode==2){
			_pSprite->Begin( D3DXSPRITE_ALPHABLEND );
			if(pMat)
				_pSprite->SetTransform(pMat);
		}
	}else if(drawMode==2){
		if(!pMat)
			pMat = new D3DXMATRIX();
		_pSprite->GetTransform(pMat);
		if(mode==0){
			_pSprite->End();
			_pDevice->EndScene();
		}else if(mode==1){
			_pSprite->End();
		}
	}
	if(mode==1){
		D3DXMATRIX transform;
		if(systemIni.windowed)
			D3DXMatrixOrthoOffCenterRH(&transform, 0, WIDTH, HEIGHT, 0, -100, 100);
		else
			D3DXMatrixOrthoOffCenterRH(&transform, 0, WIDTH, (FLOAT)WIDTH/base::screen_rect.right*base::screen_rect.bottom, 0, -100, 100);
		_pDevice->SetTransform(D3DTS_PROJECTION, &transform);
		
		_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		_pDevice->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		/*
		_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_GAUSSIANQUAD);
		_pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);*/
	}
	drawMode = mode;
}

HRESULT GraphEngine::Present()
{
	SetDrawMode(0);
	return _pDevice->Present(NULL, NULL, 0, 0);
}


/*--------------------------------End Graph Engine Methods-------------------------------------------------------------
void GraphEngine::OnLostDevice()
{
	for(map<string, map<int, map<bool, map<DWORD, ID3DXFont*>>>>::iterator ptr1 = fonts.begin();ptr1!=fonts.end();ptr1++)
	{
		for(map<int, map<bool, map<DWORD, ID3DXFont*>>>::iterator ptr2 = ptr1->second.begin();ptr2!=ptr1->second.end();ptr2++)
		{
			for(map<bool, map<DWORD, ID3DXFont*>>::iterator ptr3 = ptr2->second.begin();ptr3!=ptr2->second.end();ptr3++)
			{
				for(map<DWORD, ID3DXFont*>::iterator ptr4 = ptr3->second.begin();ptr4!=ptr3->second.end();ptr4++)
					ptr4->second->OnLostDevice();
			}
		}
	}
}

void GraphEngine::OnResetDevice(IDirect3DDevice9* pDevice,ID3DXSprite* pSprite)
{
	_pDevice = pDevice;
	_pSprite = pSprite;
	for(map<string, map<int, map<bool, map<DWORD, ID3DXFont*>>>>::iterator ptr1 = fonts.begin();ptr1!=fonts.end();ptr1++)
	{
		for(map<int, map<bool, map<DWORD, ID3DXFont*>>>::iterator ptr2 = ptr1->second.begin();ptr2!=ptr1->second.end();ptr2++)
		{
			for(map<bool, map<DWORD, ID3DXFont*>>::iterator ptr3 = ptr2->second.begin();ptr3!=ptr2->second.end();ptr3++)
			{
				for(map<DWORD, ID3DXFont*>::iterator ptr4 = ptr3->second.begin();ptr4!=ptr3->second.end();ptr4++)
					ptr4->second->OnResetDevice();
			}
		}
	}
}
*/