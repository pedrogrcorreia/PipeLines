#pragma once
#include <stdbool.h>
#include "mapa.h"

#define SEMAFORO_EXECUCAO TEXT("SEMAFORO_EXECUCOES")
#define SEMAFORO_ITENS TEXT("SEMAFORO_ITENS")
#define SEMAFORO_VAZIOS TEXT("SEMAFORO_VAZIOS")
#define SEM_MUTEX_P TEXT("SEM_MUTEX_P")
#define SEM_MUTEX_C TEXT("SEM_MUTEX_C")
#define MEMORIA TEXT("Memoria partilhada")
#define MODELO TEXT("Modelo")

#define MUTEX_AGUA TEXT("MUTEX_AGUA")
#define EVENT_ATUALIZAR TEXT("EVENT_ATUALIZAR")

#define BUFFER 100

typedef struct {
	int x;
	int y;
} Barreira;

typedef struct { 
	bool atualizar;
	int agua;
	bool aleatorio;
	bool insereBarreira;
	Barreira barreira;
} Jogo;

typedef struct {
	int ent;
	int sai;
	Jogo jogosBuffer[BUFFER];
} Modelo;

typedef struct {
	Mapa mapas[2];
	bool terminar;
	int lin;
	int col;
} Memoria;

typedef struct {
	Memoria* ptr_memoria;
	Modelo* ptr_modelo;
	HANDLE sem_itens;
	HANDLE sem_vazios;
	HANDLE sem_mutex_p;
	HANDLE sem_mutex_c;
	HANDLE mutex_agua;
	HANDLE event_atualiza;
	int tempo;
	int lin;
	int col;
	Jogo jogo;
} TDados;