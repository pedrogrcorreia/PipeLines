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

#define Cl_Sz sizeof(Cliente)

HANDLE event; //DEBUG RETIRAR DEPOIS

//int ClienteWrite(Cliente eu) {
//	BOOL fSuccess = FALSE;
//	DWORD cbWritten;
//	HANDLE WriteReady;
//	OVERLAPPED OverlWr = { 0 };
//	HANDLE hPipe = eu.hPipe;
//	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
//
//	ZeroMemory(&OverlWr, sizeof(OverlWr));
//	ResetEvent(WriteReady);
//	OverlWr.hEvent = WriteReady;
//
//	fSuccess = WriteFile(hPipe, &eu, Cl_Sz, &cbWritten, &OverlWr);
//
//	WaitForSingleObject(WriteReady, INFINITE);
//
//	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
//
//	return 1;
//}

int getSquare(int x, int y) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (x > j * 100 && x < j * 100 + 100 && y > i && y < i * 100 + 100) {
				return j + i * 3; // o 3 é o n de linhas
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

DWORD WINAPI ThreadClienteReader(LPVOID param) {
	Cliente FromServer;
	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;
	Cliente* eu = (Cliente*)param;
	//TDados* dados = (TDados*)param;
	//Cliente* eu = &dados->eu;
	HANDLE hPipe = eu->hPipe;
	OVERLAPPED OverlRd = { 0 };
	DWORD result;
	HANDLE ReadReady;
	TCHAR msg[100];
	ReadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	_tprintf(TEXT("THREAD PARA OUVIR DO SERVIDOR LANÇADA!\n"));
	while (!eu->termina) {
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);
		
		fSuccess = ReadFile(hPipe, &FromServer, Cl_Sz, &cbBytesRead, &OverlRd);

		WaitForSingleObject(ReadReady, INFINITE);

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE);

		if (FromServer.termina) {
			_tprintf(TEXT("Recebi uma mensagem: %s\n"), FromServer.mensagem);
			eu->termina = true;
			break;
		}
		_tprintf(TEXT("Recebi uma mensagem: %s\n"), FromServer.mensagem);
		//HDC hdc = GetDC(dados->hWnd);
		//_stprintf_s(msg, 100, TEXT("%d"), FromServer.mensagem);
		//TextOut(hdc, 400, 400, msg, _tcslen(msg));
		//InvalidateRect(dados->hWnd, NULL, TRUE);
	}

	CloseHandle(ReadReady);
	return 1;
}

DWORD WINAPI ThreadClienteWritter(LPVOID param) {
	Cliente* eu = (Cliente*)param;
	BOOL fSuccess = FALSE;
	DWORD cbWritten;
	HANDLE WriteReady;
	OVERLAPPED OverlWr = { 0 };
	HANDLE hPipe = eu->hPipe;
	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	_tcscpy_s(eu->nome, 20, TEXT("PEDRO CORREIA"));
	eu->termina = false;
	eu->x = 0;
	eu->y = 0;
	event = CreateEvent(NULL, FALSE, FALSE, TEXT("EVENTO"));
	while (!eu->termina) {
		WaitForSingleObject(event, INFINITE);
		ZeroMemory(&OverlWr, sizeof(OverlWr));
		ResetEvent(WriteReady);
		OverlWr.hEvent = WriteReady;

		fSuccess = WriteFile(hPipe, eu, Cl_Sz, &cbWritten, &OverlWr);

		WaitForSingleObject(WriteReady, INFINITE);

		GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
		_tprintf(TEXT("ESCREVI PARA O SERVIDOR!"));
		if (eu->termina) {
			break;
		}
	}

	CloseHandle(WriteReady);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

	HANDLE hPipe;
	BOOL fSuccess = FALSE;
	DWORD cbWritten, dwMode;
	HANDLE hThread[2];
	DWORD dwThreadId = 0;
	TCHAR janelaPrinc[] = TEXT("PipeGame");

	TDados dados;
	HWND hWnd;
	MSG msg;

	Cliente eu;


	/* CRIACAO DE CONSOLA PARA DEBUG */
	AllocConsole();
	HANDLE stdHandle;
	int hConsole;
	FILE* fp;
	stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConsole = _open_osfhandle((long)stdHandle, _O_TEXT);
	fp = _fdopen(hConsole, "w");

	freopen_s(&fp, "CONOUT$", "w", stdout);
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

	while (1) {
		hPipe = CreateFile(PIPE_SERVER, GENERIC_READ | GENERIC_WRITE, 0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);

		if (hPipe != INVALID_HANDLE_VALUE) {
			eu.hPipe = hPipe;
			break;
		}
	}

	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
	eu.hPipe = hPipe;
	hThread[0] = CreateThread(NULL, 0, ThreadClienteReader, (LPVOID)&eu, 0, 0);
	hThread[1] = CreateThread(NULL, 0, ThreadClienteWritter, (LPVOID)&eu, 0, 0);

	if (!RegistaClasse(hInst, janelaPrinc)) {
		return 0;
	}

	dados.hWnd = CriarJanela(hInst, janelaPrinc);

	HDC hdc = GetDC(dados.hWnd);

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

	ShowWindow(dados.hWnd, nCmdShow);
	UpdateWindow(dados.hWnd);


	//hThread[1] = CreateThread(NULL, 0, ThreadClienteWriter, (LPVOID)&eu, 0, 0);

	//DWORD result;
	//result = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	//HANDLE WriteReady;
	//OVERLAPPED OverlWr = { 0 };
	//WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	//ZeroMemory(&OverlWr, sizeof(OverlWr));
	//ResetEvent(WriteReady);
	//OverlWr.hEvent = WriteReady;

	//eu.termina = true;

	//fSuccess = WriteFile(hPipe, &eu, Cl_Sz, &cbWritten, &OverlWr);
	//CloseHandle(WriteReady);
	//CloseHandle(hPipe);

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
	Cliente c;
	event = CreateEvent(NULL, FALSE, FALSE, TEXT("EVENTO"));
	_tcscpy_s(c.nome, 20, TEXT("PEDRO CORREIA"));
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
		SetEvent(event);
		//ClienteWrite(c);
		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;  // break tecnicamente desnecessário por causa do return
	}
	return(0);
}

