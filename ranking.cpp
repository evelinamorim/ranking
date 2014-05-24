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
#include <algorithm>
#include <bitset>
#include <limits>

#include "ranking.h"
#include "util.h"

using namespace std;
const string Ranking::nome_wd_arquivo = "wd_compacta.txt";
const float Ranking::b = 0.75;

bool comparadocid1(resultado_pesquisa_t t1,resultado_pesquisa_t t2){
    return t1.docid<t2.docid;
}

Ranking::Ranking(string arquivotam,bool wd_constroi,bool pr_constroi,float potencia):nome_tam_arquivos(arquivotam){
    flag_wd = wd_constroi;
    p = potencia;
    flag_pr = pr_constroi;
    max_nota = numeric_limits<double>::min();
}

const int Ranking::pega_num_docs(){
    return num_docs;
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

void Ranking::carrega_wd(unordered_map<unsigned int,vector<int> > docs){
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

vector<unsigned int> Ranking::carrega_info_arquivos(string arquivotam,double& mediatam){

    //vetor posicoes guarda as posicoes das palavras do dicionario no disco
    ifstream fd_arquivotam(arquivotam,ios::in);
    string linha;
    vector<unsigned int> v;
    mediatam = 0;
    double max_pr = numeric_limits<double>::min();

    if (fd_arquivotam.is_open()){
	while(getline(fd_arquivotam,linha)){
	   unsigned int tam_individual;
	   double pr_individual;
	   istringstream ss(linha);
	   ss >> tam_individual;
	   ss >> pr_individual;
	   v.push_back(tam_individual);
	   if (pr_individual> max_pr)
	       max_pr = pr_individual;
	   pr.push_back(pr_individual);
	   mediatam += tam_individual;
	}

	int tam_pr = pr.size();
	for (int i =0;i<tam_pr;i++){
	    //TOD: sera que normaliza?
	    pr[i] = max_pr/(1+exp(-log(pr[i])+b));
		
	}
	//cout<<" Carregou "<<pr.size()<<endl;
        mediatam = mediatam/v.size();
    }else{
	cout<<"BM25::Nao foi possivel carregar o arquivo com o tamanho dos arquivos"<<endl;
    }


    return v;
}

void Ranking::normaliza_vetor(vector<resultado_pesquisa_t>& v){
     int tam_v = v.size();
     for(int i = 0;i<tam_v;i++){
	 v[i].nota = v[i].nota/max_nota;
     }
}

vector<double> Ranking::pega_pr(){
    return pr;
}

/* Modelo Vetorial */

Vetorial::Vetorial(string arquivotam,bool wd_constroi,bool pr_constroi,float potencia):Ranking(arquivotam,wd_constroi,pr_constroi,potencia){}

double Vetorial::computa_doc(vector<double> doc, vector<double> consulta,double wd){
    //doc: peso de cada termo do documento
    //consulta: peso de cada termo na consulta

    vector<double> resultado(doc);

    double numerador = 0.0;
    int tam_doc = doc.size();

    for(int i=0;i<tam_doc;i++){
	numerador += doc[i]*consulta[i];
    }

    return (numerador/sqrt(wd));
}



vector<resultado_pesquisa_t> Vetorial::computa(){
    //guardar o idf dos termos na colecao
    vector<double> idf;
    vector<resultado_pesquisa_t> rordenado;

    carrega_wd(lista_docs);

    double mediatam;
    vector<unsigned int> lista_tam; 
    if (flag_pr) lista_tam = carrega_info_arquivos(nome_tam_arquivos,mediatam);

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
	if (r > max_nota){
	    max_nota = r;
	}
	rordenado.push_back(tt);
	it_tfdocs++;
    }

    normaliza_vetor(rordenado);
    return rordenado;
}

/* Modelo BM25  */

BM25::BM25(string arquivotam,bool wd_constroi,bool pr_constroi,float potencia):Ranking(arquivotam,wd_constroi,pr_constroi,potencia){}

double BM25::computa_doc(vector<double> doc, vector<double> consulta,double wd){

   vector<double> resultado(doc);

   double numerador = 0.0;
   int i;
   int tam_doc = doc.size();
   for(i=0;i<tam_doc;i++){
       numerador += doc[i]*consulta[i];
   }

   return numerador;
}



vector<resultado_pesquisa_t> BM25::computa(){
    vector<resultado_pesquisa_t> rordenado;

    if (lista_docs.size()==0) return rordenado;

    cout<<"Ranking::Tamanho da lista de documentos "<<lista_docs.size()<<endl;
    double mediatam;
    vector<unsigned int> lista_tam; 
    if (flag_pr) lista_tam = carrega_info_arquivos(nome_tam_arquivos,mediatam);
   // cout<<" Testando dentro do BM25 "<< pr.size()<<endl;

    unordered_map<unsigned int,vector<int> >::iterator it_docs;
    unordered_map<unsigned int,vector<int> >::iterator it_docs_fim;

    unordered_map<unsigned int,vector<double> > Bij;

    it_docs = lista_docs.begin();
    it_docs_fim = lista_docs.end();     
 
    while (it_docs!=it_docs_fim){

	vector<int>::iterator it_termos = it_docs->second.begin();
	vector<int>::iterator it_termos_fim = it_docs->second.end();

	int iddoc = it_docs->first;
        //Bij[iddoc] = 0;
	for(;it_termos!=it_termos_fim;it_termos++){
            double numerador = (K1 + 1)*(*it_termos);
	    double denominador = (K1*((1-b)+b*(lista_tam[iddoc-1]/mediatam))) + (*it_termos);
            Bij[iddoc].push_back(numerador/denominador);
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
	if (r > max_nota){
	    max_nota = r;
	}
	rordenado.push_back(tt);
	it_bij++;

    }

    normaliza_vetor(rordenado);
    return rordenado;
}


/* Ranking que combina outros rankings */

MIX::MIX(string arquivotam,bool wd_constroi,bool pr_constroi,float potencia):Ranking(arquivotam,wd_constroi,pr_constroi,potencia){}

double MIX::computa_doc(vector<double> doc, vector<double> consulta,double wd){

   return 0;
}

vector<resultado_pesquisa_t> MIX::computa(){
    vector<resultado_pesquisa_t> rordenado,rbm25,rvet;

    if (lista_docs.size()==0) return rordenado;

    cout<<"Ranking::Tamanho da lista de documentos "<<lista_docs.size()<<endl;
    //rankings a serem usados
    BM25 rankBM25 = BM25(nome_tam_arquivos,flag_wd,flag_pr,p);
    rankBM25.inicia_lista_docs(lista_docs);


    Vetorial rankVetorial = Vetorial(nome_tam_arquivos,flag_wd,false,p);
    rankVetorial.inicia_lista_docs(lista_docs);

    rbm25 = rankBM25.computa();
    rvet = rankVetorial.computa();
    pr = rankBM25.pega_pr();

    bitset<num_docs> docs_iguais;
    docs_iguais.reset();

    //sort(rbm25.begin(),rbm25.end(),comparadocid1);
    //sort(rvet.begin(),rvet.end(),comparadocid1);

    resultado_pesquisa_t tt; 
    vector<resultado_pesquisa_t>::iterator it_rbm25 = rbm25.begin();
    vector<resultado_pesquisa_t>::iterator it_rbm25_fim = rbm25.end();

    vector<resultado_pesquisa_t>::iterator it_rvet = rvet.begin();
    vector<resultado_pesquisa_t>::iterator it_rvet_fim = rvet.end();

    vector<resultado_pesquisa_t> docs_diff;
    int ii;
    while(it_rbm25!=it_rbm25_fim){
         it_rvet = rvet.begin();

	 resultado_pesquisa_t tt_bm25;
	 tt_bm25.docid = it_rbm25->docid;
	 tt_bm25.nota = it_rbm25->nota;
	 docs_diff.push_back(tt_bm25);

	 ii = 0;
	 while(it_rvet!=it_rvet_fim){
	     if (it_rvet->docid == it_rbm25->docid){
		  tt.docid = it_rvet->docid;
		  double stat_nota = ((pow(it_rvet->nota,p)+pow(it_rbm25->nota,p))+pow((num_docs*pr[tt.docid-1]),p))/3.0;

		  /*if (stat_nota!=0)
		      tt.nota = (stat_nota+pow(ALPHA*pr[tt.docid-1],p))/3;
		  else tt.nota = 0;*/

		  //TODO: ainda acho que stat_nota nao deveria ser 0?
		  //cout<<"1 --> "<< tt.nota << " " << stat_nota << " " << it_rbm25->nota<<" "<<it_rvet->nota<< endl;
		  tt.nota = pow(stat_nota,(1.0/p));
		  //tt.nota = pow(stat_nota,(1.0/p));
		  //cout<<"2 --> "<< tt.nota <<" "<< stat_nota << " "<< (num_docs*pr[tt.docid-1])<< " "<<tt.docid << endl;
		  rordenado.push_back(tt);
		  docs_diff.pop_back();
		  docs_iguais.set(tt.docid);
		  break;
	     }
	     ii++;
	     it_rvet++;
	 }

	 if (!docs_diff.empty()){
	     tt.docid = docs_diff.back().docid;
	     double stat_nota = (pow(docs_diff.back().nota,p)+pow((num_docs*pr[tt.docid-1]),p))/3.0;
	     /*if (stat_nota!=0)
	         tt.nota =  (stat_nota+pow(ALPHA*pr[tt.docid-1],p))/3;
	     else tt.nota = 0;*/
             tt.nota = pow(tt.nota,(1.0/p));
             //tt.nota = pow(tt.nota,(1.0/p));
	     docs_diff.pop_back();
             rordenado.push_back(tt);
	 }
	 it_rbm25++;
    }

    ii = 0;
    while(ii<rvet.size()){
	//cout<<">> "<<ii<<" "<<docs_iguais.size()<<" "<<rvet.size()<<" "endl;
	if (!docs_iguais.test(rvet[ii].docid)){
	    tt.docid = rvet[ii].docid;
	    double stat_nota = (pow(rvet[ii].nota,p) +pow((num_docs*pr[tt.docid-1]),p))/3.0;
	    /* if (stat_nota!=0){
	       tt.nota = (stat_nota + pow(ALPHA*pr[tt.docid-1],p))/2;
	    }
            else tt.nota = 0;*/ 
            tt.nota = pow(tt.nota,(1.0/p));
            //tt.nota = pow(tt.nota,(1.0/p));
	}
	ii++;

    }
    return rordenado;
}
