#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include "../util.h"
#include "../Mapa.h"

#define MAX 256

/* Thread que coloca no Modelo Produtor - Consumidor os
Comandos enviados pelo utilizador */
DWORD WINAPI enviaComandos(LPVOID param) {
	TCHAR cmd[250];
	Jogo jogo;
	TDados* dados = (TDados*)param;
	do {

		/* Controlo dos comandos do utilizador */
		_fgetts(cmd, 250, stdin);
		if (_tcsicmp(cmd, TEXT("stop\n")) == 0) {
			jogo.agua = 10;
		}

		//// Esperar pelo semáforo dos vazios
		//WaitForSingleObject(dados->sem_vazios, INFINITE);
		//// Copiar para a memória do modelo o item a enviar
		//CopyMemory(&dados->ptr_modelo->jogosBuffer[dados->ptr_modelo->sai], &jogo, sizeof(Jogo));
		//// Decrementar a posição de escrita
		//dados->ptr_modelo->sai = (dados->ptr_modelo->sai + 1) % BUFFER;
		//// Assinalar o semáforo dos itens
		//ReleaseSemaphore(dados->sem_itens, 1, NULL);

	} while (_tcsicmp(cmd, TEXT("fim\n")) != 0);
}

DWORD WINAPI recebeComandos(LPVOID param) {
	TDados* dados = (TDados*)param;
	do {
		// Esperar semáforo dos itens
		WaitForSingleObject(dados->sem_itens, INFINITE);
		// Copiar para a memória do processo o item consumido
		CopyMemory(&dados->jogo, &dados->ptr_modelo->jogosBuffer[dados->ptr_modelo->ent], sizeof(Jogo));
		// Incrementa a posição de leitura
		dados->ptr_modelo->ent = (dados->ptr_modelo->ent + 1) % BUFFER;
		_tprintf(TEXT("%d\n"), dados->jogo.atualizar);
		/* Processar o que recebeu*/
		if (dados->jogo.atualizar == true) {
			_tprintf(TEXT("A água mexeu-se...\n"));
			printMapa(dados->ptr_memoria->mapas[0]);
			_tprintf(TEXT("\n"));
		}
		ReleaseSemaphore(dados->sem_vazios, 1, NULL);

	} while (!dados->ptr_memoria->terminar);
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
		_tprintf(TEXT("Não existe nenhum servidor ativo. Tente novamente.\n"));
		return -1;
	}

	/* Cria memória partilhada para ver as posições no mapa */
	HANDLE objMapMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMapMem == NULL) {
		_tprintf(TEXT("Impossível criar mapeamento de memória. A sair..\n"));
		return -1;
	}
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMapMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	/* Cria memória partilhada para o Modelo Produtor - Consumidor */
	HANDLE objMapMod = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Modelo), MODELO);
	if (objMapMod == NULL) {
		_tprintf(TEXT("Impossível criar mapeamento de memória. A sair..\n"));
		return -1;
	}
	dados.ptr_modelo = (Modelo*)MapViewOfFile(objMapMod, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	/* Cria semáforos para o modelo produtor consumidor */

	dados.sem_itens = CreateSemaphore(NULL, 0, BUFFER, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, BUFFER, BUFFER, SEMAFORO_VAZIOS);

	Jogo jogo;

	printMapa(dados.ptr_memoria->mapas[0]);

	HANDLE hThread[2];

	/* Lança thread para o utilizador enviar comandos para o servidor */
	hThread[0] = CreateThread(NULL, 0, enviaComandos, &dados, 0, NULL);

	hThread[1] = CreateThread(NULL, 0, recebeComandos, &dados, 0, NULL);

	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	/* Thread principal fica responsável por consumir os itens do Modelo */
	//do {
	//	// Esperar semáforo dos itens
	//	WaitForSingleObject(dados.sem_itens, INFINITE);
	//	// Copiar para a memória do processo o item consumido
	//	CopyMemory(&jogo, &dados.ptr_modelo->jogosBuffer[dados.ptr_modelo->ent], sizeof(Jogo));
	//	// Incrementa a posição de leitura
	//	dados.ptr_modelo->ent = (dados.ptr_modelo->ent + 1) % BUFFER;
	//	_tprintf(TEXT("%d\n"), jogo.atualizar);
	//	/* Processar o que recebeu*/
	//	if (jogo.atualizar == true) {
	//		_tprintf(TEXT("A água mexeu-se...\n"));
	//		printMapa(dados.ptr_memoria->mapas[0]);
	//		_tprintf(TEXT("\n"));
	//	}
	//	ReleaseSemaphore(dados.sem_vazios, 1, NULL);

	//} while (!dados.ptr_memoria->terminar);
}
