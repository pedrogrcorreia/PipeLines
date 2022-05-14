#pragma once
#define MAX_LENGTH 20

// Estrutura que representa o mapa do jogo
typedef struct {
	int jogador;
	int lin;
	int col;
	TCHAR board[MAX_LENGTH][MAX_LENGTH];
} Mapa;

// Estrutura auxiliar para calcular o movimento da �gua
typedef struct {
	int prox_lin;
	int prox_col;
	Mapa mapa;
} Agua;

// Cria��o de um mapa com posi��es iniciais random
Mapa criaMapa(Mapa mapa);

// Cria��o de um mapa de Debug com posi��es e caminho definidos
Mapa criaMapaDebug(Mapa mapa);

// Imprimir um mapa
void printMapa(Mapa mapa);

// Movimentar a �gua pelos tubos
Agua moverAgua(Agua agua, int lin, int col);