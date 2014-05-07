/*
 * =====================================================================================
 *
 *       Filename:  avaliacao.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/28/14 16:09:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

#ifndef __AVALIACAO_H_
#define __AVALIACAO_H_

#define TAM_LINK 256
class Avaliacao{
        vector<double> em_comum;//documentos em comum entre relevantes e recuperados
	int num_respostas;
	vector<int> pos_rel;
    public:
	Avaliacao(int nr);
	vector<string> ler_arquivo_links(ifstream& arquivo);
	vector<double> intersecao(vector<string> relevantes,vector<string> recuperados);
	vector<double> precisao(vector<string> relevantes,vector<string> recuperados);
	vector<double> revocacao(vector<string> relevantes,vector<string> recuperados);

	void inicia_em_comum(vector<double> ec);
};
#endif
