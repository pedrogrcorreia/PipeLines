#pragma once
#define MAX_LENGTH 20

typedef struct {
	int jogador;
	int lin;
	int col;
	char board[MAX_LENGTH][MAX_LENGTH];
} Mapa;

Mapa criaMapa(Mapa mapa);

void printMapa(Mapa mapa);