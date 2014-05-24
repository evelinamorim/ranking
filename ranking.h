/*
 * =====================================================================================
 *
 *       Filename:  ranking.h
 *
 *    Description:  arquivo de header de ranking.cpp
 *
 *        Version:  1.0
 *        Created:  03/25/14 22:01:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#include <iostream>
#include <vector>
#include <unordered_map>

#include "le.h"

using namespace std;

#ifndef __RANKING_H_
#define __RANKING_H_

typedef struct resultado_pesquisa{
    unsigned int docid;
    double nota;
}resultado_pesquisa_t;


class Ranking{
    protected:
	//cada resultado da pesquisa de um termo da consulta fica aqui
    unordered_map<unsigned int,vector<int> > lista_docs;
    static const int num_docs = 945642;
    static const string nome_wd_arquivo;
    unordered_map<unsigned int,double> wd;
    string nome_tam_arquivos;
    vector<unsigned int> tam_arquivos;
    vector<double> pr;
    double max_nota;

    //parametros para o modelo BM25
    static const int K1 = 1;
    static const float b;

    //parametro para combinar os dois modelos 
    float p;

    bool flag_wd;
    bool flag_pr;

    public:

    Ranking(string arquivotam,bool wd_constroi,bool pr_constroi,float p);

    void inicia_lista_docs(unordered_map<unsigned int,vector<int> > lstdocs);
    void escreve_wd(Le* leitura,vector<unsigned long int> posicoes,vector<unsigned int> ft);
    vector<unsigned int> carrega_info_arquivos(string tamarquivo,double& mediatam);
    const int pega_num_docs();
    vector<double> pega_pr();
    void normaliza_vetor(vector<resultado_pesquisa_t>& v);

    void carrega_wd(unordered_map<unsigned int,vector<int> > docs);


    //metodos virtuais puros
    virtual double computa_doc(vector<double> doc,vector<double> consulta,double wd) = 0;
    virtual vector<resultado_pesquisa_t> computa() = 0;

};

class Vetorial: public Ranking{
    public:
    Vetorial(string arquivotam,bool wd_constroi,bool pr_constroi,float p);

    double computa_doc(vector<double> doc,vector<double> consulta,double wd) ;
    vector<resultado_pesquisa_t> computa();
};


class BM25: public Ranking{
    public:
    BM25(string arquivotam,bool wd_constroi,bool pr_constroi,float p);
    double computa_doc(vector<double> doc,vector<double> consulta,double wd) ;
    vector<resultado_pesquisa_t> computa();
    //vector<unsigned int> carrega_tam_arquivos(string arquivotam,double& mediatam);
};

class MIX: public Ranking{
    public:
    MIX(string arquivotam,bool wd_constroi,bool pr_constroi,float p);
    double computa_doc(vector<double> doc,vector<double> consulta,double wd) ;
    vector<resultado_pesquisa_t> computa();
};

#endif
