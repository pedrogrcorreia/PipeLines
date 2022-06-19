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

int writeClienteASINC(HANDLE hPipe, Cliente c) {
	DWORD cbWritten = 0;
	BOOL fSuccess = FALSE;

	OVERLAPPED OverlWr = { 0 };

	ZeroMemory(&OverlWr, sizeof(OverlWr));
	ResetEvent(WriteReady);
	OverlWr.hEvent = WriteReady;

	c.moveRato = false;

	fSuccess = WriteFile(hPipe, &c, Cl_Sz, &cbWritten, &OverlWr);
	WaitForSingleObject(WriteReady, 15*1000); // Fica aqui 15 segundos a espera, depois desliga TO DO desligar
	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
	return 1;
}

/* Thread para receber input do utilizador*/

DWORD WINAPI recebeInput(LPVOID param) {
	TDados* dados = (TDados*)param;
	TCHAR cmd[BUFFER_CHAR];
	while (!dados->ptr_memoria->terminar) {
		_fgetts(cmd, BUFFER_CHAR, stdin);
		if (_tcsicmp(cmd, TEXT("\n")) != 0) {
			cmd[_tcslen(cmd) - 1] = '\0';
		}
		if (_tcsicmp(cmd, TEXT("fim")) == 0) {
			dados->ptr_memoria->terminar = true;
			Cliente c;
			c.termina = true;
			for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
				writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
			}
			break;
		}
		if (_tcsicmp(cmd, TEXT("listar")) == 0) {
			printClientes(dados);
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
	Cliente* recebido = (Cliente*)param;

	
	WaitForSingleObject(recebido->mutexAgua, INFINITE);
	WaitForSingleObject(recebido->event_rato, INFINITE);
	ReleaseMutex(recebido->mutexAgua);
	_tprintf(TEXT("A sair da thread...\n"));
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
		if (dados->jogo.aleatorio == true && jogo.aleatorio == true) {
			Cliente c;
			for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
				CopyMemory(&c, &dados->ptr_memoria->clientes[i], sizeof(Cliente));
				_tcscpy_s(c.mensagem, 20, TEXT("ALEATORIO"));
				c.aleatorio = false;
				writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
			}
			_tprintf(TEXT("Modo aleatório desativado.\n"));

			dados->jogo.aleatorio = false;
			continue;
		}
		if (dados->jogo.aleatorio == false && jogo.aleatorio == true) {
			Cliente c;
			for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
				CopyMemory(&c, &dados->ptr_memoria->clientes[i], sizeof(Cliente));
				_tcscpy_s(c.mensagem, 20, TEXT("ALEATORIO"));
				c.aleatorio = true;
				writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
			}
			dados->jogo.aleatorio = true;
			_tprintf(TEXT("Modo aleatório ativado.\n"));
			continue;
		}

		// Inserir bloco
		if (jogo.insereBarreira == true) {
			_tprintf(TEXT("Foi inserida uma barreira na posição %d %d"), jogo.barreira.x, jogo.barreira.y);
			//dados->ptr_memoria->mapas[0].board[jogo.barreira.x][jogo.barreira.y] = '|';
			for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
				dados->ptr_memoria->clientes[i].mapa.board[jogo.barreira.x][jogo.barreira.y] = '|';
			}
			SetEvent(dados->event_atualiza);
		}

		// Assinalar semáforo dos vazios
		ReleaseSemaphore(dados->sem_vazios, 1, NULL);
		
	} while (!dados->ptr_memoria->terminar);
}

DWORD WINAPI moveAgua(LPVOID param) {
	TDados* dados = (TDados*)param;
	
	int nCliente = -1;
	for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == dados->eu.hPipe) {
			nCliente = i;
			break;
		}

	}
	_tprintf(TEXT("Thread para mover a água do cliente %d lançada.\n"), nCliente);
	Cliente c;
	c.termina = false;
	c.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
	c.agua = dados->ptr_memoria->clientes[nCliente].agua;
	c.aleatorio = dados->ptr_memoria->clientes[nCliente].aleatorio;
	c.nivel = dados->ptr_memoria->clientes[nCliente].nivel;
	c.individual = dados->ptr_memoria->clientes[nCliente].individual;
	for (int i = 1; i <= dados->tempo; i++) {
		Sleep(1000);
		_tcscpy_s(c.mensagem, 20, TEXT("TEMPO"));
		c.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
		c.tempo = dados->tempo - i;
		if (dados->ptr_memoria->clientes[nCliente].termina) {
			break;
		}
		writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
	}

	int i = 0;
	dados->jogo.atualizar = true;
	Agua agua;
	
	for (int i = 0; i < dados->ptr_memoria->clientes[nCliente].mapa.lin; i++) {
		for (int j = 0; j < dados->ptr_memoria->clientes[nCliente].mapa.col; j++) {
			if (dados->ptr_memoria->clientes[nCliente].mapa.board[i][j] == TEXT('i')) {
				agua.prox_lin = i;
				agua.prox_col = j;
				break;
			}
		}
	}
	agua.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
	agua.atual_col = -1;
	agua.atual_lin = -1;
	agua.perdeu = false;
	agua.ganhou = false;
	dados->ptr_memoria->clientes[nCliente].aguaAtual = agua;
	while(!dados->ptr_memoria->clientes[nCliente].termina){//do {
		// Esperar pelo mutex que pode estar a suspender a água
		DWORD result = WaitForSingleObject(dados->mutex_agua, INFINITE);

		result = WaitForSingleObject(dados->ptr_memoria->clientes[nCliente].mutexAgua, INFINITE);

		if (agua.ganhou) {
			_tcscpy_s(c.mensagem, 20, TEXT("GANHOU"));
			writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
			break;
		}
		if (agua.perdeu) {
			_tcscpy_s(c.mensagem, 20, TEXT("PERDEU"));
			writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
			break;
		}
		if (agua.prox_lin >= dados->ptr_memoria->clientes[nCliente].mapa.lin) {
			_tcscpy_s(c.mensagem, 20, TEXT("PERDEU"));
			writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
			break;
		}
		if (agua.prox_col >= dados->ptr_memoria->clientes[nCliente].mapa.col) {
			_tcscpy_s(c.mensagem, 20, TEXT("PERDEU"));
			writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
			break;
		}
		agua.mapa = atualizaAgua(dados->ptr_memoria->clientes[nCliente].mapa, agua.mapa);
		agua = moverAgua(agua, agua.prox_lin, agua.prox_col);
		dados->ptr_memoria->clientes[nCliente].agua = agua.mapa;
		dados->ptr_memoria->clientes[nCliente].aguaAtual = agua;
		//_tprintf(TEXT("Agua a mexer no cliente %d...\n"), nCliente);

		//Atualizar o monitor...
		SetEvent(dados->event_atualiza);

		c.agua = agua.mapa;
		c.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
		c.aguaAtual = agua;
		c.termina = false;
		_tcscpy_s(c.mensagem, 20, TEXT("AGUA"));
		if (!dados->ptr_memoria->clientes[nCliente].termina) {
			writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
		}

		//Sleep(1000); // Sleep de 1 segundo para a água não correr rápido demais
		Sleep((11 - dados->ptr_memoria->clientes[nCliente].nivel) * 1000); // decresce 1 segundo por nivel a velocidade da agua

		// Libertar mutex
		ReleaseMutex(dados->mutex_agua);
		ReleaseMutex(dados->ptr_memoria->clientes[nCliente].mutexAgua);
	} 
	_tprintf(TEXT("Thread para movimentar a água concluída...\n"));
	
}

DWORD WINAPI moveAguaCompeticao(LPVOID param) {
	TDados* dados = (TDados*)param;
	_tprintf(TEXT("Thread para mover a água da competicao lançada.\n"));
	Cliente c;
	for (int i = 0; i < MAX_CLI; i++) {
		c.termina = false;
		c.mapa = dados->ptr_memoria->clientes[i].mapa;
		c.agua = dados->ptr_memoria->clientes[i].agua;
		c.aleatorio = dados->ptr_memoria->clientes[i].aleatorio;
		c.nivel = dados->ptr_memoria->clientes[i].nivel;
	}
	for (int i = 1; i <= dados->tempo; i++) {
		Sleep(1000); // Espera 1 segundo dados->tempo vezes para começar a àgua
		for (int nCliente = 0; nCliente < MAX_CLI; nCliente++) {
			_tcscpy_s(c.mensagem, 20, TEXT("TEMPO"));
			c.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
			c.tempo = dados->tempo - i;
			if (dados->ptr_memoria->clientes[nCliente].termina) {
				break;
			}
			writeClienteASINC(dados->ptr_memoria->clientes[nCliente].hPipe, c);
		}
	}

	int i = 0;
	dados->jogo.atualizar = true;
	Agua agua[2];

	for (int i = 0; i < dados->ptr_memoria->clientes[0].mapa.lin; i++) {
		for (int j = 0; j < dados->ptr_memoria->clientes[0].mapa.col; j++) {
			for (int k = 0; k < 2; k++) {
				if (dados->ptr_memoria->clientes[0].mapa.board[i][j] == TEXT('i')) {
					agua[k].prox_lin = i;
					agua[k].prox_col = j;
					agua[k].perdeu = false;
					agua[k].ganhou = false;
				}
			}
		}
	}

	for (int nCliente = 0; nCliente < MAX_CLI; nCliente++) {
		agua[nCliente].mapa = dados->ptr_memoria->clientes[nCliente].mapa;
	}

	bool termina = false;

	while (!dados->ptr_memoria->terminar) {
		// Esperar pelo mutex que pode estar a suspender a água
		DWORD result = WaitForSingleObject(dados->mutex_agua, INFINITE);

		for (int i = 0; i < MAX_CLI; i++) {
			result = WaitForSingleObject(dados->ptr_memoria->clientes[i].mutexAgua, INFINITE);
			if (agua[i].ganhou) {
				_tcscpy_s(c.mensagem, 20, TEXT("GANHOU COMP"));
				writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
				if (i == 1) {
					_tcscpy_s(c.mensagem, 20, TEXT("PERDEU COMP"));
					writeClienteASINC(dados->ptr_memoria->clientes[i - 1].hPipe, c);
				}
				termina = true;
				break;
			}
			if (agua[i].perdeu || agua[i].prox_lin >= dados->ptr_memoria->clientes[i].mapa.lin || agua[i].prox_col >= dados->ptr_memoria->clientes[i].mapa.col) {
				_tcscpy_s(c.mensagem, 20, TEXT("PERDEU COMP"));
				writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
				_tcscpy_s(c.mensagem, 20, TEXT("GANHOU COMP"));
				if (i == 1) {
					writeClienteASINC(dados->ptr_memoria->clientes[i - 1].hPipe, c);
				}
				else {
					writeClienteASINC(dados->ptr_memoria->clientes[i + 1].hPipe, c);
				}
				termina = true;
				break;
			}

			agua[i].mapa = atualizaAgua(dados->ptr_memoria->clientes[i].mapa, agua[i].mapa);
			agua[i] = moverAgua(agua[i], agua[i].prox_lin, agua[i].prox_col);
			dados->ptr_memoria->clientes[i].agua = agua[i].mapa;
			//_tprintf(TEXT("Agua a mexer no cliente %d...\n"), i);

			c.agua = agua[i].mapa;
			c.mapa = dados->ptr_memoria->clientes[i].mapa;
			
			c.termina = false;
			_tcscpy_s(c.mensagem, 20, TEXT("AGUA"));
			if (!dados->ptr_memoria->clientes[i].termina) {
				writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
			}
			ReleaseMutex(dados->ptr_memoria->clientes[i].mutexAgua);
		}
		//Atualizar o monitor...
		SetEvent(dados->event_atualiza);
		if (termina) {
			break;
		}
		Sleep(5000); // Sleep de 5 segundos para a água não correr rápido demais
		ReleaseMutex(dados->mutex_agua);
	}
	_tprintf(TEXT("Thread para movimentar a água da competição concluída...\n"));
	for (int i = 0; i < MAX_CLI; i++) {
		dados->comp.nJogadores--;
	}
}

DWORD WINAPI competicaoThread(LPVOID param) {
	TDados* dados = (TDados*)param;

	dados->comp.event_comp = CreateEvent(NULL, FALSE, FALSE, NULL);
	dados->comp.nJogadores = 0;
	Cliente c;
	c.termina = false;
	do {
		dados->comp.nJogadores = 0;
		WaitForSingleObject(dados->comp.event_comp, INFINITE);
		for (int i = 0; i < MAX_CLI; i++) {
			if (dados->ptr_memoria->clientes[i].individual == false) {
				dados->comp.nJogadores++;
			}
		}
		if (dados->comp.nJogadores == 2) {
			HANDLE hThread = CreateThread(NULL, 0, moveAguaCompeticao, (LPVOID)dados, 0, NULL);
		}
		else {
			for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
				if (dados->ptr_memoria->clientes[i].individual == false) {
					_tcscpy_s(c.mensagem, 20, TEXT("ESPERAR"));
					writeClienteASINC(dados->ptr_memoria->clientes[i].hPipe, c);
				}
			}
		}
	} while (!dados->ptr_memoria->terminar);
}

DWORD WINAPI ClienteThread(LPVOID param) {
	TDados* dados = (TDados*)param;
	Cliente recebido, enviado;
	DWORD cbBytesRead = 0, cbReplyBytes = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = dados->serverPipe;
	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };
	DWORD result = GetCurrentThreadId();

	Mapa novoMapa;
	novoMapa.lin = dados->lin;
	novoMapa.col = dados->col;

	ReadReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	int nCliente = adicionaCliente(dados, hPipe);

	dados->ptr_memoria->clientes[nCliente].mapa = criaMapa(novoMapa); // Criar um novo mapa para um cliente novo

	enviado.hPipe = hPipe;
	enviado.termina = false;
	enviado.aleatorio = dados->jogo.aleatorio;
	enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
	enviado.nivel = 1;
	_tcscpy_s(enviado.mensagem, 20, TEXT("CONECTADO"));
	HANDLE aguaThread = NULL;
	SetEvent(dados->event_atualiza); // atualizar o monitor
	writeClienteASINC(hPipe, enviado);
	while (!dados->ptr_memoria->terminar) {
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		ResetEvent(ReadReady);
		OverlRd.hEvent = ReadReady;

		fSuccess = ReadFile(hPipe, &recebido, Cl_Sz, &cbBytesRead, &OverlRd);

		result = WaitForSingleObject(ReadReady, HEART_BEAT);
		if (result == WAIT_TIMEOUT) {
			_tcscpy_s(enviado.mensagem, 20, TEXT("TIMEOUT"));
			enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
			enviado.agua = dados->ptr_memoria->clientes[nCliente].agua;
			enviado.termina = true;
			dados->ptr_memoria->clientes[nCliente].termina = true;
			writeClienteASINC(hPipe, enviado);
			removeCliente(dados, hPipe);
			break;
		}
		_tprintf(TEXT("Recebida mensagem do cliente %s: %s pelo pipe %d\n"), recebido.nome, recebido.mensagem, dados->ptr_memoria->clientes[nCliente].hPipe);
		recebido.hPipe = hPipe;

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE);
		if (recebido.termina) {
			dados->ptr_memoria->clientes[nCliente].termina = true;
			removeCliente(dados, hPipe);
			break;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("NOME")) == 0){
			registaCliente(dados, recebido);
		}
		if (_tcsicmp(recebido.mensagem, TEXT("REGISTO")) == 0) {
			if (recebido.individual) {
				dados->eu.hPipe = hPipe;
				if (aguaThread == NULL) {
					_tprintf(TEXT("Jogo individual a começar!\n"));
					dados->ptr_memoria->clientes[nCliente].mapa = criaMapa(novoMapa);
					aguaThread = CreateThread(NULL, 0, moveAgua, (LPVOID)dados, 0, NULL);
				}
				else {
					resetCliente(dados, hPipe);
					_tprintf(TEXT("A começar um novo jogo..."));
					dados->ptr_memoria->clientes[nCliente].termina = true;
					WaitForSingleObject(aguaThread, INFINITE);
					dados->ptr_memoria->clientes[nCliente].termina = false;
					aguaThread = CreateThread(NULL, 0, moveAgua, (LPVOID)dados, 0, NULL);
				}
			}
			else {
				dados->ptr_memoria->clientes[nCliente].individual = false;
				SetEvent(dados->comp.event_comp);
			}
			continue;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("JOGADA")) == 0) {
			dados->ptr_memoria->clientes[nCliente].mapa = jogaPeca(dados->ptr_memoria->clientes[nCliente].mapa, recebido.x, recebido.y, recebido.peca);
			_tcscpy_s(enviado.mensagem, 20, TEXT("JOGADA"));
			enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
			enviado.agua = dados->ptr_memoria->clientes[nCliente].agua;
			SetEvent(dados->event_atualiza);
			writeClienteASINC(hPipe, enviado);
			continue;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("MUDAR")) == 0) {
			dados->ptr_memoria->clientes[nCliente].mapa = jogaPeca(dados->ptr_memoria->clientes[nCliente].mapa, recebido.x, recebido.y, recebido.peca);
			_tcscpy_s(enviado.mensagem, 20, TEXT("MUDAR"));
			enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
			enviado.agua = dados->ptr_memoria->clientes[nCliente].agua;
			SetEvent(dados->event_atualiza);
			writeClienteASINC(hPipe, enviado);
			continue;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("APAGAR")) == 0) {
			dados->ptr_memoria->clientes[nCliente].mapa = jogaPeca(dados->ptr_memoria->clientes[nCliente].mapa, recebido.x, recebido.y, recebido.peca);
			_tcscpy_s(enviado.mensagem, 20, TEXT("APAGAR"));
			enviado.mapa = dados->ptr_memoria->clientes[nCliente].mapa;
			enviado.agua = dados->ptr_memoria->clientes[nCliente].agua;
			writeClienteASINC(hPipe, enviado);
			continue;
		}
		if (_tcsicmp(recebido.mensagem, TEXT("SUSPENDER")) == 0) {
			dados->eu = recebido;
			dados->eu.hPipe = hPipe;
			HANDLE hThread;
			hThread = CreateThread(NULL, 0, suspendeAguaCliente, &dados->ptr_memoria->clientes[nCliente], 0, NULL);
		}
		if (_tcsicmp(recebido.mensagem, TEXT("RATO")) == 0) {
			SetEvent(dados->ptr_memoria->clientes[nCliente].event_rato);
		}
		if (_tcsicmp(recebido.mensagem, TEXT("CONTINUAR")) == 0) {
			dados->ptr_memoria->clientes[nCliente].mapa = criaMapa(dados->ptr_memoria->clientes[nCliente].mapa);
			if (dados->ptr_memoria->clientes[nCliente].nivel < 10) {
				dados->ptr_memoria->clientes[nCliente].nivel++; // Até nível 10
			}
		}
	}
	_tprintf(TEXT("Cliente desligado! Número de clientes em execução: %d\n"), dados->ptr_memoria->nClientes);
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

	while (!dados->ptr_memoria->terminar) {
		hPipe = CreateNamedPipe(PIPE_SERVER, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 5000, NULL);

		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		dados->serverPipe = hPipe;
		if (fConnected) {
			_tprintf(TEXT("A lançar thread para comunicar com novo cliente...\n"));
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

	mapa = criaMapa(mapa); 
	//printMapa(mapa);

	/* Criação do mapa com um caminho definido */

	//mapa = criaMapaDebug(mapa);
	//printMapa(mapa);

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
	dados.ptr_memoria->clientes[0].mutexAgua = CreateMutex(NULL, FALSE, TEXT("CLIENTE_1"));
	dados.ptr_memoria->clientes[1].mutexAgua = CreateMutex(NULL, FALSE, TEXT("CLIENTE_2"));
	dados.ptr_memoria->clientes[0].event_rato = CreateEvent(NULL, FALSE, FALSE, TEXT("EVENT_RATO_CLIENTE_1"));
	dados.ptr_memoria->clientes[1].event_rato = CreateEvent(NULL, FALSE, FALSE, TEXT("EVENT_RATO_CLIENTE_2"));

	HANDLE hThread[4];
	/* Thread para receber comandos do Monitor */
	hThread[0] = CreateThread(NULL, 0, recebeComandos, &dados, 0, NULL);
	
	/* Thread para receber comandos do utilizador */
	hThread[1] = CreateThread(NULL, 0, recebeInput, &dados, 0, NULL);

	/* Thread para receber clientes */
	hThread[2] = CreateThread(NULL, 0, recebeClientes, &dados, 0, NULL);

	/* Thread para poder realizar competições */
	hThread[3] = CreateThread(NULL, 0, competicaoThread, &dados, 0, NULL);

	/* Esperar que uma das threads termine para terminar o processo */
	WaitForMultipleObjects(4, hThread, FALSE, INFINITE);

	// Terminar todos os processos que estejam à espera
	dados.ptr_memoria->terminar = true;

	// Fechar o ficheiro de memória partilhada
	UnmapViewOfFile(dados.ptr_memoria);
	UnmapViewOfFile(dados.ptr_modelo);
	
	return 0;
}
