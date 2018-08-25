
#ifndef __CSERIALLIB_H
#define __CSERIALLIB_H

#ifdef CSERIAL_LIB
#define CSERIAL_API  extern "C" __declspec(dllexport)
#else 
#define CSERIAL_API  extern "C" __declspec(dllimport)
#endif

CSERIAL_API	int Serial_Open( int nPort );
CSERIAL_API	int Serial_Close( int nPort );
CSERIAL_API	int Serial_Config( int nPort, long nBaudRate, int nParity, int nDataBits, int nStopBits );
CSERIAL_API	int Serial_InQueue( int nPort, DWORD &nLen );
CSERIAL_API	int Serial_Clear( int nPort );
CSERIAL_API	int Serial_WriteChar( int nPort, BYTE cData );
CSERIAL_API	int Serial_ReadChar( int nPort, BYTE &cData, DWORD nTimeout = 1000 );
CSERIAL_API	int Serial_Write( int nPort, BYTE *sData, DWORD nLen );
CSERIAL_API	int Serial_Read( int nPort, BYTE *sData, DWORD nLen, DWORD nTimeout = 1000 );
CSERIAL_API	int Serial_TraceMessage( int nPort, char *sMsg );
CSERIAL_API int Serial_Config_New( int nPort, long nBaudRate, int nParity, int nDataBits, int nStopBits );//xiugai160106
#endif	// __CSERIALLIB_H
