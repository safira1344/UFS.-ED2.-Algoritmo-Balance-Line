/*
 * balanceLine.h
 *
 *  Modified on: Jul, 2023
 *      Author: raphael oliveira
 */

#include <stdio.h> /* define FILE */
#include <stdlib.h>
#include <string.h>
#include <math.h>

int tamanho_bits_clientes()
{
	return (sizeof(int) + sizeof(char) * 40 + sizeof(char) * 12);
}

int tamanho_bits_transacoes()
{
	return (sizeof(int) + sizeof(char) + sizeof(char) * 40 + sizeof(char) * 40);
}

int total_clientes(FILE *fMestre)
{
	fseek(fMestre, 0, SEEK_END);
	int tam = trunc(ftell(fMestre) / tamanho_bits_clientes());
	return tam;
}

int total_transacoes(FILE *fTransacao)
{
	fseek(fTransacao, 0, SEEK_END);
	int tam = trunc(ftell(fTransacao) / tamanho_bits_transacoes());
	return tam;
}

int menorChave(TCliente *cliente, TTransacao *transacao)
{
	if (cliente->codCliente < transacao->codCliente)
	{
		return cliente->codCliente;
	}
	else
	{
		return transacao->codCliente;
	}
}

void preencherTransacoesRestantes(TTransacao *transacao, FILE *fNovoMestre, FILE *fTransacao)
{
	TCliente *clienteNovo = (TCliente *)malloc(sizeof(TCliente));

	while (transacao)
	{
		if (transacao->tipoTransacao == 'I')
		{
			clienteNovo->codCliente = transacao->codCliente;
			strcpy(clienteNovo->nome, transacao->valor01);
			strcpy(clienteNovo->dataNascimento, transacao->valor02);

			salvaCliente(clienteNovo, fNovoMestre);
		}
		transacao = leTransacao(fTransacao);
	}
}

void preencherClientesRestantes(TCliente *cliente, FILE *fNovoMestre, FILE *fMestre)
{
	while (cliente)
	{
		salvaCliente(cliente, fNovoMestre);
		cliente = leCliente(fMestre);
	}
}

void balanceLine(FILE *fMestre, FILE *fTransacao, FILE *fNovoMestre, FILE *fErro)
{
	printf("\n\n-------------BALANCELINE--------------\n\n");

	TCliente *cliente = (TCliente *)malloc(sizeof(TCliente));
	TCliente *clienteNovo = (TCliente *)malloc(sizeof(TCliente));
	TTransacao *transacao = (TTransacao *)malloc(sizeof(TTransacao));

	int erro = 0;
	int achouCliente = 0;
	int achouTransacao = 0;

	fseek(fMestre, 0, SEEK_SET);
	fseek(fTransacao, 0, SEEK_SET);
	fseek(fNovoMestre, 0, SEEK_SET);

	cliente = leCliente(fMestre);
	transacao = leTransacao(fTransacao);

	// senão tiver clientes no arquivo mestre vai preencher com as transacoes restantes no novo arquivo mestre
	// senão tiver transações, o novo arquivo mestre será preenchido com os clientes do arquivo mestre restantes
	if (cliente == NULL || transacao == NULL)
	{
		preencherClientesRestantes(cliente, fNovoMestre, fMestre);

		preencherTransacoesRestantes(transacao, fNovoMestre, fTransacao);

		return;
	}

	int chaveAtual = menorChave(cliente, transacao);

	while (1)
	{
		if (chaveAtual == cliente->codCliente)
			achouCliente = 1;

		if (chaveAtual == transacao->codCliente)
			achouTransacao = 1;

		// quando a chave do cliente é menor que a chave da transacao
		if (achouCliente && !achouTransacao)
		{
			salvaCliente(cliente, fNovoMestre);
			cliente = leCliente(fMestre);
		}

		// quando a chave da transacao é menor que a chave do cliente do arquivo mestre
		if (achouTransacao && !achouCliente)
		{
			// se tentar excluir ou modificar um cliente inexistente irá salvar a transação do registro no arquivo de erro
			if (transacao->tipoTransacao == 'E' || transacao->tipoTransacao == 'M')
			{
				salvaTransacao(transacao, fErro);
			}

			// se tentar incluir um cliente inexistente irá salvar o cliente no arquivo novo mestre
			else if (transacao->tipoTransacao == 'I')
			{
				clienteNovo->codCliente = transacao->codCliente;
				strcpy(clienteNovo->dataNascimento, transacao->valor02);
				strcpy(clienteNovo->nome, transacao->valor01);

				salvaCliente(clienteNovo, fNovoMestre);
			}
			transacao = leTransacao(fTransacao);
		}

		// quando a chave cliente e a chave da transacao são iguais
		if (achouCliente && achouTransacao)
		{
			// se a transacao for de exclusão, não salva o cliente no novo arquivo mestre
			if (transacao->tipoTransacao == 'E')
			{
				// passando para o próximo registro do arquivo mestre e transacao
				cliente = leCliente(fMestre);
				transacao = leTransacao(fTransacao);
			}
			// se a transação for  de inclusão, salva o cliente no novo arquivo mestre
			else if (transacao->tipoTransacao == 'I')
			{
				salvaTransacao(transacao, fErro);
				salvaCliente(cliente, fNovoMestre);

				cliente = leCliente(fMestre);
				transacao = leTransacao(fTransacao);
			}
			// se a transação for de modificação, salva o cliente modificado no novo arquivo mestre
			else if (transacao->tipoTransacao == 'M')
			{
				clienteNovo->codCliente = transacao->codCliente;
				strcpy(clienteNovo->nome, transacao->valor01);
				strcpy(clienteNovo->dataNascimento, transacao->valor02);

				salvaCliente(clienteNovo, fNovoMestre);

				cliente = leCliente(fMestre);
				transacao = leTransacao(fTransacao);
			}
		}

		// se não houver mais registros no arquivo mestre ou no arquivo de transações
		if (!cliente || !transacao)
		{
			preencherClientesRestantes(cliente, fNovoMestre, fMestre);
			preencherTransacoesRestantes(transacao, fNovoMestre, fTransacao);

			break;
		}
		
		chaveAtual = menorChave(cliente, transacao);

		achouCliente = 0;
		achouTransacao = 0;
	}
}
