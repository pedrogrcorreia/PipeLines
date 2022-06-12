#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include "../Mapa.h"

Mapa criaMapa(Mapa mapa) {
	for (int i = 0; i < mapa.lin; i++) {
		for (int j = 0; j < mapa.col; j++) {
			mapa.board[i][j] = TEXT('□');
		}
	}

	srand((unsigned int)time(NULL));
	int ini = (rand() % (mapa.lin - 0)) + 0;
	int fin = (rand() % (mapa.lin - 0)) + 0;
	//_tprintf(TEXT("%d %d\n"), ini, fin);

	/* Coluna 0 e Linha random*/
	mapa.board[ini][0] = TEXT('━');

	/* Ultima coluna e Linha random */

	/* TODO DIAGONALMENTE OPOSTOS */
	mapa.board[fin][mapa.col - 1] = TEXT('━');

	return mapa;
}

Mapa criaMapaDebug(Mapa mapa) {
	for (int i = 0; i < mapa.lin; i++) {
		for (int j = 0; j < mapa.col; j++) {
			mapa.board[i][j] = TEXT('□');
		}
	}
	mapa.board[0][0] = TEXT('i');
	//mapa.board[mapa.lin - 1][mapa.col - 1] = TEXT('┃');

	for (int i = 1; i < mapa.col - 1; i++) {
		mapa.board[0][i] = TEXT('━');
	}

	mapa.board[0][mapa.col - 1] = TEXT('┓');

	for (int i = 1; i < mapa.lin - 1; i++) {
		mapa.board[i][mapa.col - 1] = TEXT('┃');
	}

	mapa.board[mapa.lin-1][mapa.col-1] = TEXT('f');

	return mapa;
}

void printMapa(Mapa mapa) {
	for (int i = 0; i < mapa.lin; i++) {
		for (int j = 0; j < mapa.col; j++) {
			_tprintf(TEXT("%c"), mapa.board[i][j]);
		}
		_tprintf(TEXT("\n"));
	}
}

Agua moverAgua(Agua agua, int lin, int col) {
	// Coloca água na peça
	//agua.mapa.board[lin][col] = 'w';
	_tprintf(TEXT("%d %d\n"), lin, col);

	//Peça f - acaba o jogo
	if (agua.mapa.board[lin][col] == TEXT('f')) {
		agua.mapa.board[lin][col] = TEXT('w');
		// ganhou TO DO
		agua.prox_lin = lin + 1;
		agua.prox_col = col;
		return agua;
	}

	// Peça i - Verifica à frente
	if (agua.mapa.board[lin][col] == TEXT('i')) {
		agua.mapa.board[lin][col] = TEXT('w');
		if (agua.mapa.board[lin][col + 1] == TEXT('□')) {
			// perdeu
			return agua;
		}
		else {
			agua.prox_lin = lin;
			agua.prox_col = col + 1;
			return agua;
		}
	}
	// Peça ━ Verifica à frente
	if (agua.mapa.board[lin][col] == TEXT('━')) {
		agua.mapa.board[lin][col] = TEXT('w');
		if (agua.mapa.board[lin][col + 1] == TEXT('□')) {
			// perdeu
			return agua;
		}
		else {
			agua.prox_lin = lin;
			agua.prox_col = col + 1;
			return agua;
		}
	}
	if (agua.mapa.board[lin][col] == TEXT('┓')) {
		agua.mapa.board[lin][col] = TEXT('w');
		if (agua.mapa.board[lin + 1][col] == TEXT('□')) {
			// perdeu
			return agua;
		}
		else {
			agua.prox_lin = lin + 1;
			agua.prox_col = col;
			return agua;
		}
	}
	if (agua.mapa.board[lin][col] == TEXT('┃')) {
		agua.mapa.board[lin][col] = TEXT('w');
		if (agua.mapa.board[lin + 1][col] == TEXT('□')) {
			// perdeu
			return agua;
		}
		else {
			agua.prox_lin = lin + 1;
			agua.prox_col = col;
			return agua;
		}
	}
	return agua;
}

Mapa jogaPeca(Mapa mapa, int lin, int col, TCHAR peca) {
	mapa.board[lin][col] = peca;
	return mapa;
}

TCHAR getProxPeca(TCHAR piece) {
	TCHAR* p;
	p = pecasText;

	//_tprintf(TEXT("PECA %s\n"), piece);
	for (int i = 0; i < 6; i++) {
		if (piece == *(p+i)) {
			if (i <= 4) {

				return *(p+i+1);
			}
			else {
				return *(p+0);
			}
		}
	}
}

TCHAR getRandomPeca() {
	srand(time(NULL));   // Initialization, should only be called once.
	int r = rand() % 6;
	TCHAR* p = pecasText;
	return *(p + r);
}
