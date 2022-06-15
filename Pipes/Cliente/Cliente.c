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

#define SQ_SZ 500
#define Cl_Sz sizeof(Cliente)


HANDLE event; //DEBUG RETIRAR DEPOIS

Jogada getSquare(TDados* dados, int x, int y, int width, int height) {
	for (int i = 0; i < dados->eu.mapa.lin; i++) {
		for (int j = 0; j < dados->eu.mapa.col; j++) {
			if (x > j * width && x < j * width + width && y > i && y < i * height + height) {
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
	wcl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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
		800,
		600,
		HWND_DESKTOP,
		(HMENU)0,
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
	while (!dados->eu.termina) {
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);

		fSuccess = ReadFile(hPipe, &FromServer, Cl_Sz, &cbBytesRead, &OverlRd);

		WaitForSingleObject(ReadReady, INFINITE);

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE);

		dados->eu.mapa = FromServer.mapa;
		dados->eu.agua = FromServer.agua;
		dados->eu.aleatorio = FromServer.aleatorio;
		dados->eu.nivel = FromServer.nivel;
		dados->eu.individual = FromServer.individual;
		dados->eu.aguaAtual = FromServer.aguaAtual;
		//dados->eu.hPipe = FromServer.hPipe;
		//dados->eu = FromServer;// Acho que é muito para assumir tudo como certo, vai dar override de certos valores
		//InvalidateRect(dados->hWnd, NULL, FALSE);
		if (FromServer.termina) {
			if (_tcsicmp(FromServer.mensagem, TEXT("TIMEOUT")) == 0) {
				MessageBox(dados->hWnd, TEXT("Foi excluído por inatividade|"), TEXT("Fechar"), MB_OK);
				dados->eu.termina = FromServer.termina;
				SetEvent(event);
				break;
			}
			MessageBox(dados->hWnd, TEXT("O Servidor terminou."), TEXT("Fechar"), MB_OK);
			dados->eu.termina = true;
			SetEvent(event);
			break;
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("GANHOU")) == 0) {
			if (MessageBox(dados->hWnd, TEXT("Ganhou! Continuar a jogar?"), TEXT("Vitória"), MB_YESNO) == IDYES) {
				_tcscpy_s(dados->eu.mensagem, 20, TEXT("CONTINUAR"));
				//dados->eu.individual = true;
				SetEvent(event);
			}
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("GANHOU COMP")) == 0) {
			MessageBox(dados->hWnd, TEXT("Ganhou a competição!"), TEXT("Vitória"), MB_OK);
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("PERDEU")) == 0) {
			if (MessageBox(dados->hWnd, TEXT("Perdeu! Jogar de novo?"), TEXT("Derrota"), MB_YESNO) == IDYES) {
				_tcscpy_s(dados->eu.mensagem, 20, TEXT("REGISTO"));
				//dados->eu.individual = true;
				SetEvent(event);
			}
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("PERDEU COMP")) == 0) {
			MessageBox(dados->hWnd, TEXT("Perdeu a competição!"), TEXT("Derrota"), MB_OK);
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
			InvalidateRect(dados->hWnd, NULL, FALSE);
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("ESPERAR")) == 0) {
			MessageBox(dados->hWnd, TEXT("À espera de adversário"), TEXT("Esperar"), MB_OK);
			//SetEvent(event);
		}
		if (_tcsicmp(FromServer.mensagem, TEXT("TEMPO")) == 0) {
			dados->eu.tempo = FromServer.tempo;
			_tprintf(TEXT("Recebi uma mensagem: %d\n"), FromServer.tempo);
			InvalidateRect(dados->hWnd, NULL, FALSE);
		}
		_tprintf(TEXT("Recebi uma mensagem: %s\n"), FromServer.mensagem);
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
	eu->termina = false;
	eu->x = 0;
	eu->y = 0;
	DWORD result = GetCurrentThreadId();
	TCHAR eventName[20];
	_stprintf_s(eventName, 20, TEXT("EVENTO %d"), result);
	event = CreateEvent(NULL, FALSE, FALSE, eventName);
	while (!dados->eu.termina) {
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
	eu.moveRato = false;
	dados.eu = eu;
	dados.eu.hPipe = hPipe;
	dados.eu.darkMode = false;
	dados.eu.tempo = 0;

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


	while (GetMessage(&msg, NULL, 0, 0) && !dados.eu.termina) {
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
		if (LOWORD(wParam) == IDC_BUTTON_OK) {
			GetDlgItemText(hWnd, IDC_EDIT_NAME, dados->eu.nome, 20);
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("NOME"));
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

	HBITMAP foto;
	static HDC double_dc;
	HDC auxdc;
	static int maxX, maxY;
	int x = 0, y = 0;
	p.x = 0; p.y = 0;

	Jogada rect;
	static HBITMAP pecas;
	static HBITMAP pecasAgua;
	static HBITMAP barreira;

	int width;
	int height;

	TDados* dados;
	dados = (TDados*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	


	bool g_fMouseTracking = FALSE;
	switch (messg) {
	case WM_CREATE:
		pecas = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECAS));
		pecasAgua = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECAS_AGUA));
		barreira = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BARREIRA));
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		HDC memDC = CreateCompatibleDC(hdc);
		RECT rcClientRect;
		GetClientRect(hWnd, &rcClientRect);

		HBITMAP bmp = CreateCompatibleBitmap(hdc, rcClientRect.right - rcClientRect.left,
			rcClientRect.bottom - rcClientRect.top);


		HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, bmp);
		auxdc = CreateCompatibleDC(memDC);
		if (!dados->eu.darkMode) {
			FillRect(memDC, &rcClientRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
		}
		else {
			FillRect(memDC, &rcClientRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}
		SelectObject(auxdc, pecas);

		width = SQ_SZ / dados->eu.mapa.col;
		height = SQ_SZ / dados->eu.mapa.lin;

		for (int i = 0; i < dados->eu.mapa.lin; i++) {
			for (int j = 0; j < dados->eu.mapa.col; j++) {

				//SelectObject(auxdc, bmpPeca);
				if (dados->eu.darkMode) {
					SelectObject(memDC, (HBRUSH)GetStockObject(BLACK_BRUSH));
					HPEN hpenDot = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
					SelectObject(memDC, hpenDot);
				}
				Rectangle(memDC, j * width, i * height, j * width + width, i * height + height);
				for (int k = 0; k < 2; k++) {
					for (int l = 0; l < 4; l++) {
						SelectObject(auxdc, pecas);
						if (dados->eu.mapa.board[i][j] == pecasText[k][l]) {
							/*BitBlt(memDC, j * SQ_SZ, i * SQ_SZ, j * SQ_SZ + SQ_SZ, i * SQ_SZ + SQ_SZ, auxdc, l * 50, k * 50, SRCCOPY);*/
							
							if (dados->eu.agua.board[i][j] == TEXT('w')) {
								SelectObject(auxdc, pecasAgua);
								//BitBlt(memDC, j * SQ_SZ, i * SQ_SZ, j * SQ_SZ + SQ_SZ, i * SQ_SZ + SQ_SZ, auxdc, l * 50, k * 50, SRCCOPY);
								//BitBlt(memDC, j * width, i * height, width, height, auxdc, l * 50, k * 50, SRCCOPY);
								StretchBlt(memDC, j * width, i * height, width, height, auxdc, l * 50, k * 50, 50, 50, SRCCOPY);
								continue;
							}
							//BitBlt(memDC, j * width, i * height, width, height, auxdc, l * 50, k * 50, SRCCOPY);
							StretchBlt(memDC, j * width, i * height, width, height, auxdc, l * 50, k * 50, 50, 50, SRCCOPY);
							continue;
						}
						if (dados->eu.mapa.board[i][j] == TEXT('|')) {
							SelectObject(auxdc, barreira);
							StretchBlt(memDC, j * width, i * height, width, height, auxdc, l * 50, k * 50, 50, 50, SRCCOPY);
							continue;
						}
					}
				}
			}
		}
		TCHAR tempo[50];
		TCHAR aleatorio[50];
		TCHAR nivel[50];
		int st = dados->eu.aleatorio;
		_stprintf_s(tempo, 50, TEXT("Segundos para começar: %d"), dados->eu.tempo);
		TextOut(memDC, 600, 100, tempo, _tcslen(tempo));
		_stprintf_s(aleatorio, 50, TEXT("Modo aleatório: %s"), status[st]);
		TextOut(memDC, 600, 200, aleatorio, _tcslen(aleatorio));
		_stprintf_s(nivel, 50, TEXT("Nível: %d"), dados->eu.nivel);
		TextOut(memDC, 600, 300, nivel, _tcslen(nivel));
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
		dados->eu.termina = true;
		SetEvent(event);
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_DARK_MODE && dados->eu.darkMode == false) {
			pecas = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECAS_DARK));
			pecasAgua = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECAS_AGUA_DARK));
			dados->eu.darkMode = true;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (LOWORD(wParam) == ID_LIGHT_MODE && dados->eu.darkMode == true) {
			pecas = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECAS));
			pecasAgua = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PECAS_AGUA));
			dados->eu.darkMode = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (LOWORD(wParam) == ID_INDIVIDUAL) {
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("REGISTO"));
			dados->eu.individual = true;
			dados->eu.ajuda = 0;
			SetEvent(event);
		}
		if (LOWORD(wParam) == ID_COMPETICAO) {
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("REGISTO"));
			dados->eu.individual = false;
			dados->eu.ajuda = 0;
			SetEvent(event);
		}
		break;
	case WM_LBUTTONDOWN:
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);

		width = SQ_SZ / dados->eu.mapa.col;
		height = SQ_SZ / dados->eu.mapa.lin;
		rect = getSquare(dados, p.x, p.y, width, height);
		if (rect.col == -1 || rect.lin == -1) {
			break;
		}

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 4; j++) {
				if (dados->eu.mapa.board[rect.lin][rect.col] == pecasText[i][j]){
					if (pecasText[i][j] != TEXT('i') && pecasText[i][j] != TEXT('f') && dados->eu.agua.board[rect.lin][rect.col] != TEXT('w')) {
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
						SetEvent(event);
						return;
					}
					else {
						break;
					}
				}
				else if(dados->eu.mapa.board[rect.lin][rect.col] == TEXT('□')) {
					dados->eu.x = rect.lin;
					dados->eu.y = rect.col;
					_tcscpy_s(dados->eu.mensagem, 20, TEXT("JOGADA"));
					if (!dados->eu.aleatorio) {
						dados->eu.peca = getProxPeca(pecasText[0][0]);
					}
					else {
						dados->eu.peca = getRandomPeca();
					}
					SetEvent(event);
					break;
				}
			}
		}
		break;
	case WM_RBUTTONDOWN:
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);
		width = SQ_SZ / dados->eu.mapa.col;
		height = SQ_SZ / dados->eu.mapa.lin;

		rect = getSquare(dados, p.x, p.y, width, height);
		
		if (dados->eu.mapa.board[rect.lin][rect.col] != TEXT('□') && dados->eu.agua.board[rect.lin][rect.col] != TEXT('w')) {
			dados->eu.peca = TEXT('□');
			dados->eu.x = rect.lin;
			dados->eu.y = rect.col;
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("APAGAR"));
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
			tme.dwHoverTime = 2000;
			g_fMouseTracking = TrackMouseEvent(&tme);
		}
		if (dados->eu.moveRato) {
			_tcscpy_s(dados->eu.mensagem, 20, TEXT("RATO"));
			dados->eu.moveRato = false;
			SetEvent(event);
		}
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		break;
	case  WM_MOUSEHOVER:
		p.x = GET_X_LPARAM(lParam);
		p.y = GET_Y_LPARAM(lParam);
		width = SQ_SZ / dados->eu.mapa.col;
		height = SQ_SZ / dados->eu.mapa.lin;

		rect = getSquare(dados, p.x, p.y, width, height);
		g_fMouseTracking = FALSE;
		if (dados->eu.ajuda < 3) {

			if (rect.lin == dados->eu.aguaAtual.atual_lin && rect.col == dados->eu.aguaAtual.atual_col) { // TO DO colocar as coordenadas da peça onde está a agua atualmente
				_tcscpy_s(dados->eu.mensagem, 20, TEXT("SUSPENDER"));
				dados->eu.ajuda++;
				_tprintf(TEXT("AJUDA N %d\n"), dados->eu.ajuda);
				dados->eu.moveRato = true;
				SetEvent(event);
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				g_fMouseTracking = TrackMouseEvent(&tme);
			}
		}

		return 0;
		break;
	case  WM_MOUSELEAVE:
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



