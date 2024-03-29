#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include "../util.h"
//#include "../Mapa.h"

#define MAX 256

/* Thread que coloca no Modelo Produtor - Consumidor os
Comandos enviados pelo utilizador */
DWORD WINAPI enviaComandos(LPVOID param) {
	TCHAR cmd[BUFFER_CHAR];
	Jogo jogo;
	TDados* dados = (TDados*)param;
	do {
		/* Controlo dos comandos do utilizador */
		_tprintf(TEXT("Introduza a op��o que pretende executar: \n"));
		_tprintf(TEXT("1. Parar a �gua\n2. Colocar barreira\n3. Ativar/Desativar pe�as aleat�rias\n"));
		_tprintf(TEXT("\nOp��o: "));

		// Obter op��o
		_fgetts(cmd, BUFFER_CHAR, stdin);
		
		// Retirar /n
		if (_tcsicmp(cmd, TEXT("\n")) != 0) {
			cmd[_tcslen(cmd) - 1] = '\0';
		}
		else {
			continue;
		}
		jogo.agua = 0;
		jogo.aleatorio = false;
		jogo.insereBarreira = false;
		int cmdOpt = _tstoi(cmd);
		switch (cmdOpt) {
			case 1:
				_tprintf(TEXT("Introduza o tempo que pretende parar a �gua.\n"));
				_tprintf(TEXT("\nTempo: "));
				_fgetts(cmd, BUFFER_CHAR, stdin);
				// Retirar /n
				if (_tcsicmp(cmd, TEXT("\n")) != 0) {
					cmd[_tcslen(cmd) - 1] = '\0';
				}
				else {
					continue;
				}
				jogo.agua = _tstoi(cmd);
				break;
			case 2:
				jogo.insereBarreira = true;
				_tprintf(TEXT("Introduza a linha onde quer colocar a barreira.\n"));
				_tprintf(TEXT("\nLinha: "));
				_fgetts(cmd, BUFFER_CHAR, stdin);
				// Retirar /n
				if (_tcsicmp(cmd, TEXT("\n")) != 0) {
					cmd[_tcslen(cmd) - 1] = '\0';
				}
				else {
					continue;
				}
				jogo.barreira.x = _tstoi(cmd);

				_tprintf(TEXT("Introduza a coluna onde quer colocar a barreira.\n"));
				_tprintf(TEXT("\nColuna: "));
				_fgetts(cmd, BUFFER_CHAR, stdin);
				// Retirar /n
				if (_tcsicmp(cmd, TEXT("\n")) != 0) {
					cmd[_tcslen(cmd) - 1] = '\0';
				}
				else {
					continue;
				}
				jogo.barreira.y = _tstoi(cmd);
				break;
			case 3:
				jogo.aleatorio = true;
				break;
			default:
				continue;
				break;
		}

		// Esperar pelo sem�foro dos vazios
		WaitForSingleObject(dados->sem_vazios, INFINITE);
		// Esperar pelo sem�foro do produtor
		WaitForSingleObject(dados->mutex_cp, INFINITE);
		// Copiar para a mem�ria do modelo o item a enviar
		CopyMemory(&dados->ptr_modelo->jogosBuffer[dados->ptr_modelo->sai], &jogo, sizeof(Jogo));
		// Decrementar a posi��o de escrita
		dados->ptr_modelo->sai = (dados->ptr_modelo->sai + 1) % BUFFER;
		// Assinalar o sem�foro do produtor
		ReleaseSemaphore(dados->mutex_cp, 1, NULL);
		// Assinalar o sem�foro dos itens
		ReleaseSemaphore(dados->sem_itens, 1, NULL);

	} while (_tcsicmp(cmd, TEXT("fim\n")) != 0);
}


/* Thread que fica a aguardar o evento para atualizar o mapa */

DWORD WINAPI atualizar(LPVOID param) {
	TDados* dados = (TDados*)param;
	do {
		WaitForSingleObject(dados->event_atualiza, INFINITE);
		for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
			printMapa(dados->ptr_memoria->clientes[i].mapa);
			printMapa(dados->ptr_memoria->clientes[i].agua);
		}
		/*printMapa(dados->ptr_memoria->mapas[0]);
		printMapa(dados->ptr_memoria->agua);*/
		_tprintf(TEXT("\n"));
	} while (!dados->ptr_memoria->terminar);
}

/* Thread para concluir o processo caso o servidor termine */

DWORD WINAPI terminar(LPVOID param) {
	TDados* dados = (TDados*)param;
	while (!dados->ptr_memoria->terminar) {
		continue;
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

	/* Verificar se existe algum Servidor a correr */
	semaforo_execucao = CreateSemaphore(NULL, 0, 1, SEMAFORO_EXECUCAO);
	result = GetLastError();
	if (result != ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("N�o existe nenhum servidor ativo. Tente novamente.\n"));
		return -1;
	}

	/* Cria mem�ria partilhada para ver as posi��es no mapa */
	HANDLE objMapMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMapMem == NULL) {
		_tprintf(TEXT("Imposs�vel criar mapeamento de mem�ria. A sair..\n"));
		return -1;
	}
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMapMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	/* Cria mem�ria partilhada para o Modelo Produtor - Consumidor */
	HANDLE objMapMod = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Modelo), MODELO);
	if (objMapMod == NULL) {
		_tprintf(TEXT("Imposs�vel criar mapeamento de mem�ria. A sair..\n"));
		return -1;
	}
	dados.ptr_modelo = (Modelo*)MapViewOfFile(objMapMod, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	/* Cria sem�foros para o modelo produtor consumidor */

	dados.sem_itens = CreateSemaphore(NULL, 0, BUFFER, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, BUFFER, BUFFER, SEMAFORO_VAZIOS);
	dados.mutex_cp = CreateSemaphore(NULL, 1, 1, MUTEX_CP);

	dados.event_atualiza = CreateEvent(NULL, FALSE, FALSE, EVENT_ATUALIZAR);

	Jogo jogo;
	Barreira b;

	for (int i = 0; i < dados.ptr_memoria->nClientes; i++) {
		printMapa(dados.ptr_memoria->clientes[i].mapa);
	}

	//printMapa(dados.ptr_memoria->mapas[0]);

	HANDLE hThread[3];

	/* Lan�a thread para o utilizador enviar comandos para o servidor */
	hThread[0] = CreateThread(NULL, 0, enviaComandos, &dados, 0, NULL);

	/* Lan�a thread para atualizar o mapa */
	hThread[1] = CreateThread(NULL, 0, atualizar, &dados, 0, NULL);

	/* Lan�a thread para terminar o processo */
	hThread[2] = CreateThread(NULL, 0, terminar, &dados, 0, NULL);

	/* Esperar que apenas uma das threads termine para terminar o processo */
	WaitForMultipleObjects(3, hThread, FALSE, INFINITE);

	// Fechar o ficheiro de mem�ria partilhada
	UnmapViewOfFile(dados.ptr_memoria);
	UnmapViewOfFile(dados.ptr_modelo);
}
