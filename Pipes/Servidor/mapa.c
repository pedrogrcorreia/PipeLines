#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "../Mapa.h"

Mapa criaMapa(Mapa mapa) {
	for (int i = 0; i < mapa.lin; i++) {
		for (int j = 0; j < mapa.col; j++) {
			mapa.board[i][j] = TEXT('□');
		}
	}

	srand((unsigned int)time(NULL));
	int ini = (rand() % ((mapa.lin-1) - 0)) + 0;
	int fin = (rand() % ((mapa.lin-1) - 1 + 1)) + 1;
	/* Coluna 0 e Linha random*/
	mapa.board[ini][0] = TEXT('i');

	/* Ultima coluna e Linha random */

	/* TODO DIAGONALMENTE OPOSTOS */
	mapa.board[fin][mapa.col - 1] = TEXT('f');

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

Agua moverAguaDebug(Agua agua, int lin, int col) {
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


Agua moverAgua(Agua agua, int lin, int col) {
	_tprintf(TEXT("%d %d\n"), lin, col);
	agua.atual_lin = lin;
	agua.atual_col = col;
	//Peça f - acaba o jogo
	if (agua.mapa.board[lin][col] == TEXT('f')) {
		agua.mapa.board[lin][col] = TEXT('w');
		agua.ganhou = true;
		agua.prox_lin = lin + 1;
		agua.prox_col = col;
		return agua;
	}

	// Peça i - Verifica em cima
	if (agua.mapa.board[lin][col] == TEXT('i')) {
		agua.mapa.board[lin][col] = TEXT('w');
		if (agua.mapa.board[lin][col + 1] == TEXT('□') || agua.mapa.board[lin][col + 1] == pecasText[0][0] ||
			agua.mapa.board[lin][col + 1] == pecasText[1][0] || agua.mapa.board[lin][col + 1] == pecasText[1][1]) {
			agua.perdeu = true;
			return agua;
		}
		else {
			agua.prox_lin = lin;
			agua.prox_col = col + 1;
			return agua;
		}
	}


	if (agua.mapa.board[lin][col] == TEXT('━')) {
		agua.mapa.board[lin][col] = TEXT('w');

		// Verifica da direita para a esquerda
		if (agua.mapa.board[lin][col + 1] == TEXT('w')) {
			if (agua.mapa.board[lin][col - 1] == TEXT('□') || agua.mapa.board[lin][col - 1] == pecasText[0][0] || 
				agua.mapa.board[lin][col - 1] == pecasText[0][2] || agua.mapa.board[lin][col - 1] == pecasText[1][2]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin;
				agua.prox_col = col - 1;
				return agua;
			}
		}
		// Verifica da esquerda para a direita
		else {
			if (agua.mapa.board[lin][col +1] == TEXT('□') || agua.mapa.board[lin][col +1] == pecasText[0][0] ||
				agua.mapa.board[lin][col +1] == pecasText[1][0] || agua.mapa.board[lin][col +1] == pecasText[1][1]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin;
				agua.prox_col = col + 1;
				return agua;
			}
		}
	}

	if (agua.mapa.board[lin][col] == TEXT('┃')) {
		agua.mapa.board[lin][col] = TEXT('w');

		// Verifica de baixo para cima
		if (agua.mapa.board[lin+1][col] == TEXT('w')) {
			if (agua.mapa.board[lin - 1][col] == TEXT('□') || agua.mapa.board[lin - 1][col] == pecasText[0][1] ||
				agua.mapa.board[lin - 1][col] == pecasText[0][2] || agua.mapa.board[lin - 1][col] == pecasText[1][0]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin - 1;
				agua.prox_col = col;
				return agua;
			}
		}
		// Verifica de cima para baixo
		else {
			if (agua.mapa.board[lin + 1][col] == TEXT('□') || agua.mapa.board[lin + 1][col] == pecasText[0][1] ||
				agua.mapa.board[lin + 1][col] == pecasText[1][1] || agua.mapa.board[lin + 1][col] == pecasText[1][2]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin + 1;
				agua.prox_col = col;
				return agua;
			}
		}
	}


	if (agua.mapa.board[lin][col] == TEXT('┏')) {
		agua.mapa.board[lin][col] = TEXT('w');

		// Verifica de cima para baixo
		if (agua.mapa.board[lin][col - 1] == TEXT('w')) {
			if (agua.mapa.board[lin + 1][col] == TEXT('□') || agua.mapa.board[lin + 1][col] == pecasText[0][1] ||
				agua.mapa.board[lin + 1][col] == pecasText[1][1] || agua.mapa.board[lin + 1][col] == pecasText[1][2]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin + 1;
				agua.prox_col = col;
				return agua;
			}
		}
		// Verifica de baixo para cima
		else {
			if (agua.mapa.board[lin][col + 1] == TEXT('□') || agua.mapa.board[lin][col + 1] == pecasText[0][0] ||
				agua.mapa.board[lin][col + 1] == pecasText[1][0] || agua.mapa.board[lin][col + 1] == pecasText[1][1]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin;
				agua.prox_col = col + 1;
				return agua;
			}
		}
	}

	
	if (agua.mapa.board[lin][col] == TEXT('┛')) {
		agua.mapa.board[lin][col] = TEXT('w');

		// Verifica de baixo para cima
		if (agua.mapa.board[lin][col - 1] == TEXT('w')) {
			if (agua.mapa.board[lin - 1][col] == TEXT('□') || agua.mapa.board[lin - 1][col] == pecasText[0][1] ||
				agua.mapa.board[lin - 1][col] == pecasText[0][2] || agua.mapa.board[lin - 1][col] == pecasText[1][0]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin - 1;
				agua.prox_col = col;
				return agua;
			}
		}
		// Verifica de cima para baixo
		else {
			if (agua.mapa.board[lin][col - 1] == TEXT('□') || agua.mapa.board[lin][col - 1] == pecasText[0][0] || 
				agua.mapa.board[lin][col - 1] == pecasText[0][2] || agua.mapa.board[lin][col - 1] == pecasText[1][2]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin;
				agua.prox_col = col - 1;
				return agua;
			}
		}
	}


	if (agua.mapa.board[lin][col] == TEXT('┓')) {
		agua.mapa.board[lin][col] = TEXT('w');
		// Verifica de cima para baixo
		if (agua.mapa.board[lin + 1][col] == TEXT('w')) {
			if (agua.mapa.board[lin][col - 1] == TEXT('□') || agua.mapa.board[lin][col - 1] == pecasText[0][0] || 
				agua.mapa.board[lin][col - 1] == pecasText[0][2] || agua.mapa.board[lin][col - 1] == pecasText[1][2]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin;
				agua.prox_col = col - 1;
				return agua;
			}
		}
		// Verifica de baixo para cima
		else {
			if (agua.mapa.board[lin + 1][col] == TEXT('□') || agua.mapa.board[lin + 1][col] == pecasText[0][1] ||
				agua.mapa.board[lin + 1][col] == pecasText[1][1] || agua.mapa.board[lin + 1][col] == pecasText[1][2]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin + 1;
				agua.prox_col = col;
				return agua;
			}
		}
	}

	if (agua.mapa.board[lin][col] == TEXT('┗')) {
		agua.mapa.board[lin][col] = TEXT('w');
		// Verifica de cima para baixo
		if (agua.mapa.board[lin][col + 1] == TEXT('w')) {
			if (agua.mapa.board[lin - 1][col] == TEXT('□') || agua.mapa.board[lin - 1][col] == pecasText[0][1] || 
				agua.mapa.board[lin - 1][col] == pecasText[0][2] || agua.mapa.board[lin - 1][col] == pecasText[1][0]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin - 1;
				agua.prox_col = col;
				return agua;
			}
		}
		// Verifica de baixo para cima
		else {
			if (agua.mapa.board[lin][col + 1] == TEXT('□') || agua.mapa.board[lin][col + 1] == pecasText[0][0] ||
				agua.mapa.board[lin][col + 1] == pecasText[1][0] || agua.mapa.board[lin][col + 1] == pecasText[1][1]) {
				agua.perdeu = true;
				return agua;
			}
			else {
				agua.prox_lin = lin;
				agua.prox_col = col + 1;
				return agua;
			}
		}
	}

	return agua;
}

Mapa jogaPeca(Mapa mapa, int lin, int col, TCHAR peca) {
	mapa.board[lin][col] = peca;
	return mapa;
}

Mapa atualizaAgua(Mapa mapa, Mapa agua) {
	for (int i = 0; i < mapa.lin; i++) {
		for (int j = 0; j < mapa.col; j++) {
			if (agua.board[i][j] == TEXT('w')) {
				continue;
			}
			else {
				agua.board[i][j] = mapa.board[i][j];
			}
		}
	}
	return agua;
}

TCHAR getProxPeca(TCHAR piece) {
	TCHAR* p;
	p = pecasText;

	for (int i = 0; i < 3; i++) {
		if (piece == *(p + i)) {
			if (i == 2) {
				return *(p + i + 2);
			}
			return *(p + i + 1);
		}
	}
	for (int i = 4; i < 7; i++) {
		if (piece == *(p + i)) {
			if (i == 6) {
				return *(p + 0);
			}
			return *(p + i + 1);
		}
	}
}

TCHAR getRandomPeca() {
	srand(time(NULL));
	int r = rand() % 6;
	TCHAR* p = pecasText;
	if (r == 3) {
		return *(p + r + 1); // para não ser a peça inicial
	}
	return *(p + r);
}
