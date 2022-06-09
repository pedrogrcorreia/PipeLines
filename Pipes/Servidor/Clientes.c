#pragma once
#include "Clientes.h"

#define MAX 256
#define BUFSIZE 2048
#define Cl_Sz sizeof(Cliente)
HANDLE WriteReady;

void iniciaClientes(TDados* dados) {
	for (int i = 0; i < MAX_CLI; i++) {
		dados->ptr_memoria->clientes[i].hPipe = NULL;
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

void adicionaCliente(TDados* dados, HANDLE hPipe) {
	for (int i = 0; i < MAX_CLI; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == NULL) {
			dados->ptr_memoria->clientes[i].hPipe = hPipe;
			dados->ptr_memoria->nClientes++;
			_tprintf(TEXT("Cliente adicionado com sucesso! Na posição %d.\n"), i);
			return;
		}
	}
}

void registaCliente(TDados* dados, Cliente c) {
	for (int i = 0; i < MAX_CLI; i++) {
		if (dados->ptr_memoria->clientes[i].hPipe == c.hPipe) {
			_tprintf(TEXT("A escrever para o cliente\n"));
			_tcscpy_s(dados->ptr_memoria->clientes[i].nome, BUFFER, c.nome);
			_tcscpy_s(dados->ptr_memoria->clientes[i].mensagem, BUFFER, TEXT("Cliente registado"));
			dados->ptr_memoria->clientes[i].x = c.x;
			dados->ptr_memoria->clientes[i].y = c.y;
			//writeClienteASINC(dados->clientes[i].hPipe, dados->clientes[i]);
			return;
		}
		_tprintf(TEXT("Não foi possível encontrar o cliente!\n"));
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