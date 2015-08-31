#include "defines.h"
#include "UIScreen.h"
#include "Base.h"
#include "mplayer.h"
#include "Gui.h"
#include "resource.hpp"
#include <iostream>
#include <sstream>
using namespace base;
using namespace std;

SplashScreen *logo1,*logo2,*logo3,*logo4,*loading;

GameCore core;
GUIMessageBox *pausedmenu;

bool Keys[256] = {0};
int ScreenState;
void (*LoadingToDo)() = NULL;
void (*LoadingOver)() = NULL;

MainMenu *menu;

void loading_InitOver()
{
	ScreenState = SCREEN_LOADING;
	if(LoadingToDo)
		LoadingToDo();
	loading->End();
}
void loading_SplashOut()
{
	if(LoadingOver)
		LoadingOver();
}


void beginSongInit()
{
	core.SetNew(
		menu->songInfo.songs[menu->songlb->btns->GetSelectedIndex()].songFName[menu->selectSong->btns->GetSelectedIndex()],
		menu->songInfo.songs[menu->songlb->btns->GetSelectedIndex()].songFNameBase,
		menu->selectSong->playMode
	);
}
void beginSongOver()
{
	ScreenState = SCREEN_GAMEMAIN;
}


void logo1_SplashOut()
{
	ScreenState = SCREEN_LOGO2;
	logo2->Begin();
}
void logo2_SplashOut()
{
	ScreenState = SCREEN_LOGO3;
	logo3->Begin();
}
void logo3_SplashOut()
{
	ScreenState = SCREEN_LOGO4;
	logo4->Begin();
	soundEngine->PlaySound("gamedata\\SEGA.wav");
}

void menu_mainlb_OnButtonClick(int id)
{
	switch(id){
	case 0:
		if(menu->songlb->btns->GetButtonNum()>0)
		{
			menu->MainStGoto = MAINST_FREE;
			menu->bottom->MainMenuStateChanged( MAINST_FREE );
			menu->mainDetail->efvct.ReleaseEffects();
			menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,0));
			menu->mainlb->GetOut();
			soundEngine->StopMusic(0,true);
			soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
			soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
		}
		break;
	case 1:
		menu->MainStGoto = MAINST_MPLAY;
		menu->mainDetail->efvct.ReleaseEffects();
		menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,0));
		menu->mainlb->GetOut();
		soundEngine->StopMusic(0,true);
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
		break;
	case 2:
		menu->MainStGoto = MAINST_RUBIX;
		menu->mainDetail->efvct.ReleaseEffects();
		menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,0));
		menu->mainlb->GetOut();
		soundEngine->StopMusic(0,true);
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
		break;
	case 3:
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		ShellExecute(NULL, "open", "http://map.divaol.com", NULL, NULL, SW_SHOWNORMAL);
		break;
	case 4:
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		ShellExecute(NULL, "open", "http://www.divaol.com", NULL, NULL, SW_SHOWNORMAL);
		break;
	case 5:
		menu_configlb_RefreshDetail();
		menu->MainStGoto = MAINST_CONFIG;
		menu->mainDetail->efvct.ReleaseEffects();
		menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,0));
		menu->mainlb->GetOut();
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
		break;
	case 6:
		menu->MainStGoto = MAINST_CG;
		menu->bottom->MainMenuStateChanged( MAINST_CG );
		menu->mainDetail->efvct.ReleaseEffects();
		menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,0));
		menu->mainlb->GetOut();
		soundEngine->StopMusic(0,true);
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
		break;
	case 7:
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		MessageBoxW(base::hwnd, Ansi2WChar(StringTable(102)).data(), Ansi2WChar(StringTable(103)).data(), MB_OK);
		break;
	case 8:
		menu->MainStGoto = MAINST_QUIT;
		menu->bottom->MainMenuStateChanged( MAINST_QUIT );
		menu->mainDetail->efvct.ReleaseEffects();
		menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,0));
		menu->mainlb->GetOut();
		soundEngine->StopMusic(0,true);
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
		break;
	}
}
void menu_mainlb_OnSelectedIndexChanged(int id)
{
	switch(id)
	{
	case 0:
		id = (menu->songlb->btns->GetButtonNum()<=0)?3:4;
		break;
	case 1: id=135; break;
	case 2: id=114; break;
	case 3: id=5; break;
	case 4: id=6; break;
	case 5: id=7; break;
	case 6: id=8; break;
	case 7: id=9; break;
	case 8: id=10; break;
	case 9: id=11; break;
	}
	menu->mainDetailWords_index = id;
	soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
}
void menu_mainlb_GetInOver()
{
	menu->mainDetail->efvct.ReleaseEffects();
	menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,255));
	RegisterDragDrop(hwnd, &mplayer);
}
void menu_songlb_GetOutOver()
{
	LoadingToDo = beginSongInit;
	LoadingOver = beginSongOver;
	loading->Begin();
	RevokeDragDrop(hwnd);
}
void menu_songlb_OnSelectedIndexChanged(int id)
{
	soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
}

void menu_songlb_GetInOver()
{
	menu->songPicBack->efvct.ReleaseEffects();
	menu->blackBack->efvct.ReleaseEffects();
	menu->songPicBack->efvct.AddEffect(new GUIEF_Alpha(menu->songPicBack->efvct.a,25,255));
	menu->blackBack->efvct.AddEffect(new GUIEF_Alpha(menu->blackBack->efvct.a,25,255));
}

bool songLbButtonFirstClick=false;
void menu_songlb_OnButtonClick(int id)
{
	if(id>=0)
	{
		if(menu->songInfo.songs[id].BPM==0){
			string name = menu->songInfo.songs[id].noter;
			if(name=="..\\"){
				SongInfoManager::currPath.resize(SongInfoManager::currPath.length()-1);
				int posi = SongInfoManager::currPath.rfind('\\');
				SongInfoManager::currPath.resize(posi+1);
			}else{
				SongInfoManager::currPath+=name;
			}
			menu->songlb->bRefresh = true;
		}else{
			songLbButtonFirstClick=true;
			menu->songlb->BtnSleep();
			menu->selectSong->ResetButton(menu->songInfo.songs[id]);
			menu->MainMenuState = MAINST_SELECTSONG;
			soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
			songLbButtonFirstClick=false;
			menu->bottom->MainMenuStateChanged( MAINST_SELECTSONG );
			menu->bottom->MainMenuStateChangedEnd();
		}
	}
}
void menu_selectSong_OnButtonClick(int id)
{
	if(id!=-3)
	{
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		soundEngine->PlaySound("gamedata\\Fly.wav",systemIni.sndVolume);
		if(id==-2)
			menu->selectSong->playMode = GAME_MODE_AUTO;
		else if(id==-1)
			menu->selectSong->playMode = GAME_MODE_PV;
		else
			menu->selectSong->playMode = GAME_MODE_NORMAL;
		menu->songlb->BtnWakeUp();
		menu->songlb->GetOutOver = menu_songlb_GetOutOver;
		menu->songlb->GetOut();
		menu->songPicBack->efvct.ReleaseEffects();
		menu->blackBack->efvct.ReleaseEffects();
		menu->songPicBack->efvct.AddEffect(new GUIEF_Alpha(menu->songPicBack->efvct.a,25,0));
		menu->blackBack->efvct.AddEffect(new GUIEF_Alpha(menu->blackBack->efvct.a,25,0));
		menu->selectSong->UnsetHighLight();
		soundEngine->StopMusic(1,true);
	}
}
void menu_selectSong_OnEscClick()
{
	menu->songlb->BtnWakeUp();
	menu->MainMenuState = MAINST_FREE;
	menu->bottom->MainMenuStateChanged( MAINST_FREE );
	menu->bottom->MainMenuStateChangedEnd();
}
void menu_selectSong_OnSelectedIndexChanged(int id)
{
	if(!songLbButtonFirstClick)
		soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
}
string getCenterText(vector <string> &text, int posi)
{
	ostringstream oss;
	int start = max(0, posi-6);
	int end = min(text.size(), start+13);
	string original = text[posi];
	text[posi] = ">"+text[posi]+"<";
	if(start>0)
		oss << "...[" << start << "]..." << endl;
	oss << join(vector <string> (text.begin()+start, text.begin()+end), "\n");
	if(end<text.size())
		oss << endl << "...[" << (text.size()-end) << "]...";
	text[posi] = original;
	return oss.str();
}
void RefreshConfigDetail()
{
	int keytype;
	ostringstream oss;
	switch(menu->configlb->btns->GetSelectedIndex()){
	case 0:
		menu->configDetailWords_string = StringTable(44) + "\n"
			+ getCenterText(systemIni.GetLanguageNames(), systemIni.language);
		return;
	case 1:
		keytype = systemIni.GameKeyType;
		for(int x=0; x<8; x++)
			oss << g_button_symbols[x] << " : " << printKeys(7-x,40) << "\n";
		if(keytype==key_mapsN)
			oss << StringTable(46);
		menu->configDetailWords_string = oss.str();
		return;
	case 2:
		oss << StringTable(11);
		for(int x=0; x<systemIni.window_sizes.size(); x++)
			oss << "\n" << systemIni.window_sizes[x].first << g_button_symbols[5] << systemIni.window_sizes[x].second;
		oss << "\n" << GetSystemMetrics(SM_CXSCREEN) << g_button_symbols[5] << GetSystemMetrics(SM_CYSCREEN) << StringTable(36);
		menu->configDetailWords_string = oss.str();
		return;
	case 3:	menu->configDetailWords_index = 138;break;
	case 4:	menu->configDetailWords_index = 12;	break;
	case 5:	menu->configDetailWords_index = 45;	break;
	case 6: menu->configDetailWords_index = systemIni.lowVideo?82:79;	break;
	case 7: menu->configDetailWords_index = 81;	break;
	case 8:	menu->configDetailWords_index = 70;	break;
	case 9: menu->configDetailWords_index = 77;	break;
	case 10:	menu->configDetailWords_index = 69;	break;
	case 11:	// font
		menu->configDetailWords_string = StringTable(88) + "\n"
			+ getCenterText(systemIni.fonts_utf8, systemIni.fontIndex);
		return;
	case 12:
		systemIni.getAntiAliasingInfo();
		oss << StringTable(92);
		if(systemIni.antialiasing==0){
			oss << StringTable(97);
		}else{
			auto aa = systemIni.multisampling[systemIni.antialiasing-1];
			oss << (int)aa.first << "_SAMPLES Quality=" << aa.second;
		}
		menu->configDetailWords_string = oss.str();
		return;
	case 13:menu->configDetailWords_index = 96;		break;
	case 14:menu->configDetailWords_index = 101;	break;
	case 15:menu->configDetailWords_index = 108;	break;
	case 16:menu->configDetailWords_index = 113;	break;
	case 17:menu->configDetailWords_index = 13;		break;
	}
	menu->configDetailWords_string.clear();
}
void menu_configlb_RefreshDetail()
{
	string KeyType[] = {
		"PC",  "PSP", "O2Jam-PC", "O2Jam-PSP", StringTable(83)
	};
	static int particle_text_ids[] = {57,58,59,60};
	static int FPS_state_ids[] = {66,67,68};
	static int pause_method_ids[] = {74,75,76};
	static int full_screen[] = { 136, 137 };

	ostringstream oss;
	oss << window_rect.right << g_button_symbols[5] << window_rect.bottom;
	string s_window_size = oss.str();

	// set 2nd entry
	menu->configlb->btns->GetButton(0)->text2_string = StringTable(0);
	menu->configlb->btns->GetButton(1)->text2_string = KeyType[systemIni.GameKeyType];
	menu->configlb->btns->GetButton(2)->text2_string = s_window_size;
	menu->configlb->btns->GetButton(3)->text2_string = StringTable(full_screen[systemIni.windowed]);
	menu->configlb->btns->GetButton(4)->text2_string = ToString(systemIni.soundVolume) + "/10";
	menu->configlb->btns->GetButton(5)->text2_string = StringTable(particle_text_ids[systemIni.particle]);
	menu->configlb->btns->GetButton(6)->text2_string = ToString(systemIni.lowVideo);
	menu->configlb->btns->GetButton(7)->text2_string = ToString(systemIni.dataSet);
	menu->configlb->btns->GetButton(8)->text2_string = ToString(systemIni.keyHook);
	menu->configlb->btns->GetButton(9)->text2_string = StringTable(pause_method_ids[systemIni.autoPause]);
	menu->configlb->btns->GetButton(10)->text2_string = StringTable(FPS_state_ids[systemIni.showFPS]);
	menu->configlb->btns->GetButton(11)->text2_string = systemIni.font_utf8;
	menu->configlb->btns->GetButton(12)->text2_string = ToString(systemIni.antialiasing);
	menu->configlb->btns->GetButton(13)->text2_string = StringTable(94+systemIni.judgement);
	menu->configlb->btns->GetButton(14)->text2_string = ToString(systemIni.DelayCompen);
	menu->configlb->btns->GetButton(15)->text2_string = StringTable(systemIni.KeepPitch?64:65);
	menu->configlb->btns->GetButton(16)->text2_string = (systemIni.sound_system==0?"FMOD":"BASS");

	// key hook start failed
	if(systemIni.keyHook==1 && !base::keyboard_thread_id)
		MessageBoxW(hwnd, Ansi2WChar(StringTable(72)).data(), NULL, MB_OK|MB_ICONEXCLAMATION);
}
void menu_configlb_OnLeftRightClick(int id, int amount)
{
	if(id>=0 && id<cfg_menu_option_id.size())
	{
		id = cfg_menu_option_id[id];
		Option &option = options[id];
		int old_value = *option.pValue;
		(*option.pValue) += amount;
		if(*option.pValue < option.min_val)
			*option.pValue = option.min_val;
		else if(*option.pValue > option.max_val)
			*option.pValue = option.max_val;
		if(*option.pValue!=old_value)
		{
			(systemIni.*(option.ChangeSettings))(*option.pValue,true);
			soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
			menu_configlb_RefreshDetail();
			RefreshConfigDetail();
		}
	}
}

void menu_configlb_OnButtonClick(int id)
{
	id = cfg_menu_option_id[id];
	Option &option = options[id];
	if(option.EnterSettings!=NULL)
		(systemIni.*(option.EnterSettings))(*option.pValue,true);
	menu_configlb_RefreshDetail();
	RefreshConfigDetail();
}

void menu_configlb_OnSelectedIndexChanged(int id)
{
	RefreshConfigDetail();
	soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
}
void menu_configlb_GetInOver()
{
	menu->mainDetail->efvct.ReleaseEffects();
	menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,255));
}



void menu_cgmenu_load()
{
	menu->cgmenu->PrepareCG();
	menu->cgmenu->EnterCGMenu();
	ScreenState = SCREEN_MAIN;
	soundEngine->PlayMusic("gamedata\\cg.mp3",1,true);
}

void menu_ListBoxGetOutOver()
{
	ScreenState = SCREEN_MAIN;
	int lastMenuState = menu->MainMenuState;
	menu->MainMenuState = menu->MainStGoto;
	menu->bottom->MainMenuStateChangedEnd();
	if(menu->MainStGoto==MAINST_MAIN)
	{
		menu->mainlb->GetIn();
		if(lastMenuState!=MAINST_CONFIG)
			soundEngine->PlayMusic("gamedata\\mainmenu.mp3",0,true);
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
	}
	else if(menu->MainStGoto==MAINST_FREE)
	{
		menu->songlb->GetIn();
		soundEngine->PlayMusic("gamedata\\selectSong.mp3",1,true);
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
	}
	else if(menu->MainStGoto==MAINST_MPLAY)
	{
		ScreenState==SCREEN_MPLAYER;
		menu->MainStGoto=MAINST_MAIN;
	}
	else if(menu->MainStGoto==MAINST_RUBIX)
	{
		pRubixWindow = new RubixWindow();
		pRubixWindow->Init(hwnd, Device, &hDlg);
		pRubixWindow->OnReturn = &menu_ListBoxGetOutOver;
		menu->MainStGoto=MAINST_MAIN;
	}
	else if(menu->MainStGoto==MAINST_CG)
	{
		loading->Begin();
		LoadingToDo = menu_cgmenu_load;
		LoadingOver = NULL;
	}
	else if(menu->MainStGoto==MAINST_CONFIG)
	{
		menu->configlb->GetIn();
		soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
	}
	else if(menu->MainStGoto==MAINST_QUIT)
	{
		soundEngine->StopMusic(0,true);
		PostMessage(base::hwnd,WM_QUIT,0,0);
	}
}

void menu_cgmenu_BackToMain()
{
	menu->cgmenu->ReleaseCG();
	menu->MainStGoto = MAINST_MAIN;
	menu->bottom->MainMenuStateChanged( MAINST_MAIN );
	menu_ListBoxGetOutOver();
}
void menu_cgmenu_OnEscClick()
{
	loading->Begin();
	LoadingToDo = menu_cgmenu_BackToMain;
	LoadingOver = NULL;
	soundEngine->StopMusic(1,true);
}

void menu_configlb_EscClick()
{
	menu->songlb->GetOutOver = menu_ListBoxGetOutOver;
	menu->MainStGoto = MAINST_MAIN;
	menu->bottom->MainMenuStateChanged( MAINST_MAIN );
	menu->mainDetail->efvct.ReleaseEffects();
	menu->mainDetail->efvct.AddEffect(new GUIEF_Alpha(menu->mainDetail->efvct.a,25,0));
	menu->configlb->GetOut();

	soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
}

void menu_songlb_EscClick()
{
	menu->songlb->GetOutOver = menu_ListBoxGetOutOver;
	menu->MainStGoto = MAINST_MAIN;
	menu->bottom->MainMenuStateChanged( MAINST_MAIN );
	menu->songlb->GetOut();
	menu->songPicBack->efvct.ReleaseEffects();
	menu->blackBack->efvct.ReleaseEffects();
	menu->songPicBack->efvct.AddEffect(new GUIEF_Alpha(menu->songPicBack->efvct.a,25,0));
	menu->blackBack->efvct.AddEffect(new GUIEF_Alpha(menu->blackBack->efvct.a,25,0));
	soundEngine->StopMusic(1,true);
	soundEngine->PlaySound("gamedata\\Sweep.wav",systemIni.sndVolume);
}

bool scoreMenuButtonFirstClick=false;
void menu_scoreMenu_OnButtonClick(int id)
{
	if(id==0)
	{
		if(!loading->_begin)
		{
			menu_songlb_GetOutOver();
			soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
		}
	}
	else if(id==1)
	{
		menu->MainStGoto = MAINST_FREE;
		menu->bottom->MainMenuStateChanged( MAINST_FREE );
		LoadingToDo = menu_ListBoxGetOutOver;
		LoadingOver = NULL;
		loading->Begin();
		soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
	}
}

void menu_scoreMenu_OnSelectedIndexChanged(int id)
{
	if(!scoreMenuButtonFirstClick)
	{
		soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
	}
	else
		scoreMenuButtonFirstClick = false;
}

void PausedMenuSelectOver(int id)
{
	if(id==0)
		core.PauseGame(false);
	else if(id==1)
	{
		core.Clear();
		menu_songlb_GetOutOver();
	}
	else
	{
		core.Clear();
		menu->MainStGoto = MAINST_FREE;
		menu->bottom->MainMenuStateChanged( MAINST_FREE );
		LoadingToDo = menu_ListBoxGetOutOver;
		LoadingOver = NULL;
		loading->Begin();
	}
}

void pausedMenu_OnSelectedIndexChanged(int id)
{
	soundEngine->PlaySound("gamedata\\Button.wav",systemIni.sndVolume);
}

void pausedMenu_OnMenuSelectBegin(int id)
{
	soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
}

void MainFirstInit()
{
	graphEngine.ReleaseTex(2);
	//_notemap.Init();
	menu = new MainMenu();
	RefreshConfigDetail();
	menu->mainlb->GetOutOver = menu_ListBoxGetOutOver;
	menu->mainlb->GetInOver = menu_mainlb_GetInOver;
	menu->mainlb->btns->p_OnButtonClick = menu_mainlb_OnButtonClick;
	menu->mainlb->btns->p_OnSelectedIndexChanged = menu_mainlb_OnSelectedIndexChanged;

	menu->songlb->btns->p_EscClick = menu_songlb_EscClick;
	menu->songlb->btns->p_OnButtonClick = menu_songlb_OnButtonClick;
	menu->songlb->GetInOver = menu_songlb_GetInOver;
	menu->songlb->btns->p_OnSelectedIndexChanged = menu_songlb_OnSelectedIndexChanged;

	menu->configlb->GetOutOver = menu_ListBoxGetOutOver;
	menu->configlb->GetInOver = menu_configlb_GetInOver;
	menu->configlb->btns->p_EscClick = menu_configlb_EscClick;
	menu->configlb->btns->p_OnButtonClick = menu_configlb_OnButtonClick;
	menu->configlb->btns->p_OnSelectedIndexChanged = menu_configlb_OnSelectedIndexChanged;
	menu->configlb->btns->p_OnLeftClick = menu_configlb_OnLeftRightClick;
	menu->configlb->btns->p_OnRightClick = menu_configlb_OnLeftRightClick;

	menu->selectSong->btns->p_OnButtonClick = menu_selectSong_OnButtonClick;
	menu->selectSong->btns->p_EscClick = menu_selectSong_OnEscClick;
	menu->selectSong->btns->p_OnSelectedIndexChanged = menu_selectSong_OnSelectedIndexChanged;

	menu->scoreMenu->btns->p_OnButtonClick = menu_scoreMenu_OnButtonClick;
	menu->scoreMenu->btns->p_OnSelectedIndexChanged = menu_scoreMenu_OnSelectedIndexChanged;
	menu->cgmenu->OnEscClick = menu_cgmenu_OnEscClick;
	ScreenState = SCREEN_MAIN;
	menu->bottom->MainMenuStateChanged( MAINST_MAIN );
	menu->bottom->MainMenuStateChangedEnd();
	soundEngine->PlayMusic("gamedata\\mainmenu.mp3",0,true);

	pausedmenu = new GUIMessageBox();
	pausedmenu->PrepareMenu(14,120);
	pausedmenu->AddItem(15);
	pausedmenu->AddItem(16);
	pausedmenu->AddItem(17);
	pausedmenu->p_OnMenuSelectOver = PausedMenuSelectOver;
	pausedmenu->p_OnSelectedIndexChanged = pausedMenu_OnSelectedIndexChanged;
	pausedmenu->p_OnMenuSelectBegin = pausedMenu_OnMenuSelectBegin;

	core.SetPausedMenu(pausedmenu);
}

void logo4_SplashOut()
{
	ScreenState = SCREEN_LOADING;
	loading->Begin();
	LoadingToDo = MainFirstInit;
	LoadingOver = NULL;
}

void BackToUIScreen(int coolCnt, int fineCnt, int safeCnt, int sadCnt, int worstCnt, int comboCnt, int chancetimeCnt, int scoreCnt, bool ifclear,int maxScoreCanGet)
{
	graphEngine.ReleaseTex(GameCoreRes);
	scoreMenuButtonFirstClick = true;
	menu->scoreMenu->ResetResult(coolCnt,fineCnt,safeCnt,sadCnt,worstCnt,comboCnt,chancetimeCnt,scoreCnt,ifclear,maxScoreCanGet,
		menu->songInfo.songs[menu->songlb->btns->GetSelectedIndex()].songName+" - "
			+StringTable(HardType[menu->selectSong->btns->GetSelectedIndex()]) + 
			(menu->selectSong->playMode==GAME_MODE_AUTO?"\nAuto":""));

	if(menu->selectSong->playMode == GAME_MODE_NORMAL)
		menu->songInfo.songs[menu->songlb->btns->GetSelectedIndex()].GetResult(scoreCnt,comboCnt,menu->scoreMenu->result,
			menu->selectSong->btns->GetSelectedIndex(),menu->scoreMenu->ifClear);

	if(menu->selectSong->playMode != GAME_MODE_PV)
	{
		ScreenState = SCREEN_MAIN;
		menu->MainMenuState = MAINST_SCORE;
	}
	else
	{
		ScreenState = SCREEN_LOADING;
		menu->MainStGoto = MAINST_FREE;
		menu->bottom->MainMenuStateChanged( MAINST_FREE );
		LoadingToDo = menu_ListBoxGetOutOver;
		LoadingOver = NULL;
		loading->Begin();
	}
}

void InitUIScreen()
{
	srand(time(0));
	core.Init();

	loading = new SplashScreen(NULL,10000000);
	loading->isloading = true;
	loading->InitOver = loading_InitOver;
	loading->SplashOut = loading_SplashOut;

	logo1 = new SplashScreen(new GUIStaticImage("logo1"),60);//MakeRect(20,230,480,272)

	logo1->Begin();
	ScreenState = SCREEN_LOGO1;
	logo1->SplashOut = logo1_SplashOut;

	logo2 = new SplashScreen(new GUIStaticImage("logo2"),60);//MakeRect(16,120,480,272)
	logo2->SplashOut = logo2_SplashOut;

	logo3 = new SplashScreen(new GUIStaticImage("logo3"),2);
	logo3->SplashOut = logo3_SplashOut;

	logo4 = new SplashScreen(new GUIStaticImage("logo4"),2);
	logo4->SplashOut = logo4_SplashOut;
}

void GameMainDraw()
{
	if(ScreenState==SCREEN_GAMEMAIN)
		core.Draw(base::Device);
	else if(ScreenState==SCREEN_MPLAYER)
		mplayer.Draw(base::Device);
	else if(ScreenState==SCREEN_LOGO1)
		logo1->Draw();
	else if(ScreenState==SCREEN_LOGO2)
		logo2->Draw();
	else if(ScreenState==SCREEN_LOGO3)
		logo3->Draw();
	else if(ScreenState==SCREEN_LOGO4)
		logo4->Draw();
	else if(ScreenState==SCREEN_MAIN)
		menu->Draw();
	loading->Draw();
}

void GameMainUpdate(double dwTime)
{
	float dwTimeMilliSecond = dwTime*base::InvMilliSecond;
	soundEngine->Update(dwTimeMilliSecond);
	if(pRubixWindow)
		pRubixWindow->Update(dwTimeMilliSecond);
	else if(ScreenState==SCREEN_GAMEMAIN)
		core.Update(dwTime);
	else if(ScreenState==SCREEN_LOGO1)
		logo1->Update();
	else if(ScreenState==SCREEN_LOGO2)
		logo2->Update();
	else if(ScreenState==SCREEN_LOGO3)
		logo3->Update();
	else if(ScreenState==SCREEN_LOGO4)
		logo4->Update();
	else if(ScreenState==SCREEN_MAIN)
		menu->Update();
	loading->Update();
}

void DeleteUIScreen()
{
	SAFE_DELETE(logo1);
	SAFE_DELETE(logo2);
	SAFE_DELETE(logo3);
	SAFE_DELETE(logo4);
	SAFE_DELETE(loading);
	SAFE_DELETE(menu);
}

void GameMainOnMouseEvent(int xPos, int yPos, UINT msg, WPARAM wParam)
{
	static HCURSOR hCursor_arrow = LoadCursor(NULL, IDC_ARROW);
	static HCURSOR hCursor_hand = LoadCursor(NULL, IDC_HAND);
	static bool inside_seekbar = false;
	int val;
	char key;
	if( ScreenState<=SCREEN_MAIN )
		switch(msg){
		case WM_MOUSEWHEEL:
			val = (short)HIWORD(wParam);
			key = val<0?VK_DOWN:VK_UP;
			for(int x=0;x<abs(val);x+=WHEEL_DELTA){
				GameMainOnKeyEvent(key,true);
				GameMainOnKeyEvent(key,false);
			}
			break;
		case WM_LBUTTONDOWN:
			GameMainOnKeyEvent(VK_RETURN,true);
			break;
		case WM_LBUTTONUP:
			GameMainOnKeyEvent(VK_RETURN,false);
			break;
		case WM_RBUTTONDOWN:
			GameMainOnKeyEvent(VK_ESCAPE,true);
			break;
		case WM_RBUTTONUP:
			GameMainOnKeyEvent(VK_ESCAPE,false);
			break;
		}
	else
		switch(msg){
		case WM_LBUTTONDOWN:
			if(inside_seekbar)
				core.SetTimePosi((float)(xPos-base::window_rect.left)/(base::window_rect.right-base::window_rect.left));
			break;
		case WM_MOUSEMOVE:
			ImageResource *res = (ImageResource*)g_res.getResource("progress_bar_back");
			fRECT rect = base::MakeDrawRect(res->dstRect);
			if(InsideRect(xPos,yPos,rect))
				SetCursor(hCursor_hand), inside_seekbar=true;
			else
				SetCursor(hCursor_arrow), inside_seekbar=false;
			break;
		}
}

void GameMainOnKeyEvent( UINT nChar, bool bKeyDown )
{
	bool bAlreadyDown = Keys[nChar];
	Keys[nChar] = bKeyDown;

	if( ScreenState==SCREEN_GAMEMAIN )
	{
		if(!( bKeyDown && bAlreadyDown ))
			core.OnKeyEvent(nChar,bKeyDown);
	}
	else if(lock_msgbox)
		lock_msgbox->KeyHandler(nChar, bKeyDown);
	else if( ScreenState==SCREEN_MAIN && bKeyDown )
		menu->OnKeyEvent(KeyEventValue);
	else if( ScreenState<SCREEN_LOADING && nChar==VK_ESCAPE )
		MainFirstInit();
}
