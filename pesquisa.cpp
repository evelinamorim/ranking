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

using namespace std;
using namespace RICPNS;

const string Pesquisa::nome_arquivo_vocabulario = "../voc_compacta.txt";
const string Pesquisa::nome_arquivo_indice = "../index_compacta.bin";
const string Pesquisa::nome_tam_arquivos = "../tam_arquivos.txt";

Pesquisa::Pesquisa(bool compacta,int rankopt){
    clock_t  t;
    col = new Colecao(compacta);

    bool constroi_wd = true;

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
	rank = new BM25(nome_tam_arquivos,constroi_wd);
    }

    if (rankopt == VECTORCODIGO){
	rank = new Vetorial(nome_tam_arquivos,constroi_wd);
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

    for_each(consulta.begin(),consulta.end(),imprime_string);
    cout<< endl;

    unordered_map<unsigned int,vector<unsigned int> > resultado_atual,resultado_antigo;
    
    vector<string>::iterator it_consulta = consulta.begin();
    vector<string>::iterator it_consulta_fim = consulta.end();

    unordered_map<unsigned int,vector<int> > resultado;

    while(it_consulta!=it_consulta_fim){
	resultado_atual = executa_termo(*it_consulta);
	cout<<"Consulta do termo "<<*it_consulta<<" possui "<<resultado_atual.size()<<" documentos."<<endl;

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

	    unordered_map<unsigned int,vector<unsigned int> >::iterator it_atual;
	    //incluindo a frequencia de termos atuais
	    for (it_atual=resultado_atual.begin();it_atual!=resultado_atual.end();it_atual++){
		resultado[it_atual->first].push_back(it_atual->second.size());
	    }

	}else{
	    unordered_map<unsigned int,vector<unsigned int> >::iterator it_resultado;
	    for (it_resultado=resultado_atual.begin();it_resultado!=resultado_atual.end();it_resultado++)
	          resultado[it_resultado->first].push_back(it_resultado->second.size());
	}

	//TODO: sera que copia?
	resultado_antigo = resultado_atual;
	it_consulta++;
    }

    //em resultado1 fica os documentos restantes que contem todos os termos
    //Entao agora calcular ranking
    rank->inicia_lista_docs(resultado);
    vector<resultado_pesquisa_t> ordem =  rank->computa();

    return ordem;
}

void Pesquisa::imprime_docs_resultados(vector<resultado_pesquisa_t>  resultado,string dir_entrada,string nome_indice)
{
    //dado o resultado de uma consulta. Percorre os documentos da base 
    //para imprimir aqueles do resultado
    CollectionReader* leitor = new CollectionReader(dir_entrada,nome_indice);
    Document doc;
    unordered_map<unsigned int,string> listaLinks;
    queue<unsigned int> docid;

    //ordenar por id
    sort(resultado.begin(),resultado.end(),comparadocid);

    vector<resultado_pesquisa_t>::iterator it_resultado = resultado.begin();
    vector<resultado_pesquisa_t>::iterator it_resultado_fim = resultado.end();

    while (it_resultado!=it_resultado_fim){
	docid.push(it_resultado->docid);
	it_resultado++;
    }


    unsigned int i = 1;
    while(leitor->getNextDocument(doc)){

	if (docid.empty()) break;

	if (docid.front() == i){
	    listaLinks[docid.front()].reserve(doc.getURL().size());
	    listaLinks[docid.front()] = doc.getURL();
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
	cout<<listaLinks[it_resultado->docid]<<endl;
	it_resultado++;
    }
}
 

int main(int argc,char** argv){
    //TODO: mudar esta main. Agora tem ranking
    Pesquisa* p;
    bool compacta = false;
    string dir_entrada = argv[1];
    string nome_indice = argv[2];

    if (argc == 4){
	if (strncmp(argv[3],"-c",2)==0) compacta = true;
    }

   int rankopt = VECTORCODIGO;
   p = new Pesquisa(compacta,rankopt);
   string palavra;
   vector<resultado_pesquisa_t> r;

    while (getline(cin,palavra)){
	cout << "Pesquisa das palavras: " << palavra << endl;
	r = p->executa(palavra);
	p->imprime_docs_resultados(r,dir_entrada,nome_indice);
	r.clear();
	cout << endl;
    }
    return 0;
}

