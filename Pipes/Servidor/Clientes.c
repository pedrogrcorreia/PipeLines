#pragma once
#include "Clientes.h"

#define MAX 256
#define BUFSIZE 2048
#define Cl_Sz sizeof(Cliente)
HANDLE WriteReady;

void iniciaClientes(TDados* dados) {
	for (int i = 0; i < MAX_CLI; i++) {
		dados->ptr_memoria->clientes[i].hPipe = NULL;
		dados->ptr_memoria->clientes[i].individual = true;
	}
}

//int writeClienteASINC(HANDLE hPipe, Cliente c) {
//	DWORD cbWritten = 0;
//	BOOL fSuccess = FALSE;
//
//	OVERLAPPED OverlWr = { 0 };
//
//	ZeroMemory(&OverlWr, sizeof(OverlWr));
//
//	fSuccess = WriteFile(hPipe, &c, Cl_Sz, &cbWritten, &OverlWr);
//	WaitForSingleObject(WriteReady, INFINITE);
//	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
//	return 1;
//}

int adicionaCliente(TDados* dados, HANDLE hPipe) {
	for (int i = 0; i < MAX_CLI; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == NULL) {
			dados->ptr_memoria->clientes[i].hPipe = hPipe;
			dados->ptr_memoria->nClientes++;
			_tprintf(TEXT("Cliente adicionado com sucesso! Na posição %d.\n"), i);
			return i;
		}
	}
}

void registaCliente(TDados* dados, Cliente c) {
	for (int i = 0; i < MAX_CLI; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == c.hPipe) {
			_tprintf(TEXT("A escrever para o cliente\n"));
			_tcscpy_s(dados->ptr_memoria->clientes[i].nome, MAX_LETTERS, c.nome);
			dados->ptr_memoria->clientes[i].x = c.x;
			dados->ptr_memoria->clientes[i].y = c.y;
			dados->ptr_memoria->clientes[i].termina = false;
			dados->ptr_memoria->clientes[i].aleatorio = c.aleatorio;
			dados->ptr_memoria->clientes[i].nivel = c.nivel;
			return;
		}
		_tprintf(TEXT("Não foi possível encontrar o cliente!\n"));
	}
}

void printClientes(TDados* dados) {
	for (int i = 0; i < dados->ptr_memoria->nClientes; i++) {
		_tprintf(TEXT("Nome: %s\n"), dados->ptr_memoria->clientes[i].nome);
		_tprintf(TEXT("Nível: %d\n"), dados->ptr_memoria->clientes[i].nivel);
		_tprintf(TEXT("Modo de jogo: %d"), dados->ptr_memoria->clientes[i].individual);
		_tprintf(TEXT("Mapa atual:\n"));
		printMapa(dados->ptr_memoria->clientes[i].mapa);
	}
}

void removeCliente(TDados* dados, HANDLE hPipe) {
	for (int i = 0; i < MAX_CLI; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == hPipe) {
			dados->ptr_memoria->clientes[i].hPipe = NULL;
			dados->ptr_memoria->nClientes--;
			return;
		}
	}
}