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

void printMapa(Mapa mapa) {
	for (int i = 0; i < mapa.lin; i++) {
		for (int j = 0; j < mapa.col; j++) {
			_tprintf(TEXT("%c"), mapa.board[i][j]);
		}
		_tprintf(TEXT("\n"));
	}
}