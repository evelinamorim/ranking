/*
 * =====================================================================================
 *
 *       Filename:  documento.cpp
 *
 *    Description:  Arquivo que manipula documentos do repositorio
 *
 *        Version:  1.0
 *        Created:  02/06/14 22:37:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  evelinamorim, 
 *   Organization:  
 *
 * =====================================================================================
 */
int testeuou = 0;
int testepalavras = 0;
#include "colecao.h"
#include "CollectionReader.h"
#include "util.h"
#include "ordena.h"
#include "Url.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <cstdio>
#include <regex>
#include <cstring>
#include <ctime>
#include <htmlcxx/html/Node.h>
#include <htmlcxx/html/ParserDom.h>

using namespace std;
using namespace RICPNS;

const string Colecao::nome_arquivo_indice="index_compacta.bin";
const string Colecao::nome_arquivo_vocabulario="voc_compacta.txt";
const string Colecao::nome_info_arquivos="info_arquivos.txt";

//regex html_expr("(http|https|ftp|ftps)\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(\/\S*)?");

Colecao::~Colecao(){

    delete escrita;
}

Colecao::Colecao(bool compacta){

    if (compacta) escrita = new EscreveCompacta(nome_arquivo_indice);
    else escrita = new EscreveNormal(nome_arquivo_indice);

    unordered_map<string,vector<int> >::iterator it_voc_inicio;
    unordered_map<string,vector<int> >::iterator it_voc_fim;

    buffer_chaves = new char*[TAM_VOC];
    buffer_links = new char*[TAM_COL+1];

    contaPalavras = 1;
    ft.push_back(0);
}

const string Colecao::pega_nome_arquivo_indice(){
    return nome_arquivo_indice;
}

int Colecao::pega_tamanho_vocabulario(){
    return vocabulario.size();
}

int Colecao::pega_tamanho_vocabulario_invertido(){
    return vocabulario_invertido.size();
}

vector<unsigned int> Colecao::pega_ft(){
    return ft;
}

void Colecao::inicia_indice_link(string dirEntrada,string nomeIndice){

    ifstream arquivo_link(dirEntrada+nomeIndice,ios::in);

    clock_t  t;
    t = clock();

    if (arquivo_link.is_open()){
	string linha;
	int ii = 0;
	float media_tam_link = 0;
	while(!arquivo_link.eof()){
	    getline(arquivo_link,linha);
	   istringstream ss(linha);

	   string link,folder,pos;
	   ss >> link;
	   ss >> folder;
	   ss >> pos;
	   ss >> pos;
	   ss >> pos;

	   int tam_link = link.size();
	   buffer_links[ii] = new char[tam_link+1];
	   if (buffer_links[ii]!=NULL){
	       memset(buffer_links[ii],'\0',tam_link+1);
	       copy(link.begin(),link.end(),buffer_links[ii]);
	       indice_links[buffer_links[ii]] = vector<int>();
	   }
	   Fu.push_back(0);
	   //media_tam_link += link.size();
	   ii++;
	}
	cout<<"Iniciar o indice de links levou: "<<((float)t/CLOCKS_PER_SEC) << "s" << endl;
	//cout<<" Em media os links possuem "<<(media_tam_link/945642)<<" caracteres "<<endl;
	arquivo_link.close();
    }else{
	cout<<"Colecao::inicia_indice_link::Problema ao abrir o arquivo de indice"<<endl;
    }

}

void Colecao::ler(string dirEntrada,string nomeIndice){
    CollectionReader* leitor = new CollectionReader(dirEntrada,nomeIndice);
    Document doc;

    unordered_map<int,vector<int> > termos_pos;

    htmlcxx::HTML::ParserDom parser;

    doc.clear();

    inicia_indice_link(dirEntrada,nomeIndice);

    int i = 1;
    clock_t  t;
    t = clock();
    ofstream arquivo_info_arquivos (nome_info_arquivos, ios::out|ios::app);
    while(leitor->getNextDocument(doc)){

	
	//cout << "[" << doc.getURL() << "]" << endl;
	tree<htmlcxx::HTML::Node> dom = parser.parseTree(doc.getText());
        ler_arvore_dom(dom,i,termos_pos,arquivo_info_arquivos,doc.getURL());

	armazena_termos_doc(termos_pos,i);
	    
	//cout << dom << endl;
	++i;
    }
    //cout<<"Media tam Links: "<<(media_tam_link/945642)<<"  MAX tam "<<max_url_tam<<endl;

    if (arquivo_info_arquivos.is_open())
        arquivo_info_arquivos.close();
    t = clock() - t;
    cout << "Tempo Colecao::ler: "<< ((float)t/CLOCKS_PER_SEC) << "s" << endl;

    t = clock();
    escreve_info_links(dirEntrada,nomeIndice);
    t = clock() - t;
    cout << "Tempo Colecao::escreve_info_links: "<< ((float)t/CLOCKS_PER_SEC) << "s" << endl;

    cout << "Colecao::Numero triplas: " << testeuou << endl;
    cout << "Colecao::Numero de palavras: " << testepalavras << endl;


    //finaliza o armazenamento aqui
    if(dynamic_cast<EscreveCompacta*>(escrita) != 0){
        if (escrita->pega_excedente()!=0)
	    escrita->escreve_excedente();
    }
    escrita->fecha_arquivo();

    delete leitor;
}


 void Colecao::ler_arvore_dom(tree<htmlcxx::HTML::Node> dom,int idArvore,unordered_map<int,vector<int> >& termos_pos,ofstream& arquivotam,string url){
    // cout << "ler_arvore_dom " << idArvore << endl;
    tree<htmlcxx::HTML::Node>::iterator it = dom.begin();
    tree<htmlcxx::HTML::Node>::iterator end = dom.end();
    vector<string> listaPalavras;
    char* palavra = new char[MAIOR_PALAVRA+2];
    char* link_tmp = new char[MAIOR_LINK+2];
    oneurl curl;
    //para guardar relacao (termo,lista_posicoes_doc)

    vector<int> lpos;
    int i = 0;
    //contador para saber em que posicao esta uma dada palavra
    int palavraPos = 0; 

   //cout << "DOCUMENTO " << idArvore << " " << dom.size() << endl;


    for (; it != end; ++it,i++)
    {  
	    //aparentemente existe um header em toda pagina html
	    //entao eu ignoro este header aqui
	   if (i == 1) continue;

	    bool isscript;
	    bool islink;
	    string tag(it->tagName());

	    converteParaMinusculo(tag);

	    if (it->isTag()){
		if (tag=="script") isscript = true;
	        else isscript = false;

		if (tag=="a" || tag=="A"){ 
		    islink = true;
		     it->parseAttributes();
		     string link_href;
		     link_href.reserve(MAIOR_LINK+2);
		     link_href = curl.CNormalize(it->attribute("href").second);
		     if (link_href.size()>0){

		        //copiando os 800 primeiros caracteres
		        memset(link_tmp,'\0',MAIOR_LINK+2);
		        snprintf(link_tmp,MAIOR_LINK,"%.800s",link_href.c_str());

		        //esse aqui vai ajudar a computar o conjunto Bu
		        if (indice_links.find(link_tmp) != indice_links.end()){
		           indice_links[link_tmp].push_back(idArvore);
		        }
		        Fu[idArvore-1] =  Fu[idArvore-1] + 1;
		     }

		}
		else islink = false;
	    }

	    //por enquanto indexando anchor text junto com as palavras
	    //pq esta dando tantas palavras? Se ele estava computando antes anchor text? 
	    //conferir com um conjunto pequeno!
	     if (!it->isComment() && !it->isTag() && !isscript){
		 //if (islink) 
		 //    cout<<"Anchor text: "<< it->text()<<" Tag: "<<tag<<endl;

		 listaPalavras.clear();

		 tokenizar(it->text(),listaPalavras);
		 int ii = 0;
	         vector<string>::iterator it_palavras = listaPalavras.begin();

	          while(ii<listaPalavras.size()){
	             // cout << "Percorrendo listaPalavras " << ii << endl;

		      memset(palavra,'\0',MAIOR_PALAVRA+2);
		      copy(listaPalavras[ii].begin(),listaPalavras[ii].end(),palavra);

		      palavraPos++;

	              int tamanho_palavra = strlen(palavra);

		      if (palavra!=NULL && tamanho_palavra!=0){
		          char* tmp = strdup(palavra);

		          converteParaMinusculo_char(tmp);
			  strcpy(palavra,tmp);
			  //se der falha eh aqui!
			  if (tmp!=NULL) free(tmp);

			  testepalavras++;
		          if (vocabulario.find(palavra) == vocabulario.end()){

				buffer_chaves[contaPalavras] = new char[tamanho_palavra+1];
				memset(buffer_chaves[contaPalavras],'\0',tamanho_palavra+1);
				strncpy(buffer_chaves[contaPalavras],palavra,tamanho_palavra);

				
				vocabulario[buffer_chaves[contaPalavras]] = contaPalavras;
			
			
			        vocabulario_invertido[contaPalavras] = buffer_chaves[contaPalavras];
			 
			        termos_pos[contaPalavras] = vector<int>();
			        termos_pos[contaPalavras].push_back(palavraPos);
			
			        ft.push_back(1);
			        contaPalavras++;
			 }else{
			
			      if (termos_pos.find(vocabulario[palavra]) == termos_pos.end()){
			           int idpalavra = vocabulario[palavra];
			           termos_pos[idpalavra] = vector<int>();
			           //so contar frequency term quando for a primeira vez neste doc
			          ft[idpalavra] = ft[idpalavra]+1;
			      }
			      termos_pos[vocabulario[palavra]].push_back(palavraPos);
			 }
		      }
		      ii++;
		      it_palavras++;
		   }
	       }//end-if !it->isComment() && !it->isTag() && !isscript
			
    }

     if (arquivotam.is_open()){
	  arquivotam << palavraPos;
          arquivotam << endl;
     }
     if (palavra!=NULL) delete[] palavra;
     if (link_tmp!=NULL) delete[] link_tmp;
     
}

void Colecao::armazena_termos_doc(unordered_map<int,vector<int> >& termos_pos,int doc){
    //dado o codigo do item lexical armazena em disco o
    //lexical
   // cout << "armazena_termos_doc" << endl;

    unordered_map<int,vector<int> >::iterator it_termo = termos_pos.begin();
    vector<unsigned int> v;
    int pos_gap;


    //armazenando todos os docs!!os anteriores tbm!
    while(it_termo != termos_pos.end()){

        vector<int>::iterator it_pos;

        for(it_pos=it_termo->second.begin();it_pos!=it_termo->second.end();it_pos++){
            if (it_pos!=it_termo->second.begin())
                pos_gap = *(it_pos)-*(it_pos-1);
            else pos_gap = *it_pos;

            v.push_back(it_termo->first);
            v.push_back(doc);
            v.push_back(*it_pos);

            testeuou++;
        }
        ++it_termo;
    }

    //TODO: estava dentro do laco de repeticao
    escrita->escreve_tripla(v);
    //cout<<"Escrevendo "<<testeuou<<" triplas com "<<escrita->pega_conta_bits_global()<<endl;

    //cout << "Armazena: " << v.size() << endl;

    //TODO: acho que nao tem necessidade, mas...
    v.clear();

    it_termo = termos_pos.begin();

    while(it_termo != termos_pos.end()){
       it_termo->second.clear();
       it_termo++;
    }
    termos_pos.clear();
}

void Colecao::atualiza_vocabulario(int lex,unsigned long int pos){
    vocabulario[vocabulario_invertido[lex]] = pos;
}

vector<double> Colecao::computa_info_links(string dirEntrada,string nomeIndice){
    //computa o pr das paginas
    ifstream arquivo_link(dirEntrada+nomeIndice,ios::in);
    vector<vector<int> > back_links;
    char* link_tmp = new char [MAIOR_LINK+2];
    vector<double> pr;

    if (arquivo_link.is_open()){
	//inicia matriz de backlinks
	while(!arquivo_link.eof()){
	    string linha;

	    getline(arquivo_link,linha);

            istringstream ss(linha);

	    string link,folder,pos;
	    ss >> link;
	    ss >> folder;
	    ss >> pos;
	    ss >> pos;
	    ss >> pos;

	   int tam_link = link.size();
	    if (link_tmp!=NULL){
	        memset(link_tmp,'\0',MAIOR_LINK+2);
	        snprintf(link_tmp,MAIOR_LINK,"%.200s",link.c_str());
		back_links.push_back(indice_links[link_tmp]);
	    }

	    //para cada doc inicializar o rank como 1/numero de documentos
	   // pr.push_back(1/num_docs);
	   pr.push_back(1); 

	}


	vector<double> old_pr(pr);

	int ii = 0;
	//a partir dos backlinks calcular o pagerank
	while(ii< ITER_PR){//calcular x vezes para cad a pagina
	     int jj = 0;
	     //iterando sobre cada pagina
	     while (jj<back_links.size()){
		// cout<<" --> "<<jj<<" "<<back_links.size()<<endl;
		 vector<int>::iterator it_bl_ii = back_links[jj].begin();
		 vector<int>::iterator it_bl_ii_fim = back_links[jj].end();
	         //iterando sobre o conjunto Bu de cada pagina
		 double pr_valor = 0;
		 while (it_bl_ii!= it_bl_ii_fim){
		     int k = (*it_bl_ii)-1;
		     if (Fu[k]!=0)
		         pr_valor +=  old_pr[k]/Fu[k];
		     it_bl_ii++;
		 }
		 pr_valor = D_FACTOR*pr_valor; 
		 pr_valor = (1-D_FACTOR) + pr_valor;
		 pr[jj] = pr_valor; 
		 jj++;
	     }
	     old_pr = pr;
	     ii++;
	}

    }else{
	cout<<"Colecao::computa_info_links Problema ao abrir arquivo"<<endl;
    }

    //TODO: Algum lugar com segmentation fault e divisao por zero em pr
    if (link_tmp!=NULL) delete[] link_tmp;

    return pr;
}

void Colecao::escreve_info_links(string dirEntrada,string nomeIndice){

    ofstream arquivo(nome_info_arquivos + ".tmp",ios::out|ios::app);

    if (arquivo.is_open()){
	ifstream arquivo_antigo(nome_info_arquivos,ios::in);
	vector<double> pr_links = computa_info_links(dirEntrada,nomeIndice);
        int ii = 0;
	while (!arquivo_antigo.eof()){
	    string linha;

	    getline(arquivo_antigo,linha);

	    istringstream ss(linha);

	    int tam;
	    ss >> tam;

	    
	    arquivo << tam << " "<<pr_links[ii]<<endl;

	    ii++;
	}
        arquivo.close();
	arquivo_antigo.close();
	/* 
        if (buffer_links !=NULL){
	   int n = indice_links.size(); 
           for(int i=0;i<n;i++){
	      if (buffer_links[i] !=NULL) 
	          delete[] buffer_links[i];
	   }
           delete[] buffer_links;
        } */
	string nome_arquivo_antigo = nome_info_arquivos + ".tmp";

        char narquivo[100]; 
        memset(narquivo,0,100);
        strcpy(narquivo,nome_info_arquivos.c_str());

        char narquivo_antigo[100]; 
        memset(narquivo_antigo,0,100);
        strcpy(narquivo_antigo,nome_arquivo_antigo.c_str());
	//    cout<<"Removendo: "<<narquivo<<endl;
	//    cout<<"Renomeando "<<narquivo_antigo<<" para "<<narquivo<<endl;

        if (narquivo!=NULL){
            remove(narquivo);
	}         
	if (narquivo_antigo!=NULL && narquivo!=NULL){
           rename(narquivo_antigo,narquivo);
	} 
    }else{
	cout<<"Colecao::escreve_info_links Problema ao abrir arquivo"<<endl;
    }
}

void Colecao::escreve_vocabulario(){
    ofstream arquivo (nome_arquivo_vocabulario, ios::out|ios::app);
    if (arquivo.is_open()){

        unordered_map<char*,unsigned long int>::iterator it_voc = vocabulario.begin();
        int i=1;
	int ntermos = vocabulario_invertido.size();
        while(i<=ntermos){
	  arquivo << vocabulario_invertido[i] << " " <<  vocabulario[vocabulario_invertido[i]] << " "<< ft[i] << endl;
	    i++;
	}
	arquivo.close();
    }else{
	cout << "Colecao::atualiza_vocabulario::Problema ao abrir arquivo." << endl;
    }
    int n = vocabulario.size();
    if (buffer_chaves !=NULL){
        for(int i=0;i<n;i++){
	   if (buffer_chaves[i] !=NULL) 
	       delete[] buffer_chaves[i];
	}
        delete[] buffer_chaves;
    }
}

vector<unsigned long int> Colecao::carrega_vocabulario(const string arquivo_vocabulario){
    ifstream arquivo(arquivo_vocabulario,ios::in);
    string linha;
    int i = 1;
    vector<unsigned long int> posicoes;
    char* palavra = new char[MAIOR_PALAVRA+2]();
    int tamanho_palavra;
    vector<string> tokens;

    if (arquivo.is_open()){
	while(getline(arquivo,linha)){
	    stringstream linhastream(linha);
	    string dado;
	    string lex;
	    unsigned long int pos;
	    unsigned int ft_individual;

            tokenizar(linha,tokens);

	    //linhastream >> lex;
	    //linhastream >> pos;
	    //linhastream >> ft_individual;
	    lex = tokens[0];
	    pos = strtoul(tokens[1].c_str(),NULL,0);
	    ft_individual = strtoul(tokens[2].c_str(),NULL,0);

            //cout<<"--> "<<i<<" "<<pos<<" "<<ft_individual<<endl;
	    memset(palavra,'\0',MAIOR_PALAVRA+2);
            copy(lex.begin(),lex.end(),palavra);

	    if (palavra!= NULL){
		tamanho_palavra = strlen(palavra)+1;
	        buffer_chaves[i] = new char[tamanho_palavra+1];

		memset(buffer_chaves[i],'\0',tamanho_palavra+1);
                strncpy(buffer_chaves[i],palavra,tamanho_palavra);

	        vocabulario[buffer_chaves[i]] = i;
	        i++;
	        posicoes.push_back(pos);
                //cout<<"--> "<<i<<" "<<pos<<" "<<ft_individual<<" "<<posicoes.back()<<endl;
		//if (i==5321) cout<<"==>"<<pos<<" "<<posicoes.back()<<endl;

		ft.push_back(ft_individual);

	        vocabulario_invertido[i] = buffer_chaves[i];
	    }

	    tokens.clear();
	}
    }else{
	cout << "Colecao::carrega_vocabulario: Nao foi possivel abrir vocabulario." << endl;
    }
    if (palavra!=NULL) delete[] palavra;

   return posicoes; 

}

unsigned long int Colecao::pega_lexico_inteiro(string p){
    char* palavra = new char[p.size()+2];
    memset(palavra,'\0',p.size()+1);

    unsigned long int lex;

    copy(p.begin(),p.end(),palavra);
    if (palavra!=NULL){
       lex = vocabulario[palavra];
       delete[] palavra;
    }
    return lex;
}

char* Colecao::pega_lexico(int p){
    return vocabulario_invertido[p];
}

