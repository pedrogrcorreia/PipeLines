#pragma once
#include <stdbool.h>
#include "mapa.h"

#define SEMAFORO_EXECUCAO TEXT("SEMAFORO_EXECUCOES")
#define SEMAFORO_ITENS TEXT("SEMAFORO_ITENS")
#define SEMAFORO_VAZIOS TEXT("SEMAFORO_VAZIOS")
#define MUTEX_CP TEXT("MUTEX_CP")
#define MEMORIA TEXT("Memoria partilhada")
#define MODELO TEXT("Modelo")

#define MUTEX_AGUA TEXT("MUTEX_AGUA")
#define EVENT_ATUALIZAR TEXT("EVENT_ATUALIZAR")

#define PIPE_SERVER TEXT("\\\\.\\pipe\\server")

#define BUFFER 100
#define BUFFER_CHAR 100
#define MAX_LETTERS 20
#define MAX_CLI 2

// Estrutura que representa uma barreira e a sua posição
typedef struct {
	int x;
	int y;
} Barreira;

typedef struct {
	int lin;
	int col;
} Jogada;

// Estrutura que representa o estado do jogo
typedef struct { 
	bool atualizar;
	int agua;
	bool aleatorio;
	bool insereBarreira;
	Barreira barreira;
} Jogo;

typedef struct {
	int x;
	int y;
	int square;
	int ajuda;
	TCHAR peca;
	Mapa mapa;
	Mapa agua;
	bool aleatorio;
	bool individual;
	TCHAR nome[MAX_LETTERS];
	TCHAR mensagem[MAX_LETTERS];
	bool termina;
	HANDLE hPipe;
	HANDLE event;
	HANDLE mutexAgua;
	HANDLE event_rato;
	bool moveRato;
} Cliente;

// Estrutura para utilizar no modelo produtor consumidor como
// Memória partilhada
typedef struct {
	int ent;
	int sai;
	Jogo jogosBuffer[BUFFER];
} Modelo;

// Estrutura para colocar na memória partilhada
typedef struct {
	//Mapa mapas[2];
	//Mapa agua;
	Cliente clientes[MAX_CLI];
	bool terminar;
	int lin;
	int col;
	int nClientes;
} Memoria;

// Estrutura auxiliar para enviar os dados para as Threads
typedef struct {
	Memoria* ptr_memoria;
	Modelo* ptr_modelo;
	HANDLE sem_itens;
	HANDLE sem_vazios;
	HANDLE mutex_cp;
	HANDLE mutex_agua;
	HANDLE event_atualiza;
	HANDLE serverPipe;
	HWND hWnd;
	Cliente eu;
	int tempo;
	int lin;
	int col;
	Jogo jogo;
	//Cliente clientes[MAX_CLI];
} TDados;