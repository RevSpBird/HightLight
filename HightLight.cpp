/* HightLight
*/
#include "loader.hpp"
#include "hexrays.hpp"
#include "pro.h"
#include "HightLight.h"
#include "diskio.hpp"

#include "kernwin.hpp"
#include <windows.h>

#define APPNAME "HightLight"
#define HEADKEY_NAME "HeadColor"
#define BODYKEY_NAME "BodyColor"
#define FileNAME "SpBird.ini"


hexdsp_t *hexdsp;
HightLight *pHLight = NULL;

static bool findMatchPos(strvec_t*pStrvec, int lineNum, int xPos, int &matchLine, int &matchXpos)
{
	qstring strBuf;
	int pos = xPos - 1;
	int line = lineNum;
	tag_remove(&strBuf, (*pStrvec)[line].line.c_str(), MAX_NUMBUF);
	char bMatched = strBuf[pos];
	if (bMatched == ')' ||
		bMatched == '(')
	{

		bool bSearchDown = true; // 是否向下查找
		char bMatch = 0; // 待匹配查找字符
		int nStep = 1;
		if (bMatched == ')')
		{
			bMatch = '(';
			bSearchDown = false;
			nStep = -1;
		}
		else
		{
			bMatch = ')';
		}
		int matchTime = 1;
		bool bMat = false;

		pos = pos + nStep;
		do
		{
			while (pos >= 0 && pos < strBuf.size())
			{
				if (strBuf[pos] == bMatch)
				{
					matchTime--;
				}
				else if (strBuf[pos] == bMatched)
				{
					matchTime++;
				}
				if (matchTime == 0)
				{
					// 匹配到
					bMat = true;
					break;
				}
				pos = pos + nStep;
			}
			if (bMat)
			{
				matchLine = line;
				matchXpos = pos;
				break;
			}
			// 要换行匹配
			line = line + nStep;
			tag_remove(&strBuf, (*pStrvec)[line].line.c_str(), MAX_NUMBUF);
			if (bSearchDown)
			{
				pos = 0;
			}
			else
			{
				pos = strBuf.size() - 1;
			}

		} while (line >= 0 && line < pStrvec->size());

		return bMat;
	}
	else
	{
		// 暂时只支持 ( and )
		return false;
	}

}



static void convert_zeroes(cfunc_t *cfunc);
void get_ini_path(qstring &path)
{
	path = idadir(PLG_SUBDIR);
	path += "\\";
	path += FileNAME;
}

int GetConf(qstring &path, bgcolor_t &headColor, bgcolor_t &bodyColor)
{
	int headValue = GetPrivateProfileIntA(APPNAME, HEADKEY_NAME, 0, path.c_str());
	if (headValue == 0)
	{
		return 1;
	}
	int bodyValue = GetPrivateProfileIntA(APPNAME, BODYKEY_NAME, 0, path.c_str());
	if (bodyValue == 0)
	{
		return 2;
	}
	headColor = headValue;
	bodyColor = bodyValue;
	return 0;
}

int SetConf(qstring &path, qstring &headColor, qstring &bodyColor)
{
	if (WritePrivateProfileStringA(APPNAME, HEADKEY_NAME, headColor.c_str(), path.c_str()) == FALSE)
	{
		int err = GetLastError();
		return err;
	}
	if (WritePrivateProfileStringA(APPNAME, BODYKEY_NAME, bodyColor.c_str(), path.c_str()) == FALSE)
	{
		int err = GetLastError();
		return err;
	}
	return 0;
}

bool GetConfColor(bgcolor_t &headColor, bgcolor_t &bodyColor)
{
	bool ret = false;
	qstring iniPath;
	get_ini_path(iniPath);
	if (!GetConf(iniPath, headColor, bodyColor))
	{
		ret = true;
	}
	return ret;
}

bool SetConfColor(qstring &headColor, qstring &bodyColor)
{
	bool ret = false;
	qstring iniPath;
	get_ini_path(iniPath);
	if (!SetConf(iniPath, headColor, bodyColor))
	{
		ret = true;
	}
	return ret;
}

ssize_t idaapi myhexrays_cb_t(void *ud, hexrays_event_t event, va_list va)
{
	HightLight *pTempHlight = (HightLight *)ud;
	switch (event)
	{
	case hxe_maturity:
		break;
	case hxe_open_pseudocode:
	case hxe_switch_pseudocode:
	case hxe_refresh_pseudocode:
	case hxe_close_pseudocode:
	{
								 pTempHlight->clear();
								 pTempHlight->ClearColor();
								 break;
	}
	case hxe_curpos:
	{
					   // 鼠标点击
					   vdui_t *vu = va_arg(va, vdui_t*);
					   cfuncptr_t* pFun = &vu->cfunc;
					   if ((*pFun)->maturity != CMAT_FINAL)
					   {
						   return 0;
					   }
					   if (!vu->visible())
					   {
						   return 0;
					   }
					   if (!vu->refresh_cpos(USE_KEYBOARD))
					   {
						   return 0;
					   }
					   ctext_position_t cPos = vu->cpos;
					   // 添加 () [] 等匹配跳转
					   int yPos = cPos.lnnum;
					   int xPos = cPos.x;
					   if (yPos == 0)
					   {
						   // 第一行函数声明 只有开头和结尾有"\x01\x17"和"\x02\x17"，(和）无Punctuation
						   break;
					   }
					   strvec_t* str_t = (strvec_t*)&(*pFun)->get_pseudocode();
					   qstring strBuf;
					   int matchLine = 0;
					   int matchxpos = 0;
					   char selectBuf = 0;
					   //TEST 
					   //		char *pAdvance = (char *)tag_advance((*str_t)[yPos].line.c_str(), 0);
					   //		msg("check tag_advance : %s", pAdvance);
					   if (pTempHlight->ResetNoRefresh(str_t))
					   {
						   refresh_idaview_anyway();
					   }
					   tag_remove(&strBuf, (*str_t)[yPos].line.c_str(), MAX_NUMBUF);
					   if (xPos > 1 && xPos < strBuf.size())
					   {
						   selectBuf = strBuf[xPos - 1];
					   }
					   if (selectBuf == '(' || selectBuf == ')')
					   {
						   if (findMatchPos(str_t, yPos, xPos, matchLine, matchxpos))
						   {
							   if (matchLine == 0)
							   {
								   return 0;
							   }
							   char *pAdvBegin = (char *)(*str_t)[yPos].line.c_str();
							   int iAdvLen = (*str_t)[yPos].line.size();
							   char *pAdvance = (char *)tag_advance(pAdvBegin, xPos - 1);
							   char *pMatchBegin = (char *)(*str_t)[matchLine].line.c_str();
							   int iMatchLen = (*str_t)[matchLine].line.size();
							   char *pMatch = (char *)tag_advance(pMatchBegin, matchxpos);

							   while (*pAdvance != COLOR_ON || *(pAdvance + 1) != COLOR_SYMBOL)
							   {
								   // 解决未知情况下越界问题
								   if (pAdvance < &pAdvBegin[iAdvLen - 3])
								   {
									   pAdvance++;
								   }
								   else
								   {
									   // 要越界，直接返回
									   return 0;
								   }

							   }
							   while (*pMatch != COLOR_ON || *(pMatch + 1) != COLOR_SYMBOL)
							   {
								   // 解决未知情况下越界问题
								   if (pMatch < &pMatchBegin[iMatchLen - 3])
								   {
									   pMatch++;
								   }
								   else
								   {
									   // 要越界，直接返回
									   return 0;
								   }
							   }
							   pTempHlight->InsertColor(yPos, (*str_t)[yPos].line);
							   if (yPos != matchLine)
							   {
								   pTempHlight->InsertColor(matchLine, (*str_t)[matchLine].line);
							   }
							   pAdvance[1] = COLOR_ERROR;
							   pAdvance[4] = COLOR_ERROR;
							   pMatch[1] = COLOR_ERROR;
							   pMatch[4] = COLOR_ERROR;
							   refresh_idaview_anyway();
						   }
					   }
					   else
					   {
						   // do nothing yet
					   }



					   break;
	}
	case hxe_double_click:
	{
							 // 当前位置发生改变
							 vdui_t *vu = va_arg(va, vdui_t*);
							 cfuncptr_t* pFun = &vu->cfunc;
							 ctree_maturity_t eLevel = (*pFun)->maturity;
							 if (eLevel != CMAT_FINAL)
							 {
								 msg("ctree_maturity_t level is %d\n", eLevel);
								 return 0;
							 }
							 if (!vu->visible())
							 {
								 return 0;
							 }
							 if (!vu->refresh_cpos(USE_KEYBOARD))
							 {
								 return 0;
							 }

							 ctext_position_t cPos = vu->cpos;
							 int yPos = cPos.lnnum;
							 strvec_t* str_t = (strvec_t*)&(*pFun)->get_pseudocode();

							 qstring strBuf;
							 tag_remove(&strBuf, (*str_t)[yPos].line.c_str(), MAX_NUMBUF);
							 if (strBuf.find("//") > 0)
							 {
								 strBuf = strBuf.substr(0, strBuf.find("//"));
							 }
							 int blockBegin = strBuf.find('{');
							 int blockEnd = strBuf.find('}');
							 if (blockBegin >= 0 || blockEnd >= 0)
							 {
								 pTempHlight->restore_color(str_t);
								 char findChar = '}';
								 int findIndex = 1;
								 int findPos = blockBegin;

								 if (blockEnd >= 0)
								 {
									 findChar = '{';
									 findIndex = -1;
									 findPos = blockEnd;
								 }
								 (*str_t)[yPos].bgcolor = pTempHlight->get_hcolor();
								 //
								 int j = yPos + findIndex;
								 int max = (*str_t).size();
								 while (j >= 0 && j < max)
								 {
									 qstring outStr;
									 tag_remove(&outStr, (*str_t)[j].line.c_str(), MAX_NUMBUF);
									 int findLen = outStr.find("//");
									 if (findLen > 0)
									 {
										 outStr = outStr.substr(0, findLen);
									 }
									 if (outStr.find(findChar) == findPos)
									 {
										 (*str_t)[j].bgcolor = pTempHlight->get_hcolor();
										 break;
									 }
									 else
									 {
										 (*str_t)[j].bgcolor = pTempHlight->get_bcolor();
									 }
									 j += findIndex;
								 }
								 if (findIndex > 0)
								 {
									 pTempHlight->set_begin_end(yPos, j);
								 }
								 else
								 {
									 pTempHlight->set_begin_end(j, yPos);
								 }
								 pTempHlight->set_hlight(true);
								 pTempHlight->set_jLine(yPos);
								 //vu.refresh_ctext(true);
								 refresh_idaview_anyway();
							 }
							 else
							 {
								 if (pTempHlight->isHLight())
								 {
									 pTempHlight->restore_color(str_t);
									 pTempHlight->clear();
									 refresh_idaview_anyway();
								 }
							 }
							 break;
	}
	case hxe_keyboard:
	{
						 vdui_t &vu = *va_arg(va, vdui_t*);
						 int keyCode = va_arg(va, int);
						 int shift_state = va_arg(va, int);
						 if (keyCode == 'j' || keyCode == 'J')
						 {
							 int x = 0;
							 int y = 0;
							 place_t *pLace = get_custom_viewer_place(vu.ct, false, &x, &y);
							 simpleline_place_t *simPlace = (simpleline_place_t *)pLace->clone();
							 simPlace->n = pTempHlight->get_jmpline();
							 if (simPlace->n != -1)
							 {
								 jumpto(vu.ct, (place_t *)simPlace, x, y);
								 if (pTempHlight->isHLight())
								 {
									 pTempHlight->set_jLine(simPlace->n);
								 }
							 }

						 }
						 break;
	}
	case hxe_right_click:
	{
							vdui_t &vu = *va_arg(va, vdui_t*);

							break;
	}
	case hxe_populating_popup:
	{
								 TWidget &form = *va_arg(va, TWidget*);
								 TPopupMenu &popup_handle = *va_arg(va, TPopupMenu*);
								 vdui_t &vu = *va_arg(va, vdui_t*);
								 break;
	}

		///< Populating popup menu. We can add menu items now.
		///< TForm *form
		///< TPopupMenu *popup_handle
		///< vdui_t *vu
	}
	return 0;
}

int idaapi init(void)
{
	msg("HightLight init begin \n");
	if (!init_hexrays_plugin())
	{
		return PLUGIN_SKIP;
	}

	// 开始添加hook
	if (pHLight == NULL)
	{
		pHLight = new HightLight();
		if (pHLight == NULL)
		{
			msg("there is no enough memory \n");
			return PLUGIN_SKIP;
		}
	}
	bgcolor_t hColor = DEFAULT_HCOLOR;
	bgcolor_t bColor = DEFAULT_BCOLOR;
	GetConfColor(hColor, bColor);
	pHLight->set_bcolor(bColor);
	pHLight->set_hcolor(hColor);
	if (!install_hexrays_callback(myhexrays_cb_t, pHLight))
	{
		if (pHLight)
		{
			delete pHLight;
			pHLight = NULL;
		}
		msg("install_hexrays_callback fail\n");
		return PLUGIN_SKIP;
	}

	msg("HeyRays version %s has been detected \n", get_hexrays_version());
	msg("**********HightLight tool************************\n");
	msg("**********Author: SpBird data: 2017-09-10**********\n");
	return PLUGIN_KEEP;
}

void idaapi term()
{
	// exit code
	// 结束hook
	remove_hexrays_callback(myhexrays_cb_t, NULL);
	if (pHLight)
	{
		delete pHLight;
		pHLight = NULL;
	}
	return;
}

bool idaapi run(size_t arg)
{
	unsigned short bRem = 1;
	bgcolor_t headC = DEFAULT_HCOLOR;
	bgcolor_t bodyC = DEFAULT_BCOLOR;

	GetConfColor(headC, bodyC);
	const char strFormat[] =
		"STARTITEM 0\n"
		"Options\n"
		"< # store the color #"
		"remember :C:1:::>>\n"
		"<~H~eadColor :K:16:32::>\n"
		"<~B~odyColor :K:16:32::>\n";
	if (!ask_form(strFormat, &bRem, &headC, &bodyC))
	{
		msg("--------   ");
	}
	pHLight->set_bcolor(bodyC);
	pHLight->set_hcolor(headC);
	if (bRem)
	{
		qstring headColor;
		qstring bodyColor;
		headColor.sprnt("0x%x", headC);
		bodyColor.sprnt("0x%x", bodyC);
		SetConfColor(headColor, bodyColor);
	}

	return true;
}

const char comment[] = "HightLight Plugin";
const char help[] = "Hotkey to set colors is Alt-5;if error occure, contact to SpBird";             ///< Multiline help about the plugin
const char wanted_name[] = "HightLight";      ///< The preferred short name of the plugin
const char wanted_hotkey[] = "Alt-3";    ///< The preferred hotkey to run the plugin

// init the export struct for ida.exe
plugin_t PLUGIN
{
	IDP_INTERFACE_VERSION,
	0,
	init,
	term,
	run,
	comment,
	help,
	wanted_name,
	wanted_hotkey
};
