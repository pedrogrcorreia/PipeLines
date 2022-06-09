#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include "../util.h"
//#include "../mapa.h"
//#include "Clientes.h"

#define MAX 256
#define BUFSIZE 2048
#define Cl_Sz sizeof(Cliente)
HANDLE WriteReady;

void comecaIndividual(TDados* dados, HANDLE hPipe);

int writeClienteASINC(HANDLE hPipe, Cliente c) {
	DWORD cbWritten = 0;
	BOOL fSuccess = FALSE;

	OVERLAPPED OverlWr = { 0 };

	ZeroMemory(&OverlWr, sizeof(OverlWr));
	ResetEvent(WriteReady);
	OverlWr.hEvent = WriteReady;

	fSuccess = WriteFile(hPipe, &c, Cl_Sz, &cbWritten, &OverlWr);
	WaitForSingleObject(WriteReady, INFINITE);
	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
	return 1;
}

/* Thread para receber input do utilizador
	NESTA META APENAS SERVE PARA TERMINAR O SERVIDOR*/

DWORD WINAPI recebeInput(LPVOID param) {
	TDados* dados = (TDados*)param;
	TCHAR cmd[BUFFER_CHAR];
	while (!dados->ptr_memoria->terminar) {
		_fgetts(cmd, BUFFER_CHAR, stdin);
		if (_tcsicmp(cmd, TEXT("\n")) != 0) {
			cmd[_tcslen(cmd) - 1] = '\0';
		}
		if (_tcsicmp(cmd, TEXT("fim")) == 0) {
			break;
		}
		else {
			continue;
		}
	}
}

/* Thread para ficar a assinalar o mutex que suspende a água */

DWORD WINAPI suspendeAgua(LPVOID param) {
	TDados** dados = (TDados**)param;

	_tprintf(TEXT("Thread para suspender a água lançada...\n"));
	_tprintf(TEXT("Vai suspender a água por %d segundos\n"), (*dados)->jogo.agua);
	WaitForSingleObject(( * dados)->mutex_agua, INFINITE);
	Sleep((*dados)->jogo.agua * 1000);
	ReleaseMutex((*dados)->mutex_agua);
	_tprintf(TEXT("Thread para suspender a água terminada...\n"));
}

DWORD WINAPI suspendeAguaCliente(LPVOID param) {
	TDados* dados = (TDados*)param;
	int nCliente = -1;
	for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == dados->eu.hPipe) {
			nCliente = i;
			break;
		}
	}
	_tprintf(TEXT("Cliente %d suspendeu a água.\n"));

	WaitForSingleObject(dados->ptr_memoria->clientes[nCliente].mutexAgua, INFINITE);
	Sleep(2000);
	ReleaseMutex(dados->ptr_memoria->clientes[nCliente].mutexAgua);
}

/* Thread para receber comandos do processo Monitor
	Através do modelo Produtor - Consumidor */

DWORD WINAPI recebeComandos(LPVOID param) {
	TDados* dados = (TDados*)param;

	do {	
		Jogo jogo;
		// Esperar pelo semáforo dos itens
		WaitForSingleObject(dados->sem_itens, INFINITE);
		// Copiar para o processo o item a consumir
		CopyMemory(&jogo, &dados->ptr_modelo->jogosBuffer[dados->ptr_modelo->ent], sizeof(Jogo));
		// Incrementar posição de leitura
		dados->ptr_modelo->ent = (dados->ptr_modelo->ent + 1) % BUFFER;

		/* Processar os pedidos */
	
		// Suspender a agua
		if (jogo.agua != 0) {
			dados->jogo.agua = jogo.agua;
			_tprintf(TEXT("A agua foi suspensa por %d segundos"), dados->jogo.agua);
			HANDLE thread = CreateThread(NULL, 0, suspendeAgua, &dados, 0, NULL);
		}
		
		// Ativar aleatorio
		if (dados->jogo.aleatorio == true && jogo.aleatorio == false) {
			Cliente c;
			_tcscpy_s(c.mensagem, 20, TEXT("ALEATORIO"));
			c.aleatorio = false;
			for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
				writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
			}
			_tprintf(TEXT("Modo aleatório desativado.\n"));
		}
		if (dados->jogo.aleatorio == false && jogo.aleatorio == true) {
			Cliente c;
			_tcscpy_s(c.mensagem, 20, TEXT("ALEATORIO"));
			c.aleatorio = true;
			c.termina = false;
			for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
				_tprintf(TEXT("ESCREVI!\n"));
				writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
			}
			_tprintf(TEXT("Modo aleatório ativado.\n"));
		}

		// Inserir bloco
		if (jogo.insereBarreira == true) {
			_tprintf(TEXT("Foi inserida uma barreira na posição %d %d"), jogo.barreira.x, jogo.barreira.y);
			//dados->ptr_memoria->mapas[0].board[jogo.barreira.x][jogo.barreira.y] = '|';
			for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
				dados->ptr_memoria->clientes[i].mapa.board[jogo.barreira.x][jogo.barreira.y] = '|';
			}
			SetEvent(dados->event_atualiza);
			//printMapa(dados->ptr_memoria->mapas[0]);
		}

		// Assinalar semáforo dos vazios
		ReleaseSemaphore(dados->sem_vazios, 1, NULL);
		
	} while (!dados->ptr_memoria->terminar);
}

DWORD WINAPI moveAgua(LPVOID param) {
	TDados* dados = (TDados*)param;
	_tprintf(TEXT("Thread para mover a água lançada.\n"));
	int nCliente = -1;
	for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == dados->eu.hPipe) {
			nCliente = i;
			break;
		}
	}
	Sleep(dados->tempo * 1000);
	int i = 0;
	dados->jogo.atualizar = true;
	Agua agua;
	
	agua.prox_lin = 0;
	agua.prox_col = 0;
	
	//_tprintf(TEXT("nCliente: %d\n"), nCliente);
	//agua.mapa = dados->ptr_memoria->mapas[nCliente];
	agua.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
	do {
		Cliente c;
		// Esperar pelo mutex que pode estar a suspender a água
		WaitForSingleObject(dados->mutex_agua, INFINITE);
		WaitForSingleObject(dados->ptr_memoria->clientes[nCliente].mutexAgua, INFINITE);
		if (agua.prox_lin >= dados->ptr_memoria->clientes[nCliente].mapa.lin) {
			_tcscpy_s(c.mensagem, 20, TEXT("GANHOU"));
			writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
			break;
		}
		if (agua.prox_col >= dados->ptr_memoria->clientes[nCliente].mapa.col) {
			_tcscpy_s(c.mensagem, 20, TEXT("GANHOU"));
			writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
			break;
		}
		agua = moverAgua(agua, agua.prox_lin, agua.prox_col);
		dados->ptr_memoria->clientes[nCliente].agua = agua.mapa;
		_tprintf(TEXT("Agua a mexer no cliente %d...\n"), nCliente);
		//Atualizar o monitor...
		SetEvent(dados->event_atualiza);

		c.agua = agua.mapa;
		c.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
		//printMapa(c.mapa);
		c.termina = false;
		_tcscpy_s(c.mensagem, 20, TEXT("AGUA"));
		writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
		Sleep(1000); // Sleep de 1 segundo para a água não correr rápido demais
		// Libertar mutex
		ReleaseMutex(dados->mutex_agua);
		ReleaseMutex(dados->ptr_memoria->clientes[nCliente].mutexAgua);
	} while (1);
	//dados->ptr_memoria->nClientes--; // limpar o mapa
}

DWORD WINAPI ClienteThread(LPVOID param) {
	TDados* dados = (TDados*)param;
	Cliente recebido, enviado;
	DWORD cbBytesRead = 0, cbReplyBytes = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = dados->serverPipe;
	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };
	_tprintf(TEXT("ENTROU CLIENTE\n"));

	ReadReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	adicionaCliente(dados, hPipe);
	int nCliente = -1;
	for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == hPipe) {
			nCliente = i;
			break;
		}
	}
	enviado.hPipe = hPipe;
	enviado.termina = false;
	enviado.aleatorio = dados->jogo.aleatorio;
	enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
	_tcscpy_s(enviado.mensagem, 20, TEXT("ESTOU A FUNCIONAR!"));
	//registaCliente(dados, recebido);
	SetEvent(dados->event_atualiza); // atualizar o monitor
	writeClienteASINC(hPipe, enviado);
	while (1) {
		_tprintf(TEXT("RECEBI UMA MENSAGEM!!!\n"));
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		ResetEvent(ReadReady);
		OverlRd.hEvent = ReadReady;

		fSuccess = ReadFile(hPipe, &recebido, Cl_Sz, &cbBytesRead, &OverlRd);
		//_tprintf(TEXT("%d"), fSuccess);
		WaitForSingleObject(ReadReady, INFINITE);

		printMapa(dados->ptr_memoria->clientes[0].mapa);
		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE);
		if (recebido.termina) {
			removeCliente(dados, hPipe);
			break;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("REGISTO")) == 0) {
			if (recebido.individual) {
				comecaIndividual(dados, hPipe);
			}
			else {
				//comecaCompeticao(dados, hPipe);
			}
			continue;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("JOGADA")) == 0) {
			dados->ptr_memoria->clientes[nCliente].mapa = jogaPeca(dados->ptr_memoria->clientes[nCliente].mapa, recebido.x, recebido.y, recebido.peca);
			_tcscpy_s(enviado.mensagem, 20, TEXT("JOGADA"));
			enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
			enviado.agua = dados->ptr_memoria->clientes[nCliente].agua;
			writeClienteASINC(hPipe, enviado);
			continue;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("MUDAR")) == 0) {
			dados->ptr_memoria->clientes[nCliente].mapa = jogaPeca(dados->ptr_memoria->clientes[nCliente].mapa, recebido.x, recebido.y, recebido.peca);
			_tcscpy_s(enviado.mensagem, 20, TEXT("MUDAR"));
			enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
			writeClienteASINC(hPipe, enviado);
			continue;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("APAGAR")) == 0) {
			dados->ptr_memoria->clientes[nCliente].mapa = jogaPeca(dados->ptr_memoria->clientes[nCliente].mapa, recebido.x, recebido.y, recebido.peca);
			_tcscpy_s(enviado.mensagem, 20, TEXT("APAGAR"));
			enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
			writeClienteASINC(hPipe, enviado);
			continue;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("SUSPENDER")) == 0) {
			// TO DO 
		}
		//enviado.hPipe = hPipe;
		//enviado.termina = false;
		//enviado.mapa = dados->ptr_memoria->mapas[0];
		//enviado.agua = dados->ptr_memoria->agua;
		//_tcscpy_s(enviado.mensagem, 20, TEXT("ESTOU A FUNCIONAR!"));
		//registaCliente(dados, recebido);
		//writeClienteASINC(hPipe, enviado);
		//_tprintf(TEXT("%d\n"), recebido.individual);
		//_tprintf(TEXT("%s\n"), recebido.nome);
		//_tprintf(TEXT("%d\n"), recebido.square);
	}

	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	return 1;
}

DWORD WINAPI recebeClientes(LPVOID param) {
	TDados* dados = (TDados*)param;
	BOOL fConnected = FALSE;
	DWORD dwThreadId = 0;
	HANDLE hThread = NULL;
	HANDLE hPipe;
	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	while (1) {
		hPipe = CreateNamedPipe(PIPE_SERVER, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 5000, NULL);

		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		dados->serverPipe = hPipe;
		if (fConnected) {
			hThread = CreateThread(NULL, 0, ClienteThread, (LPVOID)dados, 0, &dwThreadId);
			if (hThread == NULL) {
				return -1;
			}
			else {
				CloseHandle(hThread);
			}
		}
		else {
			CloseHandle(hPipe);
		}
	}
	return 0;
}

void comecaIndividual(TDados* dados, HANDLE hPipe) {
	HANDLE hThread;
	//Mapa mapa;
	//mapa.lin = dados->lin;
	//mapa.col = dados->col;
	//mapa = criaMapaDebug(mapa);
	//dados->ptr_memoria->mapas[dados->ptr_memoria->nClientes - 1] = mapa;
	_tprintf(TEXT("Jogo individual a começar!\n"));
	dados->eu.hPipe = hPipe;
	hThread = CreateThread(NULL, 0, moveAgua, (LPVOID)dados, 0, NULL);
	//WaitForSingleObject(hThread, INFINITE);
	return;
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

	/* Criação do mapa com posições random*/

	//mapa = criaMapa(mapa); 
	//printMapa(mapa);

	/* Criação do mapa com um caminho definido */

	mapa = criaMapaDebug(mapa);
	printMapa(mapa);

	/* Iniciar memoria partilhada */

	HANDLE objMapMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMapMem == NULL) {
		_tprintf(TEXT("Erro a criar ficheiro de memória partilhada.\n"));
		return -1;
	}
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMapMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	HANDLE objMapMod = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Modelo), MODELO);
	if (objMapMod == NULL) {
		_tprintf(TEXT("Erro a criar ficheiro de memória partilhada.\n"));
		return -1;
	}
	dados.ptr_modelo = (Modelo*)MapViewOfFile(objMapMod, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	iniciaClientes(&dados);


	/* Inicializar a estrutura de dados */
	Jogo jogo;
	Barreira b;
	b.x = 0;
	b.y = 0;
	jogo.agua = 0;
	jogo.aleatorio = false;
	jogo.atualizar = false;
	jogo.insereBarreira = false;
	jogo.barreira = b;
	//dados.ptr_memoria->mapas[0] = mapa;
	//dados.ptr_memoria->mapas[1] = mapa;

	dados.ptr_memoria->clientes[0].mapa = mapa;
	dados.ptr_memoria->clientes[1].mapa = mapa;
	dados.ptr_memoria->nClientes = 0;
	dados.ptr_memoria->terminar = false;
	dados.jogo = jogo;
	
	/* Cria semáforos para o modelo produtor consumidor */
	dados.sem_itens = CreateSemaphore(NULL, 0, BUFFER, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, BUFFER, BUFFER, SEMAFORO_VAZIOS);

	/* Mutex para controlar a suspensão da água */
	dados.mutex_agua = CreateMutex(NULL, FALSE, MUTEX_AGUA);
	dados.event_atualiza = CreateEvent(NULL, FALSE, FALSE, EVENT_ATUALIZAR);

	/* Mutex para controloar a suspensao de água pelos clientes */
	dados.ptr_memoria->clientes[0].mutexAgua = CreateMutex(NULL, FALSE, TEXT("MUTEX_AGUA_CLIENTE_1"));
	dados.ptr_memoria->clientes[1].mutexAgua = CreateMutex(NULL, FALSE, TEXT("MUTEX_AGUA_CLIENTE_2"));

	HANDLE hThread[3];
	/* Thread para receber comandos do Monitor */
	hThread[0] = CreateThread(NULL, 0, recebeComandos, &dados, 0, NULL);

	/* Thread para movimentar a água */
	//hThread[1] = CreateThread(NULL, 0, moveAgua, &dados, 0, NULL);
	
	/* Thread para receber comandos do utilizador */
	hThread[1] = CreateThread(NULL, 0, recebeInput, &dados, 0, NULL);

	/* Thread para receber clientes */
	hThread[2] = CreateThread(NULL, 0, recebeClientes, &dados, 0, NULL);

	/* Esperar que uma das threads termine para terminar o processo */
	WaitForMultipleObjects(3, hThread, FALSE, INFINITE);

	// Terminar todos os processos que estejam à espera
	dados.ptr_memoria->terminar = true;

	// Fechar o ficheiro de memória partilhada
	UnmapViewOfFile(dados.ptr_memoria);
	UnmapViewOfFile(dados.ptr_modelo);
	
	return 0;
}
