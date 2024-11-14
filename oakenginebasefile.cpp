#include <bits/stdc++.h>
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

struct sprite
{
    int32_t height = 0;
    int32_t width = 0;
    uint32_t *ptrtoimgdata = nullptr;
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
int widthofwindowinpixel = 800;
int heightofwindowinpixel = 800;

uint32_t GetColorByRGBA(int red, int green, int blue, int alpha = 255)
{
    uint32_t color = blue | (green << 8) | (red << 16) | (alpha << 24);
    return color;
}

void SetPixel(OffScreenBuffer &Buffer, int x, int y, uint32_t &color)
{
    if (x < 0 || x >= Buffer.Width || y < 0 || y >= Buffer.Height)
    {
        return; 
    }

    uint32_t *pixel = (uint32_t *)Buffer.Memory;
    pixel += y * Buffer.Width + x;

    uint8_t *extract = (uint8_t *)pixel;
    uint8_t bufferblue = *(extract++);
    uint8_t buffergreen = *(extract++);
    uint8_t bufferred = *(extract++);
    uint8_t bufferalpha = 255; 

    uint8_t colorblue = color & 0xff;
    uint8_t colorgreen = (color >> 8) & 0xff;
    uint8_t colorred = (color >> 16) & 0xff;
    uint8_t coloralpha = (color >> 24) & 0xff;

    float alpha = coloralpha / 255.0f;

    uint8_t finalcolorblue = (uint8_t)((alpha * colorblue) + ((1 - alpha) * bufferblue));
    uint8_t finalcolorgreen = (uint8_t)((alpha * colorgreen) + ((1 - alpha) * buffergreen));
    uint8_t finalcolorred = (uint8_t)((alpha * colorred) + ((1 - alpha) * bufferred));

    uint8_t finalcoloralpha = 255; 

    uint32_t finalcolor = (finalcoloralpha << 24) | (finalcolorred << 16) | (finalcolorgreen << 8) | finalcolorblue;

    *pixel = finalcolor;
}





void SetPixelRGBA(OffScreenBuffer &Buffer, int x, int y, uint32_t &color)
{
    uint8_t r = (color >> 24) & 0xFF;  
    uint8_t g = (color >> 16) & 0xFF;  
    uint8_t b = (color >> 8) & 0xFF;   
    uint8_t a = color & 0xFF;          

    uint32_t bgraColor = (b << 24) | (g << 16) | (r << 8) | a;
    uint32_t *pixel = (uint32_t *)Buffer.Memory;
    pixel += y * Buffer.Width;
    pixel += x;
    *pixel = bgraColor;
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

void DrawCircle(int16_t xc, int16_t yc, int16_t radius, uint32_t color)
{
    int16_t x = 0;
    int16_t y = radius;
    int16_t d = 1 - radius;

    while (x <= y)
    {
        SetPixel(Buffer01, xc + x, yc + y, color);
        SetPixel(Buffer01, xc - x, yc + y, color);
        SetPixel(Buffer01, xc + x, yc - y, color);
        SetPixel(Buffer01, xc - x, yc - y, color);
        SetPixel(Buffer01, xc + y, yc + x, color);
        SetPixel(Buffer01, xc - y, yc + x, color);
        SetPixel(Buffer01, xc + y, yc - x, color);
        SetPixel(Buffer01, xc - y, yc - x, color);

        x++;
        if (d < 0)
        {
            d += 2 * x + 1;
        }
        else
        {
            y--;
            d += 2 * (x - y) + 1;
        }
    }
}

void DrawFilledCircle(int16_t xc, int16_t yc, int16_t radius, uint32_t color)
{
    int16_t x = 0;
    int16_t y = radius;
    int16_t d = 1 - radius;

    while (x <= y)
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
        if (d < 0)
        {
            d += 2 * x + 1;
        }
        else
        {
            y--;
            d += 2 * (x - y) + 1;
        }
    }
}


sprite* loadspritebmp(const string &filename) 
{
    sprite* NewSprite = new sprite;

    ifstream bmpfile(filename, ios::binary);
    if (!bmpfile) {
        cout << "ERROR: LOADING THE FILE" << endl;
        return NewSprite;
    }

    uint16_t fileType;
    bmpfile.read(reinterpret_cast<char*>(&fileType), sizeof(fileType));
    if (fileType != 0x4D42) {  
        cout << "ERROR: Not a valid BMP file." << endl;
        bmpfile.close();
        return NewSprite;
    }

    uint32_t dataOffset;
    bmpfile.seekg(10, ios::beg);
    bmpfile.read(reinterpret_cast<char*>(&dataOffset), sizeof(dataOffset));

    int32_t width, height;
    bmpfile.seekg(18, ios::beg);
    bmpfile.read(reinterpret_cast<char*>(&width), sizeof(width));
    bmpfile.read(reinterpret_cast<char*>(&height), sizeof(height));

    NewSprite->width = width;
    NewSprite->height = abs(height);

    uint16_t bitDepth;
    bmpfile.seekg(28, ios::beg);
    bmpfile.read(reinterpret_cast<char*>(&bitDepth), sizeof(bitDepth));
    if (bitDepth != 32) {
        cout << "ERROR: Only 32-bit BMP files are supported." << endl;
        bmpfile.close();
        delete NewSprite;
        return nullptr;
    }

    bmpfile.seekg(dataOffset, ios::beg);

    uint32_t numPixels = width * abs(height);
    uint32_t *pixelData = new uint32_t[numPixels];

    for (int32_t y = abs(height)-1; y >= 0; y--) {
        bmpfile.read(reinterpret_cast<char*>(pixelData + (y * width)), width * sizeof(uint32_t));
    }

    bmpfile.close();
    NewSprite->ptrtoimgdata = pixelData;
    cout << "LOADED IMAGE" << endl;

    return NewSprite;
}

void drawonscreen(int x, int y, sprite* spritetodraw, OffScreenBuffer &Buffer01)
{
    uint32_t *temppointer;
    for (int i = 0; i < spritetodraw->height; i++)
    {
        temppointer = spritetodraw->ptrtoimgdata + (i * spritetodraw->width);
        for (int j = 0; j < spritetodraw->width; j++)
        {
            uint32_t colorp = *temppointer;
            temppointer++;
            int targetx = x + j;
            int targety = y + i;
            if ((targetx >= 0 && targetx < Buffer01.Width) && (targety >= 0 && targety < Buffer01.Height))
            {
                SetPixel(Buffer01, targetx, targety, colorp);
            }
        }
    }
}


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
    for (int i = 0; i < Buffer.Height; i++)
    {
        for (int j = 0; j < Buffer.Width; j++)
        {
            uint32_t blac = 0xff010101;
            SetPixel(Buffer,j,i,blac);
        }
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

bool probability(float p)
{
    int comp = p * 100000;
    int randomnum = abs(rand() % 1000000);
    return (randomnum <= comp);
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

void readdata(string filename, void *&ptrwheredatawillberead, int numberofbytes)
{
    ptrwheredatawillberead = new char[numberofbytes];
    fstream file;
    file.open(filename, ios::in | ios::binary);
    if (file.is_open())
    {
        file.read(reinterpret_cast<char *>(ptrwheredatawillberead), numberofbytes);
        file.close();
    }
    else
    {
        cout << "ERROR: Could not open file for reading." << endl;
    }
}

struct GameState
{

};

GameState &GetGameState()
{
    static GameState state;
    return state;
}

void gameinit()
{
    RandomColor(Buffer01);
}

void updatebuffer(float dt)
{
    ClearBuffer(Buffer01);
    GameState gs = GetGameState();
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
        HWND WindowHandle = CreateWindowExA(0, WindowClass.lpszClassName, "OakEngine", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, 0, 0, widthofwindowinpixel * widthofpixel ,heightofwindowinpixel * heightofpixel, 0, 0, Instance, 0);
        if (WindowHandle)
        {
            gameinit();
            IsRunning = true;
            MSG Message;
            auto lastTime = std::chrono::high_resolution_clock::now();
            float deltaTime = 0.0f;
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
                auto currentTime = std::chrono::high_resolution_clock::now();
                deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
                lastTime = currentTime;
                updatebuffer(deltaTime);
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