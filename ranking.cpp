/*
 * =====================================================================================
 *
 *       Filename:  ranking.cpp
 *
 *    Description:  arquivo onde algoritmos de ranking estao implementados
 *
 *        Version:  1.0
 *        Created:  03/25/14 21:49:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <cmath>
#include <functional>
#include <numeric>
#include <unordered_map>

#include "ranking.h"

bool comparanota(resultado_ordenado_t t1,resultado_ordenado_t t2){
    return t1.nota<t2.nota;
}

void Ranking::inicia_lista_docs(unordered_map<unsigned int,vector<int> > lstdocs){
    lista_docs = lstdocs;
}

/* Modelo Vetorial */

double Vetorial::computa_doc(vector<double> doc, vector<double> consulta,double wd){
    //doc: peso de cada termo do documento
    //consulta: peso de cada termo na consulta

    vector<double> resultado(doc);

    transform(doc.begin(),doc.end(),consulta.begin(),resultado.begin(),multiplies<double>());
    double numerador = accumulate(resultado.begin(),resultado.end(),0);


    return (numerador/wd);
}



vector<resultado_pesquisa_t> Vetorial::computa(unordered_map<unsigned int,double>& acc){
    //guardar o idf dos termos na colecao
    vector<double> idf;

    cout<<"Ranking::Tamanho da lista de documentos "<<lista_docs.size()<<endl;

    /* computar o idf dos termos na colecao que eh o mesmo para a consulta e para os docs */
    unordered_map<unsigned int,vector<int> >::iterator it_docs = lista_docs.begin();
    unordered_map<unsigned int,vector<int> >::iterator it_docs_fim = lista_docs.end();

    //todos os documentos tem a mesma quantidade de elementos, cada elemento 
    //representando a frequencia de um dado termo da consulta no respectivo documento
    //cada coluna eh um termo
    int ntermos = it_docs->second.size();
    for(int ii=0;ii<ntermos;ii++){

	int ni = 0;
        while (it_docs!=it_docs_fim){
	    ni+=it_docs->second.at(ii);
	    it_docs++;
        }
        it_docs = lista_docs.begin();
        idf.push_back(log (num_docs/ni));
    }

    /* Neste caso os pesos dos termos na consulta será exatamente igual a idf*/
    //entao como guardo nos vetores de tf os pesos finais ja inicializo com o peso 
    //final que eh o proprio idf
    vector<double> tf_consulta(idf);

    /*Computa o tf dos termos dos documentos*/
    //tf_docs: <doc,list of terms frequency in doc>
    unordered_map<unsigned int, vector<double> > tf_docs;
    it_docs = lista_docs.begin();

    while (it_docs!=it_docs_fim){

	vector<int>::iterator it_termos = it_docs->second.begin();
	vector<int>::iterator it_termos_fim = it_docs->second.end();

	for(;it_termos!=it_termos_fim;it_termos++){
	    tf_docs[it_docs->first].push_back(1+log(*it_termos));
	}
	it_docs++;
    }

    /* Computar o ranking de cada doc */


    //computando o vetor de pesos de cada doc se ja nao foi computado
    //Aqui este peso vai ser atribuido a tf_docs
    unordered_map<unsigned int,vector<double> >::iterator it_tfdocs = tf_docs.begin();
    unordered_map<unsigned int,vector<double> >::iterator it_tfdocs_fim = tf_docs.end();
    while (it_tfdocs!=it_tfdocs_fim){
        transform(it_tfdocs->second.begin(),it_tfdocs->second.end(),idf.begin(),it_tfdocs->second.begin(),multiplies<double>());

	//calcular Wd, caso ainda nao tenha sido calculado
	//TODO: por enquanto estou soh armazenando
	if (acc.find(it_tfdocs->first)==acc.end()){
	    acc[it_tfdocs->first] = 0;
	    int ntermos = it_tfdocs->second.size();
	    for(int i=0;i<ntermos;i++) 
		acc[it_tfdocs->first] += it_tfdocs->second.at(i)*it_tfdocs->second.at(i);
	    acc[it_tfdocs->first] = sqrt(acc[it_tfdocs->first]);
	}
	it_tfdocs++;
    }

    //aqui vai computar o ranking de cada documento
    it_tfdocs = tf_docs.begin();
    vector<resultado_pesquisa_t> rordenado;
    resultado_ordenado_t tt;

    while (it_tfdocs!=it_tfdocs_fim){
	double r = computa_doc(it_tfdocs->second,tf_consulta,acc[it_tfdocs->first]);
	tt.docid = it_tfdocs->first;
	tt.nota = r;
	rordenado.push_back(tt);
	it_tfdocs++;
    }


    return rordenado;
}

/* Modelo BM25  */

double BM25::computa_doc(vector<double> doc, vector<double> consulta,double wd){
   return 0;
}



vector<resultado_pesquisa_t> BM25::computa(unordered_map<unsigned int,double>& acc){
    //guardar o idf dos termos na colecao
    vector<double> idf;
    vector<resultado_ordenado_t> t;

    return t;

}
