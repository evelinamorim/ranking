/*
 * =====================================================================================
 *
 *       Filename:  pesquisa.h
 *
 *    Description:  arquivo de header de pesquisa
 *
 *        Version:  1.0
 *        Created:  02/25/14 22:51:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>

#include "colecao.h"
#include "le.h"
#include "ranking.h"

using namespace std;

#ifndef __PESQUISA_H_
#define __PESQUISA_H_

bool comparanota(resultado_pesquisa_t t1,resultado_pesquisa_t t2){
    return t1.nota<t2.nota;
}

bool comparadocid(resultado_pesquisa_t t1,resultado_pesquisa_t t2){
    return t1.docid<t2.docid;
}


class Pesquisa{
    static const string nome_arquivo_vocabulario;
    static const string nome_arquivo_indice;
    static const string nome_tam_arquivos;
    static const string nome_dir_saida;
    vector<unsigned long int> posicoes_palavras;
    Le* leitura;
    Colecao* col;
    Ranking* rank;

    public:
    Pesquisa(bool compact,int rankopt);
    unordered_map<unsigned int,vector<unsigned int> > executa_termo(string palavra);
    vector<resultado_pesquisa_t> executa(string palavra);
    void imprime_docs_resultados(vector<resultado_pesquisa_t>  resultado,string termos_pesquisa,string dir_entrada,string nome_indice);
    vector<string> processa_consulta(string consulta,int& tipo_consulta);
    void intersecao(unordered_map<unsigned int,vector<unsigned int> >& r1, unordered_map<unsigned int,vector<unsigned int> >& r2);
};
#endif
