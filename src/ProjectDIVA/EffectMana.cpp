#include "EffectMana.h"
#include "GameMana.h"

EffectMana effectMana;
NumberMana numberMana;

void gen_table(Effect_params &out, float *in)
{
	int X = (int)in[1];
	float *size_table = new float [X+1];
	float *alpha_table = new float [X+1];
	for(int x=0; x<=X; x++)
	{// Effect parameters: start_frame end_frame initial_size final_size size_gamma initial_alpha final_alpha alpha_gamma
		size_table[x] = in[2]+pow((double)x/X,(double)in[4])*(in[3]-in[2]);		// 2 3 4
		alpha_table[x] = in[5]+pow((double)x/X,(double)in[7])*(in[6]-in[5]);	// 5 6 7
	}
	out.start_frame = in[0];
	out.stop_frame = in[1];
	out.size_table = size_table;
	out.alpha_table = alpha_table;
}

void EffectMana::Init()
{
	effect_note_in_rc = (ImageResource*)g_res.getResource("effect_note_in");
	effect_note_press_burst1 = (ImageResource*)g_res.getResource("effect_note_press_burst1[8]");
	effect_note_press_burst2 = (ImageResource*)g_res.getResource("effect_note_press_burst2[8]");
	effect_note_press_burst_cool = (ImageResource*)g_res.getResource("effect_note_press_burst_cool[12]");
	effect_note_press_rhyme1 = (ImageResource*)g_res.getResource("effect_note_press_rhyme1");
	effect_note_press_rhyme2 = (ImageResource*)g_res.getResource("effect_note_press_rhyme2");
	effect_note_press_aura = (ImageResource*)g_res.getResource("effect_note_press_aura");

	effect_aura_params = (FloatResource*)g_res.getResource("effect_aura_params[8]");
	effect_rhyme_params = (FloatResource*)g_res.getResource("effect_rhyme_params[8]");
	effect_ripple_params = (FloatResource*)g_res.getResource("effect_ripple_params[8]");

	gen_table(aura_param, effect_aura_params->value);
	gen_table(rhyme_param, effect_rhyme_params->value);
	gen_table(ripple_param, effect_ripple_params->value);

	numTex = graphEngine.AddTexture("pic\\num.png");
}

void EffectMana::AddEffectNoteIn(int x, int y, int *color)
{
	auto new_effect = new Effect_SingleFrame(x,y,ripple_param,effect_note_in_rc);
	effects.push_back((Effect_Base*)new_effect);
	if(color)
		new_effect->color = D3DCOLOR_ARGB(255,color[0],color[1],color[2]);
}

void EffectMana::AddEffectNotePress(int x,int y,PLAYSTATE id)
{
	if(id==PLAYSTATE::SAFE||id==PLAYSTATE::FINE||id==PLAYSTATE::COOL)
	{
		effects.push_back((Effect_Base*)(new Effect_SingleFrame(x,y,rhyme_param,
			(GAMEDATA.combo<100)?effect_note_press_rhyme1:effect_note_press_rhyme2)));
		effects.push_back((Effect_Base*)(new Effect_SingleFrame(x,y,aura_param,effect_note_press_aura)));
	}
	if(id==PLAYSTATE::COOL)
		effects.push_back((Effect_Base*)(new Effect_MultipleFrame(x,y,16,
			(GAMEDATA.combo<100)?effect_note_press_burst1:effect_note_press_burst2)));
	if(id==PLAYSTATE::FINE)
		effects.push_back((Effect_Base*)(new Effect_MultipleFrame(x,y,24,effect_note_press_burst_cool)));
}

void EffectMana::SetNew()
{
	Clear();
}

void EffectMana::Clear()
{
	for(auto ptr = effects.begin();ptr!=effects.end();ptr++)
		delete (*ptr);
	effects.clear();
	numberMana.Clear();
}

void EffectMana::Draw()
{
	for(auto ptr = effects.begin();ptr!=effects.end();ptr++)
		(*ptr)->Draw();
	numberMana.DrawCombo();
}

void EffectMana::Update(float dwTimeMilliSecond)
{
	for(auto ptr = effects.begin();ptr!=effects.end();)
	{
		if(!(*ptr)->Update(dwTimeMilliSecond))
		//if((*ptr)->Die())
		{
			delete (*ptr);
			ptr=effects.erase(ptr);
			continue;
		}
		ptr++;
	}

	numberMana.Update(dwTimeMilliSecond);
}

void Effect_SingleFrame::Draw()
{
	if(nowframe<0) return;
	float delta = size_table[(int)nowframe];
	((BYTE*)&color)[3] = (BYTE)alpha_table[(int)nowframe];
	graphEngine.Draw(
		_image_rc->GetTexture(),
		_image_rc->srcRect,
		MakefRect(_x-delta,_y-delta,delta*2,delta*2),
		color
	);
}

bool Effect_SingleFrame::Update(float dwTimeMilliSecond)
{
	nowframe += dwTimeMilliSecond;
	return nowframe<desframe;
}

void Effect_MultipleFrame::Draw()
{
	if(nowframe<0) return;
	fRECT dest_rect = {_x,_y,_x,_y};
	graphEngine.Draw(
		_image_rc->GetTexture(),
		_image_rc[((int)(nowframe-0.5f))>>1].srcRect,
		dest_rect += _image_rc->dstRect,
		_image_rc->color
	);
}

bool Effect_MultipleFrame::Update(float dwTimeMilliSecond)
{
	nowframe += dwTimeMilliSecond;
	return nowframe<desframe;
}

void Effect_ExtraScore::Draw()
{
	if(_extraScore!=0)
	{
		graphEngine.Draw(_drawTex,MakeRect(0,58,15,14),_x-29,_y-35);
		numberMana.Draw(_extraScore,MakeRect(_x-14,_y-35,0,0),NumberStyle::EXTRASCORE,NumberDwFlags::DWFLAG_LEFT,D3DCOLOR_ARGB(255,255,255,255),0);
	}
}

bool Effect_ExtraScore::Update(float dwTimeMilliSecond)
{
	nowframe += dwTimeMilliSecond;
	_y--;
	return nowframe<desframe;
}

void Effect_ChanceTimeEnd::Draw()
{
	int drawAlpha;
	if(nowframe<20)
		drawAlpha = int(nowframe/20.0f*255.0f);
	else if(nowframe<160)
		drawAlpha = 255;
	else
		drawAlpha = int((180-nowframe)/20.0f*255.0f);
	graphEngine.Draw(_drawTex,MakeRect(0,128,368,24),56,225,D3DCOLOR_ARGB(drawAlpha,255,255,255));
	numberMana.Draw(_chanceTimeScore,MakeRect(56,208,368,0),NumberStyle::CHANCE,NumberDwFlags::MID,D3DCOLOR_ARGB(drawAlpha,255,255,255),0);
}

bool Effect_ChanceTimeEnd::Update(float dwTimeMilliSecond)
{
	nowframe += dwTimeMilliSecond;
	return nowframe<desframe;
}

void Effect_ChanceTimeStart::Draw()
{
	int drawAlpha;
	if(nowframe<20)
		drawAlpha = int(nowframe/20.0f*255.0f);
	else if(nowframe<160)
		drawAlpha = 255;
	else
		drawAlpha = int((180-nowframe)/20.0f*255.0f);
	graphEngine.Draw(_drawTex,MakeRect(0,104,368,24),56,225,D3DCOLOR_ARGB(drawAlpha,255,255,255));
}

bool Effect_ChanceTimeStart::Update(float dwTimeMilliSecond)
{
	nowframe += dwTimeMilliSecond;
	return nowframe<desframe;
}

void NumberMana::Draw(int number, const RECT &drawRect, NumberStyle style, NumberDwFlags flag,D3DCOLOR color,int inc,int fill)
{
	int srcWidth,srcHeight,srcLeft,srcTop,deltaWidth;
	switch(style)
	{
	case NumberStyle::COMBO:
		srcWidth=18,srcHeight=18,srcLeft=0,srcTop=0,deltaWidth=16;
		break;
	case NumberStyle::SCORE:
		srcWidth=18,srcHeight=18,srcLeft=0,srcTop=21,deltaWidth=13;
		break;
	case NumberStyle::CHANCE:
		srcWidth=15,srcHeight=14,srcLeft=0,srcTop=41,deltaWidth=12;
		break;
	case NumberStyle::EXTRASCORE:
		srcWidth=15,srcHeight=14,srcLeft=15,srcTop=58,deltaWidth=12;
		break;
	}
	vector<int> drawNum;
	if(number==0)
		drawNum.push_back(0);
	while(number)
	{
		drawNum.push_back(number%10);
		number/=10;
	}
	fill-=drawNum.size();
	for(int i=0;i<fill;i++)
		drawNum.push_back(0);
	switch(flag)
	{
	case NumberDwFlags::DWFLAG_LEFT:
		for(int i=drawNum.size()-1;i>=0;i--)
			graphEngine.Draw(numTex,MakeRect(srcLeft+((drawNum[i]==0?10:drawNum[i])-1)*srcWidth,srcTop,srcWidth,srcHeight),
				MakefRect(drawRect.left+(drawNum.size()-i-1)*deltaWidth-inc,drawRect.top-inc,srcWidth+inc*2,srcHeight+inc*2),color);
		break;
	case NumberDwFlags::DWFLAG_RIGHT:
		for(int i=0;i<drawNum.size();i++)
			graphEngine.Draw(numTex,MakeRect(srcLeft+((drawNum[i]==0?10:drawNum[i])-1)*srcWidth,srcTop,srcWidth,srcHeight),
				MakefRect(drawRect.right-(i+1)*deltaWidth-inc,drawRect.top-inc,srcWidth+inc*2,srcHeight+inc*2),color);
		break;
	case NumberDwFlags::MID:
		for(int i=drawNum.size()-1;i>=0;i--)
			graphEngine.Draw(numTex,MakeRect(srcLeft+((drawNum[i]==0?10:drawNum[i])-1)*srcWidth,srcTop,srcWidth,srcHeight),
				MakefRect((drawRect.left+drawRect.right)/2-srcWidth*drawNum.size()/2+(drawNum.size()-i-1)*deltaWidth-inc,drawRect.top-inc,srcWidth+inc*2,srcHeight+inc*2),color);
		break;
	}
}

void NumberMana::Init()
{
	numTex = graphEngine.AddTexture("pic\\num.png");
	comboDraw = NULL;
	extraScoreDraw = NULL;
	_nowDrawScore = _nowScore = 0;
	score_dash = (ImageResource*)g_res.getResource("score_dash");
	IntResource* score_data = (IntResource*)g_res.getResource("score_data[10]");
	int *ptr = score_data->value;
	score_rect = MakeRect(ptr[0],ptr[1],ptr[2],ptr[3]);
	score_color = D3DCOLOR_RGBA(ptr[4],ptr[5],ptr[6],ptr[7]);
	score_inc = ptr[8];
	score_fill = ptr[9];
}

void NumberMana::SetNew()
{
	score_state = 0;
}

void NumberMana::DrawScore(int score)
{
	_nowScore = score;
	numberMana.Draw(_nowDrawScore,score_rect,NumberStyle::SCORE,NumberDwFlags::DWFLAG_LEFT,score_color,score_inc,score_fill);
	if(score_state&3)
		graphEngine.Draw(score_dash);
}

void NumberMana::SetCombo(int combo,int x,int y)
{
	SAFE_DELETE(comboDraw);
	comboDraw = new Effect_Combo(x,y,combo,numTex);
}

void NumberMana::SetExtraScore(int extraScore,int x,int y)
{
	SAFE_DELETE(extraScoreDraw);
	extraScoreDraw = new Effect_ExtraScore(x,y,extraScore,numTex);
}

void NumberMana::DrawCombo()
{
	if(comboDraw)
		comboDraw->Draw();
	if(extraScoreDraw)
		extraScoreDraw->Draw();
}

void NumberMana::Update(float dwTimeMilliSecond)
{
	if(comboDraw)
	{
		if(!comboDraw->Update(dwTimeMilliSecond))
		//if(comboDraw->Die())
			SAFE_DELETE(comboDraw);
	}
	if(extraScoreDraw)
	{
		if(!extraScoreDraw->Update(dwTimeMilliSecond))
		//if(extraScoreDraw->Die())
			SAFE_DELETE(extraScoreDraw);
	}
	if(_nowDrawScore != _nowScore)
	{
		if(_nowScore<_nowDrawScore||_nowScore - _nowDrawScore <= 5)
			_nowDrawScore = _nowScore;
		else
			_nowDrawScore += (_nowScore-_nowDrawScore) * 1 / 3;
	}
}

void NumberMana::Clear()
{
	_nowDrawScore = _nowScore = 0;
	SAFE_DELETE(comboDraw);
	SAFE_DELETE(extraScoreDraw);
}

void Effect_Combo::Draw()
{
	if(_combo>4)
	{
		int nowA,nowInc;
		if(nowframe<3)
			nowInc=0,nowA=255;
		else if(nowframe<10)
			nowInc=4,nowA=255;
		else if(nowframe<30)
			nowInc=0,nowA=255;
		else
			nowInc=0,nowA=int(255.0f*(desframe-nowframe)/10.0f);
		graphEngine.Draw(_drawTex,MakeRect(4,77,97,15),_x-45,_y-24,D3DCOLOR_ARGB(nowA,255,255,255));
		graphEngine.Draw(_drawTex,MakeRect(157,77,38,9),_x+10,_y-24,D3DCOLOR_ARGB(nowA,255,255,255));
		numberMana.Draw(_combo,MakeRect(_x+10,_y-30,0,0),NumberStyle::COMBO,NumberDwFlags::DWFLAG_RIGHT,D3DCOLOR_ARGB(nowA,255,255,255),nowInc);
	}
}

bool Effect_Combo::Update(float dwTimeMilliSecond)
{
	nowframe += dwTimeMilliSecond;
	return nowframe<desframe;
}
