
/******************************************************************************
 *             Cellon - TEST APPLICATION     								  *
 ******************************************************************************
 * File name : CSerial.cpp													  *
 * Version : V2.00															  *
 * Project : STANDARD														  *
 *----------------------------------------------------------------------------* 
 *                               DESCRIPTION					              *
 *----------------------------------------------------------------------------*
 *						CSerial Driver  									  *
 *																			  *
 *----------------------------------------------------------------------------* 
 *                                EVOLUTION									  *
 *----------------------------------------------------------------------------* 
 * Date      | Author      | Version	|Descriptio							  *
 *----------------------------------------------------------------------------* 
 * 07 Jun, 06|Duke.Du    |V1.00		|Creation					          *
 * 30 Aug, 16|derek       |	v2.00   |Update
 1. delete call SendSerialData function 
 2. Update Serial_Read function, delete buffer length check
 3. add Serial_Config_New function instead Serial_Config, add set DCB struct and timeout struct.
 *----------------------------------------------------------------------------*
 *****************************************************************************/

// CSerial.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "CSerial.h"
#include "CSerialLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CCSerialApp

#define MAX_PORT_NUM	32
HWND hSerialTracerWnd = NULL;	

BEGIN_MESSAGE_MAP(CCSerialApp, CWinApp)
	//{{AFX_MSG_MAP(CCSerialApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCSerialApp construction

CCSerialApp::CCSerialApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	if( hSerialTracerWnd == NULL )	
	{
		hSerialTracerWnd = ::FindWindow( NULL, "SerialTracer" );   
	}
}

int SendSerialData(int nPort, char *sRW, BYTE *sData, int nLen, int nStatus)
{
	if( hSerialTracerWnd == NULL ) return 1;

	char			s[4096], s1[4096];
	int				i;
	COPYDATASTRUCT	cd;

	if( nStatus == 0 )
		sprintf( s, "%d:%s:", nPort, sRW );
	else
		sprintf( s, "%d:%s:[%d]", nPort, sRW, nStatus );

	for( i=0; i<nLen; i++ )
	{
		sprintf( s1, "%02X ", sData[i] );
		strcat( s, s1 );
	}

	cd.cbData = strlen(s);
	cd.lpData = s;

	::SendMessage( hSerialTracerWnd, WM_COPYDATA, NULL, (LPARAM)&cd );

	return 0;
}

int Serial_TraceMessage( int nPort, char *sMsg )
{
	if( hSerialTracerWnd == NULL ) return -20016;
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	CString			s;
	COPYDATASTRUCT	cd;

	s.Format( "%d:M:%s", nPort, sMsg );

	cd.cbData = s.GetLength();
	cd.lpData = s.GetBuffer(0);
	s.ReleaseBuffer();

	::SendMessage( hSerialTracerWnd, WM_COPYDATA, NULL, (LPARAM)&cd );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCSerialApp object

CCSerialApp theApp;
HANDLE		hComm[MAX_PORT_NUM] = {INVALID_HANDLE_VALUE};
OVERLAPPED	ovRead[MAX_PORT_NUM], ovWrite[MAX_PORT_NUM];

int _Config2( int nPort )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	BOOL	bRet;
	bRet = SetupComm( hComm[nPort], 32768, 32768 );		//in bytes, of the device's internal input/output buffer
	if( ! bRet ) return -20003;

	COMMTIMEOUTS	to;

	to.ReadIntervalTimeout = -1;			//1000		读间隔超时
	to.ReadTotalTimeoutMultiplier = 0;		//1000		读时间系数
	to.ReadTotalTimeoutConstant = 0;		//1000		读时间常量
	to.WriteTotalTimeoutMultiplier = 10;	//1000		写时间系数
	to.WriteTotalTimeoutConstant =100 ;		//1000		写时间常量

	bRet = SetCommTimeouts( hComm[nPort], &to );
	if( ! bRet ) return -20004;

	return 0;
}

int Serial_Open( int nPort )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	DWORD	dwError;
	char	sPort[20];
	int		nStatus;

	Serial_Close( nPort );

	sprintf( sPort, "\\\\.\\COM%d", nPort );

	hComm[nPort] = CreateFile( sPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
	dwError = GetLastError();
	if( hComm[nPort] == INVALID_HANDLE_VALUE || dwError != NO_ERROR ) return -20001;	

	nStatus = _Config2( nPort );
	if( nStatus != 0 )
	{
		Serial_Close( nPort );
		return nStatus;
	}
	
	nStatus = Serial_Config( nPort, CBR_115200, NOPARITY, 8, ONESTOPBIT );
	if( nStatus != 0 )
	{
		Serial_Close( nPort );
		return nStatus;
	}

	return Serial_Clear( nPort );
}

int Serial_Config( int nPort, long nBaudRate, int nParity, int nDataBits, int nStopBits )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	BOOL	bRet;
	DCB		dcb;

	bRet = GetCommState( hComm[nPort], &dcb );
	if( ! bRet ) return -20008;

	dcb.BaudRate = nBaudRate;
	dcb.ByteSize = nDataBits;
	dcb.Parity = nParity;
	dcb.StopBits = nStopBits;

	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fNull = FALSE;
	dcb.EvtChar = 0x49;//kw add for qualcomm  7e
	
	bRet = SetCommState( hComm[nPort], &dcb );
	if( ! bRet ) return -20005;

	return 0;
}

int Serial_Close( int nPort )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	BOOL	bRet;
	DWORD	dwError;

	if( hComm[nPort] != INVALID_HANDLE_VALUE )
	{
		bRet = CloseHandle( hComm[nPort] );
		dwError = GetLastError();
		if( ! bRet ) return -20002;
	
		hComm[nPort] = INVALID_HANDLE_VALUE;
	}

	return 0;
}

int Serial_InQueue( int nPort, DWORD &nLen )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	BOOL	bRet;
	COMSTAT stat;
	DWORD	dwError;

	bRet = ClearCommError( hComm[nPort], &dwError, &stat );
	if( ! bRet ) return -20007;

	nLen = stat.cbInQue;

	return 0;
}

int Serial_Clear( int nPort )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	BOOL	bRet;
	DWORD	dwError;

	bRet = PurgeComm( hComm[nPort], PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT );
	if( ! bRet ) return -20006;

	bRet = ClearCommError( hComm[nPort], &dwError, NULL );
	if( ! bRet ) return -20007;

	return 0;
}

int Serial_WriteChar( int nPort, BYTE cData )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	BYTE	sData[2] = {NULL};

	sData[0] = cData;

	return Serial_Write( nPort, sData, 1 );
}

int Serial_ReadChar( int nPort, BYTE &cData, DWORD nTimeout )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	int		ret;
	BYTE	sData[2] = {NULL};

	ret = Serial_Read( nPort, sData, 1, nTimeout );
	if( ret == 0 )
		cData = sData[0];

	return ret;
}

int Serial_Write( int nPort, BYTE *sData, DWORD nLen )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	BOOL	bRet;
	DWORD	nWriteCount;
	DWORD	dwError;
	int		nStatus;
	
	bRet = Serial_Clear( nPort );   
	
	bRet = WriteFile( hComm[nPort], sData, nLen, &nWriteCount, &ovWrite[nPort] );

	if( ! bRet )  
	{
		dwError = GetLastError();
		if( dwError == ERROR_IO_PENDING )
		{
			bRet = GetOverlappedResult( hComm[nPort], &ovWrite[nPort], &nWriteCount, TRUE );
			if( bRet )
				nStatus = 0;
			else 
				nStatus = -20013;
		}
		else
			nStatus = -20012;
	}
	else
		nStatus = 0;
#if 0 //modify 20151118
	SendSerialData(nPort, "W", sData, nLen, nStatus );		

	if( nStatus != 0 ) return nStatus;	
#endif
	if( nLen != nWriteCount ) return -20014;

	return 0;
}

int Serial_Read( int nPort, BYTE *sData, DWORD nLen, DWORD nTimeout )
{
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;

	BOOL	bRet;
//	DWORD	nStart;
	DWORD	nInCount, nReadCount;
//	COMSTAT stat;
	DWORD	dwError;
	int		nStatus;

#if 0  //modify 20151118
	nStart = GetTickCount();
	do{
		Sleep( 10 );
		bRet = ClearCommError( hComm[nPort], &dwError, &stat );
		if( ! bRet ) return -20007;
		nInCount = stat.cbInQue;
	}while( (nInCount < nLen) && (GetTickCount() - nStart < nTimeout) );

	if( nInCount < nLen) return -20009;
	Sleep(100);
	
#endif
	bRet = ReadFile( hComm[nPort], sData, nLen, &nReadCount, &ovRead[nPort] );

	if( ! bRet ) 
	{
		dwError = GetLastError();
		if(dwError==ERROR_IO_PENDING)
		{
			Sleep(500);
			bRet = GetOverlappedResult(hComm[nPort], &ovRead[nPort], 
                &nReadCount, FALSE) ; 
			
		}
		nStatus = -20010;
	}
	else nStatus = 0;
#if 0 //modify 20151118
	SendSerialData(nPort, "R", sData, nLen, nStatus );		

	if( nStatus != 0 ) return nStatus;	
#endif
	if( nLen != nReadCount ) return -20011;

	return 0;
}
// 
// int Serial_Read( int nPort, BYTE *sData, DWORD nLen, DWORD nTimeout )
// {
// 	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;
// 
// 	BOOL	bRet;
// 	DWORD	nStart;
// 	DWORD	nInCount, nReadCount;
// 	COMSTAT stat;
// 	DWORD	dwError;
// /*	int		nStatus;*/
// 
// 	nStart = GetTickCount();
// 	do{
// 		Sleep( 10 );
// 		bRet = ClearCommError( hComm[nPort], &dwError, &stat );
// 		if( ! bRet ) return -20007;
// 		nInCount = stat.cbInQue;
// 	}while( (nInCount < nLen) && (GetTickCount() - nStart < nTimeout) );
// 
// 	if( nInCount < nLen) return -20009;
// 	Sleep(100);
// 
// 	bRet = ReadFile( hComm[nPort], sData, nLen, &nReadCount, &ovRead[nPort] );
// 
// // 	if( ! bRet ) 
// // 	{
// // 		dwError = GetLastError();
// // 		if(dwError==ERROR_IO_PENDING)
// // 		{
// // 		//	Sleep(500);
// // 			bRet = GetOverlappedResult(hComm[nPort], &ovRead[nPort], 
// //                 &nReadCount, FALSE) ; 
// // 			
// // 		}
// // 		nStatus = -20010;
// // 	}
// // 	else nStatus = 0;
// // 	SendSerialData(nPort, "R", sData, nLen, nStatus );		
// // 
// // 	if( nStatus != 0 ) return nStatus;
// // 	if( nLen != nReadCount ) return -20011;
// 
// 	return 0;
// }

int Serial_Config_New( int nPort, long nBaudRate, int nParity, int nDataBits, int nStopBits )
{
	
	COMMTIMEOUTS timeouts;
	
	if( nPort < 1 || nPort >= MAX_PORT_NUM ) return -20015;
	
	
	BOOL	bRet;
	
	DCB		dcb;
	
	
	bRet = GetCommState( hComm[nPort], &dcb );
	
	if( ! bRet ) return -20008;
	
	
	dcb.BaudRate = nBaudRate;
	
	dcb.ByteSize = nDataBits;
	
	dcb.Parity = nParity;
	
	dcb.StopBits = nStopBits;
	// 	dcb.fOutxCtsFlow = FALSE;
	
	// 	dcb.fOutxDsrFlow = FALSE;// 	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	
	// 	dcb.fRtsControl = RTS_CONTROL_ENABLE;// 	dcb.fNull = FALSE;
	
	dcb.fBinary = TRUE;
	
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	
	dcb.fDsrSensitivity = FALSE;
	
	dcb.fTXContinueOnXoff = FALSE;
	
	dcb.fOutX = FALSE;
	
	dcb.fInX = FALSE;
	
	dcb.fErrorChar = FALSE;
	
	dcb.fNull = FALSE;
	
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	
	dcb.fAbortOnError = FALSE;
	
	dcb.fOutxCtsFlow = FALSE;
	
	dcb.fOutxDsrFlow = FALSE;
	
	dcb.EvtChar = 0x49;//kw add for qualcomm  7e
	
	
	bRet = SetCommState( hComm[nPort], &dcb );
	
	if( ! bRet ) return -20005;
	
	
//	将ReadIntervalTimeout设置为MAXDWORD，并将ReadTotalTimeoutMultiplier 和ReadTotalTimeoutConstant设置为0，表示读取操作将立即返回存放在输入缓冲区的字符。 

	timeouts.ReadIntervalTimeout = MAXWORD;//1
	
	timeouts.ReadTotalTimeoutMultiplier = 0;
	
	timeouts.ReadTotalTimeoutConstant = 0;
	
	timeouts.WriteTotalTimeoutMultiplier = 0;
	
	timeouts.WriteTotalTimeoutConstant = 0;
	
	if (!SetCommTimeouts(hComm[nPort], &timeouts))
		return -20006;	
	
	return 0;
}