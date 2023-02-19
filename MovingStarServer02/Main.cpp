#include "Main.h"

#include "Network.h"

void Render();

int wmain(int argc, const wchar_t** argv)
{
    CONSOLE_CURSOR_INFO info{};
    info.dwSize = sizeof(info);
    info.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

    InitService();

	while (true)
	{
		NetworkProc();

		Render();
	}

	CloseService();
	return 0;
}

#define WIDTH 80
#define HEIGHT 23

static char s_szScreen[HEIGHT][WIDTH + 1];

inline void RenderStar(int x, int y)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

	s_szScreen[y][x] = '*';
}
 
void Render()
{
    memset(s_szScreen, ' ', sizeof(s_szScreen));
    
	for (int row = 0; row < HEIGHT; ++row)
        s_szScreen[row][WIDTH] = '\n';
    s_szScreen[HEIGHT - 1][WIDTH] = '\0';

	for (Session* pSession : g_sessionList)
        RenderStar(pSession->X, pSession->Y);

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
	printf((char*)s_szScreen);
}