#pragma once

enum MsgType : int
{
	MsgType_AssignID = 0,
	MsgType_CreateStar = 1,
	MsgType_DeleteStar = 2,
	MsgType_MoveStar = 3
};

struct Msg_AssignID
{
    MsgType Type;
    int ID;
    int unused[2];
};

struct Msg_CreateStar
{
    MsgType Type;
    int ID;
    int X;
    int Y;
};

struct Msg_DeleteStar
{
    MsgType Type;
    int ID;
    int unused[2];
};

struct Msg_MoveStar
{
    MsgType Type;
    int ID;
    int X;
    int Y;
};