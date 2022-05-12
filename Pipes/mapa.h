#pragma once
#define MAX_LENGTH 20


typedef struct {
	int jogador;
	int lin;
	int col;
	TCHAR board[MAX_LENGTH][MAX_LENGTH];
} Mapa;

typedef struct {
	int prox_lin;
	int prox_col;
	Mapa mapa;
} Agua;

Mapa criaMapa(Mapa mapa);

Mapa criaMapaDebug(Mapa mapa);

void printMapa(Mapa mapa);

Agua moverAgua(Agua agua, int lin, int col);