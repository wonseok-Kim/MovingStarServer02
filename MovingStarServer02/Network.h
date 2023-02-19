#pragma once

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Container/List.h"
#include "Container/RingBuffer.h"

#define SERVER_PORT 3000

struct Session
{
	SOCKET Socket;
	RingBuffer SendQue;
	RingBuffer RecvQue;
    bool IsDisconnected;

	WCHAR IP[16];
	USHORT Port;

	int ID;
	int X;
	int Y;
};

extern SOCKET g_listenSocket;
extern List<Session*> g_sessionList;
extern int g_nextId;

void InitService();
void CloseService();

void NetworkProc();

void AcceptProc();
void RecvProc(Session* pSession);
void SendProc(Session* pSession);

void SendUnicast(Session* pToSend, void* data);
void SendBroadcast(Session* pToExcludeOrNull, void* data);

void Disconnect(Session* pSession);
