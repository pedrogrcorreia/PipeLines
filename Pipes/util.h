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

#define BUFFER 100
#define BUFFER_CHAR 100

// Estrutura que representa uma barreira e a sua posi��o
typedef struct {
	int x;
	int y;
} Barreira;

// Estrutura que representa o estado do jogo
typedef struct { 
	bool atualizar;
	int agua;
	bool aleatorio;
	bool insereBarreira;
	Barreira barreira;
} Jogo;

// Estrutura para utilizar no modelo produtor consumidor como
// Mem�ria partilhada
typedef struct {
	int ent;
	int sai;
	Jogo jogosBuffer[BUFFER];
} Modelo;

// Estrutura para colocar na mem�ria partilhada
typedef struct {
	Mapa mapas[2];
	bool terminar;
	int lin;
	int col;
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
	int tempo;
	int lin;
	int col;
	Jogo jogo;
} TDados;