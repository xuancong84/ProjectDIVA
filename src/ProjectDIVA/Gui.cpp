#include "Gui.h"
#include "Base.h"
#include "graphEngine.h"
#include "GameMana.h"
#include "EffectMana.h"
#include <iomanip>



//-------------------------Effect System----------------------------

//-------------GUIEF_Alpha----------
GUIEF_Alpha::GUIEF_Alpha(int *pAlpha, float speed, float destination,int delay)
{
	_pAlpha = pAlpha;
	_nowAlpha = (*pAlpha);
	_speed = speed;
	_dest = destination;
	_delay = delay;
	paused=false;
}

bool GUIEF_Alpha::OnWork()
{
	if(_delay)
	{
		_delay--;
		return true;
	}
	if(!paused)
		if(_nowAlpha<_dest)
		{
			_nowAlpha+=_speed;
			if(_nowAlpha>_dest)
				_nowAlpha=_dest;
		}
		else
		{
			_nowAlpha-=_speed;
			if(_nowAlpha<_dest)
				_nowAlpha=_dest;
		}
	(*_pAlpha) = int(_nowAlpha);
	return !(fabs(_nowAlpha-_dest)<1e-6);
}

//---------End GUIEF_Alpha----------


//--------------Spark---------------
Spark::Spark(int *pAlpha, float speed, bool startAtTop,int life, float startAlpha, float endAlpha, float minAlpha, float maxAlpha)
{
	_pAlpha=pAlpha;
	_speed=speed;
	isDowning = startAtTop;
	restlife = life;
	_nowAlpha = (*_pAlpha);
	_end = endAlpha;
	_minAlpha = minAlpha;
	_maxAlpha = maxAlpha;
	paused=false;
}
bool Spark::OnWork()
{
	if(restlife==0) 
	{
		_nowAlpha = _end;
		(*_pAlpha) = int(_end);
		return false;
	}
	if(!paused)
	{
		if(isDowning)
		{
			_nowAlpha -= _speed;
			if(_nowAlpha<=_minAlpha)
			{
				_nowAlpha=_minAlpha;
				isDowning = false;
			}
		}
		else
		{
			_nowAlpha += _speed;
			if(_nowAlpha>=_maxAlpha)
			{
				_nowAlpha=_maxAlpha;
				isDowning = true;
			}
		}
		restlife--;
		(*_pAlpha) = int(_nowAlpha);
	}
	return true;
}

//-----------End Spark---------------

//------------Move-------------------

GUIEF_Move::GUIEF_Move(RECT *drawRect, int maxLife, int desx, int desy, int delay)
{
	_drawRect = drawRect;
	_beginx = (*_drawRect).left;
	_beginy = (*_drawRect).top;
	_width = (*_drawRect).right-(*_drawRect).left;
	_height = (*_drawRect).bottom-(*_drawRect).top;
	_maxLife = maxLife;
	_nowLife = 0;
	_desx = desx;
	_desy = desy;
	_delay = delay;
	paused = false;
}

bool GUIEF_Move::OnWork()
{
	if(!paused)
	{
		if(_delay>0)
		{
			_delay--;
			return true;
		}
		_nowLife++;
		int deltax = int(float(_desx-_beginx)*float(_nowLife)/float(_maxLife)),
			deltay = int(float(_desy-_beginy)*float(_nowLife)/float(_maxLife));
		(*_drawRect) = MakeRect(_beginx+deltax,_beginy+deltay,_width,_height);
		if(_nowLife==_maxLife)
			return false;
	}
	return true;
}

//-----------End Move----------------

//-----------Begin Stretch-----------

GUIEF_Stretch::GUIEF_Stretch(RECT *pRect, int maxLife, RECT &desRect, int delay)
{
	_pRect = pRect;
	_maxLife = maxLife;
	_nowLife = 0;
	_beginRect = (*pRect);
	_desRect = desRect;
	_delay = delay;
	paused=false;
}

GUIEF_Stretch::GUIEF_Stretch(RECT *pRect, RECT &beginRect, RECT &finalRect, int maxLife, int delay)
{
	_pRect = pRect;
	_maxLife = maxLife;
	_nowLife = 0;
	_beginRect = beginRect;
	_desRect = finalRect;
	_delay = delay;
	paused = false;
}

bool GUIEF_Stretch::OnWork()
{
	if(!paused)
	{
		if(_delay>0)
		{
			_delay--;
			return true;
		}
		_nowLife++;
		float ratio = float(_nowLife) / float(_maxLife);
		SetRect(_pRect,_beginRect.left + int(float(_desRect.left - _beginRect.left) * ratio),
					_beginRect.top + int(float(_desRect.top - _beginRect.top) * ratio),
					_beginRect.right + int(float(_desRect.right - _beginRect.right) * ratio),
					_beginRect.bottom + int(float(_desRect.bottom - _beginRect.bottom) * ratio));
		if(_nowLife==_maxLife)
			return false;
	}
	return true;
}
//-----------End Stretch-------------

//-----------Effect Vector-----------

EffectVector::EffectVector()
{
	memset(effects,NULL,sizeof(effects));
	effectnums=0;paused=false;
	a=r=g=b=NULL;drawRect=srcRect=NULL;
}

EffectVector::~EffectVector()
{
	ReleaseEffects();
}

//-----------------------------
//增加指定的特效
//-----------------------------
void EffectVector::AddEffect(BaseEffect *effect)
{
	if(effectnums<MAXEFFECTNUM)
		effects[effectnums++]=effect;
	else
		SAFE_DELETE(effect);
}

//-----------------------------
//删除已经死亡的特效
//-----------------------------
void EffectVector::EffectDie(int id)
{
	delete effects[id];
	effects[id]=effects[effectnums-1];
	effects[--effectnums]=NULL;
}

void EffectVector::ReleaseEffects()
{
	for(int i=0;i<effectnums;i++)
		SAFE_DELETE(effects[i]);
	effectnums=0;
}

//-----------------------------
//特效工作
//-----------------------------
bool EffectVector::OnWork()
{
	for(int i=0;i<effectnums;i++)
		if(!effects[i]->OnWork())
		{
			EffectDie(i);
			i--;
		}
	return true;
}

//--End Effect Vector----------


//----------------------End Effect System---------------------------








void GUIControl::Register(GUIControl* src)
{
	panel.push_back(src);
	src->SetTab(panel.size());
}

GUIControl::GUIControl()
{
	visible = true;
}

void GUIControl::Draw(int Basex, int Basey)
{
	if(visible)
	{
		for(vector<GUIControl*>::iterator ptr = panel.begin();ptr!=panel.end();ptr++)
			(*ptr)->Draw(drawRect.left+Basex,drawRect.top+Basey);
	}
}

void GUIControl::Update()
{
	for(vector<GUIControl*>::iterator ptr = panel.begin();ptr!=panel.end();ptr++)
		(*ptr)->Update();
}

GUIControl::~GUIControl()
{
	for(vector<GUIControl*>::iterator ptr = panel.begin();ptr!=panel.end();ptr++)
		delete (*ptr);
	panel.clear();
}

void GUIControl::UnRegister(GUIControl* src)
{
	for(vector<GUIControl*>::iterator ptr = panel.begin();ptr!=panel.end();ptr++)
		if((*ptr)==src)
		{
			panel.erase(ptr);
			break;
		}
}

void GUIControl::OnKeyEvent(unsigned int nChar,bool bKeyDown)
{
	for(vector<GUIControl*>::iterator ptr = panel.begin();ptr!=panel.end();ptr++)
		(*ptr)->OnKeyEvent(KeyEventValue);
}

GUIStaticImage::GUIStaticImage(std::string filename, RECT _srcRect, RECT _drawRect,int texPool)
{
	drawTex = graphEngine.AddTexture(filename,texPool);
	srcRect = _srcRect;
	drawRect = _drawRect;
	modifyRect = drawRect; //default
	canGetFocus = false;
	focusOn = false;
	focus = 0;
	_a=_r=_g=_b=255;
	efvct.setPointer(&_a,&_r,&_g,&_b,&drawRect,&srcRect);
}

GUIStaticImage::GUIStaticImage(char *res_name)
{
	ImageResource *pRes = (ImageResource*) g_res.getResource(res_name);
	drawTex = pRes->GetTexture();
	srcRect = pRes->srcRect;
	drawRect = MakeRect(pRes->dstRect);
	modifyRect = drawRect; //default
	canGetFocus = false;
	focusOn = false;
	focus = 0;
	D3DCOLOR color = pRes->color;
	_b = color&0xff;
	_g = (color>>8)&0xff;
	_r = (color>>16)&0xff;
	_a = (color>>24)&0xff;
	efvct.setPointer(&_a,&_r,&_g,&_b,&drawRect,&srcRect);
}

void GUIStaticImage::Draw(int Basex,int Basey)
{
	if(visible)
	{
		graphEngine.Draw(drawTex,srcRect,MakefRect(drawRect.left+Basex,drawRect.top+Basey,drawRect.right-drawRect.left,drawRect.bottom-drawRect.top),D3DCOLOR_ARGB(_a,_r,_g,_b));
		GUIControl::Draw(Basex,Basey);
	}
}

void GUIStaticImage::Update()
{
	efvct.OnWork();
	GUIControl::Update();
}

void GUIStaticImage::SetColor(int a,int r,int g,int b)
{
	_a=a;_r=r;_g=g;_b=b;
}




GUIVertStretchBlock::GUIVertStretchBlock(string lSideFilename, RECT lSideSrcRect, 
										 string rSideFilename, RECT rSideSrcRect, 
										 string midFilename, RECT midSrcRect, RECT drawRect, int texPool)
{
	lSide = new GUIStaticImage(lSideFilename, lSideSrcRect, MakeRect(0,0,0,0), texPool);
	rSide = new GUIStaticImage(rSideFilename, rSideSrcRect, MakeRect(0,0,0,0), texPool);
	middle = new GUIStaticImage(midFilename, midSrcRect, MakeRect(0,0,0,0), texPool);
	SetDrawRect(drawRect);

	modifyRect = drawRect; //default
	canGetFocus = false;
	focusOn = false;
	focus = 0;
}

void GUIVertStretchBlock::SetDrawRect(RECT drawRect)
{
	_drawRect = drawRect;
	lSide->drawRect = MakeRect(_drawRect.left,_drawRect.top,
		float(_drawRect.bottom - _drawRect.top) / float((*lSide->efvct.srcRect).bottom-(*lSide->efvct.srcRect).top)
		* float((*lSide->efvct.srcRect).right-(*lSide->efvct.srcRect).left), _drawRect.bottom - _drawRect.top);

	int Rwidth = int(float(_drawRect.bottom - _drawRect.top) / float((*rSide->efvct.srcRect).bottom-(*rSide->efvct.srcRect).top)
		* float((*rSide->efvct.srcRect).right-(*rSide->efvct.srcRect).left));
	rSide->drawRect = MakeRect(_drawRect.right-Rwidth,_drawRect.top,
		Rwidth, _drawRect.bottom - _drawRect.top);

	SetRect(&middle->drawRect, lSide->drawRect.right, _drawRect.top, rSide->drawRect.left, _drawRect.bottom);

}

RECT GUIVertStretchBlock::GetDrawRect()
{
	return _drawRect;
}

void GUIVertStretchBlock::Draw(int Basex, int Basey)
{
	lSide->Draw(Basex,Basey);
	rSide->Draw(Basex,Basey);
	middle->Draw(Basex,Basey);
}

void GUIVertStretchBlock::Update()
{
	lSide->Update();
	rSide->Update();
	middle->Update();
}

void GUIVertStretchBlock::SetColor(int a,int r,int g,int b)
{
	lSide->SetColor(a,r,g,b);
	rSide->SetColor(a,r,g,b);
	middle->SetColor(a,r,g,b);
}



NormalButton::NormalButton(RECT pos, string texNormal, RECT srcRectNormal, string texHighLight, RECT srcRectHighLight, 
						   string texDown, RECT srcRectDown, string texDisable, RECT srcRectDisable,int texPool)
{
	RECT tdrawRect = MakeRect(0,0,pos.right-pos.left,pos.bottom-pos.top);
	normal = new GUIStaticImage(texNormal,srcRectNormal,tdrawRect,texPool);
	highLight = new GUIStaticImage(texHighLight,srcRectHighLight,tdrawRect,texPool);
	down = new GUIStaticImage(texDown,srcRectDown,tdrawRect,texPool);
	disable = new GUIStaticImage(texDisable,srcRectDisable,tdrawRect,texPool);
	btnState = BTNSTATE_NORMAL;
	drawRect = pos;
	modifyRect = drawRect;
	canGetFocus = true;
	focusOn = false;
	focus = 0;
	text_index = 40;

	p_ButtonClick = NULL;
	p_ButtonStateChanged = NULL;
}

NormalButton::NormalButton(RECT pos, string texNormal, RECT srcRectNormal, RECT drawRectNormal, 
							string texHighLight, RECT srcRectHighLight,RECT drawRectHighLight,
							string texDown,RECT srcRectDown,RECT drawRectDown,
							string texDisable,RECT srcRectDisable,RECT drawRectDisable,int texPool)
{
	normal = new GUIStaticImage(texNormal,srcRectNormal,drawRectNormal,texPool);
	highLight = new GUIStaticImage(texHighLight,srcRectHighLight,drawRectHighLight,texPool);
	down = new GUIStaticImage(texDown,srcRectDown,drawRectDown,texPool);
	disable = new GUIStaticImage(texDisable,srcRectDisable,drawRectDisable,texPool);
	btnState = BTNSTATE_NORMAL;
	drawRect = pos;
	modifyRect = drawRect;
	canGetFocus = true;
	focusOn = false;
	focus = 0;
	text_index = 40;

	p_ButtonClick = NULL;
	p_ButtonStateChanged = NULL;
}

NormalButton::~NormalButton()
{
	SAFE_DELETE(normal);
	SAFE_DELETE(highLight);
	SAFE_DELETE(down);
	SAFE_DELETE(disable);
}

void NormalButton::Draw(int Basex, int Basey)
{
	if(visible)
	{
		switch(btnState)
		{
		case BTNSTATE_NORMAL:
			normal->Draw(Basex+drawRect.left,Basey+drawRect.top);
			break;
		case BTNSTATE_HIGHLIGHT:
			highLight->Draw(Basex+drawRect.left,Basey+drawRect.top);
			break;
		case BTNSTATE_DOWN:
			down->Draw(Basex+drawRect.left,Basey+drawRect.top);
			break;
		case BTNSTATE_DISABLE:
			disable->Draw(Basex+drawRect.left,Basey+drawRect.top);
			break;
		}
		graphEngine.SetFont(systemIni.font,font.ftSize,font.ftItalic,font.ftWeight);
		// draw text option name
		graphEngine.DrawText(	text_string.empty()?
								StringTable((text_index>=0&&text_index<MAX_STRING_INDEX)?text_index:40)
								:text_string,
								MakeRect(	Basex+drawRect.left+ftDrawRect.left,
											Basey+drawRect.top+ftDrawRect.top,
											ftDrawRect.right-ftDrawRect.left,
											ftDrawRect.bottom-ftDrawRect.top	),
								DT_VCENTER | DT_NOCLIP,
								D3DXCOLOR(D3DCOLOR_ARGB(_a,_r,_g,_b)));
		if(!text2_string.empty())	// draw text option value
		{
			graphEngine.DrawText(	text2_string,
									MakeRect(	Basex+drawRect.left+ftDrawRect.left+125,
												Basey+drawRect.top+ftDrawRect.top,
												ftDrawRect.right-ftDrawRect.left,
												ftDrawRect.bottom-ftDrawRect.top	),
									DT_CENTER | DT_VCENTER | DT_NOCLIP,
									D3DXCOLOR(D3DCOLOR_ARGB(_a,_r,_g,_b)));
		}
		GUIControl::Draw(Basex,Basey);
	}
}

void NormalButton::Update()
{
	normal->Update();
	highLight->Update();
	down->Update();
	disable->Update();
	GUIControl::Update();
}

void NormalButton::Focus()
{
	if(btnState != BTNSTATE_DISABLE)
		SetButtonState(BTNSTATE_HIGHLIGHT);
}

void NormalButton::LostFocus()
{
	if(btnState != BTNSTATE_DISABLE)
		SetButtonState(BTNSTATE_NORMAL);
}

void NormalButton::SetButtonState(int _state)
{
	btnState = _state;
}

int NormalButton::GetButtonState()
{
	return btnState;
}

void NormalButton::SetColor(int a,int r,int g,int b)
{
	normal->SetColor(a,r,g,b);
	highLight->SetColor(a,r,g,b);
	down->SetColor(a,r,g,b);
	disable->SetColor(a,r,g,b);
	_a=a;_r=r;_g=g;_b=b;
}

void NormalButton::SetModifyRect(RECT _modifyRect)
{
	modifyRect = _modifyRect;
}

void NormalButton::SetText(int _text_index,UserFont _font,RECT _ftDrawRect)
{
	text_index = _text_index;
	font = _font;
	ftDrawRect = _ftDrawRect;
}

void NormalButton::SetTextString(string _text_string,UserFont _font,RECT _ftDrawRect)
{
	text_string = _text_string;
	font = _font;
	ftDrawRect = _ftDrawRect;
}

NormalButtonGroup::NormalButtonGroup(RECT pos)
{
	drawRect = pos;
	visible = true;
	canGetFocus = true;
	focus = -1;
	focusOn = false;
	p_OnButtonClick = NULL;
	p_OnSelectedIndexChanged = NULL;
	p_EscClick = NULL;
	p_OnLeftClick = NULL;
	p_OnRightClick = NULL;
	color = D3DCOLOR_ARGB(255,255,255,255);
}

void NormalButtonGroup::AddButton(NormalButton *src)
{
	GUIControl::Register((GUIControl*)src);
	//if(panel.size()==1)
		//panel[0]->Focus();
}

NormalButton* NormalButtonGroup::GetButton(int id)
{
	if(id>=panel.size())
		return (NormalButton*)panel[panel.size()-1];
	else if(id<0)
		return (NormalButton*)panel[0];
	else
		return (NormalButton*)panel[id];
}

void NormalButtonGroup::Clear()
{
	for(vector<GUIControl*>::iterator ptr = panel.begin();ptr!=panel.end();ptr++)
		delete (*ptr);
	panel.clear();
	focus = -1;
}


void NormalButtonGroup::OnKeyEvent(unsigned int nChar, bool bKeyDown)
{
	if(focusOn&&bKeyDown)
	{
		if(focus==-1)
			return;
		int lastfocus=focus;
		panel[focus]->LostFocus();
		int amount=0;
		switch(nChar){
		case VK_PRIOR:
			amount=-11;
		case KeyLeft:
			amount--;
			if(!p_OnLeftClick)
			{
				int tcount=0,tlast=focus;
				while(tcount!=3&&focus>0)
					if(GetButton(--focus)->GetButtonState()!=BTNSTATE_DISABLE)
					{
						tcount++;
						tlast=focus;
					}
				focus=tlast;
			}
			else
			{
				p_OnLeftClick(focus,amount);
			}
			break;
		case VK_NEXT:
			amount=11;
		case KeyRight:
			amount++;
			if(!p_OnRightClick)
			{
				int tcount=0,tlast=focus;
				while(tcount!=3&&focus<panel.size()-1)
					if(GetButton(++focus)->GetButtonState()!=BTNSTATE_DISABLE)
					{
						tcount++;
						tlast=focus;
					}
				focus=tlast;
			}
			else
			{
				p_OnRightClick(focus,amount);
			}
			break;
		case KeyUp:
		{
			int tcount=0,tlast=focus;
			while(tcount!=1)
			{
				if(focus==0)
					focus=panel.size();
				if(GetButton(--focus)->GetButtonState()!=BTNSTATE_DISABLE)
				{
					tcount++;
					tlast=focus;
				}
			}
			focus=tlast;
			break;
		}
		case KeyDown:
		{
			int tcount=0,tlast=focus;
			while(tcount!=1)
			{
				if(focus==panel.size()-1)
					focus=-1;
				if(GetButton(++focus)->GetButtonState()!=BTNSTATE_DISABLE)
				{
					tcount++;
					tlast=focus;
				}
			}
			focus=tlast;
			break;
		}
		}
		panel[focus]->Focus();
		if(lastfocus!=focus&&p_OnSelectedIndexChanged!=NULL)
			p_OnSelectedIndexChanged(focus);

		if(p_OnButtonClick!=NULL)
		{
			if(nChar==KeyEnter||nChar==KeySpace)
				p_OnButtonClick(focus);
			else if(nChar==(unsigned int)(char('P')))
				p_OnButtonClick(-1);
			else if(nChar==(unsigned int)(char('L')))
				p_OnButtonClick(-2);
		}

		if(nChar==KeyEsc)
			if(p_EscClick!=NULL)
				p_EscClick();
	}
}

void NormalButtonGroup::SetFocusOn(int id)
{
	if(focus!=-1)
		panel[focus]->LostFocus();
	focus=id;
	if(focus!=-1)
		panel[focus]->Focus();
	if(p_OnSelectedIndexChanged!=NULL)
		p_OnSelectedIndexChanged(focus);
}


extern GUIMessageBox *lock_msgbox;

ListBox::ListBox(RECT drawRect,string noItem)
{
	btns = new NormalButtonGroup(MakeRect(0,0,0,0));
	highLightExtra = new GUIStaticImage("pic\\menu_basic04.png",MakeRect(0,63,206,31),MakeRect(0,-7,206,31));
	highLightExtra->efvct.AddEffect(new Spark(highLightExtra->efvct.a,7.0,true,-1,255.0,255.0));

	up = new NormalButton(MakeRect(5,73,7,4),"pic\\menu_basic04.png",MakeRect(214,0,12,10),MakeRect(-2,-3,12,10),
							"pic\\menu_basic04.png",MakeRect(214,0,12,10),MakeRect(-2,-3,12,10),
							"pic\\menu_basic04.png",MakeRect(214,0,12,10),MakeRect(-2,-3,12,10),
							"pic\\menu_basic04.png",MakeRect(239,0,7,4),MakeRect(0,0,7,4));

	down = new NormalButton(MakeRect(5,223,7,4),"pic\\menu_basic04.png",MakeRect(226,0,13,10),MakeRect(-3,-3,13,10),
							"pic\\menu_basic04.png",MakeRect(226,0,13,10),MakeRect(-3,-3,13,10),
							"pic\\menu_basic04.png",MakeRect(226,0,13,10),MakeRect(-3,-3,13,10),
							"pic\\menu_basic04.png",MakeRect(239,4,7,4),MakeRect(0,0,7,4));

	title = new GUIStaticImage("listbox_title");
	text_index = 40;

	_drawRect = drawRect;
	_noItem = noItem;
	btns->Focus();

	GetInOver=NULL;
	GetOutOver=NULL;
	_getin=_getout=0;
	sleep = false;
	bRefresh = false;
}

ListBox::~ListBox()
{
	for(int i=0;i<btns->GetButtonNum();i++)
		btns->GetButton(i)->UnRegister((GUIControl*) highLightExtra);
	SAFE_DELETE(btns);
	SAFE_DELETE(highLightExtra);
	SAFE_DELETE(up);
	SAFE_DELETE(down);
	SAFE_DELETE(title);
}
extern MainMenu *menu;
extern void menu_songlb_EscClick();
extern void menu_songlb_OnButtonClick(int);
extern void menu_songlb_GetInOver();
extern void menu_songlb_OnSelectedIndexChanged(int);
extern void pausedMenu_OnMenuSelectBegin(int);
int RefreshSongList()
{
	pausedMenu_OnMenuSelectBegin(0);
	auto old_songlb = menu->songlb;
	menu->songlb = new ListBox(MakeRect(0,0,0,0),StringTable(18));
	menu->InitSongLb();
	menu->songlb->text_index = 19;
	menu->songlb->btns->p_EscClick = menu_songlb_EscClick;
	menu->songlb->btns->p_OnButtonClick = menu_songlb_OnButtonClick;
	menu->songlb->GetInOver = menu_songlb_GetInOver;
	menu->songlb->btns->p_OnSelectedIndexChanged = menu_songlb_OnSelectedIndexChanged;
	delete old_songlb;
	return 0;
}
void ListBox::OnKeyEvent(unsigned int nChar, bool bKeyDown)
{
	if(_getin||_getout||sleep)
		return;
	if(this==menu->songlb && nChar==VK_F5){
		bRefresh=true;
		return;
	}
	
	int tfocus = btns->GetSelectedIndex();
	btns->OnKeyEvent(KeyEventValue);
	if(bRefresh)
		return;
	int tfocus2 = btns->GetSelectedIndex();
	if(tfocus!=tfocus2)
	{
		btns->GetButton(tfocus)->UnRegister((GUIControl*) highLightExtra);
		ReplaceItem(tfocus2);
	}
}

void ListBox::GetIn()
{
	title->drawRect.left = -164;
	title->drawRect.right = 0;
	title->efvct.AddEffect(new GUIEF_Move(title->efvct.drawRect,10,0,title->drawRect.top,15));
	_getin = 30;
	int tf = btns->GetSelectedIndex();
	for(int i=tf-3;i<=tf+3;i++)
		if(i>=0&&i<btns->GetButtonNum())
		{
			btns->GetButton(i)->drawRect.left = -204;
			btns->GetButton(i)->normal->efvct.ReleaseEffects();
			btns->GetButton(i)->normal->efvct.AddEffect(new GUIEF_Move(&btns->GetButton(i)->drawRect,10,0,btns->GetButton(i)->drawRect.top,
				abs(tf-i)*8+(i==tf?30:46)));
			_getin = max(_getin,abs(tf-i)*8+(i==tf?30:46)+10);
		}
}

void ListBox::GetOut()
{
	_getout = 0;
	int tf = btns->GetSelectedIndex();
	for(int i=tf-3;i<=tf+3;i++)
		if(i>=0&&i<btns->GetButtonNum())
		{
			btns->GetButton(i)->drawRect.left = 0;
			btns->GetButton(i)->normal->efvct.ReleaseEffects();
			btns->GetButton(i)->normal->efvct.AddEffect(new GUIEF_Move(&btns->GetButton(i)->drawRect,10,-204,btns->GetButton(i)->drawRect.top,
				abs(tf-i)*8+(i==tf?0:16)));
			_getout = max(_getout,abs(tf-i)*8+(i==tf?0:16)+10);
		}
	title->drawRect.left = 0;
	title->drawRect.right = 164;
	title->efvct.AddEffect(new GUIEF_Move(title->efvct.drawRect,10,-164,title->drawRect.top,_getout+5));
	_getout+=15;
}

void ListBox::Draw()
{
	title->Draw();
	graphEngine.SetFont(systemIni.font,13,false,FW_BOLD);
	graphEngine.DrawText(StringTable(text_index),title->drawRect.left+8,title->drawRect.top+3);
	btns->Draw();
	up->Draw();
	down->Draw();

	if(lock_msgbox)
		lock_msgbox->Draw();
}

void ListBox::Update()
{
	if(lock_msgbox)
		lock_msgbox->Update();

	if(sleep)
		return;
	btns->Update();
	title->Update();
	if(_getin)
	{
		_getin--;
		if(!_getin&&GetInOver)
			GetInOver();
	}
	if(_getout)
	{
		_getout--;
		if(!_getout&&GetOutOver)
			GetOutOver();
	}
	if(btns->drawRect.top != btnsYDes)
	{
		if(btns->drawRect.top > btnsYDes)
		{
			btns->drawRect.top -= (btns->drawRect.top - btnsYDes)/3;
			if(btns->drawRect.top - btnsYDes <= 3)
				btns->drawRect.top = btnsYDes;
		}
		else if(btns->drawRect.top < btnsYDes)
		{
			btns->drawRect.top += (btnsYDes - btns->drawRect.top)/3;
			if(btnsYDes-btns->drawRect.top <=3)
				btns->drawRect.top = btnsYDes;
		}
	}
	for(int i=0;i<btns->GetButtonNum();i++)
	{
		int ttop = btns->GetButton(i)->drawRect.top + btns->drawRect.top;
		if(ttop<224&&ttop>60)
			btns->GetButton(i)->SetColor(float(82-abs(ttop-142))/82.0*255.0,255,255,255);
		else
			btns->GetButton(i)->SetColor(0,255,255,255);
	}
	up->SetButtonState(BTNSTATE_NORMAL);
	down->SetButtonState(BTNSTATE_NORMAL);
	up->Update();
	down->Update();
	if(btns->GetSelectedIndex()==0)
		up->SetButtonState(BTNSTATE_DISABLE);
	if(btns->GetSelectedIndex()==btns->GetButtonNum()-1)
		down->SetButtonState(BTNSTATE_DISABLE);
}

void ListBox::AddItem(int string_index,int style)
{
	if(style==0)
	{
		btns->AddButton(new NormalButton(MakeRect(0,0,204,17),
					"pic\\menu_basic.png",MakeRect(0,17,204,17),MakeRect(0,0,204,17),
					"pic\\menu_basic04.png",MakeRect(0,97,206,31),MakeRect(0,-7,206,31),
					"pic\\menu_basic.png",MakeRect(0,17,204,17),MakeRect(0,0,0,0),
					"pic\\menu_basic.png",MakeRect(0,17,204,17),MakeRect(0,0,0,0)));
	}
	else
	{
		btns->AddButton(new NormalButton(MakeRect(0,0,204,17),
					"pic\\menu_basic.png",MakeRect(0,34,204,17),MakeRect(0,0,204,17),
					"pic\\menu_basic04.png",MakeRect(0,31,206,31),MakeRect(0,-7,206,31),
					"pic\\menu_basic.png",MakeRect(0,34,204,17),MakeRect(0,0,0,0),
					"pic\\menu_basic.png",MakeRect(0,34,204,17),MakeRect(0,0,0,0)));
	}
	btns->GetButton(btns->GetButtonNum()-1)->SetText(string_index,UserFont(systemIni.font,12,false,FW_BOLD),MakeRect(16,8,0,0));
}

void ListBox::AddItemString(string text,int style)
{
	if(style==0)
	{
		btns->AddButton(new NormalButton(MakeRect(0,0,204,17),
					"pic\\menu_basic.png",MakeRect(0,17,204,17),MakeRect(0,0,204,17),
					"pic\\menu_basic04.png",MakeRect(0,97,206,31),MakeRect(0,-7,206,31),
					"pic\\menu_basic.png",MakeRect(0,17,204,17),MakeRect(0,0,0,0),
					"pic\\menu_basic.png",MakeRect(0,17,204,17),MakeRect(0,0,0,0)));
	}
	else
	{
		btns->AddButton(new NormalButton(MakeRect(0,0,204,17),
					"pic\\menu_basic.png",MakeRect(0,34,204,17),MakeRect(0,0,204,17),
					"pic\\menu_basic04.png",MakeRect(0,31,206,31),MakeRect(0,-7,206,31),
					"pic\\menu_basic.png",MakeRect(0,34,204,17),MakeRect(0,0,0,0),
					"pic\\menu_basic.png",MakeRect(0,34,204,17),MakeRect(0,0,0,0)));
	}
	btns->GetButton(btns->GetButtonNum()-1)->SetTextString(text,UserFont(systemIni.font,12,false,FW_BOLD),MakeRect(16,8,0,0));
}

void ListBox::ReplaceItem(int tfocus,bool force)
{
	if(btns->GetButtonNum()<=0) return;
	int ttop=0;
	for(int i=0;i<btns->GetButtonNum();i++)
	{
		btns->GetButton(i)->drawRect.top = ttop;
		btns->GetButton(i)->drawRect.left = 0;
		if(i==tfocus)
		{
			btnsYDes = 142-ttop;
			ttop+=22;
		}
		else if(i==tfocus-1)
			ttop+=22;
		else
			ttop+=20;
	}
	btns->SetFocusOn(tfocus);
	btns->GetButton(tfocus)->Register((GUIControl*) highLightExtra);
	if(force)
		btns->drawRect = MakeRect(0,btnsYDes,0,0);
}

void ListBox::BtnSleep()
{
	for(int i=btns->GetSelectedIndex()-3;i<btns->GetSelectedIndex()+3;i++)
		if(i>=0&&i<btns->GetButtonNum()&&i!=btns->GetSelectedIndex())
			btns->GetButton(i)->SetColor(100,255,255,255);
	btns->GetButton(btns->GetSelectedIndex())->SetButtonState(BTNSTATE_NORMAL);
	btns->GetButton(btns->GetSelectedIndex())->UnRegister( (GUIControl*) highLightExtra);
	sleep=true;
}

void ListBox::BtnWakeUp()
{
	btns->GetButton(btns->GetSelectedIndex())->SetButtonState(BTNSTATE_HIGHLIGHT);
	btns->GetButton(btns->GetSelectedIndex())->Register( (GUIControl*) highLightExtra);
	sleep=false;
}

SplashScreen::SplashScreen(GUIStaticImage *logo, int delay)
{
	_logo = logo;
	_maxDelay = delay;
	_delayBuffer = 0;
	InitOver = NULL;
	SplashOut = NULL;
	_begin = false;
	isloading = false;
	_nowLoading = NULL;
}

SplashScreen::~SplashScreen()
{
	SAFE_DELETE(_logo);
	SAFE_DELETE(_nowLoading);
}

void SplashScreen::Begin()
{
	_delayBuffer = 0;
	if(isloading)
	{
		graphEngine.ReleaseTex(5);
		SAFE_DELETE(_logo);
		string tFilename = cgFileInfo._cgFilename[rand()%cgFileInfo._cgFilename.size()];
		_logo = new GUIStaticImage(tFilename,MakeRect(0,0,480,272),MakeRect(0,0,480,272),5);

		SAFE_DELETE(_nowLoading);
		_nowLoading = new GUIStaticImage("pic\\loading.png",MakeRect(0,0,11*16,16),MakeRect(295,241,11*16,16),5);
	}
	_logo->SetColor(0,255,255,255);
	_logo->efvct.ReleaseEffects();
	_logo->efvct.AddEffect((BaseEffect*)new GUIEF_Alpha(_logo->efvct.a,5,255));
	if(_nowLoading)
	{
		_nowLoading->SetColor(0,255,255,255);
		_nowLoading->efvct.ReleaseEffects();
		_nowLoading->efvct.AddEffect((BaseEffect*)new GUIEF_Alpha(_nowLoading->efvct.a,5,255));
	}
	_begin = true;
}

void SplashScreen::End()
{
	_delayBuffer = _maxDelay - 10;
}

void SplashScreen::Draw()
{
	if(_begin)
	{
		if(_logo)
			_logo->Draw(0,0);
		if(_nowLoading)
			_nowLoading->Draw();
	}
}

void SplashScreen::Update()
{
	if(_begin)
		if(_logo)
		{
			_logo->Update();
			if(_nowLoading)
				_nowLoading->Update();
			if(_delayBuffer==0)
			{
				if((*_logo->efvct.a)==255)
				{
					if(InitOver)
						InitOver();
					_delayBuffer++;
				}
			}
			else
			{
				if(_delayBuffer<_maxDelay)
				{
					_delayBuffer++;
					if(_delayBuffer==_maxDelay)
					{
						_logo->efvct.ReleaseEffects();
						_logo->efvct.AddEffect((BaseEffect*)new GUIEF_Alpha(_logo->efvct.a,5,0));
						if(_nowLoading)
						{
							_nowLoading->efvct.ReleaseEffects();
							_nowLoading->efvct.AddEffect((BaseEffect*)new GUIEF_Alpha(_nowLoading->efvct.a,5,0));
						}
					}
				}
				else
					if((*_logo->efvct.a)==0)
					{
						if(SplashOut)
							SplashOut();
						_begin=false;
					}
			}
		}
}

MainMenu::MainMenu()
{
	menu_back_win	= (ImageResource*)g_res.getResource("songmenu_back_win");
	menu_back_fail	= (ImageResource*)g_res.getResource("songmenu_back_fail");
	main_menu_top1	= (ImageResource*)g_res.getResource("main_menu_top1");
	main_menu_top2	= (ImageResource*)g_res.getResource("main_menu_top2");

	title	= new GUIStaticImage("main_menu_title");
	bottom	= new BottomBar();
	songlb	= new ListBox(MakeRect(0,0,0,0),StringTable(18));
	InitSongLb();
	songlb->text_index = 19;

	// build main menu
	back = new MainMenuBack();
	mainDetail = new GUIStaticImage("main_menu_detail");
	mainlb = new ListBox(MakeRect(0,0,0,0),"FUCKSHIT");
	mainlb->AddItem((songlb->btns->GetButtonNum()<=0)?20:19);
	mainlb->AddItem(134);
	mainlb->AddItem(114);
	mainlb->AddItem(21);
	mainlb->AddItem(22);
	mainlb->AddItem(23);
	mainlb->AddItem(24);
	mainlb->AddItem(25);
	mainlb->AddItem(26);
	mainlb->ReplaceItem(0,true);
	mainlb->GetIn();
	mainlb->text_index = 27;
	(*mainDetail->efvct.a) = 0;
	if(songlb->btns->GetButtonNum()<=0)
		mainDetailWords_index = 3;
	else
		mainDetailWords_index = 4;
	MainMenuState = MAINST_MAIN;

	// build config menu
	configlb = new ListBox(MakeRect(0,0,0,0),"I love miku");
	configlb->text_index = 23;
	for (int x = 0; x < options.size(); ++x)
		if (options[x].string_table_id){
			configlb->AddItem(abs(options[x].string_table_id), options[x].string_table_id > 0 ? 1 : 0);
			cfg_menu_option_id.push_back(x);
		}
	configlb->ReplaceItem(0,0);

	cgmenu = new CGMenu();
	cgmenu->PrepareCG();

	songPicBack = new GUIStaticImage("song_pic_back");
	(*songPicBack->efvct.a) = 0;
	blackBack = new GUIStaticImage("black_back");
	(*blackBack->efvct.a) = 0;

	selectSong = new SelectSong();
	selectSong->alphaHook = blackBack->efvct.a;

	scoreMenu = new ScoreMenu();

	lastSongSelect = -1;
	songOverviewPic = NULL;
}

MainMenu::~MainMenu()
{
	SAFE_DELETE(title);
	SAFE_DELETE(bottom);
	SAFE_DELETE(back);
	SAFE_DELETE(mainDetail);
	SAFE_DELETE(mainlb);
	SAFE_DELETE(configlb);
	SAFE_DELETE(songPicBack);
	SAFE_DELETE(blackBack);
	SAFE_DELETE(songlb);
	SAFE_DELETE(songOverviewPic);
	SAFE_DELETE(selectSong);
	SAFE_DELETE(cgmenu);
	SAFE_DELETE(scoreMenu);
}

void MainMenu::Draw()
{
	back->Draw();
	
	if(MainMenuState==MAINST_SCORE)
	{
		graphEngine.Draw(scoreMenu->ifClear?menu_back_win:menu_back_fail);
		scoreMenu->Draw();
		return;
	}

	if(MainMenuState==MAINST_FREE || MainMenuState==MAINST_SELECTSONG)	// draw singers
	{
		if(songlb->btns->GetButtonNum()>0)
		{
			// obtain matchable singer name list
			istringstream iss(songInfo.songs[songlb->btns->GetSelectedIndex()].singer);
			string char_name;
			vector <ImageResource*> singers;
			float width = 0;
			while(iss>>char_name)
			{
				ImageResource *singer = (ImageResource*)g_res.getResource(char_name);
				if(singer)
				{
					singers.push_back(singer);
					width += (singer->dstRect.right-singer->dstRect.left);
				}
			}

			width *= 0.25f;
			for(int x=singers.size()-1; x>=0; x--)	// draw each character
			{
				ImageResource *singer = singers[x];
				width -= (singer->dstRect.right-singer->dstRect.left)*0.25f;
				graphEngine.DrawCenter(singer,WIDTH*0.5f+width,HEIGHT*0.5f);
				width -= (singer->dstRect.right-singer->dstRect.left)*0.25f;
			}
		}
	}

	graphEngine.Draw(main_menu_top1);
	graphEngine.Draw(main_menu_top2);
	title->Draw();

	bottom->Draw();

	mainDetail->Draw();

	blackBack->Draw();
	songPicBack->Draw();
	if(songOverviewPic)
		songOverviewPic->Draw();
	if(MainMenuState==MAINST_MAIN)
	{
		mainlb->Draw();
		graphEngine.SetFont(systemIni.font,12);
		graphEngine.DrawText(StringTable(mainDetailWords_index),307,105,D3DXCOLOR(D3DCOLOR_ARGB((*mainDetail->efvct.a),255,255,255)));
	}
	else if(MainMenuState==MAINST_FREE||MainMenuState==MAINST_SELECTSONG)
	{
		songlb->Draw();
		if(MainMenuState==MAINST_FREE)
		{
			if(songlb->btns->GetButtonNum()>0)
				if(songInfo.songs[songlb->btns->GetSelectedIndex()].BPM != 0)
			{
				graphEngine.SetFont(systemIni.font,12);
				graphEngine.DrawText("BPM",MakeRect(0,123,405,13),DT_RIGHT | DT_NOCLIP,D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)));

				ostringstream oss;
				oss << setprecision(2) << songInfo.songs[songlb->btns->GetSelectedIndex()].BPM;
				graphEngine.DrawText(oss.str(), 410, 123, D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)));
				//graphEngine.DrawText(410,123,D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)),"%.2lf",songInfo.songs[songlb->btns->GetSelectedIndex()].BPM);

				graphEngine.DrawText("Noter",MakeRect(0,145,335,13),DT_RIGHT | DT_NOCLIP,D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)));
				graphEngine.DrawText(songInfo.songs[songlb->btns->GetSelectedIndex()].noter,MakeRect(340,145,0,13),D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)));

				graphEngine.DrawText(StringTable(28),MakeRect(0,158,335,13),DT_RIGHT | DT_NOCLIP,D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)));
				graphEngine.DrawText(songInfo.songs[songlb->btns->GetSelectedIndex()].musician,MakeRect(340,158,0,13),D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)));

				// difficulty levels
				for(int x=0; x<5; x++){
					graphEngine.DrawText(StringTable(29+x),MakeRect(0,171+x*13,335,13),DT_RIGHT | DT_NOCLIP,D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)));
					graphEngine.DrawText(songInfo.songs[songlb->btns->GetSelectedIndex()].hardText[x],MakeRect(340,171+x*13,0,13),D3DXCOLOR(D3DCOLOR_ARGB((*blackBack->efvct.a),255,255,255)));
				}
			}
		}
		else
		{
			selectSong->Draw();
		}
	}
	else if(MainMenuState==MAINST_CG)
	{
		cgmenu->Draw();
	}
	else if(MainMenuState==MAINST_CONFIG)
	{
		graphEngine.SetFont(systemIni.font,12);
		bool bLongList = split(configDetailWords_string,"\n").size()>8;
		graphEngine.DrawText(configDetailWords_string.empty()?
			StringTable(configDetailWords_index):configDetailWords_string,
			306, bLongList>10?63:85, D3DXCOLOR(D3DCOLOR_ARGB((*mainDetail->efvct.a),255,255,255)));
		configlb->Draw();
		if(bLongList)
			graphEngine.DrawText("PageUP\nPageDown", 256, 148, DT_CENTER|DT_VCENTER|DT_NOCLIP);
		if(GetKeyState(VK_CAPITAL)&1)
			if(configlb->btns->GetButton(configlb->btns->GetSelectedIndex())->text_index==80)
			{
				GAMEDATA.combo_draw=GAMEDATA.combo=100;
				GAMEDATA.hp=GAMEDATA.hp/2;
				gameui.Draw(GAME_MODE_NORMAL);
			}
	}
}

void MainMenu::Update()
{
	back->Update();
	if(MainMenuState==MAINST_SCORE)
	{
		scoreMenu->Update();
		return;
	}
	title->Update();

	bottom->Update();

	mainDetail->Update();
	blackBack->Update();
	songPicBack->Update();
	if(MainMenuState==MAINST_MAIN)
		mainlb->Update();
	else if(MainMenuState==MAINST_FREE||MainMenuState==MAINST_SELECTSONG)
	{
		songlb->Update();
		if(MainMenuState==MAINST_SELECTSONG)
			selectSong->Update();
		if(songlb->btns->GetSelectedIndex()!=lastSongSelect)
		{
			lastSongSelect = songlb->btns->GetSelectedIndex();
			SAFE_DELETE(songOverviewPic);
			string tfilename = songInfo.songs[lastSongSelect].songFNameBase + songInfo.songs[lastSongSelect].overviewPic;
			if(isImage(tfilename))
				songOverviewPic = new GUIStaticImage(tfilename,MakeRect(0,0,128,64),MakeRect(320,63,112,56));
			else
				songOverviewPic = new GUIStaticImage("pic\\noImg.png",MakeRect(0,0,128,64),MakeRect(320,63,112,56));
			songOverviewPic->efvct.AddEffect(new GUIEF_Stretch(songOverviewPic->efvct.drawRect, 5,MakeRect(312,59,128,64)));
		}
		if(songOverviewPic)
		{
			songOverviewPic->Update();
			songOverviewPic->SetColor((*songPicBack->efvct.a),255,255,255);
		}
	}
	else if(MainMenuState==MAINST_CG)
	{
		cgmenu->Update();
	}
	else if(MainMenuState==MAINST_CONFIG)
	{
		configlb->Update();
	}
}


void MainMenu::OnKeyEvent(unsigned int nChar, bool bKeyDown)
{
	if(MainMenuState==MAINST_MAIN)
	{
		mainlb->OnKeyEvent(KeyEventValue);
	}
	else if(MainMenuState==MAINST_FREE)
	{
		songlb->OnKeyEvent(KeyEventValue);
		if(songlb->bRefresh)
			RefreshSongList();
	}
	else if(MainMenuState==MAINST_SELECTSONG)
	{
		selectSong->OnKeyEvent(KeyEventValue);
	}
	else if(MainMenuState==MAINST_SCORE)
	{
		scoreMenu->OnKeyEvent(KeyEventValue);
	}
	else if(MainMenuState==MAINST_CG)
	{
		cgmenu->OnKeyEvent(KeyEventValue);
	}
	else if(MainMenuState==MAINST_CONFIG)
	{
		configlb->OnKeyEvent(KeyEventValue);
	}
}

void MainMenu::InitSongLb()
{
	songInfo.ReReadInfo();
	for(int i=0;i<songInfo.songs.size();i++)
		songlb->AddItemString(songInfo.songs[i].songName);
	songlb->ReplaceItem(0,0);
}

string SongInfoManager::songPath = DEFAULT_SONG_PATH;
string SongInfoManager::currPath = DEFAULT_SONG_PATH;
string add_slash(string fn)
{
	if(fn.length()>0){
		if(*fn.rbegin()!='\\' && *fn.rbegin()!='/')
			fn+="\\";
	}
	return fn;
}
void SongInfoManager::ReReadInfo()
{
	songs.clear();

	currPath = add_slash(currPath);
	songPath = add_slash(songPath);

	string basePath = currPath;
	string findPath = basePath+"*.*";

	int n_folders = 0;
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(findPath.c_str(),&wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		if(currPath!=songPath){
			songs.push_back(SongInfo(".."));
			++n_folders;
		}

		do
		{
			if(wfd.cFileName[0]=='.')
				continue;
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				string findPath_map = basePath+wfd.cFileName+"\\*.diva";
				string basePath_map = basePath+wfd.cFileName+"\\";
				WIN32_FIND_DATA wfdDiva;
				HANDLE hFindDiva = FindFirstFile(findPath_map.c_str(),&wfdDiva);
				if(hFindDiva==INVALID_HANDLE_VALUE){	// is a child folder
					songs.insert(songs.begin()+n_folders,SongInfo(wfd.cFileName));
					++n_folders;
				}else{									// is a map folder
					SongInfo tinfo;
					do
					{
						tinfo.songFNames.push_back(basePath_map+wfdDiva.cFileName);
					}
					while(FindNextFile(hFindDiva,&wfdDiva));
					tinfo.songFNameBase = basePath_map;
					tinfo.GetInfoFromFile();
					songs.push_back(tinfo);
					FindClose(hFindDiva);
				}
			}
		}
		while(FindNextFile(hFind,&wfd));
		FindClose(hFind);
	}
}

void SongInfo::GetInfoFromFile()
{
	memset(hard,-1,sizeof(hard));
	for(int i=0;i<5;i++)
		hardText[i]="N/A";
	for(int i=0;i<songFNames.size();i++)
	{
		FILE *fin = fopen(songFNames[i].c_str(),"r");
		char buffer[300];
		if(!fin)
			return;

		fgets(buffer,300,fin);
		checkTail0(buffer,300);
		//EditorVer

		fgets(buffer,300,fin);
		checkTail0(buffer,300);
		songName = Ansi2UTF8(buffer);

		fgets(buffer,300,fin);
		checkTail0(buffer,300);
		noter = Ansi2UTF8(buffer);

		fgets(buffer,300,fin);
		checkTail0(buffer,300);
		musician = Ansi2UTF8(buffer);

		fgets(buffer,300,fin);
		checkTail0(buffer,300);
		singer = Ansi2UTF8(buffer);

		fgets(buffer,300,fin);
		checkTail0(buffer,300);
		overviewPic = buffer;

		int ilevel;
		fscanf(fin,"%d\n",&ilevel);
		fscanf(fin,"%d\n%lf\n",&hard[ilevel-1],&BPM);
		songFName[ilevel-1]=songFNames[i];
		if(hard[ilevel-1]>10)
		{
			char buffer[256];
			itoa(hard[ilevel-1],buffer,10);
			hardText[ilevel-1]=buffer;
			hardText[ilevel-1]+=StringTable(34);
		}
		else
		{
			hardText[ilevel-1]="";
			for(int i=0;i<hard[ilevel-1];i++)
				hardText[ilevel-1]+=g_button_symbols[8];
			for(int i=hard[ilevel-1];i<10;i++)
				hardText[ilevel-1]+=g_button_symbols[9];
		}
		fclose(fin);
	}

	//highScore
	string scoreFile = songFNameBase + "result.inf";
	FILE* fin = fopen(scoreFile.c_str(),"r");
	if(!fin)
	{
		fin = fopen(scoreFile.c_str(),"w");
		fprintf(fin,"0 0 0 0\n0 0 0 0\n0 0 0 0\n0 0 0 0\n0 0 0 0\n");
		memset(highScore,0,sizeof(highScore));
		memset(maxCombo,0,sizeof(maxCombo));
		memset(highResult,0,sizeof(highResult));
		memset(clearNum,0,sizeof(clearNum));
		fclose(fin);
	}
	else
	{
		for(int i=0;i<5;i++)
			fscanf(fin,"%d %d %d %d\n",&highScore[i],&maxCombo[i],&highResult[i],&clearNum[i]);
		fclose(fin);
	}
}

void SongInfo::SaveInfoToFile()
{
	string scoreFile = songFNameBase + "result.inf";
	FILE* fin = fopen(scoreFile.c_str(),"w");
	for(int i=0;i<5;i++)
		fprintf(fin,"%d %d %d %d\n",highScore[i],maxCombo[i],highResult[i],clearNum[i]);
	fclose(fin);
}

void SongInfo::GetResult(int score,int combo,int result,int whichHard,bool clear)
{
	highScore[whichHard]=max(highScore[whichHard],score);
	maxCombo[whichHard]=max(maxCombo[whichHard],combo);
	highResult[whichHard]=max(highResult[whichHard],result);
	if(clear)
		clearNum[whichHard]++;
	SaveInfoToFile();
}

SelectSong::SelectSong()
{
	btns = new NormalButtonGroup(MakeRect(0,0,0,0));

	btns->AddButton(new NormalButton(MakeRect(300,143,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(201,229,59,31),MakeRect(-5,-2,59,31),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17)));

	btns->AddButton(new NormalButton(MakeRect(300,162,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(201,229,59,31),MakeRect(-5,-2,59,31),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17)));

	btns->AddButton(new NormalButton(MakeRect(300,181,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(201,229,59,31),MakeRect(-5,-2,59,31),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17)));

	btns->AddButton(new NormalButton(MakeRect(300,200,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(201,229,59,31),MakeRect(-5,-2,59,31),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17)));

	btns->AddButton(new NormalButton(MakeRect(300,219,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(201,229,59,31),MakeRect(-5,-2,59,31),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17),
					"pic\\menu_common.png",MakeRect(268,231,48,17),MakeRect(0,0,48,17)));

	btns->Focus();

	highLightExtra = new GUIStaticImage("pic\\menu_common.png",MakeRect(199,201,61,25),MakeRect(-6,-4,61,25));
	highLightExtra->efvct.AddEffect(new Spark(highLightExtra->efvct.a,7.0,true,-1,255.0,255.0));

	btns->SetFocusOn(-1);
	alphaHook = NULL;
	
	sleep=true;
}

SelectSong::~SelectSong()
{
	for(int i=0;i<btns->GetButtonNum();i++)
		btns->GetButton(i)->UnRegister((GUIControl*) highLightExtra);
	SAFE_DELETE(highLightExtra);
	SAFE_DELETE(btns);
}

void SelectSong::ResetButton(SongInfo &songInfo)
{
	nowSong=&songInfo;
	if(btns->GetSelectedIndex()>=0)
		btns->GetButton(btns->GetSelectedIndex())->UnRegister((GUIControl*) highLightExtra);
	bool flag=false;
	for(int i=0;i<5;i++)
		if(songInfo.hard[i]==-1)
			btns->GetButton(i)->SetButtonState(BTNSTATE_DISABLE);
		else
		{
			if(!flag)
			{
				btns->GetButton(i)->Register((GUIControl*) highLightExtra);
				btns->GetButton(i)->SetButtonState(BTNSTATE_HIGHLIGHT);
				btns->SetFocusOn(i);
				flag=true;
			}
			else
				btns->GetButton(i)->SetButtonState(BTNSTATE_NORMAL);
		}
	sleep=false;
}

void SelectSong::OnKeyEvent(unsigned int nChar,bool bKeyDown)
{
	if(sleep)
		return;
	int tfocus = btns->GetSelectedIndex();
	btns->OnKeyEvent(KeyEventValue);
	if(tfocus!=btns->GetSelectedIndex())
	{
		if(tfocus>=0)
			btns->GetButton(tfocus)->UnRegister((GUIControl*) highLightExtra);
		if(btns->GetSelectedIndex()>=0)
			btns->GetButton(btns->GetSelectedIndex())->Register((GUIControl*) highLightExtra);
	}
}

void SelectSong::Draw()
{
	btns->Draw();
	graphEngine.SetFont(systemIni.font,14);
	int talpha=255;
	if(alphaHook)
		talpha=(*alphaHook);
	for(int i=0;i<5;i++)
		if(btns->GetButton(i)->GetButtonState()!=BTNSTATE_DISABLE)
			graphEngine.DrawText(StringTable(HardType[i]),btns->GetButton(i)->drawRect,
				DT_NOCLIP | DT_CENTER | DT_VCENTER,D3DXCOLOR(D3DCOLOR_ARGB(talpha,255,255,255)));
		else
			graphEngine.DrawText(StringTable(HardType[i]),btns->GetButton(i)->drawRect,
				DT_NOCLIP | DT_CENTER | DT_VCENTER,D3DXCOLOR(D3DCOLOR_ARGB(talpha,100,100,100)));
	graphEngine.SetFont(systemIni.font,12);
	int ts=btns->GetSelectedIndex();
	if(ts!=-1){
		char buf[1024];
		sprintf(buf, StringTable(35).c_str(), StringTable(HardType[ts]).c_str(),
			nowSong->hard[ts], nowSong->highScore[ts], nowSong->maxCombo[ts],
			StringTable(SongResult[nowSong->highResult[ts]]).c_str(),nowSong->clearNum[ts]);
		graphEngine.DrawText(buf, MakeRect(356,150,0,0),DT_NOCLIP,D3DXCOLOR(D3DCOLOR_ARGB(talpha,255,255,255)));
	}
	/*	graphEngine.DrawText(MakeRect(356,150,0,0),DT_NOCLIP,D3DXCOLOR(D3DCOLOR_ARGB(talpha,255,255,255)),
			StringTable(35).c_str(),StringTable(HardType[ts]).c_str(),nowSong->hard[ts],nowSong->highScore[ts],
			nowSong->maxCombo[ts],StringTable(SongResult[nowSong->highResult[ts]]).c_str(),nowSong->clearNum[ts]);
			*/
}

void SelectSong::Update()
{
	btns->Update();
	int talpha=255;
	if(alphaHook)
		talpha=(*alphaHook);
	for(int i=0;i<5;i++)
		btns->GetButton(i)->SetColor(talpha,255,255,255);
}

void SelectSong::UnsetHighLight()
{
	int tfocus = btns->GetSelectedIndex();
	if(tfocus!=-1)
		btns->GetButton(tfocus)->UnRegister((GUIControl*) highLightExtra);
	sleep=true;
}

void SelectSong::ResetHighLight()
{
	int tfocus = btns->GetSelectedIndex();
	if(tfocus!=-1)
		btns->GetButton(tfocus)->Register((GUIControl*) highLightExtra);
	sleep=false;
}
char *score_names[NUMSCOREROWS]={
	"score_scorecnt[10]",
	"score_coolcnt[10]",
	"score_finecnt[10]",
	"score_safecnt[10]",
	"score_sadcnt[10]",
	"score_worstcnt[10]",
	"score_combocnt[10]",
	"score_chancecnt[10]",
};
ScoreMenu::ScoreMenu()
{
	up = new GUIStaticImage("pic\\menu_result.png",MakeRect(0,0,480,21),MakeRect(0,0,480,21));
	down = new GUIStaticImage("pic\\menu_result.png",MakeRect(0,44,480,25),MakeRect(0,247,480,25));
	scoreBackWin = new GUIStaticImage("pic\\result_win_nm.png",MakeRect(0,0,240,212),MakeRect(10,30,240,212));
	scoreBackFail = new GUIStaticImage("pic\\result_fail_nm.png",MakeRect(0,0,240,212),MakeRect(10,30,240,212));

	clear = new GUIStaticImage("pic\\menu_result.png",MakeRect(257,96,89,15),MakeRect(321,195,89,15));
	clear->efvct.AddEffect(new Spark(clear->efvct.a,3,true,-1,255.0,255.0,150.0));
	notClear = new GUIStaticImage("pic\\menu_result.png",MakeRect(229,119,145,15),MakeRect(293,195,145,15));
	notClear->efvct.AddEffect(new Spark(notClear->efvct.a,3,true,-1,255.0,255.0,150.0));
	clearBack = new GUIStaticImage("pic\\menu_result.png",MakeRect(200,70,203,21),MakeRect(264,192,203,21));
	highLightExtra = new GUIStaticImage("pic\\menu_result.png",MakeRect(290,140,88,25),MakeRect(-9,-4,88,25));
	highLightExtra->efvct.AddEffect(new Spark(highLightExtra->efvct.a,7.0,true,-1,255.0,255.0));

	resultWord=NULL;
	btns = new NormalButtonGroup(MakeRect(0,0,0,0));
	btns->AddButton(new NormalButton(MakeRect(291,225,69,17),
						"pic\\menu_result.png",MakeRect(290,23,69,17),MakeRect(0,0,69,17),
						"pic\\menu_result.png",MakeRect(202,142,81,21),MakeRect(-5,-2,81,21),
						"pic\\menu_result.png",MakeRect(290,23,69,17),MakeRect(0,0,69,17),
						"pic\\menu_result.png",MakeRect(290,23,69,17),MakeRect(0,0,69,17)));
	btns->AddButton(new NormalButton(MakeRect(372,225,69,17),
						"pic\\menu_result.png",MakeRect(290,23,69,17),MakeRect(0,0,69,17),
						"pic\\menu_result.png",MakeRect(202,142,81,21),MakeRect(-5,-2,81,21),
						"pic\\menu_result.png",MakeRect(290,23,69,17),MakeRect(0,0,69,17),
						"pic\\menu_result.png",MakeRect(290,23,69,17),MakeRect(0,0,69,17)));
	btns->Focus();

	// load score resource
	for(int x=0; x<NUMSCOREROWS; x++)
	{
		IntResource *pRes = (IntResource*)g_res.getResource(score_names[x]);
		int *ptr = pRes->value;
		all_rect[x] = MakeRect(ptr[0], ptr[1], ptr[2], ptr[3]);
		all_color[x] = D3DCOLOR_RGBA(ptr[4],ptr[5],ptr[6],ptr[7]);
		all_inc[x] = ptr[8];
		all_fill[x] = ptr[9];
	}
}

ScoreMenu::~ScoreMenu()
{
	SAFE_DELETE(up);
	SAFE_DELETE(down);
	SAFE_DELETE(scoreBackWin);
	SAFE_DELETE(scoreBackFail);
	SAFE_DELETE(clear);
	SAFE_DELETE(notClear);
	SAFE_DELETE(clearBack);
	SAFE_DELETE(resultWord);
	for(int i=0;i<btns->GetButtonNum();i++)
		btns->GetButton(i)->UnRegister((GUIControl*) highLightExtra);
	SAFE_DELETE(highLightExtra);
	SAFE_DELETE(btns);
}

void ScoreMenu::OnKeyEvent(unsigned int nChar, bool bKeyDown)
{
	if(menust==SCOREMENUST_NUM&&nChar==KeyEnter&&bKeyDown)
	{
		all_cnt[1]=coolcnt;
		all_cnt[2]=finecnt;
		all_cnt[3]=safecnt;
		all_cnt[4]=sadcnt;
		all_cnt[5]=worstcnt;
		all_cnt[6]=combocnt;
		all_cnt[7]=chancetimecnt;
		all_cnt[0]=scorecnt;
	}
	else if(menust==SCOREMENUST_OVER)
	{
		int tfocus = btns->GetSelectedIndex();
		btns->OnKeyEvent(KeyEventValue);
		if(tfocus!=btns->GetSelectedIndex())
		{
			if(tfocus>=0)
				btns->GetButton(tfocus)->UnRegister((GUIControl*) highLightExtra);
			if(btns->GetSelectedIndex()>=0)
				btns->GetButton(btns->GetSelectedIndex())->Register((GUIControl*) highLightExtra);
		}
	}
}

void ScoreMenu::Draw()
{
	up->Draw();
	down->Draw();
	(ifClear?scoreBackWin:scoreBackFail)->Draw();
	if(menust==SCOREMENUST_OVER)
	{
		clearBack->Draw();
		if(ifClear)
			clear->Draw();
		else
			notClear->Draw();
		if(resultWord)
			resultWord->Draw();
		btns->Draw();
	}
	
	graphEngine.SetFont(systemIni.font,12);
	static RECT _rect = MakeRect(scoreBackWin->drawRect.left,scoreBackWin->drawRect.top + 26,scoreBackWin->drawRect.right-scoreBackWin->drawRect.left,24);
	graphEngine.DrawText(_songInfo,_rect, DT_CENTER | DT_VCENTER | DT_NOCLIP, D3DCOLOR_ARGB(255,255,255,255));

	graphEngine.SetFont(systemIni.font,14);
	
	for(int x=0; x<NUMSCOREROWS; x++)
		numberMana.Draw(all_cnt[x], all_rect[x], NumberStyle::SCORE, NumberDwFlags::DWFLAG_RIGHT, all_color[x], all_inc[x], all_fill[x]);

	if(menust==SCOREMENUST_OVER)
	{
		graphEngine.SetFont(systemIni.font,12);
		graphEngine.DrawText(StringTable(37),btns->GetButton(0)->drawRect,DT_CENTER | DT_VCENTER | DT_NOCLIP,D3DXCOLOR(D3DCOLOR_ARGB(255,255,255,255)));
		graphEngine.DrawText(StringTable(38),btns->GetButton(1)->drawRect,DT_CENTER | DT_VCENTER | DT_NOCLIP,D3DXCOLOR(D3DCOLOR_ARGB(255,255,255,255)));
	}
}

void GetOnValue(int &num,int des,int grow,bool &changed)
{
	if(num==des)
		return;
	int tnum=num;
	num+=(des-num)/grow;
	changed=true;
	if(tnum==num)
		num=des;
}

void ScoreMenu::Update()
{
	up->Update();
	down->Update();
	(ifClear?scoreBackWin:scoreBackFail)->Update();
	clearBack->Update();
	clear->Update();
	notClear->Update();
	if(resultWord)
		resultWord->Update();
	btns->Update();
	if(menust==SCOREMENUST_NUM)
	{
		bool flag=false;
		GetOnValue(all_cnt[1],coolcnt,8,flag);
		GetOnValue(all_cnt[2],finecnt,8,flag);
		GetOnValue(all_cnt[3],safecnt,8,flag);
		GetOnValue(all_cnt[4],sadcnt,8,flag);
		GetOnValue(all_cnt[5],worstcnt,8,flag);
		GetOnValue(all_cnt[6],combocnt,8,flag);
		GetOnValue(all_cnt[7],chancetimecnt,8,flag);
		GetOnValue(all_cnt[0],scorecnt,8,flag);
		if(!flag)
		{
			menust=SCOREMENUST_OVER;
			if(resultWord)
			{
				if(result==SONGRESULT_MISSTAKE)
					soundEngine->PlaySound("gamedata\\mistake.wav",systemIni.sndVolume);
				else if(result==SONGRESULT_CHEAP)
					soundEngine->PlaySound("gamedata\\cheap.wav",systemIni.sndVolume);
				else if(result==SONGRESULT_STANDARD)
					soundEngine->PlaySound("gamedata\\standard.wav",systemIni.sndVolume);
				else if(result==SONGRESULT_GREAT)
					soundEngine->PlaySound("gamedata\\great.wav",systemIni.sndVolume);
				else if(result==SONGRESULT_PERFECT)
					soundEngine->PlaySound("gamedata\\perfect.wav",systemIni.sndVolume);
				resultWord->efvct.AddEffect(new GUIEF_Move(resultWord->efvct.drawRect,20,
					(228-(resultWord->drawRect.right-resultWord->drawRect.left))/2+16,resultWord->drawRect.top));
				resultWord->efvct.AddEffect(new GUIEF_Alpha(resultWord->efvct.a,13,255));
				for(int i=0;i<btns->GetButtonNum();i++)
					btns->GetButton(i)->UnRegister((GUIControl*)highLightExtra);
				btns->SetFocusOn(0);
				btns->GetButton(0)->Register((GUIControl*)highLightExtra);
			}
		}
	}
}

#define GREATPES 0.7
#define STANDARDPES 0.5

void ScoreMenu::ResetResult(int coolCnt, int fineCnt, int safeCnt, int sadCnt, int worstCnt, int comboCnt, int chancetimeCnt, int scoreCnt, bool ifclear,int maxScoreCanGet,string songInfo)
{
	coolcnt=coolCnt;
	finecnt=fineCnt;
	safecnt=safeCnt;
	sadcnt=sadCnt;
	worstcnt=worstCnt;
	combocnt=comboCnt;
	chancetimecnt=chancetimeCnt;
	scorecnt=scoreCnt;
	ifClear=ifclear;
	all_cnt[1]=all_cnt[2]=all_cnt[3]=all_cnt[4]=all_cnt[5]=all_cnt[6]=all_cnt[7]=all_cnt[0]=0;
	int total=coolcnt+finecnt+safecnt+sadcnt+worstcnt;
	_songInfo = songInfo;
	
	SAFE_DELETE(resultWord);
	if(!ifClear)
	{
		result=SONGRESULT_MISSTAKE;
		resultWord = new GUIStaticImage("pic\\menu_result.png",MakeRect(0,168,194,16),MakeRect(130,34,194,16));
	}
	else if(combocnt==total)
	{
		result=SONGRESULT_PERFECT;
		resultWord = new GUIStaticImage("pic\\menu_result.png",MakeRect(0,72,166,16),MakeRect(130,34,166,16));
	}
	else if(scorecnt>=int(float(maxScoreCanGet)*GREATPES))
	{
		result=SONGRESULT_GREAT;
		resultWord = new GUIStaticImage("pic\\menu_result.png",MakeRect(0,104,111,16),MakeRect(130,34,111,16));
	}
	else if(scorecnt>=int(float(maxScoreCanGet)*STANDARDPES))
	{
		result=SONGRESULT_STANDARD;
		resultWord = new GUIStaticImage("pic\\menu_result.png",MakeRect(0,200,184,16),MakeRect(130,34,184,16));
	}
	else
	{
		result=SONGRESULT_CHEAP;
		ifClear = false;
		resultWord = new GUIStaticImage("pic\\menu_result.png",MakeRect(0,136,110,16),MakeRect(130,34,110,16));
	}
	resultWord->SetColor(0,255,255,255);
	menust=SCOREMENUST_NUM;
}

MainMenuBack_Scroll::MainMenuBack_Scroll(int delay,int delta,RECT drawRect)
{
	_a = 100;
	p1 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,0,0,0),MakeRect(0,0,0,0));
	p2 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,0,0,0),MakeRect(0,0,0,0));
	_delay = delay;
	_delayBuffer = 0;
	_delta = delta;
	_drawRect = drawRect;
	breakPoint = 271;
}
MainMenuBack_Scroll::~MainMenuBack_Scroll()
{
	SAFE_DELETE(p1);
	SAFE_DELETE(p2);
}

void MainMenuBack_Scroll::Update()
{
	p1->Update();
	p2->Update();
	(*p1->efvct.a) = _a;
	(*p2->efvct.a) = _a;
	if(_delayBuffer<_delay)
	{
		_delayBuffer++;
		if(_delayBuffer==_delay)
		{
			_delayBuffer=0;
			breakPoint -= _delta;
			if(breakPoint<0)
				breakPoint+=272;
		}
	}
	p1->drawRect = MakeRect(_drawRect.left,0,12,272-breakPoint);
	(*p1->efvct.srcRect) = MakeRect(0,breakPoint,12,272-breakPoint);
	p2->drawRect = MakeRect(_drawRect.left,272-breakPoint,12,breakPoint);
	(*p2->efvct.srcRect) = MakeRect(0,0,12,breakPoint);
}

void MainMenuBack_Scroll::Draw()
{
	p1->Draw();
	p2->Draw();
}

MainMenuBack_ScrollHoriz::MainMenuBack_ScrollHoriz()
{
	p1 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,0,0,0),MakeRect(0,0,0,0));
	(*p1->efvct.a)=50;
	p2 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,0,0,0),MakeRect(0,0,0,0));
	(*p2->efvct.a)=50;
	breakPoint=0;
}

MainMenuBack_ScrollHoriz::~MainMenuBack_ScrollHoriz()
{
	SAFE_DELETE(p1);
	SAFE_DELETE(p2);
}

void MainMenuBack_ScrollHoriz::Update()
{
	breakPoint+=1;
	if(breakPoint==480)
		breakPoint=0;
	(*p1->efvct.srcRect) = MakeRect(breakPoint,900,480-breakPoint,27);
	p1->drawRect = MakeRect(0,272-27,480-breakPoint,27);
	(*p2->efvct.srcRect) = MakeRect(0,900,breakPoint,27);
	p2->drawRect = MakeRect(480-breakPoint,272-27,breakPoint,27);
}

void MainMenuBack_ScrollHoriz::Draw()
{
	p1->Draw();
	p2->Draw();
}

MainMenuBack::MainMenuBack()
{
	menu_back		= (ImageResource*)g_res.getResource("songmenu_back");

	circleOut = new GUIStaticImage("pic\\menu_com.png",MakeRect(325,0,699,697),MakeRect(355,0,175,175));//530 135 圆心 350
	circleOutout = new GUIStaticImage("pic\\menu_com.png",MakeRect(325,0,699,697),MakeRect(355,0,175,175));

	rightScroll = new MainMenuBack_Scroll(20,4,MakeRect(436,0,12,272));
	flyScroll = new MainMenuBack_Scroll(5,6,MakeRect(30,0,12,272));
	flyScroll->_a = 0;

	rightLine = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(435,0,2,272));
	(*rightLine->efvct.a)=100;

	downLine = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(0,231,480,2));
	(*downLine->efvct.a)=0;
	_downLinedelay = 0;

	vertLine1 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(206,0,2,272));
	(*vertLine1->efvct.a)=0;
	vertLine2 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(221,0,1,272));
	(*vertLine2->efvct.a)=0;

	vertLine3 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(36,0,1,272));
	(*vertLine3->efvct.a)=0;
	vertLine4 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(47,0,2,272));
	(*vertLine4->efvct.a)=0;

	block1 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(480,183,0,26));
	(*block1->efvct.a)=0;
	block2 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(480,183,0,26));
	(*block2->efvct.a)=0;
	block3 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(480,183,0,26));
	(*block3->efvct.a)=0;
	block4 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(480,183,0,26));
	(*block4->efvct.a)=0;
	_block4AllDelay=30;
	block5 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(480,183,0,26));
	(*block5->efvct.a)=0;
	_block5AllDelay=60;

	block6 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(235,40,10,10));
	(*block6->efvct.a)=20;

	block7 = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,280,20,20),MakeRect(200,0,80,272));
	(*block7->efvct.a)=10;

	flyUp = new GUIStaticImage("pic\\menu_com.png",MakeRect(0,750,480,150),MakeRect(0,272,480,150));
	(*flyUp->efvct.a)=70;

	downScroll = new MainMenuBack_ScrollHoriz();
}
MainMenuBack::~MainMenuBack()
{
	SAFE_DELETE(circleOut);
	SAFE_DELETE(circleOutout);

	SAFE_DELETE(rightScroll);
	SAFE_DELETE(rightLine);

	SAFE_DELETE(downLine);

	SAFE_DELETE(vertLine1);
	SAFE_DELETE(vertLine2);
	SAFE_DELETE(vertLine3);
	SAFE_DELETE(vertLine4);

	SAFE_DELETE(flyScroll);

	SAFE_DELETE(block1);
	SAFE_DELETE(block2);
	SAFE_DELETE(block3);
	SAFE_DELETE(block4);
	SAFE_DELETE(block5);
	SAFE_DELETE(block6);
	SAFE_DELETE(block7);

	SAFE_DELETE(flyUp);

	SAFE_DELETE(downScroll);
}

void MainMenuBack::Draw()
{
	graphEngine.Draw(menu_back);

	circleOut->Draw();
	circleOutout->Draw();
	rightScroll->Draw();
	rightLine->Draw();

	downLine->Draw();

	vertLine1->Draw();
	vertLine2->Draw();
	vertLine3->Draw();
	vertLine4->Draw();

	flyScroll->Draw();
	downScroll->Draw();

	block1->Draw();
	block2->Draw();
	block3->Draw();
	block4->Draw();
	block5->Draw();
	block6->Draw();
	block7->Draw();

	flyUp->Draw();
}

void MainMenuBack::Update()
{
	//CircleOut
	circleOut->Update();
	circleOutout->Update();
	if(circleOut->efvct.effectnums==0)
	{
		circleOutout->efvct.ReleaseEffects();
		(*circleOutout->efvct.a)=204;
		circleOutout->drawRect=circleOut->drawRect;
		circleOutout->efvct.AddEffect(new GUIEF_Stretch(circleOutout->efvct.drawRect,180,MakeRect(-70,0,600,600)));
		(*circleOut->efvct.a)=80;
		circleOut->efvct.AddEffect(new GUIEF_Alpha(circleOut->efvct.a,0.5,204,330));
	}
	if(circleOutout->efvct.effectnums==1&&circleOutout->drawRect.left<=115&&(*circleOutout->efvct.a)!=0)
	{
		circleOutout->efvct.AddEffect(new GUIEF_Alpha(circleOutout->efvct.a,3,0));
	}

	rightScroll->Update();
	rightLine->Update();

	downLine->Update();
	if(downLine->efvct.effectnums==0&&(*downLine->efvct.a)==0)
	{
		downLine->efvct.AddEffect(new GUIEF_Alpha(downLine->efvct.a,5,100,40));
		downLine->drawRect = MakeRect(0,235,480,2);
	}
	else
	{
		_downLinedelay++;
		if(_downLinedelay==22)
			downLine->drawRect.top--;
		else if(_downLinedelay==30)
		{
			downLine->drawRect.bottom--;
			if(downLine->drawRect.bottom == 223)
				downLine->efvct.AddEffect(new GUIEF_Alpha(downLine->efvct.a,5,0));
			_downLinedelay=0;
		}
	}

	vertLine1->Update();
	if(vertLine1->efvct.effectnums==0&&(*vertLine1->efvct.a)==0)
	{
		vertLine1->efvct.AddEffect(new GUIEF_Alpha(vertLine1->efvct.a,5,100,100));
		vertLine1->drawRect = MakeRect(206,0,2,272);
		vertLine1->efvct.AddEffect(new GUIEF_Stretch(vertLine1->efvct.drawRect,420,MakeRect(250,0,2,272),100));
	}
	else if(vertLine1->drawRect.left==235&&(*vertLine1->efvct.a)==100)
		vertLine1->efvct.AddEffect(new GUIEF_Alpha(vertLine1->efvct.a,4,0));

	vertLine2->Update();
	if(vertLine2->efvct.effectnums==0&&(*vertLine2->efvct.a)==0)
	{
		vertLine2->efvct.AddEffect(new GUIEF_Alpha(vertLine2->efvct.a,5,100,100));
		vertLine2->drawRect = MakeRect(221,0,1,272);
		vertLine2->efvct.AddEffect(new GUIEF_Stretch(vertLine2->efvct.drawRect,420,MakeRect(265,0,2,272),100));
	}
	else if(vertLine2->drawRect.left==250&&(*vertLine2->efvct.a)==100)
		vertLine2->efvct.AddEffect(new GUIEF_Alpha(vertLine2->efvct.a,4,0));

	vertLine3->Update();
	if(vertLine3->efvct.effectnums==0&&(*vertLine3->efvct.a)==0)
	{
		vertLine3->efvct.AddEffect(new GUIEF_Alpha(vertLine3->efvct.a,5,60,100));
		vertLine3->drawRect = MakeRect(36,0,1,272);
		vertLine3->efvct.AddEffect(new GUIEF_Stretch(vertLine3->efvct.drawRect,420,MakeRect(81,0,2,272),100));
	}
	else if(vertLine3->drawRect.left==74&&(*vertLine3->efvct.a)==60)
		vertLine3->efvct.AddEffect(new GUIEF_Alpha(vertLine3->efvct.a,4,0));

	vertLine4->Update();
	if(vertLine4->efvct.effectnums==0&&(*vertLine4->efvct.a)==0)
	{
		vertLine4->efvct.AddEffect(new GUIEF_Alpha(vertLine4->efvct.a,5,60,100));
		vertLine4->drawRect = MakeRect(47,0,2,272);
		vertLine4->efvct.AddEffect(new GUIEF_Stretch(vertLine4->efvct.drawRect,420,MakeRect(92,0,2,272),100));
	}
	else if(vertLine4->drawRect.left==85&&(*vertLine4->efvct.a)==60)
		vertLine4->efvct.AddEffect(new GUIEF_Alpha(vertLine4->efvct.a,4,0));


	//flyScroll
	flyScroll->Update();
	if(flyScroll->p1->efvct.effectnums==0&&flyScroll->_a==0)
	{
		flyScroll->p1->efvct.AddEffect(new GUIEF_Alpha(&flyScroll->_a,2,30,60));
		flyScroll->_drawRect = MakeRect(30,0,12,272);
		flyScroll->p1->efvct.AddEffect(new GUIEF_Stretch(&flyScroll->_drawRect,300,MakeRect(454,0,12,272),60));
	}
	else if(flyScroll->_drawRect.left>=400&&flyScroll->_a==30)
		flyScroll->p1->efvct.AddEffect(new GUIEF_Alpha(&flyScroll->_a,2,0));

	block1->Update();
	if(block1->efvct.effectnums==0&&(*block1->efvct.a)==0)
	{
		(*block1->efvct.a)=50;
		block1->drawRect = MakeRect(480,183,0,26);
		block1->efvct.AddEffect(new GUIEF_Stretch(block1->efvct.drawRect,10,MakeRect(354,183,126,26)));
	}
	else if(block1->efvct.effectnums==0&&(*block1->efvct.a)==50)
	{
		block1->efvct.AddEffect(new GUIEF_Stretch(block1->efvct.drawRect,420,MakeRect(264,164,306,63)));
	}
	else if(block1->drawRect.left<=300&&(*block1->efvct.a)==50)
	{
		block1->efvct.AddEffect(new GUIEF_Alpha(block1->efvct.a,0.75,0));
	}

	block2->Update();
	if(block2->efvct.effectnums==0&&(*block2->efvct.a)==0)
	{
		(*block2->efvct.a)=50;
		block2->drawRect = MakeRect(395,61,10,150);
		block2->efvct.AddEffect(new GUIEF_Stretch(block2->efvct.drawRect,10,MakeRect(390,0,21,272)));
	}
	else if(block2->efvct.effectnums==0&&(*block2->efvct.a)==50)
	{
		block2->efvct.AddEffect(new GUIEF_Stretch(block2->efvct.drawRect,420,MakeRect(384,-65,31,363)));
	}
	else if(block2->drawRect.left<=387&&(*block2->efvct.a)==50)
	{
		block2->efvct.AddEffect(new GUIEF_Alpha(block2->efvct.a,0.75,0));
	}

	block3->Update();
	if(block3->efvct.effectnums==0&&(*block3->efvct.a)==0)
	{
		(*block3->efvct.a)=50;
		block3->drawRect = MakeRect(0,34,0,10);
		block3->efvct.AddEffect(new GUIEF_Stretch(block3->efvct.drawRect,10,MakeRect(0,34,140,10),240));
	}
	else if(block3->efvct.effectnums==0&&(*block3->efvct.a)==50)
	{
		block3->efvct.AddEffect(new GUIEF_Stretch(block3->efvct.drawRect,60,MakeRect(0,0,280,95)));
	}
	else if(block3->drawRect.right>=210&&(*block3->efvct.a)==50)
	{
		block3->efvct.AddEffect(new GUIEF_Alpha(block3->efvct.a,2,0));
	}

	block4->Update();
	if(_block4AllDelay)
		_block4AllDelay--;
	else
	{
		if(block4->efvct.effectnums==0&&(*block4->efvct.a)==0)
		{
			(*block4->efvct.a)=50;
			block4->drawRect = MakeRect(0,115,0,10);
			block4->efvct.AddEffect(new GUIEF_Stretch(block4->efvct.drawRect,10,MakeRect(0,115,120,10),240));
		}
		else if(block4->efvct.effectnums==0&&(*block4->efvct.a)==50)
		{
			block4->efvct.AddEffect(new GUIEF_Stretch(block4->efvct.drawRect,60,MakeRect(0,81,240,90)));
		}
		else if(block4->drawRect.right>=180&&(*block4->efvct.a)==50)
		{
			block4->efvct.AddEffect(new GUIEF_Alpha(block4->efvct.a,2,0));
		}
	}

	block5->Update();
	if(_block5AllDelay)
		_block5AllDelay--;
	else
	{
		if(block5->efvct.effectnums==0&&(*block5->efvct.a)==0)
		{
			(*block5->efvct.a)=50;
			block5->drawRect = MakeRect(0,189,0,10);
			block5->efvct.AddEffect(new GUIEF_Stretch(block5->efvct.drawRect,10,MakeRect(0,189,100,10),240));
		}
		else if(block5->efvct.effectnums==0&&(*block5->efvct.a)==50)
		{
			block5->efvct.AddEffect(new GUIEF_Stretch(block5->efvct.drawRect,60,MakeRect(0,154,200,80)));
		}
		else if(block5->drawRect.right>=150&&(*block5->efvct.a)==50)
		{
			block5->efvct.AddEffect(new GUIEF_Alpha(block5->efvct.a,2,0));
		}
	}

	block6->Update();
	if(block6->efvct.effectnums==0)
	{
		if(block6->drawRect.left==235)
			block6->efvct.AddEffect(new GUIEF_Stretch(block6->efvct.drawRect,90,MakeRect(225,10,30,60)));
		else if(block6->drawRect.left==225)
			block6->efvct.AddEffect(new GUIEF_Stretch(block6->efvct.drawRect,90,MakeRect(232,32,15,15)));
		else if(block6->drawRect.left==232)
			block6->efvct.AddEffect(new GUIEF_Stretch(block6->efvct.drawRect,90,MakeRect(210,0,60,60)));
		else if(block6->drawRect.left==210)
			block6->efvct.AddEffect(new GUIEF_Stretch(block6->efvct.drawRect,90,MakeRect(215,50,50,10)));
		else if(block6->drawRect.left==215)
			block6->efvct.AddEffect(new GUIEF_Stretch(block6->efvct.drawRect,90,MakeRect(235,40,10,10)));
	}

	block7->Update();
	if(block7->efvct.effectnums==0)
	{
		if(block7->drawRect.left==200)
			block7->efvct.AddEffect(new GUIEF_Stretch(block7->efvct.drawRect,120,MakeRect(220,0,40,272)));
		else if(block7->drawRect.left==220)
			block7->efvct.AddEffect(new GUIEF_Stretch(block7->efvct.drawRect,120,MakeRect(200,0,80,272)));
	}

	flyUp->Update();
	if(flyUp->efvct.effectnums==0)
	{
		flyUp->drawRect = MakeRect(0,272,480,150);
		flyUp->efvct.AddEffect(new GUIEF_Stretch(flyUp->efvct.drawRect,150,MakeRect(0,-150,480,150),240));
	}

	downScroll->Update();
}

BottomBar::BottomBar()
{
	bottom2L = WIDTH;
	BarAlphaState = 0;
	Cango = false;

	// load menu resources
	struct toLoad{
		GUIStaticImage** ppRes;
		char *res_name;
	};
	static toLoad toloads[]={
		{&bottom1,			"bottom1"		},
		{&bottom2,			"bottom2"		},
		{&labelF5,			"label_F5"		},
		{&labelMove,		"label_Move"	},
		{&labelSelect,		"label_Select"	},
		{&labelPV,			"label_PV"		},
		{&labelDisplay,		"label_Display"	},
	};
	for(int x=0, X=sizeof(toloads)/sizeof(toLoad); x<X; x++)
		*toloads[x].ppRes = new GUIStaticImage(toloads[x].res_name);
}

BottomBar::~BottomBar()
{
	_UnRegisterAll();

	SAFE_DELETE(bottom1);
	SAFE_DELETE(bottom2);

	SAFE_DELETE(labelMove);
	SAFE_DELETE(labelF5);
	SAFE_DELETE(labelSelect);
	SAFE_DELETE(labelPV);
	SAFE_DELETE(labelDisplay);
}

void BottomBar::_UnRegisterAll()
{
	bottom2->UnRegister(labelMove);
	bottom2->UnRegister(labelSelect);
	bottom2->UnRegister(labelPV);
	bottom2->UnRegister(labelDisplay);
	bottom2->UnRegister(labelF5);
}

void BottomBar::_ReleaseAllEffect()
{
	labelMove->efvct.ReleaseEffects();
	labelSelect->efvct.ReleaseEffects();
	labelPV->efvct.ReleaseEffects();
	labelDisplay->efvct.ReleaseEffects();
	labelF5->efvct.ReleaseEffects();
}

void BottomBar::_SetAllAlpha(int _a)
{
	(*labelMove->efvct.a) = _a;
	(*labelF5->efvct.a) = _a;
	(*labelPV->efvct.a) = _a;
	(*labelSelect->efvct.a) = _a;
	(*labelDisplay->efvct.a) = _a;
}

void BottomBar::MainMenuStateChanged(int MainMenuState)
{
	Cango = false;
	BarAlphaState = 1;
	MainMenuStateGoTo = MainMenuState;

	_ReleaseAllEffect();
	_SetAllAlpha(255);
	labelMove->efvct.AddEffect(new GUIEF_Alpha(labelMove->efvct.a, 12.75,0));
	labelF5->efvct.AddEffect(new GUIEF_Alpha(labelF5->efvct.a, 12.75,0));
	labelSelect->efvct.AddEffect(new GUIEF_Alpha(labelSelect->efvct.a, 12.75,0));
	labelPV->efvct.AddEffect(new GUIEF_Alpha(labelPV->efvct.a, 12.75,0));
	labelDisplay->efvct.AddEffect(new GUIEF_Alpha(labelDisplay->efvct.a, 12.75,0));

	bottom2->efvct.ReleaseEffects();
	if(MainMenuStateGoTo==MAINST_MAIN||MainMenuStateGoTo==MAINST_CG||MainMenuStateGoTo==MAINST_FREE)
	{
		bottom2L = 480 - 33 -(labelMove->drawRect.right-labelMove->drawRect.left) - 16
			-(labelSelect->drawRect.right-labelSelect->drawRect.left) - 13;
	}
	else if(MainMenuStateGoTo==MAINST_SELECTSONG)
	{
		bottom2L = 480-33 -(labelMove->drawRect.right-labelMove->drawRect.left) - 16
			-(labelPV->drawRect.right-labelPV->drawRect.left) - 16
			-(labelDisplay->drawRect.right-labelDisplay->drawRect.left) - 16
			-(labelSelect->drawRect.right-labelSelect->drawRect.left) - 13;
	}
	if(MainMenuStateGoTo==MAINST_FREE)
		bottom2L -= (labelF5->drawRect.right-labelF5->drawRect.left+16);
}

void BottomBar::MainMenuStateChangedEnd()
{
	if(BarAlphaState == 1)
		Cango = true;
}

void BottomBar::Draw()
{
	bottom1->Draw();
	bottom2->Draw();
}

void BottomBar::Update()
{
	bottom1->Update();
	bottom2->Update();

	if(BarAlphaState == 1 && Cango)
	{
		Cango = false;
		BarAlphaState = 2;

		_UnRegisterAll();
		_ReleaseAllEffect();
		_SetAllAlpha(0);

		bottom2->efvct.AddEffect((BaseEffect*) new GUIEF_Move(bottom2->efvct.drawRect,15,bottom2L, bottom2->drawRect.top, 5));

		int nowPos,nowPosY=10;
		if(MainMenuStateGoTo==MAINST_MAIN||MainMenuStateGoTo==MAINST_CG||MainMenuStateGoTo==MAINST_FREE)
		{
			bottom2->Register(labelMove);
			bottom2->Register(labelSelect);
			nowPos=33;
			SetRectPos(nowPos, nowPosY, labelMove->drawRect);
			nowPos += labelMove->drawRect.right-labelMove->drawRect.left + 16;
			SetRectPos(nowPos, nowPosY, labelSelect->drawRect);
			if(MainMenuStateGoTo==MAINST_FREE)
			{
				bottom2->Register(labelF5);
				nowPos += labelF5->drawRect.right-labelF5->drawRect.left + 16;
				SetRectPos(nowPos, nowPosY, labelF5->drawRect);
			}
		}
		else if(MainMenuStateGoTo==MAINST_SELECTSONG)
		{
			bottom2->Register(labelMove);
			bottom2->Register(labelPV);
			bottom2->Register(labelDisplay);
			bottom2->Register(labelSelect);
			nowPos=33;
			SetRectPos(nowPos, nowPosY, labelMove->drawRect);
			nowPos += labelMove->drawRect.right-labelMove->drawRect.left + 16;
			SetRectPos(nowPos, nowPosY, labelPV->drawRect);
			nowPos += labelPV->drawRect.right-labelPV->drawRect.left + 16;
			SetRectPos(nowPos, nowPosY, labelDisplay->drawRect);
			nowPos += labelDisplay->drawRect.right-labelDisplay->drawRect.left + 16;
			SetRectPos(nowPos, nowPosY, labelSelect->drawRect);
		}
	}
	else if(BarAlphaState == 2 && bottom2->efvct.effectnums==0)
	{
		BarAlphaState = 0;
		labelMove->efvct.AddEffect(new GUIEF_Alpha(labelMove->efvct.a, 12.75,255));
		labelF5->efvct.AddEffect(new GUIEF_Alpha(labelF5->efvct.a, 12.75,255));
		labelSelect->efvct.AddEffect(new GUIEF_Alpha(labelSelect->efvct.a, 12.75,255));
		labelPV->efvct.AddEffect(new GUIEF_Alpha(labelPV->efvct.a, 12.75,255));
		labelDisplay->efvct.AddEffect(new GUIEF_Alpha(labelDisplay->efvct.a, 12.75,255));
	}
}

CGMenu::CGMenu()
{
	nowSelected = 0;
	OnEscClick = NULL;
	_cgShow = NULL;

	_title = new GUIStaticImage("cg_menu_title");
	_cgBack = new GUIStaticImage("cg_menu_back");

	_pageNumBack = new GUIVertStretchBlock("pic\\menu_common.png",MakeRect(187,173,3,17),
										"pic\\menu_common.png",MakeRect(253,173,3,17),
										"pic\\menu_common.png",MakeRect(190,173,63,17),
										MakeRect(_coreX,230,100,17));
	_pageNumBack->SetColor(210,255,255,255);

	_illusionBack = new GUIVertStretchBlock("pic\\menu_common.png",MakeRect(187,173,3,17),
										"pic\\menu_common.png",MakeRect(253,173,3,17),
										"pic\\menu_common.png",MakeRect(190,173,63,17),
										MakeRect(_pageNumBack->GetDrawRect().right + 50,230, 
												_coreX+_coreWidth - (_pageNumBack->GetDrawRect().right + 50),17));
	_illusionBack->SetColor(210,255,255,255);

	_lArrow = new GUIStaticImage("cg_left_arrow");
	_rArrow = new GUIStaticImage("cg_right_arrow");

	_alphaState = -1;
}

CGMenu::~CGMenu()
{
	ReleaseCG();
	SAFE_DELETE(_cgBack);
	SAFE_DELETE(_pageNumBack);
	SAFE_DELETE(_illusionBack);
	SAFE_DELETE(_lArrow);
	SAFE_DELETE(_rArrow);
	SAFE_DELETE(_title);
}

void CGMenu::PrepareCG()
{
	ReleaseCG();

	for(int i=0;i<cgFileInfo._cgFilename.size();i++)
		_cg.push_back(NULL);
						
}

void CGMenu::ReleaseCG()
{
	for(vector<GUIStaticImage*>::iterator ptr = _cg.begin();ptr!=_cg.end();ptr++)
		SAFE_DELETE((*ptr));
	_cg.clear();
	graphEngine.ReleaseTex(3);

	_alphaState = -1;
}

void CGMenu::EnterCGMenu()
{
	SAFE_DELETE(_cgShow);
	for(int i=0;i<cgFileInfo._cgFilename.size();i++)
		graphEngine.AddTexture(cgFileInfo._cgFilename[i],3);
	nowpage = 0;
	lastpage = -1;
	nowSelected = 0;
	_cgBack->drawRect = CalcSelectRect();
	PageChanged(0);
}

void CGMenu::PageChanged(int direc)
{
	int tInX,tOutX;

	switch(direc)
	{
	case -1:
		tInX = 480;
		tOutX = -480;
		break;
	case 0:
		tInX = tOutX = 0;
		break;
	case 1:
		tInX = -480;
		tOutX = 480;
		break;
	}
	
	int forToNum = min((lastpage+1)*_picNumPerPage,cgFileInfo._cgFilename.size());

	if(lastpage!=-1)
		for(int i=lastpage*_picNumPerPage;i<forToNum;i++)
			_cg[i]->efvct.AddEffect(new GUIEF_Move(_cg[i]->efvct.drawRect,30,_cg[i]->drawRect.left + tOutX,_cg[i]->drawRect.top));


	forToNum = min((nowpage+1)*_picNumPerPage,cgFileInfo._cgFilename.size());

	for(int i=nowpage*_picNumPerPage;i<forToNum;i++)
	{
		int tx = (i%_picNumPerPage) % _picNumHoriz,ty = (i%_picNumPerPage) / _picNumHoriz;

		_cg[i] = new GUIStaticImage(cgFileInfo._cgFilename[i], MakeRect(0,0,480,272), 
			MakeRect(tInX+_coreX + tx*(_picWidth + _deltaWidth),_coreY + ty*(_picHeight+_deltaHeight),_picWidth,_picHeight), _PoolUse);

		if(direc)
			_cg[i]->efvct.AddEffect(new GUIEF_Move(_cg[i]->efvct.drawRect,30,_cg[i]->drawRect.left - tInX,_cg[i]->drawRect.top));
	}

	_alphaState = direc==0?1:0;
}

RECT CGMenu::CalcSelectRect()
{
	if(nowSelected == -1)
		return MakeRect(0,0,0,0);
	int tx = (nowSelected%_picNumPerPage) % _picNumHoriz,ty = (nowSelected%_picNumPerPage) / _picNumHoriz;

	return MakeRect(_coreX + tx*(_picWidth + _deltaWidth) - 12,_coreY + ty*(_picHeight+_deltaHeight)-10,_picWidth+24,_picHeight+20);
}

void CGMenu::PageChangedOver()
{
	if(lastpage!=-1)
	{
		int forToNum = min((lastpage+1)*_picNumPerPage,cgFileInfo._cgFilename.size());
		for(int i=lastpage*_picNumPerPage;i<forToNum;i++)
			SAFE_DELETE(_cg[i]);
	}
	_alphaState = 1;
}

void CGMenu::OnKeyEvent(unsigned int nChar, bool bKeyDown)
{
	if(bKeyDown&&nChar==KeyEsc&&_alphaState==1)
	{
		if(OnEscClick)
			OnEscClick();
	}

	if(_cg.size()==0) return;

	if(bKeyDown&&nChar==KeyEnter&&_alphaState==1)
	{
		_cgShow = new GUIStaticImage(cgFileInfo._cgFilename[nowSelected], MakeRect(0,0,480,272),_cg[nowSelected]->drawRect,_PoolUse);
		(*_cgShow->efvct.a)=60;
		_cgShow->efvct.AddEffect(new GUIEF_Stretch(_cgShow->efvct.drawRect,20,MakeRect(0,0,480,272)));
		_cgShow->efvct.AddEffect(new GUIEF_Alpha(_cgShow->efvct.a,195.0/20.0,255.0));
		_alphaState=2;
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
	}
	if(bKeyDown&&nChar==KeyEnter&&_alphaState==3)
	{
		_cgShow->efvct.AddEffect(new GUIEF_Stretch(_cgShow->efvct.drawRect,20,_cg[nowSelected]->drawRect));
		_cgShow->efvct.AddEffect(new GUIEF_Alpha(_cgShow->efvct.a,255.0/20.0,0.0));
		_alphaState=4;
	}

	int tx = (nowSelected%_picNumPerPage) % _picNumHoriz,ty = (nowSelected%_picNumPerPage) / _picNumHoriz;
	if(bKeyDown&&_alphaState==1)
	{
		if(nChar==KeyRight)
		{
key_right:
			tx++;
			nowSelected = nowpage * _picNumPerPage + ty*_picNumHoriz + tx;
			if(tx==_picNumHoriz||(tx+nowpage*_picNumPerPage+ty*_picNumHoriz)==_cg.size())
			{
				lastpage = nowpage;
				int pageNum = (_cg.size()-1) / (_picNumPerPage);
				if(nowpage==pageNum)
					nowpage=0;
				else
					nowpage++;
				nowSelected = nowpage * _picNumPerPage + ty*_picNumHoriz;
				if(nowSelected>=_cg.size())
					nowSelected = nowpage * _picNumPerPage;
				PageChanged(-1);
			}
			soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
		}
		else if(nChar==KeyLeft)
		{
			tx--;
			nowSelected = nowpage * _picNumPerPage + ty*_picNumHoriz + tx;
			if(tx==-1)
			{
				lastpage = nowpage;
				if(nowpage==0)
					nowpage = (_cg.size()-1) / (_picNumPerPage);
				else
					nowpage--;
				nowSelected = nowpage * _picNumPerPage + (ty+1)*_picNumHoriz-1;
				if(nowSelected>=_cg.size())
					nowSelected = _cg.size() - 1;
				PageChanged(1);
			}
			soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
		}
		else if(nChar==KeyDown)
		{
			if(++ty==_picNumVert || (tx+nowpage*_picNumPerPage+ty*_picNumHoriz) >= _cg.size())
			{
				ty=0;
				goto key_right;
			}
			nowSelected = tx + nowpage * _picNumPerPage + ty*_picNumHoriz;
			soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
		}
		else if(nChar==KeyUp)
		{
			if(--ty==-1)
			{
				ty=_picNumVert - 1;
				tx--;
				nowSelected = nowpage * _picNumPerPage + ty*_picNumHoriz + tx;
				if(tx==-1)
				{
					lastpage = nowpage;
					if(nowpage==0)
						nowpage = (_cg.size()-1) / (_picNumPerPage);
					else
						nowpage--;
					nowSelected = nowpage * _picNumPerPage + (ty+1)*_picNumHoriz-1;
					if(nowSelected>=_cg.size())
						nowSelected = _cg.size() - 1;
					PageChanged(1);
					tx = (nowSelected%_picNumPerPage) % _picNumHoriz;
				}
				while((tx+nowpage*_picNumPerPage+ty*_picNumHoriz) >= _cg.size())
					ty--;
			}
			nowSelected = tx + nowpage * _picNumPerPage + ty*_picNumHoriz;
			soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
		}
		_cgBack->drawRect = CalcSelectRect();
	}

}

void CGMenu::Update()
{
	if(_alphaState==-1)
		return;
	_title->Update();
	_pageNumBack->Update();
	_illusionBack->Update();
	for(int i = 0;i<_cg.size();i++)
		if(_cg[i])
			_cg[i]->Update();

	if(_cg.size()!=0)
	{
		if(_alphaState==0)
			if(_cg[nowSelected]->efvct.effectnums==0)
				PageChangedOver();
		if(_alphaState==1&&_cg.size() > _picNumPerPage)
		{
			_lArrow->Update();
			_rArrow->Update();
		}
	}
	if(_alphaState>1)
	{
		_cgShow->Update();
		if(_alphaState==2&&_cgShow->efvct.effectnums==0)
			_alphaState=3;
		else if(_alphaState==4&&_cgShow->efvct.effectnums==0)
		{
			SAFE_DELETE(_cgShow);
			_alphaState=1;
		}
	}
	
}

void CGMenu::Draw()
{
	if(_alphaState==-1)
		return;
	_title->Draw();
	_pageNumBack->Draw();
	_illusionBack->Draw();
	if(_cg.size()!=0)
	{
		graphEngine.SetFont(systemIni.font,14);
		ostringstream oss;
		oss << "No. " << nowSelected+1 << " / " << _cg.size();
		graphEngine.DrawText(oss.str(), _pageNumBack->GetDrawRect(), DT_CENTER | DT_VCENTER);
		//graphEngine.DrawText(_pageNumBack->GetDrawRect(), DT_CENTER | DT_VCENTER, "No. %d / %d", nowSelected+1, _cg.size());
		graphEngine.DrawText("Illustration by " + cgFileInfo._illusion[nowSelected], _illusionBack->GetDrawRect(), DT_CENTER | DT_VCENTER);
	}
	if(_cg.size()!=0&&_alphaState==1)
	{
		_cgBack->Draw();
		if(_cg.size() > _picNumPerPage)
		{
			_lArrow->Draw();
			_rArrow->Draw();
		}
	}
	for(int i = 0;i<_cg.size();i++)
		if(_cg[i])
			_cg[i]->Draw();
	if(_alphaState>1)
		_cgShow->Draw();
}

GUIMessageBox::GUIMessageBox(int texturePool)
{
	_leftup = new GUIStaticImage("pic\\menu_basic.png",MakeRect(150,139,19,22),MakeRect(0,0,0,0),texturePool);
	_rightup = new GUIStaticImage("pic\\menu_basic.png",MakeRect(191,139,19,22),MakeRect(0,0,0,0),texturePool);
	_leftdown = new GUIStaticImage("pic\\menu_basic.png",MakeRect(150,177,19,22),MakeRect(0,0,0,0),texturePool);
	_rightdown = new GUIStaticImage("pic\\menu_basic.png",MakeRect(191,177,19,22),MakeRect(0,0,0,0),texturePool);
	_up = new GUIStaticImage("pic\\menu_basic.png",MakeRect(169,139,22,22),MakeRect(0,0,0,0),texturePool);
	_down = new GUIStaticImage("pic\\menu_basic.png",MakeRect(169,177,22,22),MakeRect(0,0,0,0),texturePool);
	_left = new GUIStaticImage("pic\\menu_basic.png",MakeRect(150,161,19,16),MakeRect(0,0,0,0),texturePool);
	_right = new GUIStaticImage("pic\\menu_basic.png",MakeRect(191,161,19,16),MakeRect(0,0,0,0),texturePool);
	_mid = new GUIStaticImage("pic\\menu_basic.png",MakeRect(169,161,22,16),MakeRect(0,0,0,0),texturePool);

	_highLight = new GUIVertStretchBlock("pic\\menu_basic04.png",MakeRect(3,194,36,29), "pic\\menu_basic04.png", MakeRect(52,194,39,29),
												"pic\\menu_basic04.png",MakeRect(39,194,13,29), MakeRect(0,0,0,0),texturePool);

	_highLight->lSide->efvct.AddEffect(new Spark(_highLight->lSide->efvct.a,7.0,true,-1,255.0,255.0));
	_highLight->rSide->efvct.AddEffect(new Spark(_highLight->rSide->efvct.a,7.0,true,-1,255.0,255.0));
	_highLight->middle->efvct.AddEffect(new Spark(_highLight->middle->efvct.a,7.0,true,-1,255.0,255.0));

	_highLightBack = new GUIVertStretchBlock("pic\\menu_basic04.png",MakeRect(97,194,36,29), "pic\\menu_basic04.png", MakeRect(146,194,39,29),
												"pic\\menu_basic04.png",MakeRect(133,194,13,29), MakeRect(0,0,0,0),texturePool);

	p_OnMenuSelectBegin = NULL;
	p_OnMenuSelectOver = NULL;
	p_OnMenuReleaseOver = NULL;
	p_OnMenuInitOver = NULL;
	p_OnSelectedIndexChanged = NULL;

	prepareState = -1;

	fontSize = 14;
	lineSepSize = 2;
	selectedIndex = -1;
}

GUIMessageBox::~GUIMessageBox()
{
	SAFE_DELETE(_leftup);
	SAFE_DELETE(_rightup);
	SAFE_DELETE(_leftdown);
	SAFE_DELETE(_rightdown);
	SAFE_DELETE(_up);
	SAFE_DELETE(_down);
	SAFE_DELETE(_left);
	SAFE_DELETE(_right);
	SAFE_DELETE(_mid);

	SAFE_DELETE(_highLight);
	SAFE_DELETE(_highLightBack);
}

void GUIMessageBox::PrepareMenu(int basicInfo,int drawWidth)
{
	_basicInfo = basicInfo;
	_drawWidth = drawWidth;
	if(_drawWidth<(_left->srcRect.right-_left->srcRect.left)+(_right->srcRect.right-_right->srcRect.left))
		_drawWidth=(_left->srcRect.right-_left->srcRect.left)+(_right->srcRect.right-_right->srcRect.left);
	_option.clear();
	prepareState = 0;
}

void GUIMessageBox::AddItem(int option)
{
	if(prepareState!=0)
		return;
	_option.push_back(option);
	selectedIndex = 0;
}

void GUIMessageBox::AddItem(string text)
{
	if(prepareState!=0)
		return;
	_text.push_back(text);
	_option.push_back(-_text.size());
	selectedIndex = 0;
}

void GUIMessageBox::ShowMenu()
{
	int drawHeight=0;
	if(_basicInfo!=40)
		drawHeight += lineSepSize + fontSize + fontSize;
	if(_option.size()!=0)
		drawHeight += (lineSepSize + fontSize)*_option.size();
	drawHeight += (_up->srcRect.bottom-_up->srcRect.top) + (_down->srcRect.bottom-_down->srcRect.top);
	showRect = MakeRect((WIDTH-_drawWidth)/2,(HEIGHT-drawHeight)/2,_drawWidth,drawHeight);
	drawRect = 
		MakeRect(showRect.left+ (_drawWidth-(_leftup->srcRect.right-_leftup->srcRect.left)-(_rightup->srcRect.right-_rightup->srcRect.left))/2,
		showRect.top+ (drawHeight-(_leftup->srcRect.bottom-_leftup->srcRect.top)-(_leftdown->srcRect.bottom-_leftdown->srcRect.top))/2,
		(_leftup->srcRect.right-_leftup->srcRect.left)+(_rightup->srcRect.right-_rightup->srcRect.left),
		(_leftup->srcRect.bottom-_leftup->srcRect.top)+(_leftdown->srcRect.bottom-_leftdown->srcRect.top));
	prepareState = 2;
	_calcImageRect();
	_up->efvct.ReleaseEffects();
	_up->efvct.AddEffect(new GUIEF_Stretch(&drawRect,10,showRect));
	if(_option.size()==0)
		selectedIndex = -1;
	else
		selectedIndex = 0;
}

void GUIMessageBox::ShowMenu(int x,int y)
{
	int drawHeight=0;
	if(_basicInfo!=40)
		drawHeight = fontSize*2;
	if(_option.size()!=0)
		drawHeight += (lineSepSize + fontSize)*_option.size() - lineSepSize;
	//if(drawHeight< (_up->srcRect.bottom-_up->srcRect.top) + (_down->srcRect.bottom-_down->srcRect.top))
	drawHeight += (_up->srcRect.bottom-_up->srcRect.top) + (_down->srcRect.bottom-_down->srcRect.top);
	showRect = MakeRect(x,y,_drawWidth,drawHeight);
	drawRect = 
		MakeRect(showRect.left+ (_drawWidth-(_leftup->srcRect.right-_leftup->srcRect.left)-(_rightup->srcRect.right-_rightup->srcRect.left))/2,
		showRect.top+ (drawHeight-(_leftup->srcRect.bottom-_leftup->srcRect.top)-(_leftdown->srcRect.bottom-_leftdown->srcRect.top))/2,
		(_leftup->srcRect.right-_leftup->srcRect.left)+(_rightup->srcRect.right-_rightup->srcRect.left),
		(_leftup->srcRect.bottom-_leftup->srcRect.top)+(_leftdown->srcRect.bottom-_leftdown->srcRect.top));
	prepareState = 2;
	_calcImageRect();
	_up->efvct.ReleaseEffects();
	_up->efvct.AddEffect(new GUIEF_Stretch(&drawRect,10,showRect));
	if(_option.size()==0)
		selectedIndex = -1;
	else
		selectedIndex = 0;
}

void GUIMessageBox::_calcImageRect()
{
	//if(prepareState!=1)
		//return;
	_leftup->drawRect = MakeRect(drawRect.left,drawRect.top,_leftup->srcRect.right-_leftup->srcRect.left,_leftup->srcRect.bottom-_leftup->srcRect.top);
	SetRect(&_rightup->drawRect,drawRect.right-(_rightup->srcRect.right-_rightup->srcRect.left), drawRect.top, drawRect.right, 
		drawRect.top + (_rightup->srcRect.bottom-_rightup->srcRect.top));
	_leftdown->drawRect = MakeRect(drawRect.left,drawRect.bottom-(_leftdown->srcRect.bottom-_leftdown->srcRect.top),
		_leftdown->srcRect.right-_leftdown->srcRect.left,_leftdown->srcRect.bottom-_leftdown->srcRect.top);
	SetRect(&_rightdown->drawRect, drawRect.right-(_rightdown->srcRect.right-_rightdown->srcRect.left), 
		drawRect.bottom-(_rightdown->srcRect.bottom-_rightdown->srcRect.top), drawRect.right,drawRect.bottom);
	SetRect(&_mid->drawRect,_leftup->drawRect.right,_leftup->drawRect.bottom,_rightdown->drawRect.left,_rightdown->drawRect.top);
	SetRect(&_up->drawRect,_leftup->drawRect.right,drawRect.top,_rightup->drawRect.left,_mid->drawRect.top);
	SetRect(&_left->drawRect,drawRect.left,_leftup->drawRect.bottom,_mid->drawRect.left,_leftdown->drawRect.top);
	SetRect(&_down->drawRect,_leftdown->drawRect.right,_mid->drawRect.bottom,_rightdown->drawRect.left,_leftdown->drawRect.bottom);
	SetRect(&_right->drawRect,_mid->drawRect.right,_rightup->drawRect.bottom,drawRect.right,_rightdown->drawRect.top);
}

void GUIMessageBox::_calcHighLight()
{
	if(selectedIndex<0) return;
	int tpos = _up->drawRect.bottom -
		((_highLightBack->middle->srcRect.bottom-_highLightBack->middle->srcRect.top) - fontSize)/2 - 2;
	if(_basicInfo!=40)
		tpos += fontSize * 2;
	if(_option.size()!=0)
		tpos += (lineSepSize+fontSize)*(selectedIndex);
	_highLightBack->SetDrawRect(MakeRect(drawRect.left + 4, tpos, drawRect.right-4-(drawRect.left + 4), 
		_highLightBack->middle->srcRect.bottom-_highLightBack->middle->srcRect.top));

	_highLight->SetDrawRect(MakeRect(drawRect.left + 4, tpos, drawRect.right-4-(drawRect.left + 4), 
		_highLightBack->middle->srcRect.bottom-_highLightBack->middle->srcRect.top));
}

void GUIMessageBox::Draw()
{
	if(prepareState<1)
		return;
	_leftup->Draw();
	_up->Draw();
	_rightup->Draw();
	_left->Draw();
	_mid->Draw();
	_right->Draw();
	_leftdown->Draw();
	_down->Draw();
	_rightdown->Draw();
	

	if(prepareState==3)
	{
		_highLightBack->Draw();
		_highLight->Draw();
		int tpos = _up->drawRect.bottom;
		graphEngine.SetFont(systemIni.font,fontSize);
		if(_basicInfo!=40)
		{
			graphEngine.DrawText(StringTable(_basicInfo), MakeRect(drawRect.left,tpos,drawRect.right-drawRect.left,fontSize), DT_CENTER,
				D3DCOLOR_ARGB(255,255,255,255));
			tpos+=fontSize*2;
		}
		if(_option.size()!=0)
		{
			for(int i=0;i<_option.size();i++)
			{
				if(i!=0) tpos+= lineSepSize;
				string text = (_option[i]>=0)?StringTable(_option[i]):_text[-_option[i]-1];
				graphEngine.DrawText(text, MakeRect(drawRect.left,tpos,drawRect.right-drawRect.left,fontSize), DT_CENTER,
					D3DCOLOR_ARGB(255,255,255,255));
				tpos+=fontSize;
			}
		}
	}
}

void GUIMessageBox::Update()
{
	if(prepareState<1)
		return;

	_leftup->Update();
	_up->Update();
	_rightup->Update();
	_left->Update();
	_mid->Update();
	_right->Update();
	_leftdown->Update();
	_down->Update();
	_rightdown->Update();
	if(prepareState==3)
	{
		_highLightBack->Update();
		_highLight->Update();
	}
	if(prepareState==2)
	{
		_calcImageRect();
		if(_up->efvct.effectnums==0)
		{
			if(p_OnMenuInitOver)
				p_OnMenuInitOver();
			_calcHighLight();
			prepareState=3;
		}
	}
	else if(prepareState==4)
	{
		_calcImageRect();
		if(_up->efvct.effectnums==0)
		{
			if(p_OnMenuSelectOver)
				p_OnMenuSelectOver(selectedIndex);
			if(p_OnMenuReleaseOver)
				p_OnMenuReleaseOver();
			prepareState=-1;
		}
	}
	else if(prepareState==5)
	{
		_calcImageRect();
		if(_up->efvct.effectnums==0)
		{
			if(p_OnMenuReleaseOver)
				p_OnMenuReleaseOver();
			prepareState=-1;
		}
	}
}


void GUIMessageBox::OnKeyEvent(unsigned int nChar,bool bKeyDown)
{
	if(prepareState!=3||(!bKeyDown))
		return;
	if(_option.size()==0)
	{
		showRect = 
		MakeRect(showRect.left+ (_drawWidth-(_leftup->srcRect.right-_leftup->srcRect.left)-(_rightup->srcRect.right-_rightup->srcRect.left))/2,
		showRect.top+ (showRect.bottom-showRect.top-(_leftup->srcRect.bottom-_leftup->srcRect.top)-(_leftdown->srcRect.bottom-_leftdown->srcRect.top))/2,
		(_leftup->srcRect.right-_leftup->srcRect.left)+(_rightup->srcRect.right-_rightup->srcRect.left),
		(_leftup->srcRect.bottom-_leftup->srcRect.top)-(_leftdown->srcRect.bottom-_leftdown->srcRect.top));

		_up->efvct.ReleaseEffects();
		_up->efvct.AddEffect(new GUIEF_Stretch(&drawRect,10,showRect));
		prepareState=5;
	}
	else if(nChar==KeyEnter)
	{
		if(p_OnMenuSelectBegin)
			p_OnMenuSelectBegin(selectedIndex);
		showRect = 
		MakeRect(showRect.left+ (_drawWidth-(_leftup->srcRect.right-_leftup->srcRect.left)-(_rightup->srcRect.right-_rightup->srcRect.left))/2,
		showRect.top+ (showRect.bottom-showRect.top-(_leftup->srcRect.bottom-_leftup->srcRect.top)-(_leftdown->srcRect.bottom-_leftdown->srcRect.top))/2,
		(_leftup->srcRect.right-_leftup->srcRect.left)+(_rightup->srcRect.right-_rightup->srcRect.left),
		(_leftup->srcRect.bottom-_leftup->srcRect.top)+(_leftdown->srcRect.bottom-_leftdown->srcRect.top));

		_up->efvct.ReleaseEffects();
		_up->efvct.AddEffect(new GUIEF_Stretch(&drawRect,10,showRect));
		prepareState=4;
	}
	else if(nChar==KeyDown)
	{
		selectedIndex++;
		if(selectedIndex==_option.size())
			selectedIndex=0;
		_calcHighLight();
		if(p_OnSelectedIndexChanged)
			p_OnSelectedIndexChanged(selectedIndex);
	}
	else if(nChar==KeyUp)
	{
		selectedIndex--;
		if(selectedIndex<0)
			selectedIndex=_option.size()-1;
		_calcHighLight();
		if(p_OnSelectedIndexChanged)
			p_OnSelectedIndexChanged(selectedIndex);
	}
}


void CGFileInfo::ReloadCGFileInfo()
{
	char szFind[1000];
	char szFindDirec[1000];
	GetCurrentDirectory(1000,szFind);
	strcat(szFind,"\\pic\\cg\\");
	strcpy(szFindDirec,szFind);
	strcat(szFind,"*.*");
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(szFind,&wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(wfd.cFileName[0]=='.')
				continue;
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{

				char szFile[1000];
				char szFileDirec[1000];
				strcpy(szFile,szFindDirec);
				strcat(szFile,wfd.cFileName);
				strcat(szFile,"\\");
				strcpy(szFileDirec,szFile);
				strcat(szFile,"*.*");
				WIN32_FIND_DATA wfdDiva;
				HANDLE hFindDiva = FindFirstFile(szFile,&wfdDiva);
				if(hFindDiva!=INVALID_HANDLE_VALUE)
				{
					do
					{
						char thisFname[1000];
						strcpy(thisFname,szFileDirec);
						strcat(thisFname,wfdDiva.cFileName);

						if(isImage(thisFname))
						{
							_illusion.push_back(Ansi2UTF8(wfd.cFileName));
							_cgFilename.push_back(thisFname);
						}
					}
					while(FindNextFile(hFindDiva,&wfdDiva));
					FindClose(hFindDiva);
				}
			}
		}
		while(FindNextFile(hFind,&wfd));
		FindClose(hFind);
	}
}