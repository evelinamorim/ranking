/*
 * =====================================================================================
 *
 *       Filename:  pesquisa.cpp
 *
 *    Description:  Arquivo que realiza a pesquisa em um arquivo de indice 
 *    invertido
 *
 *        Version:  1.0
 *        Created:  02/25/14 22:50:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <unordered_map>
#include <cstring>
#include <sstream>
#include <queue>
#include <ctime>

#include "pesquisa.h"
#include "CollectionReader.h"
#include "util.h"

#define INTERSECAO 1
#define UNIAO 2

#define BM25CODIGO 1
#define VECTORCODIGO 2
#define MIXCODIGO 3

#define POTENCIA 2
using namespace std;
using namespace RICPNS;

const string Pesquisa::nome_arquivo_vocabulario = "../voc_compacta.txt";
const string Pesquisa::nome_arquivo_indice = "../index_compacta.bin";
const string Pesquisa::nome_info_arquivos = "../info_arquivos.txt";
const string Pesquisa::nome_dir_saida = "saida/";

Pesquisa::Pesquisa(bool compacta,int rankopt){
    clock_t  t;
    col = new Colecao(compacta);

    bool constroi_wd = false;

    t = clock();
    posicoes_palavras = col->carrega_vocabulario(nome_arquivo_vocabulario);
    t = clock() - t;
    cout << "Tempo Pesquisa::carrega_vocabulario: "<< ((float)t/CLOCKS_PER_SEC) << "s" << endl;

    if (compacta){
	leitura = new LeCompacta(nome_arquivo_indice);
    }else{
	leitura = new LeNormal(nome_arquivo_indice);
    }

    if (rankopt == BM25CODIGO){
	rank = new BM25(nome_info_arquivos,constroi_wd,POTENCIA);
    }

    if (rankopt == VECTORCODIGO){
	rank = new Vetorial(nome_info_arquivos,constroi_wd,POTENCIA);
    }

    if (rankopt == MIXCODIGO){
	rank = new MIX(nome_info_arquivos,constroi_wd,POTENCIA);
    }

    t = clock();
    rank->escreve_wd(leitura,posicoes_palavras,col->pega_ft());
    t = clock() - t;
    cout << "Tempo Pesquisa::carrega_wd: "<< ((float)t/CLOCKS_PER_SEC) << "s" << endl;
    
}

vector<string> Pesquisa::processa_consulta(string consulta,int& tipo_consulta){
    //vetor vazio se houver problema com a consulta
    vector<string> lista_consulta;

    tokenizar(consulta,lista_consulta);

    return lista_consulta;
}

void Pesquisa::intersecao(unordered_map<unsigned int,vector<unsigned int> >& r1, unordered_map<unsigned int,vector<unsigned int> >& r2){

    unordered_map<unsigned int,	vector<unsigned int> >::iterator it2;

    it2=r2.begin();

    vector<unsigned int> elementos_diferentes;

    while(it2!=r2.end()){
	if (r1.find(it2->first) == r1.end()){
	    elementos_diferentes.push_back(it2->first);
	}
	it2++;
    }

    vector<unsigned int>::iterator it_dif;
    for(it_dif=elementos_diferentes.begin();it_dif!=elementos_diferentes.end();it_dif++){
	r2.erase(*it_dif);
    }

}


unordered_map<unsigned int,vector<unsigned int> > Pesquisa::executa_termo(string palavra){
    int i = col->pega_lexico_inteiro(palavra);
    unsigned long int pos_arquivo;
    int freq,final_arquivo;
    unsigned int doc;
    unordered_map<unsigned int,vector<unsigned int> > resultado;

    if (i == posicoes_palavras.size()){
	ifstream arquivo(nome_arquivo_indice,ios::in|ios::binary|ios::ate);
	//TODO: e se ler os bits finais que estao zerados?
	pos_arquivo = arquivo.tellg()*8;
	arquivo.close();
    }else  pos_arquivo = posicoes_palavras[i];


    deque<unsigned int> v;
    vector<unsigned int> tmp;

    leitura->inicia_conta_bits(posicoes_palavras[i-1]);

    final_arquivo = 1;

    while (leitura->pega_conta_bits()<pos_arquivo && final_arquivo != -1){
          leitura->ler_tripla(v,2);
	  freq = v.back();
	  v.pop_back();
	  doc = v.back();
	  v.pop_back();

	  v.clear();
          final_arquivo = leitura->ler_tripla(v,freq);
	  copy(v.begin(), v.end(), back_inserter(tmp));
          resultado[doc] = tmp;

	  v.clear();
	  tmp.clear();
    }


    return resultado;

}

vector<resultado_pesquisa_t> Pesquisa::executa(string palavra){

    vector<string> consulta;
    int tipo_consulta;
    consulta =  processa_consulta(palavra,tipo_consulta);

    //for_each(consulta.begin(),consulta.end(),imprime_string);
    //cout<< endl;

    unordered_map<unsigned int,vector<unsigned int> > resultado_atual,resultado_antigo;
    
    vector<string>::iterator it_consulta = consulta.begin();
    vector<string>::iterator it_consulta_fim = consulta.end();

    unordered_map<unsigned int,vector<int> > resultado;

    while(it_consulta!=it_consulta_fim){
	//perceba que resultado_atual guarda resultado do i-esimo termo enquanto 
	//resultado_antigo guarda
	resultado_atual = executa_termo(*it_consulta);
	//cout<<"Consulta do termo "<<*it_consulta<<" possui "<<resultado_atual.size()<<" documentos."<<endl;

	if (resultado_antigo.size()!=0){ 
	    intersecao(resultado_atual,resultado_antigo);

	    unordered_map<unsigned int,vector<int> >::iterator it_resultado;

	    //aqui estou apagando documentos nao existentes mais por nao possuirem os 
	    //o termo de pesquisa atual
	    vector<unsigned int> docs_irrelevantes;
	    for (it_resultado=resultado.begin();it_resultado!=resultado.end();it_resultado++){
		  if (resultado_antigo.find(it_resultado->first) == resultado_antigo.end()){
		      docs_irrelevantes.push_back(it_resultado->first);
		  }
	    }

	    vector<unsigned int>::iterator it_irr;
	    for(it_irr=docs_irrelevantes.begin();it_irr!=docs_irrelevantes.end();it_irr++){
		resultado.erase(*it_irr);
	    }

	    unordered_map<unsigned int,vector<unsigned int> >::iterator it_antigo;
	    //incluindo a frequencia de termos atuais
	    for (it_antigo=resultado_antigo.begin();it_antigo!=resultado_antigo.end();it_antigo++){
		//resultado antigo neste momento guarda o resultado da intersecao,
		//assim resultado atual deve conter documentos com ids it_antigo->first
		//Apenas estou acrescentando a info(i-esimo termo da consulta tokenizada) 
		//de resultado_atual a resultado
		resultado[it_antigo->first].push_back(resultado_atual[it_antigo->first].size());
	    }

	}else{
	    unordered_map<unsigned int,vector<unsigned int> >::iterator it_resultado;
	    for (it_resultado=resultado_atual.begin();it_resultado!=resultado_atual.end();it_resultado++)
	          resultado[it_resultado->first].push_back(it_resultado->second.size());
	}

	//TODO: sera que copia?
	resultado_antigo = resultado_atual;
	//cout<<"Tam resultado antigo: "<<resultado_antigo.size() <<" Tam resultado atual: "<< resultado_atual.size()<<endl;
	it_consulta++;
    }

    //em resultado1 fica os documentos restantes que contem todos os termos
    //Entao agora calcular ranking
    rank->inicia_lista_docs(resultado);
    vector<resultado_pesquisa_t> ordem =  rank->computa();

    return ordem;
}

void Pesquisa::imprime_docs_resultados(vector<resultado_pesquisa_t>  resultado,string termos_pesquisa,string nome_indice)
{
    //dado o resultado de uma consulta. Percorre os documentos da base 
    //para imprimir aqueles do resultado
    //CollectionReader* leitor = new CollectionReader(dir_entrada,nome_indice);
    ifstream indice_links(nome_indice);
    Document doc;
    unordered_map<unsigned int,string> listaLinks;
    queue<unsigned int> docid;

    ofstream arquivo_saida(nome_dir_saida+termos_pesquisa);

    //ordenar por id
    sort(resultado.begin(),resultado.end(),comparadocid);

    vector<resultado_pesquisa_t>::iterator it_resultado = resultado.begin();
    vector<resultado_pesquisa_t>::iterator it_resultado_fim = resultado.end();

    while (it_resultado!=it_resultado_fim){
	docid.push(it_resultado->docid);
	it_resultado++;
    }


    unsigned int i = 1;
    int n = rank->pega_num_docs();
    float media_tam_links = 0;
    while(i<n){
	string linha;
	getline(indice_links,linha);
	istringstream ss(linha);

	string link,pos;
	ss >> link;
	ss >> pos;

	if (docid.empty()) break;

	if (docid.front() == i){
	    listaLinks[docid.front()].reserve(link.size());
	    listaLinks[docid.front()] = link;
	    //cout<<"==> "<<link<<endl;
	    //cout<<"--> "<<listaLinks[docid.front()]<<endl;
	    docid.pop();
	   //cout << "DOCUMENTO " << i << endl;
	   //cout << "[" << doc.getURL() << "]" << endl;
	   //cout << doc.getText() << endl << endl;
	}
	///tree<htmlcxx::HTML::Node> dom = parser.parseTree(doc.getText());


	++i;
    }

    sort(resultado.begin(),resultado.end(),comparanota); 
    it_resultado = resultado.begin();
    it_resultado_fim = resultado.end();

    while(it_resultado!=it_resultado_fim){
	arquivo_saida<<listaLinks[it_resultado->docid]<<endl;
	//cout<<"LInk: "<< listaLinks[it_resultado->docid]<<" Nota "<<it_resultado->nota<<endl;
	it_resultado++;
    }
    indice_links.close();
    arquivo_saida.close();
}
 

int main(int argc,char** argv){
    //TODO: mudar esta main. Agora tem ranking
    Pesquisa* p;
    bool compacta = false;
    string dir_entrada = argv[1];
    string nome_indice = dir_entrada + argv[2];

    if (argc == 4){
	if (strncmp(argv[3],"-c",2)==0) compacta = true;
    }

   int rankopt = MIXCODIGO;
   p = new Pesquisa(compacta,rankopt);
   string palavra;
   vector<resultado_pesquisa_t> r;

   clock_t t;
    while (getline(cin,palavra)){
	t = clock();
	cout << "Pesquisa das palavras: " << palavra << endl;
	r = p->executa(palavra);
	t = clock()-t;
	cout<<" Tempo "<< ((float)t/CLOCKS_PER_SEC) <<"s"<<endl;

	t = clock();
	p->imprime_docs_resultados(r,palavra,nome_indice);
	t = clock()-t;
	cout<<" Exibicao dos resultados leva "<<((float)t/CLOCKS_PER_SEC) <<"s"<<endl;

	r.clear();
	cout << endl;
    }
    return 0;
}

