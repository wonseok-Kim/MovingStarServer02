#include "Network.h"

#include "Util/MyLog.h"
#include "Util/Util.h"

#include "Message.h"

SOCKET g_listenSocket;
List<Session*> g_sessionList;
int g_nextId;

void InitService()
{
    int ret1, ret2, ret3;    
    int err1, err2, err3, err4, err5;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        err1 = WSAGetLastError();
        Logf(L"WSAStartup err: %d", err1);
        Crash();
    }

    g_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_listenSocket == INVALID_SOCKET)
    {
        err2 = WSAGetLastError();
        Logf(L"socket() err: %d", err2);
        Crash();
    }

    SOCKADDR_IN serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERVER_PORT);

    ret1 = bind(g_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (ret1 == SOCKET_ERROR)
    {
        err3 = WSAGetLastError();
        Logf(L"bind() err: %d", err3);
        Crash();
    }

    u_long on = 1;
    ret2 = ioctlsocket(g_listenSocket, FIONBIO, &on);
    if (ret2 == SOCKET_ERROR)
    {
        err4 = WSAGetLastError();
        Logf(L"ioctlsocket() err: %d", err4);
        Crash();
    }

    ret3 = listen(g_listenSocket, SOMAXCONN);
    if (ret3 == SOCKET_ERROR)
    {
        err5 = WSAGetLastError();
        Logf(L"listen() err: %d", err5);
        Crash();
    }

    Log(L"Init Service Complete");
}

void CloseService()
{
    closesocket(g_listenSocket);
    WSACleanup();
}

void NetworkProc()
{
    int ret1;
    int err1;

    FD_SET readSet;
    FD_SET writeSet;

    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);

    FD_SET(g_listenSocket, &readSet);
    for (Session* pSession : g_sessionList)
    {
        FD_SET(pSession->Socket, &readSet);

        if (pSession->SendQue.GetUseSize() > 0)
            FD_SET(pSession->Socket, &writeSet);
    }

    ret1 = select(0, &readSet, &writeSet, NULL, NULL);
    if (ret1 == SOCKET_ERROR)
    {
        err1 = WSAGetLastError();
        Logf(L"select err: %d", err1);
        Crash();
    }

    if (FD_ISSET(g_listenSocket, &readSet))
    {
        AcceptProc();
    }
    for (Session* pSession : g_sessionList)
    {
        if (FD_ISSET(pSession->Socket, &readSet))
            RecvProc(pSession);
    }
    for (Session* pSession : g_sessionList)
    {
        if (FD_ISSET(pSession->Socket, &writeSet))
            SendProc(pSession);
    }

    List<Session*>::iterator iter = g_sessionList.begin();
    List<Session*>::iterator iterEnd = g_sessionList.end();
    while(iter != iterEnd)
    {
        if (iter->IsDisconnected)
        {
            Logf(L"closesocket() %s : %d (%d)", iter->IP, iter->Port, iter->ID);
            closesocket(iter->Socket);
            delete (*iter);
            iter = g_sessionList.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void AcceptProc()
{
    // int ret1;
    int err1, err2;

    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(clientAddr);
    SOCKET clientSock = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
    if (clientSock == INVALID_SOCKET)
    {
        err1 = WSAGetLastError();
        Logf(L"accept() err: %d", err1);
        Crash();
    }

    Session* pSession = new Session;
    pSession->Socket = clientSock;
    pSession->Port = ntohs(clientAddr.sin_port);
    if (InetNtopW(AF_INET, &clientAddr.sin_addr, pSession->IP, _countof(pSession->IP)) == nullptr)
    {
        err2 = WSAGetLastError();
        Logf(L"InetNtopW() err: %d in AcceptProc()", err2);
        Crash();
    }
    pSession->IsDisconnected = false;
    pSession->ID = g_nextId++;
    pSession->X = 40;
    pSession->Y = 12;

    Msg_AssignID msgAssignID;
    ZeroMemory(&msgAssignID, sizeof(msgAssignID));
    msgAssignID.Type = MsgType_AssignID;
    msgAssignID.ID = pSession->ID;
    SendUnicast(pSession, &msgAssignID);

    Msg_CreateStar msgCreateStar;    
    msgCreateStar.Type = MsgType_CreateStar;
    msgCreateStar.ID = pSession->ID;
    msgCreateStar.X = pSession->X;
    msgCreateStar.Y = pSession->Y;
    SendUnicast(pSession, &msgCreateStar);

    List<Session*>::iterator iter = g_sessionList.begin();
    List<Session*>::iterator iterEnd = g_sessionList.end();
    for (; iter != iterEnd; ++iter)
    {
        msgCreateStar.Type = MsgType_CreateStar;
        msgCreateStar.ID = iter->ID;
        msgCreateStar.X = iter->X;
        msgCreateStar.Y = iter->Y;
        SendUnicast(pSession, &msgCreateStar);
    }

    msgCreateStar.Type = MsgType_CreateStar;
    msgCreateStar.ID = pSession->ID;
    msgCreateStar.X = pSession->X;
    msgCreateStar.Y = pSession->Y;
    SendBroadcast(nullptr, &msgCreateStar);

    g_sessionList.push_back(pSession);
    Logf(L"Finished AcceptProc %s:%d (ID: %d)", pSession->IP, pSession->Port, pSession->ID);
}

void RecvProc(Session* pSession)
{
    int useSize;
    int dequeSize;
    int enqueSize;
    MsgType type;
    char msgBuf[16];

    char buf[1000];
    int recvRet;
    int recvErr;

    for (;;)
    {
        recvRet = recv(pSession->Socket, buf, sizeof(buf), 0);
        if (recvRet == SOCKET_ERROR)
        {
            recvErr = WSAGetLastError();
            if (recvErr != WSAEWOULDBLOCK)
            {
                Logf(L"%s:%d(%d) recv returned %d", pSession->IP, pSession->Port, pSession->ID, recvErr);
                Disconnect(pSession);                
            }
            break;
        }
        else if (recvRet == 0)
        {
            Logf(L"%s:%d(%d) recv returned 0", pSession->IP, pSession->Port, pSession->ID);
            Disconnect(pSession);
            break;
        }

        enqueSize = pSession->RecvQue.Enqueue(buf, recvRet);
        if (enqueSize != recvRet)
        {
            Logf(L"%s:%d(%d) recvQueue is full. going to disconnect", pSession->IP, pSession->Port, pSession->ID);
            Disconnect(pSession);
            break;
        }

        for (;;)
        {
            useSize = pSession->RecvQue.GetUseSize();
            if (useSize < 16)
                break;

            dequeSize = pSession->RecvQue.Dequeue(msgBuf, 16);
            if (dequeSize != 16)
            {
                Logf(L"dequeSize(%d) != msgSize(16) in %s", dequeSize, __FUNCTIONW__);
                Crash();
            }

            type = *(MsgType*)msgBuf;
            switch (type)
            {
            case MsgType_MoveStar: {
                Msg_MoveStar* pMsg = (Msg_MoveStar*)msgBuf;

                for (Session* iter : g_sessionList)
                {
                    if (iter->ID == pMsg->ID)
                    {
                        iter->X = pMsg->X;
                        iter->Y = pMsg->Y;

                        SendBroadcast(iter, pMsg);// TODO: 내가 보낼 때, 다른 놈들이 보낼때 구분
                        break;
                    }
                }
            }
            break;

            default:
                Logf(L"%s:%d(%d) send invalid type msg", pSession->IP, pSession->Port, pSession->ID);
                Disconnect(pSession); 
                return;
            }
        }
    }
}

void SendProc(Session* pSession)
{
    int err1;
    int retSend;
    int retMove;
    int peekSize;
    char buf[1000];

    int useSize = pSession->SendQue.GetUseSize();
    while (useSize > 0)
    {
        peekSize = pSession->SendQue.Peek(buf, 1000);

        retSend = send(pSession->Socket, buf, peekSize, 0);
        if (retSend == SOCKET_ERROR)
        {
            err1 = WSAGetLastError();
            if (err1 == WSAEWOULDBLOCK)
                break;

            Logf(L"%s:%d(%d) send() err: %d", pSession->IP, pSession->Port, pSession->ID, err1);
            Disconnect(pSession);
            break;
        }

        retMove = pSession->SendQue.MoveFront(retSend);
        if (retMove != retSend)
        {
            Logf(L"retMove(%d) != retSend(%d) in %s", retMove, retSend, __FUNCTIONW__);
            Crash();
        }
        useSize -= retSend;
    }
}

void SendUnicast(Session* pToSend, void* data)
{
    int enqueSize = pToSend->SendQue.Enqueue(data, 16);
    if (enqueSize != 16)
    {
        Disconnect(pToSend);
    }
}

void SendBroadcast(Session* pToExcludeOrNull, void* data)
{
    int enqueSize;
    List<Session*>::iterator iter = g_sessionList.begin();
    List<Session*>::iterator iterEnd = g_sessionList.end();

    for (; iter != iterEnd; ++iter)
    {
        if ((*iter) == pToExcludeOrNull)
            continue;

        enqueSize = iter->SendQue.Enqueue(data, 16);
        if (enqueSize != 16)
        {
            Disconnect(*iter);
        }
    }
}

void Disconnect(Session* pSession)
{
    if (pSession->IsDisconnected)
        return;

    pSession->IsDisconnected = true;

    Msg_DeleteStar msgDeleteStar{};
    msgDeleteStar.Type = MsgType_DeleteStar;
    msgDeleteStar.ID = pSession->ID;    
    SendBroadcast(pSession, &msgDeleteStar);

    Logf(L"%s : %d (%d) set disconnected", pSession->IP, pSession->Port, pSession->ID); 
}
