#include <bits/stdc++.h>
#include <thread>
#include <chrono>
#include <windows.h>
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
    int x = 0;
    int y = 0;
    void ResetMouseStates()
    {
        memset(buttonsclicked, 0, sizeof(buttonsclicked));
        memset(buttonreleased, 0, sizeof(buttonreleased));
    }
};

bool KeyPressed[KEY_COUNT] = {false};
bool KeyHeld[KEY_COUNT] = {false};
bool KeyReleased[KEY_COUNT] = {false};

Mouse CurrentMouseState;
static HDC GlobalDeviceContext;
static bool IsRunning;
static OffScreenBuffer Buffer01;

int widthofpixel = 5;
int heightofpixel = 5;
int widthofwindowinpixel = 100;
int heightofwindowinpixel = 100;

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
    for (int i = 0; i < Buffer.Height; i++)
    {
        for (int j = 0; j < Buffer.Width; j++)
        {
            uint8_t blue, red, green;
            blue = rand() % 256;
            green = rand() % 256;
            red = rand() % 256;
            uint32_t color = GetColorByRGBA(red, green, blue);
            SetPixel(Buffer01, j, i, color);
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

pair<int, int> ScreenToBitmap(int32_t x, int32_t y)
{
    if (x < 0 || y < 0 || x >= widthofwindowinpixel || y >= heightofwindowinpixel)
    {
        return make_pair(-1, -1);
    }
    int bitmapX = x / widthofpixel;
    int bitmapY = y / heightofpixel;
    return make_pair(bitmapX, bitmapY);
}

pair<int, int> BitmapToScreen(int32_t x, int32_t y)
{
    if (x < 0 || y < 0 || x >= Buffer01.Width || y >= Buffer01.Height)
    {
        return make_pair(-1, -1);
    }
    int screenX = x * widthofpixel;
    int screenY = y * heightofpixel;
    return make_pair(screenX, screenY);
}

// drawcircle if you think about symmetry you can just compute 1/2 part of 1/4th part or 1/8th part but we will choose 1/8th because in 1/2 and 1/4th we have many options but in cacl 1/8th part we are sure that we have to move one coordinate and there are only 2 choices of 2nd coordinate so we calc 1/8th part with some algo on internet and then draw whole part

void DrawCircle(int16_t xc, int16_t yc, int16_t radius, uint32_t color)
{
    // Start at the top of the circle
    int16_t x = 0;
    int16_t y = radius;

    // Initial decision parameter
    // d = decision parameter
    // Starts with 3 - 2r to handle initial pixel placement
    int16_t d = 3 - 2 * radius;

    while (y >= x)
    {
        // Draw 8 symmetric points in each octant

        // First Octant Points (x,y) and symmetric points
        SetPixel(Buffer01, xc + x, yc + y, color); // Octant 1
        SetPixel(Buffer01, xc - x, yc + y, color); // Octant 4
        SetPixel(Buffer01, xc + x, yc - y, color); // Octant 8
        SetPixel(Buffer01, xc - x, yc - y, color); // Octant 5

        // Second Octant Points (y,x) and symmetric points
        SetPixel(Buffer01, xc + y, yc + x, color); // Octant 2
        SetPixel(Buffer01, xc - y, yc + x, color); // Octant 3
        SetPixel(Buffer01, xc + y, yc - x, color); // Octant 7
        SetPixel(Buffer01, xc - y, yc - x, color); // Octant 6

        // Increment x coordinate
        x++;

        // Decide whether to move y coordinate
        if (d > 0)
        {
            // If decision parameter is positive,
            // move diagonally down
            y--;

            // Update decision parameter
            // Uses a more complex formula to track circle boundary
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            // If decision parameter is negative or zero,
            // move straight horizontally
            d = d + 4 * x + 6;
        }
    }
}

// Variation for filled circle
void DrawFilledCircle(int16_t xc, int16_t yc, int16_t radius, uint32_t color)
{
    int16_t x = 0;
    int16_t y = radius;
    int16_t d = 3 - 2 * radius;

    while (y >= x)
    {
        for (int16_t i = xc - x; i <= xc + x; i++)
        {
            SetPixel(Buffer01, i, yc + y, color);
            SetPixel(Buffer01, i, yc - y, color);
        }

        for (int16_t i = xc - y; i <= xc + y; i++)
        {
            SetPixel(Buffer01, i, yc + x, color);
            SetPixel(Buffer01, i, yc - x, color);
        }

        x++;

        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
    }
}

// my approach to drawline
// am thinking of taking gradient then render the line like lets suppose 2 points are 0,0 and 10 , 20 so gradient is 2 so i will color 2 pixels then change y and when gradient is not a perfect natural number i will take modulo and distribute extras equally
// like for example if we have 0 , 0 and 10 , 25 i will render 2 pixels of 7 for each x i will render 2 pixels and for some x i will render 3 and these 3 pixeled x's will be distributed uniformly
// https://www.youtube.com/watch?v=RGB-wlatStc and below algorithm is similar to that its just simple and easy to implement  Bresenhams algo
// althought your logic is bad big complex not efficient but it was perfect approach for nice lines and you also did handle the float part
// there is some error in modulo part but now we will use readymade bresenhams algo
//  void DrawLine(int16_t x1, int16_t y1,int16_t x2,int16_t y2,uint32_t lcolor)
//  {
//      //drawing line from x1y1 to x2y2
//      int16_t dy,dx,incx,incy,modulo,step,uniformdividemodulo;
//      dy = abs(y2-y1) + 1;
//      dx = abs(x2-x1) + 1;
//      incy = (y2 > y1) ? 1 : -1;
//      incx = (x2 > x1) ? 1 : -1;
//      if(dy > dx)
//      {
//          step = dy/dx;
//          modulo = dy%dx;
//          uniformdividemodulo = (modulo == 0) ? 0 : (dy - modulo)/ modulo;
//          int tempuniform = uniformdividemodulo;
//          int stepsizen = step;
//          int xmov = x1;
//          for(int i = y1;i != y2+incy;)
//          {
//              if(!stepsizen)
//              {
//                  stepsizen = step;
//                  if((modulo) and (!uniformdividemodulo))
//                  {
//                      uniformdividemodulo = tempuniform;
//                      stepsizen++;
//                      modulo--;
//                  }
//                  xmov += incx;
//                  uniformdividemodulo--;
//              }
//              else
//              {
//                  stepsizen--;
//                  SetPixel(Buffer01,xmov,i,lcolor);
//                  cout << xmov << " " << i <<endl;
//                  i += incy;
//              }
//          }
//      }
//      else
//      {
//          step = dx/dy;
//          modulo = dx%dy;
//          uniformdividemodulo = (modulo == 0) ? 0 : (dx - modulo)/ modulo;
//          int tempuniform = uniformdividemodulo;
//          int stepsizen = step;
//          int ymov = y1;
//          for(int i = x1;i != x2+incx;)
//          {
//              if(!stepsizen)
//              {
//                  stepsizen = step;
//                  if((modulo) and (!uniformdividemodulo))
//                  {
//                      uniformdividemodulo = tempuniform;
//                      stepsizen++;
//                      modulo--;
//                  }
//                  ymov += incy;
//                  uniformdividemodulo--;
//              }
//              else
//              {
//                  stepsizen--;
//                  SetPixel(Buffer01,i,ymov,lcolor);
//                  cout << i << " " << ymov <<endl;
//                  i+=incx;
//              }
//          }
//      }
//  }

void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color)
{
    bool steep = abs(y2 - y1) > abs(x2 - x1);

    if (steep)
    {
        swap(x1, y1);
        swap(x2, y2);
    }

    if (x1 > x2)
    {
        swap(x1, x2);
        swap(y1, y2);
    }

    int16_t dx = x2 - x1;
    int16_t dy = abs(y2 - y1);
    int16_t error = dx / 2;

    int16_t y_step = (y1 < y2) ? 1 : -1;
    int16_t y = y1;

    for (int16_t x = x1; x <= x2; x++)
    {
        if (steep)
        {
            SetPixel(Buffer01, y, x, color);
        }
        else
        {
            SetPixel(Buffer01, x, y, color);
        }

        error -= dy;
        if (error < 0)
        {
            y += y_step;
            error += dx;
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
            if ((LParam & (1 << 30)) == 0)
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
    case WM_LBUTTONDOWN:
    {

        CurrentMouseState.buttonsclicked[0] = true;
        CurrentMouseState.buttonsheld[0] = true;
        break;
    }
    case WM_LBUTTONUP:
    {

        CurrentMouseState.buttonreleased[0] = true;
        CurrentMouseState.buttonsheld[0] = false;
        break;
    }
    case WM_RBUTTONDOWN:
    {
        CurrentMouseState.buttonsclicked[1] = true;
        CurrentMouseState.buttonsheld[1] = true;
        break;
    }
    case WM_RBUTTONUP:
    {
        CurrentMouseState.buttonreleased[1] = true;
        CurrentMouseState.buttonsheld[1] = false;
        break;
    }
    case WM_MOUSEMOVE:
    {
        CurrentMouseState.x = LOWORD(LParam);
        CurrentMouseState.y = HIWORD(LParam);
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

void writedata(string filename, void *ptrtomemory, int numberofbytes)
{
    fstream file;
    file.open(filename, ios::out | ios::binary);
    if (file.is_open())
    {
        file.write(static_cast<char *>(ptrtomemory), numberofbytes);
        file.close();
    }
    else
    {
        cout << "ERROR COULD NOT OPEN FILE Write" << endl;
    }
}

void appenddata(string filename, void *ptrtomemory, int numberofbytes)
{
    fstream file;
    file.open(filename, ios::app | ios::binary);
    if (file.is_open())
    {
        file.write(static_cast<char *>(ptrtomemory), numberofbytes);
        file.close();
    }
    else
    {
        cout << "ERROR COULD NOT OPEN FILE Append" << endl;
    }
}

void readdata(string filename, void *ptrwheredatawillberead, int numberofbytes)
{
    fstream file;
    file.open(filename, ios::in | ios::binary);
    if (file.is_open())
    {
        file.read(static_cast<char *>(ptrwheredatawillberead), numberofbytes);
        file.close();
    }
    else
    {
        cout << "ERROR: Could not open file for reading." << endl;
    }
}

// 5 precision
bool probability(float p)
{
    int comp = p * 100000;
    int randomnum = abs(rand() % 1000000);
    return (randomnum <= comp);
}

struct GameState
{
    // define game variables
};

// Singleton pattern
GameState &GetGameState()
{
    static GameState state;
    return state;
}

void gameinit()
{
    // game init
}

void updatebuffer()
{
    // event handler
    CurrentMouseState.ResetMouseStates();
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
        HWND WindowHandle = CreateWindowExA(0, WindowClass.lpszClassName, "OakEngine", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, 0, 0, heightofwindowinpixel * heightofpixel, widthofwindowinpixel * widthofpixel, 0, 0, Instance, 0);
        if (WindowHandle)
        {
            gameinit();
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

// todo add serialisation for non linear data structures