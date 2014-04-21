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
#include <iostream>
#include <sstream>
#include <iomanip>

#include "ranking.h"

using namespace std;
const string Ranking::nome_wd_arquivo = "wd_compacta.txt";

Ranking::Ranking(string arquivotam,bool wd_constroi,float potencia):nome_tam_arquivos(arquivotam){
    flag_wd = wd_constroi;
    p = potencia;
}

void Ranking::inicia_lista_docs(unordered_map<unsigned int,vector<int> > lstdocs){
    lista_docs = lstdocs;
}


void Ranking::escreve_wd(Le* leitura,vector<unsigned long int> posicoes,vector<unsigned int> ft){


	if (!flag_wd){
	    string linha;
	    int i = 1;
            ifstream arquivo_wd(nome_wd_arquivo,ios::in);
	    while(getline(arquivo_wd,linha)){
	         stringstream linhastream(linha);
		 double wd_atual;

		 linhastream>>wd_atual;
		 wd[i] = wd_atual;
		 i++;
	    }
	    arquivo_wd.close();
	}else{
            ofstream arquivo_wd(nome_wd_arquivo,ios::out|ios::app|ios::ate);
	    //sera que consigo reservar espaco aqui para otimizar o tempo?
	    vector<double> idf;
	    deque<unsigned int> v;

	    float ndocs = num_docs;
       

	    idf.reserve(ft.size());

	    //percorrer o arquivo de ft e ja calculando o idf
	    vector<unsigned int>::iterator it_ft = ft.begin();
	    vector<unsigned int>::iterator it_ft_fim = ft.end();
	    while(it_ft!=it_ft_fim){
		idf.push_back(log(1+(ndocs/(*it_ft))));
		it_ft++;
	    }
	    
	    //iterar sobre o indice e para cada termo t iterar sobre cada documento e ja 
	    //recalculando wd
		 
	    int final_arquivo = 1;
	    unsigned int freq,doc;
	    int i = 0;
	    int termo = 1;
	    int ntermos = idf.size();
            unsigned long int posicao_arquivo;
	    //conferir conta_bits com posicoes[termo] antes e depois da leitura
	    while (termo<ntermos){

		     //cout<<"Termo: "<<termo<<" "<<ntermos<<" ";
		     if (termo!=(ntermos-1)) posicao_arquivo = posicoes[termo];
		     else posicao_arquivo = leitura->pega_tamanho_arquivo()*8;

		     leitura->ler_tripla_pos(v,posicao_arquivo);
		     //No termo 31 nao bate..checar
		     int teste = 0;
		     while (v.size()!=0){
			 doc = v.front();
			 v.pop_front();
			 freq = v.front();
			 v.pop_front();
			 double pesotermo = (1+log(freq))*idf[termo-1];
			 //retirar as posicoes das palavras que guardei
			 while(freq!=0){
			     v.pop_front();
			     freq--;
			 }

			 if (wd.find(doc)== wd.end())  
			     wd[doc] = pesotermo*pesotermo;
			 else  wd[doc] = wd[doc]+(pesotermo*pesotermo);
		     }
		     termo++;
		     //hora de passar para o proximo termo
		     
	  }

	  unordered_map<unsigned int,double>::iterator it_wd = wd.begin();   
	  unordered_map<unsigned int,double>::iterator it_wd_fim = wd.end();   
	  while(it_wd!=it_wd_fim){
	      arquivo_wd << fixed << setprecision(4) << it_wd->second<<endl;
	      it_wd++;
	  }
	  arquivo_wd.close();
	}

}

void Ranking::carrega_wd(unordered_map<unsigned int,vector<unsigned int> > docs){
    ifstream arquivo_wd(nome_wd_arquivo,ios::in);
    if (arquivo_wd.is_open()){
	string linha;
	int i = 1;
	while(getline(arquivo_wd,linha)){
	    if (docs.find(i) != docs.end()){
		stringstream ss(linha);
		double pesotermo;
		ss >> pesotermo;
		wd[i] = pesotermo;
	    }
	    i++;
	}
        arquivo_wd.close();
    }
}

/* Modelo Vetorial */

Vetorial::Vetorial(string arquivotam,bool wd_constroi,float potencia):Ranking(arquivotam,wd_constroi,potencia){}

double Vetorial::computa_doc(vector<double> doc, vector<double> consulta,double wd){
    //doc: peso de cada termo do documento
    //consulta: peso de cada termo na consulta

    vector<double> resultado(doc);

    transform(doc.begin(),doc.end(),consulta.begin(),resultado.begin(),multiplies<double>());
    double numerador = accumulate(resultado.begin(),resultado.end(),0);


    return (numerador/sqrt(wd));
}



vector<resultado_pesquisa_t> Vetorial::computa(){
    //guardar o idf dos termos na colecao
    vector<double> idf;
    vector<resultado_pesquisa_t> rordenado;

    carrega_wd(lista_docs);

    cout<<"Ranking::Tamanho da lista de documentos "<<lista_docs.size()<<endl;

    if (lista_docs.size()==0) return rordenado;

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

	it_tfdocs++;
    }

    //aqui vai computar o ranking de cada documento
    it_tfdocs = tf_docs.begin();
    resultado_pesquisa_t tt;

    while (it_tfdocs!=it_tfdocs_fim){
	double r = computa_doc(it_tfdocs->second,tf_consulta,wd[it_tfdocs->first]);
	tt.docid = it_tfdocs->first;
	tt.nota = r;
	rordenado.push_back(tt);
	it_tfdocs++;
    }

    return rordenado;
}

/* Modelo BM25  */

BM25::BM25(string arquivotam,bool wd_constroi,float potencia):Ranking(arquivotam,wd_constroi,potencia){}

double BM25::computa_doc(vector<double> doc, vector<double> consulta,double wd){

   vector<double> resultado(doc);

   transform(doc.begin(),doc.end(),consulta.begin(),resultado.begin(),multiplies<double>());
   double numerador = accumulate(resultado.begin(),resultado.end(),0);

   return numerador;
}



vector<resultado_pesquisa_t> BM25::computa(){
    vector<resultado_pesquisa_t> rordenado;

    if (lista_docs.size()==0) return rordenado;

    double mediatam;
    vector<unsigned int> lista_tam = carrega_tam_arquivos(nome_tam_arquivos,mediatam);

    unordered_map<unsigned int,vector<int> >::iterator it_docs;
    unordered_map<unsigned int,vector<int> >::iterator it_docs_fim;

    unordered_map<unsigned int,vector<double> > Bij;

    it_docs = lista_docs.begin();
    it_docs_fim = lista_docs.end();     
 
    while (it_docs!=it_docs_fim){

	vector<int>::iterator it_termos = it_docs->second.begin();
	vector<int>::iterator it_termos_fim = it_docs->second.end();

	int iddoc = it_docs->first;
        Bij[iddoc] = 0;
	for(;it_termos!=it_termos_fim;it_termos++){
            double numerador = (K1 + 1)*(*it_termos);
	    double denominador = (K1*((1-b)+b*(lista_tam[iddoc-1]/mediatam))) + (*it_termos);
            Bij[it_docs->first].push_back(numerador/denominador);
	}
	it_docs++;
    }

    //guardar o idf dos termos na colecao
    vector<double> idf;

    it_docs = lista_docs.begin();
    int ntermos = it_docs->second.size();
    for(int ii=0;ii<ntermos;ii++){

	int ni = 0;
        while (it_docs!=it_docs_fim){
	    ni+=it_docs->second.at(ii);
	    it_docs++;
        }
        it_docs = lista_docs.begin();
        idf.push_back(log ((num_docs-ni+0.5)/(ni+0.5)));
    }

    unordered_map<unsigned int,vector<double> >::iterator it_bij = Bij.begin();
    unordered_map<unsigned int,vector<double> >::iterator it_bij_fim = Bij.end();
    resultado_pesquisa_t tt;

    while(it_bij!=it_bij_fim){
	double r = computa_doc(it_bij->second,idf,0);
	tt.docid = it_bij->first;
	tt.nota = r;
	rordenado.push_back(tt);
	it_bij++;

    }

    return rordenado;

}

vector<unsigned int> BM25::carrega_tam_arquivos(string arquivotam,double& mediatam){

    //vetor posicoes guarda as posicoes das palavras do dicionario no disco
    ifstream fd_arquivotam(arquivotam,ios::in);
    string linha;
    vector<unsigned int> v;
    mediatam = 0;

    if (fd_arquivotam.is_open()){
	while(getline(fd_arquivotam,linha)){
	   unsigned int tam_individual;
	   istringstream ss(linha);
	   ss >> tam_individual;
	   v.push_back(tam_individual);
	   mediatam += tam_individual;
	}
        media_tam = media_tam/v.size();
    }else{
	cout<<"BM25::Nao foi possivel carregar o arquivo com o tamanho dos arquivos"<<endl;
    }


    return v;
}

/* Ranking que combina outros rankings */

MIX::MIX(string arquivotam,bool wd_constroi,float potencia):Ranking(arquivotam,wd_constroi,potencia){}

double MIX::computa_doc(vector<double> doc, vector<double> consulta,double wd){

   return 0;
}

vector<resultado_pesquisa_t> MIX::computa(){
    vector<resultado_pesquisa_t> rordenado;

    if (lista_docs.size()==0) return rordenado;

    //rankings a serem usados
    rankBM25 = BM25(nome_tam_arquivos,flag_wd,p);
    rankVetorial = Vetorial(nome_tam_arquivos,flag_wd,p);

    return rordenado;
}
