#include <windows.h>
#include <bits/stdc++.h>
using namespace std;

struct OffScreenBuffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
};

struct WindowD
{
    int width;
    int height;
};

enum Key
{
    KEY_0 = 0x30,
    KEY_1 = 0x31,
    KEY_2 = 0x32,
    KEY_3 = 0x33,
    KEY_4 = 0x34,
    KEY_5 = 0x35,
    KEY_6 = 0x36,
    KEY_7 = 0x37,
    KEY_8 = 0x38,
    KEY_9 = 0x39,


    KEY_A = 'A',
    KEY_B = 'B',
    KEY_C = 'C',
    KEY_D = 'D',
    KEY_E = 'E',
    KEY_F = 'F',
    KEY_G = 'G',
    KEY_H = 'H',
    KEY_I = 'I',
    KEY_J = 'J',
    KEY_K = 'K',
    KEY_L = 'L',
    KEY_M = 'M',
    KEY_N = 'N',
    KEY_O = 'O',
    KEY_P = 'P',
    KEY_Q = 'Q',
    KEY_R = 'R',
    KEY_S = 'S',
    KEY_T = 'T',
    KEY_U = 'U',
    KEY_V = 'V',
    KEY_W = 'W',
    KEY_X = 'X',
    KEY_Y = 'Y',
    KEY_Z = 'Z',


    KEY_SHIFT = VK_SHIFT,  
    KEY_CTRL = VK_CONTROL, 
    KEY_ALT = VK_MENU,     
    KEY_SPACE = VK_SPACE,  
    
    
    KEY_ESCAPE = VK_ESCAPE, 
    KEY_ENTER = VK_RETURN,  
    KEY_COUNT = 256         
};



struct Mouse 
{
    bool buttonsclicked[3];
    bool buttonsheld[3];
    bool buttonreleased[3];
    int x;          
    int y;    
};


bool KeyPressed[KEY_COUNT] = {false};
bool KeyHeld[KEY_COUNT] = {false};
bool KeyReleased[KEY_COUNT] = {false};

Mouse CurrentMouseState;
static HDC GlobalDeviceContext;
static bool IsRunning;
static OffScreenBuffer Buffer01;


int widthofpixel = 1;
int heightofpixel = 1;
int widthofwindowinpixel = 879;
int heightofwindowinpixel = 879;


uint32_t GetColorByRGBA(int red, int green, int blue, int alpha = 0)
{
    uint32_t color = blue | (green << 8) | (red << 16) | (alpha << 24);
    return color;
}


void SetPixel(OffScreenBuffer &Buffer, int x, int y, uint32_t color)
{
    uint32_t *pixel = (uint32_t *)Buffer.Memory;
    pixel += y * Buffer.Width;
    pixel += x;
    *pixel = color;
}



void RandomColor(OffScreenBuffer &Buffer)
{
    for(int i = 0;i < Buffer.Height;i++)
    {  
        for(int j = 0; j<Buffer.Width;j++)
        {
            uint8_t blue,red,green;
            blue = rand()%256;
            green = rand()%256;
            red = rand()%256;
            uint32_t color = GetColorByRGBA(red,green,blue);
            SetPixel(Buffer01,j,i,color);
        }
    }
}

void DRect(OffScreenBuffer &Buffer, int x, int y, int width, int height, uint32_t color)
{
    int lastw = min(x + width, Buffer.Width - 1);
    int lasth = min(y + height, Buffer.Height - 1);
    for (int i = max(x, 0); i < lastw; i++)
    {
        for (int j = max(y, 0); j < lasth; j++)
        {
            SetPixel(Buffer, i, j, color);
        }
    }
}

void ClearBuffer(OffScreenBuffer &Buffer)
{
    uint32_t *pixel = (uint32_t *)Buffer.Memory;
    for (int i = 0; i < Buffer.Height; i++)
    {
        for (int j = 0; j < Buffer.Width; j++)
        {
            *(pixel + j) = 0; 
        }
        pixel += Buffer.Width;
    }
}


void ResizeDIBSection(OffScreenBuffer &Buffer, int Width, int Height)
{
    if (Buffer.Memory)
    {
        VirtualFree(Buffer.Memory, 0, MEM_RELEASE);
    }
    Buffer.Width = Width;
    Buffer.Height = Height;
    Buffer.Info.bmiHeader.biSize = sizeof(Buffer.Info.bmiHeader);
    Buffer.Info.bmiHeader.biHeight = -Buffer.Height;
    Buffer.Info.bmiHeader.biWidth = Buffer.Width;
    Buffer.Info.bmiHeader.biPlanes = 1;
    Buffer.Info.bmiHeader.biBitCount = 32;
    Buffer.Info.bmiHeader.biCompression = BI_RGB;
    Buffer.Memory = VirtualAlloc(0, Width * Height * 4, MEM_COMMIT, PAGE_READWRITE);
    unsigned char *pixel = (unsigned char *)Buffer.Memory;
}

void UpdateFullWindow(HDC dc, WindowD dimensions, OffScreenBuffer &buffer)
{
    StretchDIBits(dc,
                  0, 0, dimensions.width, dimensions.height, 
                  0, 0, buffer.Width, buffer.Height,
                  buffer.Memory, &buffer.Info, DIB_RGB_COLORS, SRCCOPY);
}

WindowD GetWindowD(RECT *WindowRect)
{
    WindowD result;
    result.height = WindowRect->bottom - WindowRect->top;
    result.width = WindowRect->right - WindowRect->left;
    return result;
}

LRESULT CALLBACK EventHandler(HWND Window, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    LRESULT result = 0;
    switch (Msg)
    {
    case WM_CREATE:
    {
        GlobalDeviceContext = GetDC(Window);
        ResizeDIBSection(Buffer01, widthofwindowinpixel, heightofwindowinpixel);
        break;
    }
    case WM_CLOSE:
    {
        IsRunning = false;
        break;
    }
    case WM_SIZE:
    {
        break;
    }
    case WM_DESTROY:
    {
        IsRunning = false;
        ReleaseDC(Window, GlobalDeviceContext);
        break;
    }
    case WM_ACTIVATEAPP:
    {
        result = DefWindowProcA(Window, Msg, WParam, LParam);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(Window, &ps);
        RECT Rectangle;
        GetClientRect(Window, &Rectangle);
        WindowD Dimensions2 = GetWindowD(&Rectangle);
        UpdateFullWindow(GlobalDeviceContext, Dimensions2, Buffer01);
        EndPaint(Window, &ps);
        break;
    }
    case WM_KEYDOWN:
    {
        if (WParam < KEY_COUNT)
        {
            if((LParam & (1 << 30)) == 0)
            {
                KeyPressed[WParam] = true;
            }
            KeyHeld[WParam] = true;
        }
        break;
    }
    case WM_KEYUP:
    {
        if (WParam < KEY_COUNT)
        {
            KeyPressed[WParam] = false;   
            KeyHeld[WParam] = false;      
            KeyReleased[WParam] = true;
        }
        break;
    }
    default:
    {
        result = DefWindowProcA(Window, Msg, WParam, LParam);
        break;
    }
    }
    return result;
}

//5 precision
bool probability(float p)
{
    int comp = p*100000;
    int randomnum = abs(rand() % 1000000);
    return (randomnum <= comp);
}


/// @brief MAIN CODE STARTS HERE


struct coord
{
    int xposn;
    int yposn;
    static const int coordmax = 35;
};

coord getcoordinateog(int x35,int y35)
{
    coord Coordinate;
    Coordinate.xposn = 25*x35+5;
    Coordinate.yposn = 25*y35+5;
    return Coordinate;
}

coord get35coord(int xog,int yog)
{
    coord Coordinate;
    Coordinate.xposn = (xog-5)/25;
    Coordinate.yposn = (yog-5)/25;
    return Coordinate;
}

coord getrandomneighbour(int x35,int y35,bool array2d[][coord::coordmax])
{
    coord arr[4];

    arr[0].xposn = x35;
    arr[0].yposn = y35-1;

    arr[1].xposn = x35+1;
    arr[1].yposn = y35;

    arr[2].xposn = x35;
    arr[2].yposn = y35+1;

    arr[3].xposn = x35-1;
    arr[3].yposn = y35;

    vector<coord> validcoords;
    for(int i = 0;i < 4;i++)
    {
        if((arr[i].xposn < 35) and (arr[i].xposn > -1) and (arr[i].yposn < 35) and (arr[i].yposn > -1) and (array2d[arr[i].xposn][arr[i].yposn] == 0))
        {
            validcoords.push_back(arr[i]);
        }
    }

    if(validcoords.size() == 0)
    {
        coord invalidcoord;
        invalidcoord.xposn = -1;
        invalidcoord.yposn = -1;
        return invalidcoord;
    }
    else
    {
        coord valid = validcoords[abs(rand()%validcoords.size())];
        return getcoordinateog(valid.xposn,valid.yposn);
    }
}

void link2coords(coord top,coord newadd)
{
    int xdiff = (top.xposn - newadd.xposn)/25;
    int ydiff = (top.yposn - newadd.yposn)/25;
    int blitfrmx;
    int blitfrmy;
    int heightrect;
    int widthrect;
    if(xdiff + ydiff > 0)
    {
        blitfrmx = newadd.xposn; 
        blitfrmy = newadd.yposn;
        heightrect = (ydiff)*5 + 20;
        widthrect = (xdiff)*5 + 20;
    }
    else
    {
        blitfrmx = top.xposn + (-xdiff)*20; 
        blitfrmy = top.yposn + (-ydiff)*20;
        heightrect = (-ydiff)*5 + 20;
        widthrect = (-xdiff)*5 + 20;
    }
    DRect(Buffer01,blitfrmx,blitfrmy,widthrect,heightrect,GetColorByRGBA(30, 90, 234));
}

struct GameState
{
    std::stack<coord> lastprocessed;
    bool visited[35][35] = {};
    int totalvisited = 0;
};

// Singleton pattern
GameState& GetGameState()
{
    static GameState state;
    return state;
}


void gameinit()
{
    GameState& gameState = GetGameState();
    coord startingpt;
    startingpt.xposn = 5;
    startingpt.yposn = 5;
    gameState.lastprocessed.push(startingpt);
    gameState.visited[0][0] = 1;
    gameState.totalvisited = 1;
}

void updatebuffer()
{
    GameState& gameState = GetGameState();
    if(KeyPressed[KEY_C])
    {
        ClearBuffer(Buffer01);
    }
    if(KeyReleased[KEY_N])
    {
        ClearBuffer(Buffer01);
        for(int y = 0;y < Buffer01.Height;y++)
        {
            for(int x = 0;x < Buffer01.Width;x++)
            {
                if((x%25 >= 5) and (y%25 >= 5))
                {
                    uint32_t colordefault = GetColorByRGBA(234,30,90);
                    SetPixel(Buffer01,x,y,colordefault);
                }
            }
        }
        DRect(Buffer01, 0, 0, 25, 25, GetColorByRGBA(30, 90, 234));
        DRect(Buffer01, 855, 855, 25, 25, GetColorByRGBA(30, 90, 234));
    }
    if(KeyPressed[KEY_S])
    {
        if((!gameState.lastprocessed.empty()) and (gameState.totalvisited != 35*35))
        {
            coord top35 = get35coord(gameState.lastprocessed.top().xposn,gameState.lastprocessed.top().yposn);
            coord newcomer = getrandomneighbour(top35.xposn,top35.yposn,gameState.visited);
            coord newcomer35 = get35coord(newcomer.xposn,newcomer.yposn);
            if((newcomer.xposn != -1) and (newcomer.yposn != -1))
            {
                link2coords(gameState.lastprocessed.top(),newcomer);
                gameState.visited[newcomer35.xposn][newcomer35.yposn] = 1;
                gameState.totalvisited++;
                gameState.lastprocessed.push(newcomer);
            }
            else
            {
                gameState.lastprocessed.pop();
            }
        }
    }
    if(KeyHeld[KEY_F])
    {
        if((!gameState.lastprocessed.empty()) and (gameState.totalvisited != 35*35))
        {
            coord top35 = get35coord(gameState.lastprocessed.top().xposn,gameState.lastprocessed.top().yposn);
            coord newcomer = getrandomneighbour(top35.xposn,top35.yposn,gameState.visited);
            coord newcomer35 = get35coord(newcomer.xposn,newcomer.yposn);
            if((newcomer.xposn != -1) and (newcomer.yposn != -1))
            {
                link2coords(gameState.lastprocessed.top(),newcomer);
                gameState.visited[newcomer35.xposn][newcomer35.yposn] = 1;
                gameState.totalvisited++;
                gameState.lastprocessed.push(newcomer);
            }
            else
            {
                gameState.lastprocessed.pop();
            }
        }
    }
    memset(KeyPressed, 0, sizeof(KeyPressed));
    memset(KeyReleased, 0, sizeof(KeyReleased));
}


int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR StartCommand, int ShowCode)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = EventHandler;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "OakEngineWindowClass";
    if (RegisterClassA(&WindowClass))
    {
        
        //GAME INIT FUNC
        gameinit();


        HWND WindowHandle = CreateWindowExA(0, WindowClass.lpszClassName, "OakEngine",WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, 0, 0, heightofwindowinpixel * heightofpixel, widthofwindowinpixel * widthofpixel, 0, 0, Instance, 0);
        if (WindowHandle)
        {
            IsRunning = true;
            MSG Message;
            while (IsRunning)
            {
                while (PeekMessageW(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        IsRunning = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                updatebuffer(/*elapsed time*/);
                RECT Rectangle;
                GetClientRect(WindowHandle, &Rectangle);
                WindowD Dimensions = GetWindowD(&Rectangle);
                UpdateFullWindow(GlobalDeviceContext, Dimensions, Buffer01);
            }
        }
        else
        {
            OutputDebugStringA("Window Nahi bana\n");
        }
    }
    else
    {
        OutputDebugStringA("window class reg nhi hua\n");
    }
    return 0;
}