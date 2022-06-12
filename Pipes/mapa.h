#ifndef MAPA_H
#define MAPA_H

#pragma once
#define MAX_LENGTH 20

static TCHAR pecasText[2][4] = {
	{TEXT('┃'), TEXT('━'), TEXT('┛'), TEXT('i')},
	{TEXT('┗'), TEXT('┏'), TEXT('┓'), TEXT('f')}
};

// Estrutura que representa o mapa do jogo
typedef struct {
	int jogador;
	int lin;
	int col;
	TCHAR board[MAX_LENGTH][MAX_LENGTH];
} Mapa;

// Estrutura auxiliar para calcular o movimento da água
typedef struct {
	int prox_lin;
	int prox_col;
	Mapa mapa;
} Agua;

// Criação de um mapa com posições iniciais random
Mapa criaMapa(Mapa mapa);

// Criação de um mapa de Debug com posições e caminho definidos
Mapa criaMapaDebug(Mapa mapa);

// Imprimir um mapa
void printMapa(Mapa mapa);

// Movimentar a água pelos tubos
Agua moverAgua(Agua agua, int lin, int col);

Mapa jogaPeca(Mapa mapa, int lin, int col, TCHAR peca);

TCHAR getProxPeca(TCHAR piece);

TCHAR getRandomPeca();

#endif