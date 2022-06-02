#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include <strsafe.h>
#include "..\util.h"

int getSquare(int x, int y) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (x > j * 100 && x < j * 100 + 100 && y > i && y < i * 100 + 100) {
				return j + i * 3;
			}
		}
	}
	//if (x > 0 && x < 100 && y > 0 && y < 100) {
	//	return 1;
	//}
	//if (x > 100 && x < 200 && y > 0 && y < 100) {
	//	return 2;
	//}
}

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

ATOM RegistaClasse(HINSTANCE hInst, TCHAR* szWinName) {
	WNDCLASSEX wcl;

	wcl.cbSize = sizeof(WNDCLASSEX);
	wcl.hInstance = hInst;
	wcl.lpszClassName = szWinName;
	wcl.lpfnWndProc = TrataEventos;
	wcl.style = CS_HREDRAW;
	wcl.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wcl.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.lpszMenuName = NULL;
	wcl.cbClsExtra = sizeof(TDados);
	wcl.cbWndExtra = sizeof(TDados);
	wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	return RegisterClassEx(&wcl);
}

HWND CriarJanela(HINSTANCE hInst, TCHAR* szWinName) {
	return CreateWindow(
		szWinName,
		TEXT("PipeGame Cliente"),
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		1000,
		1000,
		HWND_DESKTOP,
		NULL,
		hInst,
		NULL
	);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

#ifdef UNICODE 
	if (_setmode(_fileno(stdin), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
	if (_setmode(_fileno(stdout), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
	if (_setmode(_fileno(stderr), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
#endif

	TCHAR janelaPrinc[] = TEXT("PipeGame");

	HWND hWnd;
	MSG msg;
	if (!RegistaClasse(hInst, janelaPrinc)) {
		return 0;
	}

	hWnd = CriarJanela(hInst, janelaPrinc);

	HDC hdc = GetDC(hWnd);

	for (int i = 0; i < 3; i++) {
		//HWND hwndButton = CreateWindow(
		//	L"BUTTON",  // Predefined class; Unicode assumed 
		//	NULL,      // Button text 
		//	WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		//	100*i,         // x position 
		//	10,         // y position 
		//	100,        // Button width
		//	100,        // Button height
		//	hWnd,     // Parent window
		//	i,       // No menu.
		//	(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		//	NULL);      // Pointer not needed.
		Rectangle(hdc, i, i, i + 10, i + 10);
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

}


LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC hdc = GetDC(hWnd);
	TCHAR msg[100];
	PAINTSTRUCT ps;
	POINTS p;
	switch (messg) {
	case WM_CREATE:
		
		InvalidateRect(hWnd, NULL, true);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		//for (int i = 0; i < 4; i++) {
		//	Rectangle(hdc, i * 10, 0, i * 10 + 10, 10);
		//}
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				Rectangle(hdc, j * 100, i * 100, j * 100 + 100, i * 100 + 100);
			}
		}
		/*Rectangle(hdc, 0, 0, 10, 10);
		Rectangle(hdc, 10, 0, 20, 10);
		Rectangle(hdc, 20, 0, 30, 10);
		Rectangle(hdc, 0, 10, 10, 20);
		Rectangle(hdc, 10, 10, 20, 20);
		Rectangle(hdc, 20, 10, 30, 20);*/
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:	// Destruir a janela e terminar o programa 
						// "PostQuitMessage(Exit Status)"		
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		_stprintf_s(msg, 100, TEXT("%d"), wParam);
		TextOut(hdc, 500, 500, msg, _tcslen(msg));
		break;
	case WM_LBUTTONDOWN:

		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);

		int rect = getSquare(p.x, p.y);

		/*_stprintf_s(msg, 100, TEXT("%d, %d"), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));*/
		_stprintf_s(msg, 100, TEXT("%d"), rect);
		TextOut(hdc, p.x, p.y, msg, _tcslen(msg));
		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;  // break tecnicamente desnecessário por causa do return
	}
	return(0);
}

