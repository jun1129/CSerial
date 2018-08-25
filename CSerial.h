// CSerial.h : main header file for the CSERIAL DLL
//

#if !defined(AFX_CSERIAL_H__1DFD71D7_0E18_4ED0_8E7A_0446FFA1F02E__INCLUDED_)
#define AFX_CSERIAL_H__1DFD71D7_0E18_4ED0_8E7A_0446FFA1F02E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCSerialApp
// See CSerial.cpp for the implementation of this class
//

class CCSerialApp : public CWinApp
{
public:
	CCSerialApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCSerialApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CCSerialApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSERIAL_H__1DFD71D7_0E18_4ED0_8E7A_0446FFA1F02E__INCLUDED_)
