#pragma once

#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include "../util.h"

void iniciaClientes(TDados* dados);

//int writeClienteASINC(HANDLE hPipe, Cliente c);

void adicionaCliente(TDados* dados, HANDLE hPipe);

void registaCliente(TDados* dados, Cliente c);

void removeCliente(TDados* dados, HANDLE hPipe);