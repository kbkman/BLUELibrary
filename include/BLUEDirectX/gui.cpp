#include "gui.h"
#include "guiMsgType.h"

CDXGManager::CDXGManager(void) :
m_pCurMouseInWnd(BLUENULL),
m_pCurFocusWnd(BLUENULL)
{

}

CDXGManager::~CDXGManager(void)
{

}

void CDXGManager::ProcessMessage(unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg >= DXGUIM_MOUSEFIRST && uMsg <= DXGUIM_MOUSELAST)//鼠标消息处理
	{
		CBLUEPoint pt(LOWORD(lParam), HIWORD(lParam));

		//查询鼠标消息是否落在哪个顶层窗口之上
		for (DXGWINDOWLIST::iterator it = m_topWndList.begin(), itEnd = m_topWndList.end(); it != itEnd; it++)
		{
			if ((*it)->IsProcessMsg() &&
				(*it)->IsVisible() &&
				(*it)->PointIn(pt))
			{
				/**********处理鼠标的OnMouseOut和OnMouseIn***********/
				//如果鼠标已经不在前一个窗口之上并且前一个窗口存在，则前一个窗口触发OnMouseOut事件
				if ((*it) != m_pCurMouseInWnd)
				{
					if (m_pCurMouseInWnd)
						m_pCurMouseInWnd->OnMouseOut(pt);
					
					//新收到鼠标消息的窗口触发OnMouseIn事件
					(*it)->OnMouseIn(pt);
					m_pCurMouseInWnd = (*it);//设置当前接受鼠标的顶层窗口指针
				}

				/**********处理焦点***********/
				//如果是左键或右键鼠标按下，则要判断窗口焦点是否转移
				if (uMsg == DXGUIM_LBUTTONDOWN ||
					uMsg == DXGUIM_RBUTTONDOWN)
				{
					//如果前一个焦点窗口不是当前窗口且前一个窗口存在，则触发前一个焦点窗口的推动焦点事件
					if ((*it) != m_pCurFocusWnd)
					{
						if (m_pCurFocusWnd)
						{
							m_pCurFocusWnd->m_bIsFocus = false;
							m_pCurFocusWnd->OnFocus(false);
						}
						//当前窗口获得焦点
						(*it)->m_bIsFocus = true;
						(*it)->OnFocus(true);
						m_pCurFocusWnd = (*it);
					}
				}

				//新窗口处理鼠标消息
				(*it)->ProcessMouseMsg(uMsg, pt);
				return;//只要找到了窗口则退出
			}

			if ((*it)->m_bIsModel)
				break;
		}

		//鼠标没有在任何顶层窗口中则前一个窗口触发OnMouseOut事件
		if (m_pCurMouseInWnd)
		{
			m_pCurMouseInWnd->OnMouseOut(pt);
			m_pCurMouseInWnd = BLUENULL;
		}
	}
	else if (uMsg >= DXGUIM_KEYFIRST && uMsg <= DXGUIM_KEYLAST)//键盘消息处理
	{
		//获取Tab键，来调整焦点
		if (uMsg == DXGUIM_KEYDOWN && wParam == DXGUIVK_TAB)
		{

		}

		//焦点窗口是否存在
		if (m_pCurFocusWnd)
		{
			//焦点窗口可接受键盘输入
			if (m_pCurFocusWnd->IsProcessMsg() &&
				m_pCurFocusWnd->IsVisible())
			{
				//新窗口处理鼠标消息
				m_pCurFocusWnd->ProcessKeyMsg(uMsg, (int)wParam);
				return;
			}
		}
	}
}

bool CDXGManager::AddTopWindow(CDXGWindow* pWindow, const CBLUERect& rt, bool bIsModel)
{
	BLUEASSERT(pWindow);
	BLUEASSERT(!pWindow->IsUsing());

	//父窗口指针本对象
	pWindow->m_bWindowParent = false;
	pWindow->m_parent.pManager = this;
	pWindow->m_bIsModel = bIsModel;

	//设置顶窗口坐标
	pWindow->m_absoluteRect = rt;

	//触发窗口的OnCreate事件
	if (!pWindow->OnCreate(pWindow->m_absoluteRect.GetWidth(), pWindow->m_absoluteRect.GetHeight()))
	{
		pWindow->m_parent.pManager = BLUENULL;
		return false;
	}

	//加入顶层窗口链表
	m_topWndList.push_front(pWindow);
	return true;
}

void CDXGManager::RemoveTopWindow(CDXGWindow* pWindow)
{
	BLUEASSERT(pWindow);
	BLUEASSERT(pWindow->IsUsing());
	BLUEASSERT(!pWindow->m_bWindowParent);
	BLUEASSERT(pWindow->m_parent.pManager == this);

	pWindow->RemoveAllChildWinow();
	pWindow->OnDestroy();
	m_topWndList.remove(pWindow);
	pWindow->m_parent.pManager = BLUENULL;

	//当前焦点与鼠标所在窗口设置为空
	if (pWindow == m_pCurFocusWnd)
		m_pCurFocusWnd = BLUENULL;
	if (pWindow == m_pCurMouseInWnd)
		m_pCurMouseInWnd = BLUENULL;
}

void CDXGManager::RemoveAllTopWindow(void)
{
	//先清除所有顶窗口内的子窗口
	for (DXGWINDOWLIST::iterator it = m_topWndList.begin(), itEnd = m_topWndList.end(); it != itEnd; it++)
	{
		CDXGWindow* pWindow = *it;
		BLUEASSERT(pWindow);
		BLUEASSERT(pWindow->IsUsing());
		BLUEASSERT(!pWindow->m_bWindowParent);
		BLUEASSERT(pWindow->m_parent.pManager == this);

		pWindow->RemoveAllChildWinow();
		pWindow->OnDestroy();
		pWindow->m_parent.pManager = BLUENULL;

		//当前焦点与鼠标所在窗口设置为空
		m_pCurFocusWnd = BLUENULL;
		m_pCurMouseInWnd = BLUENULL;
	}

	//清除子窗口链表
	m_topWndList.clear();
}

void CDXGManager::Render(void)
{
	//渲染所有顶层窗口，从底层窗口开始渲染
	for (DXGWINDOWLIST::reverse_iterator it = m_topWndList.rbegin(), itEnd = m_topWndList.rend(); it != itEnd; it++)
		if ((*it)->IsVisible())
			(*it)->Render();

}

CDXGWindow::CDXGWindow(void) :
m_pListener(BLUENULL),
m_bIsModel(false),
m_bIsEnabled(true),
m_bIsFocus(false),
m_pCurMouseInWnd(BLUENULL),
m_pCurFocusWnd(BLUENULL)
{
	Show();
	AcceptMsg();
	::memset(&m_parent, 0, sizeof(m_parent));
}

CDXGWindow::CDXGWindow(bool bAccept, bool bVisible) :
m_pListener(BLUENULL),
m_bIsModel(false),
m_bIsEnabled(true),
m_bIsFocus(false),
m_pCurMouseInWnd(BLUENULL),
m_pCurFocusWnd(BLUENULL)
{
	SetVisible(bVisible);
	SetAcceptMsg(bAccept);
	::memset(&m_parent, 0, sizeof(m_parent));
}

CDXGWindow::~CDXGWindow(void)
{

}

void CDXGWindow::SetFocus(bool bFocus)
{

}

void CDXGWindow::ProcessMouseMsg(unsigned int uMouseMsg, const CBLUEPoint& pt)
{
	BLUEASSERT(uMouseMsg >= DXGUIM_MOUSEFIRST && uMouseMsg <= DXGUIM_MOUSELAST);
	BLUEASSERT(PointIn(pt));

	//查询鼠标消息是否落在哪个子窗口之上
	for (DXGWINDOWLIST::iterator it = m_childWndList.begin(), itEnd = m_childWndList.end(); it != itEnd; it++)
	{
		if ((*it)->IsProcessMsg() &&
			(*it)->IsVisible() &&
			(*it)->PointIn(pt))
		{
			/**********处理鼠标的OnMouseOut和OnMouseIn***********/
			//如果鼠标已经不在前一个窗口之上并且前一个窗口存在，则前一个窗口触发OnMouseOut事件
			if ((*it) != m_pCurMouseInWnd)
			{
				if (m_pCurMouseInWnd)
					m_pCurMouseInWnd->OnMouseOut(pt);

				//新收到鼠标消息的窗口触发OnMouseIn事件
				(*it)->OnMouseIn(pt);
				m_pCurMouseInWnd = (*it);//设置当前接受鼠标的顶层窗口指针
			}
			
			/**********处理焦点***********/
			//如果是左键或右键鼠标按下，则要判断窗口焦点是否转移
			if (uMouseMsg == DXGUIM_LBUTTONDOWN ||
				uMouseMsg == DXGUIM_RBUTTONDOWN)
			{
				//如果前一个焦点窗口不是当前窗口且前一个窗口存在，则触发前一个焦点窗口的推动焦点事件
				if ((*it) != m_pCurFocusWnd)
				{
					if (m_pCurFocusWnd)
					{
						m_pCurFocusWnd->m_bIsFocus = false;
						m_pCurFocusWnd->OnFocus(false);
					}
					//当前窗口获得焦点
					(*it)->m_bIsFocus = true;
					(*it)->OnFocus(true);
					m_pCurFocusWnd = (*it);
				}
			}

			//收到鼠标的消息的窗口处理鼠标消息
			(*it)->ProcessMouseMsg(uMouseMsg, pt);
			return;//只要找到了窗口则退出
		}
	}

	//鼠标没有在任何窗口中则前一个窗口触发OnMouseOut事件
	if (m_pCurMouseInWnd)
	{
		m_pCurMouseInWnd->OnMouseOut(pt);
		m_pCurMouseInWnd = BLUENULL;
	}

	//将绝对转为本窗口的相对坐标
	CBLUEPoint relativePt;
	m_absoluteRect.RelativePoint(pt, relativePt);

	//BLUETRACE(_BLUET("%d--%d\n"), relativePt.m_nX, relativePt.m_nY);

	//根据不同消息，触发不同事件
	switch (uMouseMsg)
	{
	case DXGUIM_MOUSEMOVE:
		OnMouseMove(relativePt);
		break;
	case DXGUIM_LBUTTONDOWN:
		OnLButtonDown(relativePt);
		break;
	case DXGUIM_LBUTTONUP:
		OnLButtonUp(relativePt);
		break;
	case DXGUIM_RBUTTONDOWN:
		OnRButtonDown(relativePt);
		break;
	case DXGUIM_RBUTTONUP:
		OnRButtonUp(relativePt);
		break;
	}
}

void CDXGWindow::ProcessKeyMsg(unsigned int uKeyMsg, int code)
{
	BLUEASSERT(uKeyMsg >= DXGUIM_KEYFIRST && uKeyMsg <= DXGUIM_KEYLAST);

	//焦点窗口是否存在
	if (m_pCurFocusWnd)
	{
		//焦点窗口可接受键盘输入
		if (m_pCurFocusWnd->IsProcessMsg() &&
			m_pCurFocusWnd->IsVisible())
		{
			//新窗口处理鼠标消息
			m_pCurFocusWnd->ProcessKeyMsg(uKeyMsg, code);
			return;
		}
	}
	else//没有则说明自己没有焦点子窗口了，自己接受键盘输入
	{
		switch (uKeyMsg)
		{
		case DXGUIM_KEYDOWN:
			OnKeyDown(code);
			break;
		case DXGUIM_KEYUP:
			OnKeyUp(code);
			break;
		case DXGUIM_CHAR:
			OnChar(code);
			break;
		}

	}
}

bool CDXGWindow::AddChildWindow(CDXGWindow* pWindow, const CBLUERect& rt)
{
	BLUEASSERT(pWindow);
	BLUEASSERT(!pWindow->IsUsing());

	//根据相对坐标得到绝对坐标存入子窗口的绝对坐标变量
	m_absoluteRect.AbsoluteRect(rt, pWindow->m_absoluteRect);

	//父窗口指针本对象
	pWindow->m_bWindowParent = true;
	pWindow->m_parent.pParentWindow = this;

	//触发窗口的OnCreate事件
	if (!pWindow->OnCreate(pWindow->m_absoluteRect.GetWidth(), pWindow->m_absoluteRect.GetHeight()))
	{
		pWindow->m_parent.pParentWindow = BLUENULL;
		return false;
	}

	//窗口加入子窗口链表
	m_childWndList.push_front(pWindow);
	return true;
}

void CDXGWindow::RemoveChildWindow(CDXGWindow* pWindow)
{
	BLUEASSERT(pWindow);
	BLUEASSERT(!pWindow->IsUsing());
	BLUEASSERT(pWindow->m_bWindowParent);
	BLUEASSERT(pWindow->m_parent.pParentWindow == this);

	pWindow->RemoveAllChildWinow();
	pWindow->OnDestroy();
	m_childWndList.remove(pWindow);
	pWindow->m_parent.pParentWindow = BLUENULL;

	//当前焦点与鼠标所在窗口设置为空
	if (pWindow == m_pCurFocusWnd)
		m_pCurFocusWnd = BLUENULL;
	if (pWindow == m_pCurMouseInWnd)
		m_pCurMouseInWnd = BLUENULL;
}

void CDXGWindow::RemoveAllChildWinow(void)
{
	//从顶层子窗口向底层子窗口一个个移除
	for (DXGWINDOWLIST::iterator it = m_childWndList.begin(), itEnd = m_childWndList.end(); it != itEnd; it++)
	{
		CDXGWindow* pWindow = *it;
		BLUEASSERT(pWindow);
		BLUEASSERT(pWindow->IsUsing());
		BLUEASSERT(pWindow->m_bWindowParent);
		BLUEASSERT(pWindow->m_parent.pParentWindow == this);

		pWindow->RemoveAllChildWinow();
		pWindow->OnDestroy();
		pWindow->m_parent.pParentWindow = BLUENULL;

		//当前焦点与鼠标所在窗口设置为空
		m_pCurFocusWnd = BLUENULL;
		m_pCurMouseInWnd = BLUENULL;
	}

	//清除子窗口链表
	m_childWndList.clear();
}

void CDXGWindow::Render(void)
{
	//渲染自己
	OnRender();

	//渲染所有子窗口
	for (DXGWINDOWLIST::reverse_iterator it = m_childWndList.rbegin(), itEnd = m_childWndList.rend(); it != itEnd; it++)
		if ((*it)->IsVisible())
			(*it)->Render();
}