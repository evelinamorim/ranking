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

    public:
    void inicia_lista_docs(unordered_map<unsigned int,vector<int> > lstdocs);

    //metodos virtuais puros
    virtual double computa_doc(vector<double> doc,vector<double> consulta,double wd) = 0;
    virtual vector<resultado_pesquisa_t> computa(unordered_map<unsigned int,double>& acc) = 0;

};

class Vetorial: public Ranking{
    public:
    double computa_doc(vector<double> doc,vector<double> consulta,double wd) ;
    vector<resultado_pesquisa_t> computa(unordered_map<unsigned int,double>& acc);
};


class BM25: public Ranking{
    public:
    double computa_doc(vector<double> doc,vector<double> consulta,double wd) ;
    vector<resultado_pesquisa_t> computa(unordered_map<unsigned int,double>& acc);
};

#endif
