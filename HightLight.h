
#ifndef _HIGHT_LIGHT_H
#define _HIGHT_LIGHT_H


#include "lines.hpp"
#define DEFAULT_HCOLOR 0x58d3ff
#define DEFAULT_BCOLOR 0xe0ffea



/* 高亮管理类;分为2部分，一部分是 {}段的，另外一部分是 () 匹配的
*/
class HightLight
{
public:
	HightLight()
	{
		clear();
		m_vecInfo.clear();
	};
	~HightLight()
	{};
	void set_hcolor(bgcolor_t color){ m_hColor = color; }
	void set_bcolor(bgcolor_t color){ m_bColor = color; }
	bgcolor_t get_hcolor(){ return m_hColor; };
	bgcolor_t get_bcolor(){ return m_bColor; };
	bool isHLight() { return m_bHaveHLight; };
	void set_hlight(bool bHight){ m_bHaveHLight = bHight; };
	void set_jLine(int lineNum)
	{
		if (!isHLight())
		{
			return;
		}
		if (lineNum == m_StartLine)
		{
			m_jLine = m_EndLine;
		}
		else if (lineNum == m_EndLine)
		{
			m_jLine = m_StartLine;
		}
		else
		{
			m_jLine = -1;
		}

	};
	int get_jmpline(){ return m_jLine; };
	void set_begin_end(int ibegin, int iend){ m_StartLine = ibegin; m_EndLine = iend; };
	void get_begin_end(int &ibegin, int &iend){ ibegin = m_StartLine; iend = m_EndLine; };

	void restore_color(strvec_t* pStrVec)
	{
		if (!isHLight())
		{
			return;
		}
		int iStart = 0;
		int iEnd = 0;
		get_begin_end(iStart, iEnd);
		if (iStart <= iEnd)
		{
			for (int i = iStart; i <= iEnd; i++)
			{
				(*pStrVec)[i].bgcolor = DEFCOLOR;
			}
		}
	}

	void clear()
	{
		m_jLine = -1;
		m_bHaveHLight = false;
		m_StartLine = 0;
		m_EndLine = 0;
	}

	//  后面是 () 高亮管理

	void InsertColor(int line, qstring &str)
	{
		HightInfo stru = {};
		stru.lineNum = line;
		stru.str = str;
		m_vecInfo.push_back(stru);
	}

	void ClearColor()
	{
		m_vecInfo.clear();
	}

	bool ResetNoRefresh(strvec_t* pVec)
	{
		if (m_vecInfo.size() > 0)
		{
			for (qvector<HightInfo>::iterator item = m_vecInfo.begin(); item != m_vecInfo.end(); item++)
			{
				int line = item->lineNum;
				(*pVec)[line].line = item->str;
			}
			ClearColor();
			return true;
		}
		return false;
	}


private:
	/* 对应的跳转项line
	*/
	int m_jLine;
	/* 是否已经高亮
	*/
	bool m_bHaveHLight;
	/* 高亮的起始和结束
	*/
	int m_StartLine;
	int m_EndLine;
	/* head的颜色
	*/
	bgcolor_t m_hColor;
	/* body的颜色
	*/
	bgcolor_t m_bColor;


	//  后面是 () 高亮管理
	/*  () 匹配的
	*/
	struct HightInfo
	{
		int lineNum;
		qstring str;
	};
	qvector<HightInfo> m_vecInfo;
};












#endif