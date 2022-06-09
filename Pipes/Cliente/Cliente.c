#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include <strsafe.h>
#include "resource.h"
#include "..\util.h"
//#include "..\mapa.h"

#define SQ_SZ 50
#define Cl_Sz sizeof(Cliente)


HANDLE event; //DEBUG RETIRAR DEPOIS


//int getSquare(TDados* dados, int x, int y) {
//	for (int i = 0; i < dados->eu.mapa.lin; i++) {
//		for (int j = 0; j < dados->eu.mapa.col; j++) {
//			if (x > j * SQ_SZ && x < j * SQ_SZ + SQ_SZ && y > i && y < i * SQ_SZ + SQ_SZ) {
//				return j + i * dados->eu.mapa.lin; // o 3 é o n de linhas
//			}
//		}
//	}
//	return -1;
//	//if (x > 0 && x < 100 && y > 0 && y < 100) {
//	//	return 1;
//	//}
//	//if (x > 100 && x < 200 && y > 0 && y < 100) {
//	//	return 2;
//	//}
//}
Jogada getSquare(TDados* dados, int x, int y) {
	for (int i = 0; i < dados->eu.mapa.lin; i++) {
		for (int j = 0; j < dados->eu.mapa.col; j++) {
			if (x > j * SQ_SZ && x < j * SQ_SZ + SQ_SZ && y > i && y < i * SQ_SZ + SQ_SZ) {
				Jogada jog;
				jog.lin = i;
				jog.col = j;
				return jog;
			}
		}
	}
	Jogada jog;
	jog.lin = -1;
	jog.col = -1;
	return jog;
}

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK dNome(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

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
	//Cliente* eu = (Cliente*)param;
	TDados* dados = (TDados*)param;
	Cliente* eu = &dados->eu;
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

		dados->eu = FromServer;
		//InvalidateRect(dados->hWnd, NULL, FALSE);
		if (FromServer.termina) {
			_tprintf(TEXT("Recebi uma mensagem: %s\n"), FromServer.mensagem);
			eu->termina = true;
			break;
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("GANHOU")) == 0) {
			if (MessageBox(dados->hWnd, TEXT("Ganhou! Pretende Jogar de novo?"), TEXT("Jogar de novo"), MB_YESNO) == IDYES) {
				_tcscpy_s(dados->eu.mensagem, 20, TEXT("REGISTO"));
				//dados->eu.individual = true;
				SetEvent(event);
			}

		}
		if (_tcsicmp(FromServer.mensagem, TEXT("JOGADA")) == 0) {
			InvalidateRect(dados->hWnd, NULL, FALSE);

		}
		if (_tcsicmp(FromServer.mensagem, TEXT("AGUA")) == 0) {
			InvalidateRect(dados->hWnd, NULL, FALSE);
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("MUDAR")) == 0) {
			InvalidateRect(dados->hWnd, NULL, FALSE);
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("APAGAR")) == 0) {
			InvalidateRect(dados->hWnd, NULL, FALSE);
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("ALEATORIO")) == 0) {
			dados->eu.aleatorio = FromServer.aleatorio;
			SetEvent(event);
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
	TDados* dados = (TDados*)param;
	Cliente* eu = &dados->eu;// (Cliente*)param;
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
	eu.peca = pecasText[0][0];
	dados.eu = eu;
	hThread[0] = CreateThread(NULL, 0, ThreadClienteReader, (LPVOID)&dados, 0, 0);
	hThread[1] = CreateThread(NULL, 0, ThreadClienteWritter, (LPVOID)&dados, 0, 0);

	if (!RegistaClasse(hInst, janelaPrinc)) {
		return 0;
	}

	dados.hWnd = CriarJanela(hInst, janelaPrinc);

	HDC hdc = GetDC(dados.hWnd);

	LONG_PTR oldWnd;
	oldWnd = SetWindowLongPtr(dados.hWnd, GWLP_USERDATA, (LONG_PTR)&dados);

	DialogBoxParam((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_NAME), dados.hWnd, (DLGPROC)dNome, (LPARAM)&dados);

	ShowWindow(dados.hWnd, nCmdShow);
	UpdateWindow(dados.hWnd);


	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	HANDLE WriteReady;
	OVERLAPPED OverlWr = { 0 };
	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	ZeroMemory(&OverlWr, sizeof(OverlWr));
	ResetEvent(WriteReady);
	OverlWr.hEvent = WriteReady;

	eu.termina = true;

	fSuccess = WriteFile(hPipe, &eu, Cl_Sz, &cbWritten, &OverlWr);
	CloseHandle(WriteReady);
	CloseHandle(hPipe);
}


//LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
//	HDC hdc = GetDC(hWnd);
//	TCHAR msg[100];
//	PAINTSTRUCT ps;
//	POINTS p;
//	Cliente c;
//	event = CreateEvent(NULL, FALSE, FALSE, TEXT("EVENTO"));
//	_tcscpy_s(c.nome, 20, TEXT("PEDRO CORREIA"));
//
//	TDados* dados;
//	dados = (TDados*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
//
//
//	switch (messg) {
//	case WM_CREATE:
//
//		InvalidateRect(hWnd, NULL, true);
//		break;
//	case WM_PAINT:
//		hdc = BeginPaint(hWnd, &ps);
//		//for (int i = 0; i < 4; i++) {
//		//	Rectangle(hdc, i * 10, 0, i * 10 + 10, 10);
//		//}
//		for (int i = 0; i < dados->eu.mapa.lin; i++) {
//			for (int j = 0; j < dados->eu.mapa.col; j++) {
//				Rectangle(hdc, j * SQ_SZ, i * SQ_SZ, j * SQ_SZ + SQ_SZ, i * SQ_SZ + SQ_SZ);
//			}
//		}
//		/*Rectangle(hdc, 0, 0, 10, 10);
//		Rectangle(hdc, 10, 0, 20, 10);
//		Rectangle(hdc, 20, 0, 30, 10);
//		Rectangle(hdc, 0, 10, 10, 20);
//		Rectangle(hdc, 10, 10, 20, 20);
//		Rectangle(hdc, 20, 10, 30, 20);*/
//		EndPaint(hWnd, &ps);
//		break;
//	case WM_DESTROY:	// Destruir a janela e terminar o programa 
//						// "PostQuitMessage(Exit Status)"		
//		PostQuitMessage(0);
//		break;
//	case WM_COMMAND:
//		_stprintf_s(msg, 100, TEXT("%d"), wParam);
//		TextOut(hdc, 500, 500, msg, _tcslen(msg));
//		break;
//	case WM_LBUTTONDOWN:
//
//		p.x = GET_X_LPARAM(lParam);
//		p.y = GET_Y_LPARAM(lParam);
//
//		int rect = getSquare(dados->eu.mapa, p.x, p.y);
//		_tcscpy_s(dados->eu.nome, 20, TEXT("ZE LEITEIRO"));
//		dados->eu.square = rect;
//		/*_stprintf_s(msg, 100, TEXT("%d, %d"), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));*/
//		_stprintf_s(msg, 100, TEXT("%d"), rect);
//		TextOut(hdc, p.x, p.y, msg, _tcslen(msg));
//		SetEvent(event);
//		//ClienteWrite(c);
//		break;
//	default:
//		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
//		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
//		return(DefWindowProc(hWnd, messg, wParam, lParam));
//		break;  // break tecnicamente desnecessário por causa do return
//	}
//	return(0);
//}

static BOOL CALLBACK dNome(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	TDados* dados;
	HWND h;
	h = GetParent(hWnd);

	
	dados = (TDados*)GetWindowLongPtr(h, GWLP_USERDATA);

	switch (messg) {
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON_IND) {
			GetDlgItemText(hWnd, IDC_EDIT_NAME, dados->eu.nome, 20);
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("REGISTO"));
			dados->eu.individual = true;
			SetEvent(event);
			EndDialog(hWnd, 0);
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_BUTTON_COMP) {
			GetDlgItemText(hWnd, IDC_EDIT_NAME, dados->eu.nome, 20);
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("REGISTO"));
			dados->eu.individual = false;
			SetEvent(event);
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC hdc = GetDC(hWnd);
	TCHAR msg[100];
	PAINTSTRUCT ps;
	POINTS p;
	Cliente c;

	static HBITMAP bmpPeca;
	HBITMAP foto;
	static HDC double_dc;
	HDC auxdc;
	static int maxX, maxY;
	int x = 0, y = 0;
	p.x = 0; p.y = 0;

	//int rect = 0;
	Jogada rect;
	//static HBITMAP pecas[6];
	static HBITMAP pecas;
	static HBITMAP pecasAgua;
	static HBITMAP barreira;

	TDados* dados;
	dados = (TDados*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	//#define SQ_SZ 50/(dados->eu.mapa.lin*dados->eu.mapa.col)
	event = CreateEvent(NULL, FALSE, FALSE, TEXT("EVENTO"));
	
	bool g_fMouseTracking = FALSE;

	switch (messg) {
	case WM_CREATE:
		pecas = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECAS));
		pecasAgua = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECAS_AGUA));
		barreira = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BARREIRA));
		bmpPeca = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECA_1));
		break;
	case WM_PAINT:

			
		hdc = BeginPaint(hWnd, &ps);

		HDC memDC = CreateCompatibleDC(hdc);

		RECT rcClientRect;
		GetClientRect(hWnd, &rcClientRect);

		HBITMAP bmp = CreateCompatibleBitmap(hdc, rcClientRect.right - rcClientRect.left,
			rcClientRect.bottom - rcClientRect.top);

		HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, bmp);
		dados->tempo = 5;
		auxdc = CreateCompatibleDC(memDC);
		SelectObject(auxdc, pecas);
		for (int i = 0; i < dados->eu.mapa.lin; i++) {
			for (int j = 0; j < dados->eu.mapa.col; j++) {
				//SelectObject(auxdc, bmpPeca);
				Rectangle(memDC, j * SQ_SZ, i * SQ_SZ, j * SQ_SZ + SQ_SZ, i * SQ_SZ + SQ_SZ);
				for (int k = 0; k < 2; k++) {
					for (int l = 0; l < 3; l++) {
						SelectObject(auxdc, pecas);
						if (dados->eu.mapa.board[i][j] == pecasText[k][l]) {
							/*BitBlt(memDC, j * SQ_SZ, i * SQ_SZ, j * SQ_SZ + SQ_SZ, i * SQ_SZ + SQ_SZ, auxdc, l * 50, k * 50, SRCCOPY);*/
							
							if (dados->eu.agua.board[i][j] == TEXT('w')) {
								SelectObject(auxdc, pecasAgua);
								//BitBlt(memDC, j * SQ_SZ, i * SQ_SZ, j * SQ_SZ + SQ_SZ, i * SQ_SZ + SQ_SZ, auxdc, l * 50, k * 50, SRCCOPY);
								BitBlt(memDC, j * SQ_SZ, i * SQ_SZ, SQ_SZ, SQ_SZ, auxdc, l * 50, k * 50, SRCCOPY);
								continue;
							}
							BitBlt(memDC, j * SQ_SZ, i * SQ_SZ, SQ_SZ, SQ_SZ, auxdc, l * 50, k * 50, SRCCOPY);
							continue;
						}
						if (dados->eu.mapa.board[i][j] == TEXT('|')) {
							SelectObject(auxdc, barreira);
							BitBlt(memDC, j * SQ_SZ, i * SQ_SZ, SQ_SZ, SQ_SZ, auxdc, 0, 0, SRCCOPY);
							continue;
						}
					}
				}
			}
		}

		BitBlt(hdc, 0, 0, rcClientRect.right - rcClientRect.left,
			rcClientRect.bottom - rcClientRect.top, memDC, 0, 0, SRCCOPY);

		SelectObject(memDC, oldBmp);
		DeleteObject(bmp); 
		DeleteDC(auxdc);
		DeleteDC(memDC);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:	// Destruir a janela e terminar o programa 
						// "PostQuitMessage(Exit Status)"		
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		//_stprintf_s(msg, 100, TEXT("%d"), wParam);
		//TextOut(hdc, 500, 500, msg, _tcslen(msg));
		break;
	case WM_LBUTTONDOWN:
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);

		rect = getSquare(dados, p.x, p.y);

		//_stprintf_s(msg, 100, TEXT("%d"), rect);

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 3; j++) {
				if (dados->eu.mapa.board[rect.lin][rect.col] == pecasText[i][j]) {
					if (!dados->eu.aleatorio) {
						dados->eu.peca = getProxPeca(pecasText[i][j]);
					}
					else {
						dados->eu.peca = getRandomPeca();
					}
					//_tprintf(TEXT("\n%c\n"), dados->eu.peca);
					dados->eu.x = rect.lin;
					dados->eu.y = rect.col;
					_tcscpy_s(dados->eu.mensagem, 20, TEXT("MUDAR"));
					_tcscpy_s(dados->eu.nome, 20, TEXT("PEDRO CORREIA"));
					SetEvent(event);
					return;
				}
			}
		}


		//if (rect.lin > 0) {
		//	_tprintf(TEXT("%d\n"), rect);
		//	InvalidateRect(hWnd, NULL, FALSE);
		//}
		dados->eu.x = rect.lin;
		dados->eu.y = rect.col;
		_tcscpy_s(dados->eu.mensagem, 20, TEXT("JOGADA"));
		_tcscpy_s(dados->eu.nome, 20, TEXT("PEDRO CORREIA"));
		if (!dados->eu.aleatorio) {
			dados->eu.peca = getProxPeca(pecasText[0][0]);
		}
		else {
			dados->eu.peca = getRandomPeca();
		}
		SetEvent(event);
		break;
	case WM_RBUTTONDOWN:
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);

		rect = getSquare(dados, p.x, p.y);
		
		if (dados->eu.mapa.board[rect.lin][rect.col] != TEXT('□')) {
			dados->eu.peca = TEXT('□');
			//_tprintf(TEXT("\n%c\n"), dados->eu.peca);
			dados->eu.x = rect.lin;
			dados->eu.y = rect.col;
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("APAGAR"));
			_tcscpy_s(dados->eu.nome, 20, TEXT("PEDRO CORREIA"));
			SetEvent(event);
		}
		break;
	case  WM_MOUSEMOVE:
		if (!g_fMouseTracking)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.hwndTrack = hWnd;
			tme.dwHoverTime = 0;
			g_fMouseTracking = TrackMouseEvent(&tme);
		}
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		break;
	case  WM_MOUSEHOVER:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		g_fMouseTracking = FALSE;
		/*for (int i = 0; i < dados->ptr_memoria->; i++) {
			if (!dados->hPop) {
				if (dados->ptr_memoria->avioes[i].x <= x && (dados->ptr_memoria->avioes[i].x + 30) >= x && dados->ptr_memoria->avioes[i].y <= y && (dados->ptr_memoria->avioes[i].y + 30) >= y) {
					TCHAR text[BUFFER];
					int np = 0;
					for (int j = 0; j < dados->numpassag; j++) {
						if (dados->p[j].voo == dados->ptr_memoria->avioes[i].id) {
							np++;
						}
					}
					_stprintf_s(text, BUFFER, TEXT("Avião %d.\nPartida: %s\nDestino: %s\nNº Passageiros: %d"), dados->ptr_memoria->avioes[i].id, dados->ptr_memoria->avioes[i].inicial.nome, dados->ptr_memoria->avioes[i].destino.nome, np);
					dados->hPop = CriarJanelaPopUp(hInstance, text, hWnd, dados->ptr_memoria->avioes[i].x, dados->ptr_memoria->avioes[i].y);
					ShowWindow(dados->hPop, SW_SHOW);

					TRACKMOUSEEVENT tme;
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE;
					tme.hwndTrack = hWnd;
					g_fMouseTracking = TrackMouseEvent(&tme);
				}
			}
			
		}*/
		if (x > 50 && x < 100) {
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("SUSPENDER"));
			_tcscpy_s(dados->eu.nome, 20, TEXT("PEDRO CORREIA"));
			dados->eu.peca = TEXT('┓');
			SetEvent(event);
		}
		return 0;
		break;
	case  WM_MOUSELEAVE:
		g_fMouseTracking = FALSE;
		return 0;
		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;  // break tecnicamente desnecessário por causa do return
	}
	return(0);
}



