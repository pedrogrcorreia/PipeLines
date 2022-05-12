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
			mapa.board[i][j] = 'a';
		}
	}

	srand((unsigned int)time(NULL));
	int ini = (rand() % (mapa.lin - 0)) + 0;
	int fin = (rand() % (mapa.lin - 0)) + 0;
	//_tprintf(TEXT("%d %d\n"), ini, fin);

	///* Coluna 0 e Linha random*/
	mapa.board[ini][0] = 'i';

	///* Ultima coluna e Linha random */

	/* TODO DIAGONALMENTE OPOSTOS */
	mapa.board[fin][mapa.col - 1] = 'f';

	return mapa;
}

Mapa criaMapaDebug(Mapa mapa) {
	for (int i = 0; i < mapa.lin; i++) {
		for (int j = 0; j < mapa.col; j++) {
			mapa.board[i][j] = TEXT('□');
		}
	}
	mapa.board[0][0] = TEXT('━');
	mapa.board[mapa.lin - 1][mapa.col - 1] = TEXT('┃');

	for (int i = 1; i < mapa.col - 1; i++) {
		mapa.board[0][i] = TEXT('━');
	}

	mapa.board[0][mapa.col - 1] = TEXT('┓');

	for (int i = 1; i < mapa.lin - 1; i++) {
		mapa.board[i][mapa.col - 1] = TEXT('┃');
	}

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

	//_tprintf(TEXT("%c"), agua.mapa.board[lin][col]);
	//_tprintf(TEXT("%d\n"), _tcsicmp(agua.mapa.board[lin][col], TEXT('-')));
	//agua.prox_lin = lin;
	//agua.prox_col = col + 1;

	//_tprintf(TEXT("%c"), agua.mapa.board[lin][col] == '━');

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
	//if (_tcsicmp(TEXT('━'), TEXT('━')) == 0) {
	//	_tprintf(TEXT("CERTO"));
	//	agua.mapa.board[lin][col] = 'w';
	//	if (_tcsicmp(TEXT('━'), TEXT('□')) == 0) {
	//		// Perdeu
	//		_tprintf(TEXT("Perdeu"));
	//	}
	//	else {
	//		agua.prox_lin = lin;
	//		agua.prox_col = col + 1;
	//		return agua;
	//	}
	//}
	//return agua;
}