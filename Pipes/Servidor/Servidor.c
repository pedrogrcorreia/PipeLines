#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include "../util.h"
#include "../mapa.h"

#define MAX 256


/* Thread para receber comandos do processo Monitor 
	Através do Modelo Produtor - Consumidor */

DWORD WINAPI recebeComandos(LPVOID param) {
	TDados* dados = (TDados*)param;
	TCHAR cmd[250];
	//Jogo jogo;
	do {
		_fgetts(cmd, 250, stdin);
		/*
		Jogo jogo;
		// Esperar pelo semáforo dos itens
		WaitForSingleObject(dados->sem_itens, INFINITE);
		// Copiar para o processo o item a consumir
		CopyMemory(&jogo, &dados->ptr_modelo->jogosBuffer[dados->ptr_modelo->ent], sizeof(Jogo));
		// Incrementar posição de leitura
		dados->ptr_modelo->ent = (dados->ptr_modelo->ent + 1) % BUFFER;
		*/
		/* Processar os pedidos */
		/*
		// Suspender a agua
		if (jogo.agua != 0) {
			dados->jogo.agua = jogo.agua;
			_tprintf(TEXT("A agua foi suspensa por %d segundos"), dados->jogo.agua);
			WaitForSingleObject(dados->mutex_agua, INFINITE);
			Sleep(dados->jogo.agua * 1000);
			dados->jogo.agua = 0;
			ReleaseMutex(dados->mutex_agua);
		}
		
		// Ativar aleatorio
		if (dados->jogo.aleatorio == true && jogo.aleatorio == false) {
			_tprintf(TEXT("Modo aleatório desativado.\n"));
		}
		if (dados->jogo.aleatorio == false && jogo.aleatorio == true) {
			_tprintf(TEXT("Modo aleatório ativado.\n"));
		}

		// Inserir bloco
		if (jogo.insereBarreira == true) {
			_tprintf(TEXT("Foi inserida uma barreira na posição %d %d"), jogo.barreira.x, jogo.barreira.y);
			dados->ptr_memoria->mapas[0].board[jogo.barreira.x][jogo.barreira.y] = '|';
		}
		//if (jogo.atualizar == true) {
		//	printMapa(dados->ptr_memoria->mapas[0]);
		//}
		// Assinalar semáforo dos vazios
		ReleaseSemaphore(dados->sem_vazios, 1, NULL);
		*/
	} while (!dados->ptr_memoria->terminar);
}

DWORD WINAPI moveAgua(LPVOID param) {
	TDados* dados = (TDados*)param;
	_tprintf(TEXT("Thread para mover a água lançada."));
	Sleep(dados->tempo * 1000);
	int i = 0;
	DWORD result;
	dados->jogo.atualizar = true;
	do {
		// Esperar pelo mutex que pode estar a suspender a água
		result = WaitForSingleObject(dados->mutex_agua, INFINITE);
		// Faz a água avançar
		dados->ptr_memoria->mapas[0].board[i][i] = 'W';
		i++;
		_tprintf(TEXT("Agua a mexer...\n"));
		// Atualizar o monitor...
		//SetEvent(dados->event_atualiza);
		WaitForSingleObject(dados->sem_vazios, INFINITE);
		CopyMemory(&dados->ptr_modelo->jogosBuffer[dados->ptr_modelo->sai], &dados->jogo, sizeof(Jogo));
		dados->ptr_modelo->sai = (dados->ptr_modelo->sai + 1) % BUFFER;
		//dados->jogo.atualizar = false;
		ReleaseSemaphore(dados->sem_itens, 1, NULL);
		Sleep(2000);
		// Libertar mutex
		ReleaseMutex(dados->mutex_agua);
	} while (i < dados->col);
	dados->ptr_memoria->terminar = true;
}

DWORD WINAPI atualizaMonitor(LPVOID param) {
	TDados* dados = (TDados*)param;
	do {
		WaitForSingleObject(dados->event_atualiza, INFINITE);
		WaitForSingleObject(dados->sem_vazios, INFINITE);
		dados->jogo.atualizar = true;
		CopyMemory(&dados->ptr_modelo->jogosBuffer[dados->ptr_modelo->sai], &dados->jogo, sizeof(Jogo));
		dados->ptr_modelo->sai = (dados->ptr_modelo->sai + 1) % BUFFER;
		dados->jogo.atualizar = false;
		ReleaseSemaphore(dados->sem_itens, 1, NULL);
		//_tprintf(TEXT("TEMPO: %d, VERTICAL: %d, HORIZONTAL: %d\n"), tmp, lin, col);
		//_fgetts(cmd, 250, stdin);
		//Sleep(3000);
		//WaitForSingleObject(mutex, INFINITE);
		//mapa.board[i][i] = c;
		//i++;
		//dados.ptr_memoria->mapas[0] = mapa;
		//SetEvent(event);
		//Sleep(3000);

		//ReleaseMutex(mutex);
	} while (1/*_tcsicmp(cmd, TEXT("fim\n")) != 0*/);
}

int _tmain(int argc, LPTSTR argv[]) {
	HANDLE semaforo_execucao;
	HKEY chave;
	DWORD result, cbdata = sizeof(DWORD);
	TCHAR chave_nome[500] = TEXT("SOFTWARE\\temp\\SO2\\TP");

	TDados dados;


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

	/* Semaforo de controlo da execução. Apenas permite que um programa servidor corra em simultâneo. */
	semaforo_execucao = CreateSemaphore(NULL, 0, 1, SEMAFORO_EXECUCAO);
	result = GetLastError();
	if (result == ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("Já existe um servidor em execução.\nTermine-o para iniciar um novo"));
		return -1;
	}

	/* Obtem as dimensões do mapa pela linha de comandos. Se for mais do que 20 fica com 20 por defeito. */
	if (argc == 4) {
		_tprintf(TEXT("HERE"));
		dados.tempo = _tstoi(argv[1]);
		dados.lin = _tstoi(argv[2]);
		dados.col = _tstoi(argv[3]);
		/* Gravar o tempo no Registry*/
		if (RegCreateKeyEx(
			HKEY_CURRENT_USER,
			chave_nome,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&chave,
			&result
		) != ERROR_SUCCESS) {
			_tprintf(TEXT("Chave %s não foi criada nem aberta!\n"), chave_nome);
		}
		else {
			TCHAR tempo[10] = TEXT("TEMPO");
			if (RegSetValueEx(
				chave,
				(LPCSTR)tempo,
				0,
				REG_DWORD,
				(LPBYTE)&dados.tempo,
				sizeof(dados.tempo)
			) != ERROR_SUCCESS) {
				_tprintf(TEXT("Erro ao aceder ao atributo %s"), tempo);
			}
		}

		/* Se for mais que 20 fica a 20*/
		if (dados.lin > 20) {
			_tprintf(TEXT("As dimensões não podem exceder 20. Vai ser definido o limite máximo.\n"));
			dados.lin = 20;
		}
		/* Gravar as dimensoes verticais no Registry */
		if (RegCreateKeyEx(
			HKEY_CURRENT_USER,
			chave_nome,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&chave,
			&result
		) != ERROR_SUCCESS) {
			_tprintf(TEXT("Chave %s não foi criada nem aberta!\n"), chave_nome);
		}
		else {
			TCHAR vertical[10] = TEXT("VERTICAL");
			if (RegSetValueEx(
				chave,
				(LPCSTR)vertical,
				0,
				REG_DWORD,
				(LPBYTE)&dados.lin,
				sizeof(dados.lin)
			) != ERROR_SUCCESS) {
				_tprintf(TEXT("Erro ao aceder ao atributo %s"), vertical);
			}
		}

		/* Se for mais que 20 fica a 20 */
		if (dados.col > 20) {
			_tprintf(TEXT("As dimensões não podem exceder 20. Vai ser definido o limite máximo.\n"));
			dados.col = 20;
		}
		/* Gravar as dimensoes horizontais no Registry */
		if (RegCreateKeyEx(
			HKEY_CURRENT_USER,
			chave_nome,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&chave,
			&result
		) != ERROR_SUCCESS) {
			_tprintf(TEXT("Chave %s não foi criada nem aberta!\n"), chave_nome);
		}
		else {
			TCHAR horizontal[20] = TEXT("HORIZONTAL");
			if (RegSetValueEx(
				chave,
				(LPCSTR)horizontal,
				0,
				REG_DWORD,
				(LPBYTE)&dados.col,
				sizeof(dados.col)
			) != ERROR_SUCCESS) {
				_tprintf(TEXT("Erro ao aceder ao atributo %s"), horizontal);
			}
		}
	}

	/* Obter as dimensões pelo Registry */
	else {
		RegOpenKeyEx(HKEY_CURRENT_USER, chave_nome, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &chave);
		result = RegQueryValueEx(chave, TEXT("TEMPO"), NULL, NULL, (LPBYTE)&dados.tempo, (LPDWORD)&cbdata);
		if (result != ERROR_SUCCESS) {
			_tprintf(TEXT("\nNão foi possível ler do registo o tempo que demora a água a correr.\nVai ser definido como 30 segundos.\n"));
			dados.tempo = 30;
		}
		result = RegQueryValueEx(chave, TEXT("VERTICAL"), NULL, NULL, (LPBYTE)&dados.lin, (LPDWORD)&cbdata);
		if (result != ERROR_SUCCESS) {
			_tprintf(TEXT("\nNão foi possível ler do registo o número de linhas do mapa\nVai ser definido como 20.\n"));
			dados.lin = 20;
		}
		result = RegQueryValueEx(chave, TEXT("HORIZONTAL"), NULL, NULL, (LPBYTE)&dados.col, (LPDWORD)&cbdata);
		if (result != ERROR_SUCCESS) {
			_tprintf(TEXT("\nNão foi possível ler do registo o número de colunas do mapa\nVai ser definido como 20.\n"));
			dados.col = 20;
		}
	}

	Mapa mapa;
	mapa.lin = dados.lin;
	mapa.col = dados.col;
	mapa = criaMapa(mapa);
	printMapa(mapa);

	/* Iniciar memoria partilhada */

	HANDLE objMapMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMapMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	HANDLE objMapMod = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Modelo), MODELO);
	dados.ptr_modelo = (Modelo*)MapViewOfFile(objMapMod, FILE_MAP_ALL_ACCESS, 0, 0, 0);


	/* Inicializar a estrutura de dados */
	Jogo jogo;
	jogo.agua = 0;
	jogo.aleatorio = false;
	jogo.atualizar = false;
	jogo.insereBarreira = false;
	dados.ptr_memoria->mapas[0] = mapa;
	dados.ptr_memoria->terminar = false;
	dados.jogo = jogo;
	
	/* Cria semáforos para o modelo produtor consumidor */
	dados.sem_itens = CreateSemaphore(NULL, 0, BUFFER, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, BUFFER, BUFFER, SEMAFORO_VAZIOS);
	dados.sem_mutex_p = CreateSemaphore(NULL, 1, 1, SEM_MUTEX_P);
	dados.sem_mutex_c = CreateSemaphore(NULL, 1, 1, SEM_MUTEX_C);
	TCHAR cmd[250];
	TCHAR c = 'b';
	int i = 0;
	//Jogo jogo;

	/* Mutex para controlar a suspensão da água*/
	dados.mutex_agua = CreateMutex(NULL, FALSE, MUTEX_AGUA);
	dados.event_atualiza = CreateEvent(NULL, FALSE, FALSE, EVENT_ATUALIZAR);

	HANDLE hThread[3];
	/* Thread para receber comandos do Monitor */
	hThread[0] = CreateThread(NULL, 0, recebeComandos, &dados, 0, NULL);

	/* Thread para movimentar a água */
	hThread[1] = CreateThread(NULL, 0, moveAgua, &dados, 0, NULL);
	
	/* Thread para enviar atualizações ao Monitor */
	hThread[2] = CreateThread(NULL, 0, atualizaMonitor, &dados, 0, NULL);
	WaitForMultipleObjects(3, hThread, FALSE, INFINITE);


	//do {
	//	_fgetts(cmd, 250, stdin);
	//
	//	if (_tcsicmp(cmd, TEXT("\n")) != 0) {
	//		cmd[_tcslen(cmd) - 1] = '\0'; // retirar \n
	//	}

	//	if (_tcsicmp(cmd, TEXT("s\n")) == 0) {
	//		dados.jogo.atualizar = true;
	//	}
	//	else {
	//		dados.jogo.atualizar = false;
	//	}
	//	WaitForSingleObject(dados.sem_vazios, INFINITE);
	//	CopyMemory(&dados.ptr_modelo->jogosBuffer[dados.ptr_modelo->sai], &jogo, sizeof(Jogo));
	//	dados.ptr_modelo->sai = (dados.ptr_modelo->sai + 1) % BUFFER;
	//	ReleaseSemaphore(dados.sem_itens, 1, NULL);
	//	//_tprintf(TEXT("TEMPO: %d, VERTICAL: %d, HORIZONTAL: %d\n"), tmp, lin, col);
	//	//_fgetts(cmd, 250, stdin);
	//	//Sleep(3000);
	//	//WaitForSingleObject(mutex, INFINITE);
	//	//mapa.board[i][i] = c;
	//	//i++;
	//	//dados.ptr_memoria->mapas[0] = mapa;
	//	//SetEvent(event);
	//	//Sleep(3000);

	//	//ReleaseMutex(mutex);
	//} while (1/*_tcsicmp(cmd, TEXT("fim\n")) != 0*/);



	//for (int i = 0; i < lin; i++) {
	//	for (int j = 0; j < col; j++) {
	//		mapa.board[i][j] = 'a';
	//	}
	//}

	//for (int i = 0; i < lin; i++) {
	//	for (int j = 0; j < col; j++) {
	//		_tprintf(TEXT("%c"), mapa.mapa[i][j]);
	//	}
	//	_tprintf(TEXT("\n"));
	//}


	//char cmd[250];

	//char** mapa = (char*)malloc(col * sizeof(char*));
	//for (int i = 0; i < lin; i++) {
	//	*(mapa + i) =(char*) malloc(lin * sizeof(char));
	//}
	//for (int i = 0; i < lin; i++) {
	//	for (int j = 0; j < col; j++) {
	//		mapa[i][j] = 'a';
	//	}
	//}

	//for (int i = 0; i < lin; i++) {
	//	for (int j = 0; j < col; j++) {
	//		_tprintf(TEXT("%c"), mapa[i][j]);
	//	}
	//	_tprintf(TEXT("\n"));
	//}
	//_tprintf(TEXT("%c\n"), mapa[3][3]);


	///* Obter as posicioes iniciais do circuito */
	//srand((unsigned int)time(NULL));
	//int ini = (rand() % (lin - 0)) + 0;
	//int fin = (rand() % (lin - 0)) + 0;
	//_tprintf(TEXT("%d %d\n"), ini, fin);

	///* Coluna 0 e Linha random*/
	//mapa[ini][0] = 'i';

	///* Ultima coluna e Linha random */

	///* TODO DIAGONALMENTE OPOSTOS */
	//mapa[fin][col-1] = 'f';

	//for (int i = 0; i < lin; i++) {
	//	for (int j = 0; j < col; j++) {
	//		_tprintf(TEXT("%c"), mapa[i][j]);
	//	}
	//	_tprintf(TEXT("\n"));
	//}

	//do {
	//	_tprintf(TEXT("TEMPO: %d, VERTICAL: %d, HORIZONTAL: %d\n"), tmp, lin, col);
	//	_fgetts(cmd, 250, stdin);
	//} while (_tcsicmp(cmd, TEXT("fim\n")) != 0);

	return 0;
}
