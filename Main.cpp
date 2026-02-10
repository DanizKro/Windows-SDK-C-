// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProject1.h"
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//Lys
enum LightState {
    RED = 0, RED_YELLOW = 1, YELLOW = 2, GREEN = 3
};
static LightState lightStateVertical = RED;
static LightState lightStateHorisontal = GREEN;

static const UINT_PTR TIMER_trafficlight = 1;
static const UINT_PTR TIMER_car = 2;
static const UINT_PTR TIMER_verticalCar = 3;
static const UINT_PTR TIMER_horisontalCar = 4;

static UINT verticalCarInterval = 2000;
static UINT horisontalCarInterval = 2000;

static BOOL redLightVertical = TRUE;
static BOOL redLightHorisontal = FALSE;

enum CarDirection { CAR_HORISONTAL, CAR_VERTICAL };
//Lager et objekt av en bil
struct Car
{
    POINT pos;
    int length;
    int height;
    CarDirection dir;
    BOOL stopped;
};

//Lager en tabell med bil-Objekter
std::vector<Car> cars;

//Lage randomiser for prosen spawn av biler
 // 1. Seed the random number engine using the current time
    //    Using std::chrono::system_clock::now().time_since_epoch().count() 
    //    provides a good, unique seed each time the program runs.
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

// 2. Instantiate a Mersenne Twister 19937 engine (a high-quality PRNG)
std::mt19937 engine(seed);

// 3. Define the desired range using a uniform integer distribution
//    The range is inclusive: [min, max]
std::uniform_int_distribution<int> dist(1, 10);

// 4. Generate the random number
//int random_num = dist(engine);

static int propabilityVertical = 1;
static int propabilityHorisontal = 1;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)

{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    
    switch (message)
    {

    case WM_CREATE:
    {
        SetTimer(hWnd, TIMER_trafficlight, 4000, NULL);
        SetTimer(hWnd, TIMER_car, 100, NULL);
        SetTimer(hWnd, TIMER_verticalCar, verticalCarInterval, NULL);
        SetTimer(hWnd, TIMER_horisontalCar, horisontalCarInterval, NULL);
        break;
    }

    case WM_COMMAND:
        {
        int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_TIMER:
    {
        //Henter winduet for å kunne bruke skalering
        RECT client;
        GetClientRect(hWnd, &client);

        Car* lastCarHorisontal = nullptr;
        Car* lastCarVertical = nullptr;

        if (wParam == TIMER_car) {
            for (auto& car : cars)
            {
                if (car.dir == CAR_HORISONTAL) {
                    if (car.stopped) {
                        lastCarHorisontal = &car;
                        continue; //Hvis bilen er stoppet hopp over
                    }
                    if (redLightHorisontal && car.pos.x >= (client.right / 2) - 120 && car.pos.x < (client.right / 2) - 100) {
                        car.stopped = TRUE;
                    }
                    if (redLightHorisontal && lastCarHorisontal != nullptr && car.pos.x < client.right/2 - 120) {
                        if((car.pos.x + car.length + 35) >= lastCarHorisontal->pos.x){
                            car.stopped = TRUE;
                        }
                    }
                        car.pos.x += 15;
                        lastCarHorisontal = &car;
                } else
                {
                    if (car.stopped) {
                        lastCarVertical = &car;
                        continue; //Hvis bilen er stoppet hopp over
                    }
                    if (redLightVertical && car.pos.y >= (client.bottom / 2) - 110 && car.pos.y < (client.bottom / 2) - 90) {
                        car.stopped = TRUE;
                    }
                    if (redLightVertical && lastCarVertical != nullptr && car.pos.y < client.right / 2 - 120) {
                        if ((car.pos.y + car.length + 25) >= lastCarVertical->pos.y) {
                            car.stopped = TRUE;
                        }
                    }
                    car.pos.y += 15;
                    lastCarVertical = &car;
                }
                
            }

            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }


        if (wParam == TIMER_trafficlight)
        {
            switch (lightStateVertical)
            {
            case RED:
                lightStateVertical = RED_YELLOW;
                break;
            case RED_YELLOW:
                lightStateVertical = GREEN;
                redLightVertical = FALSE;
                for (auto& car : cars) {
                    if(car.dir == CAR_VERTICAL)
                        car.stopped = FALSE;
                }
                break;
            case YELLOW:
                lightStateVertical = RED;
                redLightVertical = TRUE;
                break;
            case GREEN:
                lightStateVertical = YELLOW;
                break;
            }

            switch (lightStateHorisontal)
            {
            case RED:
                lightStateHorisontal = RED_YELLOW;
                break;
            case RED_YELLOW:
                lightStateHorisontal = GREEN;
                redLightHorisontal = FALSE;
                for (auto& car : cars) {
                    if(car.dir == CAR_HORISONTAL)
                        car.stopped = FALSE;
                }
                break;
            case YELLOW:
                lightStateHorisontal = RED;
                redLightHorisontal = TRUE;
                break;
            case GREEN:
                lightStateHorisontal = YELLOW;
                break;
            }

            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }

        if (wParam == TIMER_verticalCar)
        {
            int random_num = dist(engine);
            if (random_num < propabilityVertical)
            {
                RECT client;
                GetClientRect(hWnd, &client);

                Car newCar;
                newCar.length = 60;
                newCar.height = 30;
                newCar.stopped = FALSE;

                //Hosirontal car
                newCar.dir = CAR_VERTICAL;
                newCar.pos.x = client.right / 2 - newCar.height / 2;
                newCar.pos.y = 0;

                cars.push_back(newCar);
                //Tegne vinduet på nytt
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }

        if (wParam == TIMER_horisontalCar)
        {
            int random_num = dist(engine);
            if (random_num < propabilityHorisontal)
            {
                RECT client;
                GetClientRect(hWnd, &client);

                Car newCar;
                newCar.length = 60;
                newCar.height = 30;
                newCar.stopped = FALSE;

                //Hosirontal car
                newCar.dir = CAR_HORISONTAL;
                newCar.pos.x = 0;
                newCar.pos.y = client.bottom / 2 - newCar.height / 2;

                cars.push_back(newCar);
                //Tegne vinduet på nytt
                InvalidateRect(hWnd, NULL, FALSE);
            }
        } break;
        
    }

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_UP:
        {
            if(propabilityVertical < 10)
            propabilityVertical += 1;
            break;
        }
        case VK_DOWN:
        {
            if(propabilityVertical > 1)
            propabilityVertical -= 1;
            break;
        }
        case VK_LEFT:
        {
            if (propabilityHorisontal < 10)
            propabilityHorisontal += 1;
            break;
        }
        case VK_RIGHT:
        {
            if (propabilityHorisontal > 1)
            propabilityHorisontal -= 1;
            break;
        }
        default:
            break;

            

        }
        
        break;
    }

    
    case WM_LBUTTONDOWN:
    {

        RECT client;
        GetClientRect(hWnd, &client);

        Car newCar;
        newCar.length = 60;
        newCar.height = 30;
        newCar.stopped = FALSE;

        //Hosirontal car
        newCar.dir = CAR_HORISONTAL;
        newCar.pos.x = 0;
        newCar.pos.y = client.bottom / 2 - newCar.height / 2;

        cars.push_back(newCar);
        //Tegne vinduet på nytt
        InvalidateRect(hWnd, NULL,FALSE);
        break;
    }

    case WM_RBUTTONDOWN:
    {
        RECT client;
        GetClientRect(hWnd, &client);

        Car newCar;
        newCar.length = 60;
        newCar.height = 30;
        newCar.stopped = FALSE;

        //Hosirontal car
        newCar.dir = CAR_VERTICAL;
        newCar.pos.x = client.right / 2 - newCar.height / 2;
        newCar.pos.y = 0;

        cars.push_back(newCar);
        //Tegne vinduet på nytt
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }
    
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            //koordinater til start og slutt av VERTIKAL vei
            RECT client;
            GetClientRect(hWnd, &client);

            //Koordinater Vertikal vei
            int verticalLeft = client.right / 2 - 35;
            int verticalRight = client.right / 2 + 35;
            //Koordinater Horisontal vei
            int horisontalTop = client.bottom / 2 - 35;
            int horisontalBottom = client.bottom / 2 + 35;

            //Lage ny penn så kanten på veien ikke blir svart i rektangelet
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(211, 211, 211)); //Lager ny farge til tegnepenn
            HPEN oldPenPeker = (HPEN)SelectObject(hdc, hPen);

            //Tegning av Vertikal vei
            HBRUSH grayBrush = CreateSolidBrush(RGB(211, 211, 211));
            HGDIOBJ pekerRoad1 = SelectObject(hdc, grayBrush);

            Rectangle(hdc, verticalLeft, 0, verticalRight, client.bottom);
            //Tekning av Horisontal vei
            HBRUSH grayBrush2 = CreateSolidBrush(RGB(211, 211, 211));
            HGDIOBJ pekerRoad2 = SelectObject(hdc, grayBrush2);
            Rectangle(hdc, client.left, horisontalTop, client.right, horisontalBottom);

            
            //Vertikal trafikklys
            int boxWidth = 50;
            int boxHight = 150;
            int justering = 5;
            //Horisontal Trafikklys
            int boxLength = 150;
            int boxThick = 50;

            //Lager Vertikal Trafikklys
            int left = verticalLeft - 50;
            int top = horisontalTop - 150;
            //RECT rektangel = { left, top, left + boxWidth, top + boxHight };
            //Lager horisontal Trafikklys
            int leftStart = horisontalTop +150;
            //RECT rektangel2 = { leftStart, top2, leftStart + boxWidth, top2 + boxHight };

            HPEN hPen2 = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
            oldPenPeker = (HPEN)SelectObject(hdc, hPen2); //bruker den gamle tegnepennen men "dypper" den i ny farge

            //Farger PÅ:
            HBRUSH svartBrush = CreateSolidBrush(RGB(53, 56, 57));
            HBRUSH redOn = CreateSolidBrush(RGB(230, 0, 0));
            HBRUSH yellowOn = CreateSolidBrush(RGB(230, 200, 0));
            HBRUSH greenOn = CreateSolidBrush(RGB(45, 201, 55));

            //Farger AV:
            HBRUSH redOff = CreateSolidBrush(RGB(80, 0, 0));
            HBRUSH yellowOff = CreateSolidBrush(RGB(80, 70, 0));
            HBRUSH greenOff = CreateSolidBrush(RGB(0, 60, 0));
            


            // -35
            //Lage boks over TrafikkLys vertikal
            //Rectangle(hdc, left, client.bottom / 2 - 35 - 150-40,boxWidth, client.bottom / 2 - 35 - 150);
            //________________________________________________________________________________________________



            //Lager trafikklys Vertikal
            HGDIOBJ pekerSvartBrush = SelectObject(hdc, svartBrush);
            Rectangle(hdc, left, top, left + boxWidth, top + boxHight);
            //Lager trafikkLys Horisontal
            Rectangle(hdc, verticalLeft - boxLength, horisontalBottom, verticalLeft, horisontalBottom+boxThick);

            //TrafikkLys Vertikal rundinger
            //Øverste runding rødt lys
            HGDIOBJ pRedOn = SelectObject(hdc, redOn);
            Ellipse(hdc,left+ justering,top+ justering,left+boxWidth - justering,top+boxHight/3- justering);
            //Midterste runding gult lys
            HGDIOBJ pYellowOn = SelectObject(hdc, yellowOn);
            Ellipse(hdc, left+ justering, top+ justering + boxHight/3, left + boxWidth- justering, top + boxHight*2/3- justering);
            //Nederste runding grønt lys
            HGDIOBJ pGreenOn = SelectObject(hdc, greenOn);
            Ellipse(hdc, left+ justering, top + boxHight*2/3 + justering, left + boxWidth- justering, top + boxHight - justering);
            
            //TrafikkLys Horisontal rundinger
            //Venst runding rødt lys
            SelectObject(hdc, redOn);
            Ellipse(hdc, verticalLeft-boxLength + justering, horisontalBottom + justering, verticalLeft - boxLength *2/3,  horisontalBottom + boxThick - justering);
            //Midterste runding gult lys
            SelectObject(hdc, yellowOn);
            Ellipse(hdc, verticalLeft - boxLength * 2 / 3 + justering, horisontalBottom + justering, verticalLeft - boxLength*1/3 + -justering , horisontalBottom + boxThick - justering);
            //Høyre runding grønt lys
            SelectObject(hdc, greenOn);
            Ellipse(hdc, verticalLeft - boxLength * 1 / 3 , horisontalBottom + justering, verticalLeft  -justering, horisontalBottom + boxThick - justering);



            //Sjekker tilstand til Vertikal Lyset
            bool redTest = (lightStateVertical == RED || lightStateVertical == RED_YELLOW);
            bool yellowTest = (lightStateVertical == YELLOW || lightStateVertical == RED_YELLOW);
            bool greenTest = (lightStateVertical == GREEN);
            //Sjekker tilstand til Horisontal Lyset
            bool redTest2 = (lightStateHorisontal == RED || lightStateHorisontal == RED_YELLOW);
            bool yellowTest2 = (lightStateHorisontal == YELLOW || lightStateHorisontal == RED_YELLOW);
            bool greenTest2 = (lightStateHorisontal == GREEN);

            //TrafikkLys AV farger Verikal
            //Tegner ny rød
            SelectObject(hdc, redTest ? redOn : redOff);
            Ellipse(hdc, left + justering, top + justering, left + boxWidth - justering, top + boxHight / 3 - justering);
            //Tegner ny gul
            SelectObject(hdc, yellowTest ? yellowOn : yellowOff);
            Ellipse(hdc, left + justering, top + justering + boxHight / 3, left + boxWidth - justering, top + boxHight * 2 / 3 - justering);
            //Tegner ny grønn
            SelectObject(hdc, greenTest ? greenOn : greenOff);
            Ellipse(hdc, left + justering, top + boxHight * 2 / 3 + justering, left + boxWidth - justering, top + boxHight - justering);
            
            //Trafikklys AV fager Horisontal
            SelectObject(hdc, redTest2 ? redOn : redOff);
            Ellipse(hdc, verticalLeft - boxLength + justering, horisontalBottom + justering, verticalLeft - boxLength * 2 / 3, horisontalBottom + boxThick - justering);
            //Tegner ny gul
            SelectObject(hdc, yellowTest2 ? yellowOn : yellowOff);
            Ellipse(hdc, verticalLeft - boxLength * 2 / 3 + justering, horisontalBottom + justering, verticalLeft - boxLength * 1 / 3 + -justering, horisontalBottom + boxThick - justering);
            //Tegner ny grønn
            SelectObject(hdc, greenTest2 ? greenOn : greenOff);
            Ellipse(hdc, verticalLeft - boxLength * 1 / 3, horisontalBottom + justering, verticalLeft - justering, horisontalBottom + boxThick - justering);


            HBRUSH carBrush = CreateSolidBrush(RGB(0, 0, 0));
            HGDIOBJ orignalCarBrush = SelectObject(hdc, carBrush);

            for (const auto& car : cars)
            {
                if (car.dir == CAR_HORISONTAL)
                {
                    Rectangle(
                        hdc,
                        car.pos.x,
                        car.pos.y,
                        car.pos.x + car.length,
                        car.pos.y + car.height
                    );
                }
                else
                {
                    Rectangle(
                        hdc,
                        car.pos.x + car.height,
                        car.pos.y + car.length -10,
                        car.pos.x ,
                        car.pos.y 
                    );
                }


            }

            //Vertikal PROSENT
            //Lage en stream av streng:
            std::wstringstream ss;
            //Legge til det som ønskes i streng streamen(konkantinering):
            ss << propabilityVertical * 10 << L" %";
            //Lagre en tekststreng av streamen:
            std::wstring text = ss.str();
            //Skriver da ut på ønsket plassering:
            TextOutW(hdc,client.right / 2 - 80,client.bottom / 2 - 205,text.c_str(),text.length());

            //Horisontal PROSENT
            std::wstringstream ss2;
            ss2 << propabilityHorisontal * 10 << L" %";
            std::wstring text2 = ss2.str();
            TextOutW(hdc, client.right / 2 - 225, client.bottom / 2 + 40, text2.c_str(), text2.length());
         
            //Sletter alle objekter for å unnngå minnelekasje
            DeleteObject(pekerRoad1);
            DeleteObject(pekerRoad2);
            DeleteObject(pekerSvartBrush);
            DeleteObject(pRedOn);
            DeleteObject(pYellowOn);
            DeleteObject(pGreenOn);

            SelectObject(hdc, redOff);
            DeleteObject(redOff);
            SelectObject(hdc, yellowOff);
            DeleteObject(yellowOff);
            SelectObject(hdc, greenOff);
            DeleteObject(greenOff);

            SelectObject(hdc, oldPenPeker);
            DeleteObject(oldPenPeker);
            //DeleteObject(orignalCarBrush);
            
            EndPaint(hWnd, &ps);
            break;
        }
        
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
