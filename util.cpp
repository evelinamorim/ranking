/*
 * =====================================================================================
 *
 *       Filename:  util.cpp
 *
 *    Description:  funcoes uteis para este trabalho
 *
 *        Version:  1.0
 *        Created:  02/10/14 14:19:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Evelin Amorim, 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <queue>
#include "util.h"

using namespace std;

void imprime_string(string s){
    cout<<s<<" ";
}

void converteParaMinusculo(string& s){
    /* COnverte uma string para minusculo */
    transform(s.begin(), s.end(), s.begin(), ::tolower);
}

void converteParaMinusculo_char(char* s){
    if (s!=NULL){
       int n= strlen(s);

       for(int i=0;i<n-1;i++){
 	  s[i] = tolower(s[i]);
       }
    }
}

bool ehPontuacao(char c){
    return (c == ';' || c == ':' || c == '.' || c == '?' || c == ',' || c == '!' || c==')' || c=='(' || c == '\n' || c == '\t' || c == '\r' || c == '-' || c=='"');
}

void tokenizar(string s,vector<string>& v){
   string::iterator it = s.begin();

   string palavra = "";
    int palavra_tam = 0;
    while(it!=s.end()){
	if (palavra_tam>MAIOR_PALAVRA){
	   if (palavra.size() > 0) v.push_back(palavra);
	   palavra = "";
	   palavra_tam = 0;
	}else{
	    if (*it == ' ' || ehPontuacao(*it)){
	       if (palavra.size() > 0) v.push_back(palavra);
	       palavra = "";
	       palavra_tam = 0;
	    }else{
	       palavra_tam++;
	       palavra += *it;
	   }
	}
	it++;
    }

    //no caso da ultima palavra nao ter sido empilhada
    if (palavra.size()!=0) v.push_back(palavra);

}


bool arquivoExiste(const string nomeArquivo){
    struct stat buf;

    if (stat(nomeArquivo.c_str(),&buf) != 1) return true;
    return false;
}

void para_codigo_gamma(unsigned int x,unsigned long int& y,unsigned int& ny){
    ny = 0;
    unsigned int tmpx = x;
    //int lpiso = floor(log2(x));
    int lpiso =0;
    unsigned int um = 1;
    while (tmpx>>=um) lpiso++;

    ny = ny + (2*lpiso) + 1;
    int cunario = para_codigo_unario(lpiso+1);
    int pbinaria = x-( um << lpiso);

    y = cunario;
    y = y << lpiso;
    y |= pbinaria;
}

int para_codigo_unario(unsigned int x){
    //dado um numero inteiro escreve um codigo unario
    int y = 0;
    for(int i=1;i<x;i++){
	y |= (1 << i);
    }
    return y;
}

unsigned int unario_para_int(deque<unsigned int>& x,int pos){
    //pegar unario a partir da posicao pos
    //y eh o elemento sem o unario

    unsigned int cbits = 0;
     bool naoterminado = true;

    int i=pos;
    for(;i>=0;i--){
	 if ((x.front() & (1 << i))!=0){
	     cbits++;
	     if (i==0 && naoterminado){
		 //x.erase(x.begin());
		 x.pop_front();
		 i=32;
	     }
	 }else{
	     naoterminado = false;
            break;
	 } 
    }


    return cbits+1;
}

unsigned int gamma_para_int(deque<unsigned int>& x,unsigned int& nx,int pos){

    //transformar de gamma para int a partir do bit pos

    unsigned int cu;
    int tamx = x.size();
    unsigned int f = x.front();

    cu = unario_para_int(x,pos);

    int pos_cb = pos+cu;//essa inicializacao eh valida quando cb e cu estao 


    unsigned int  y = x.front();
    nx = 2*cu-1;

    int cbits;
    cbits = 0;
    int resto = cu;
    int deslocamento = pos-(2*cu-1)+1;
    //no mesmo bit

    //zerando a parte unaria
    //neste caso a parte unaria esta dividida entre dois 
    //inteiros, entao a posicao do numero lido atualmente volta 
    //para o bit numero 31
    if (tamx != x.size()){ 
	resto = cu-pos-1;
	//pos+1: quantidade de bits na parte 1
	//cu+1+resto: quantidade de bits na parte 2
	deslocamento = 32-(cu-1+resto);
	//resto = cu-1+resto;
	pos = 31;
	pos_cb = pos-resto;
    }

     for(int i=pos;i>(pos-resto);i--){
	 if ((x.front() & (1 << i))!=0){
	     y &= ~(1 << i);
	     cbits++;
	 }else{
	     if (cbits!=0){
		 pos_cb = i-1;
		 break;
	     } 
	 }
    }

    unsigned int cb;
    //cout << "Deslocamento: " << deslocamento << endl;

    //Neste caso a parte binaria esta dividida em duas partes
    int qtd_bits_cb = cu-1;
    if (((pos_cb+1)-qtd_bits_cb)<0){
	int qtd_bits_dir = fabs((pos_cb+1)-((int)cu-1));
	qtd_bits_cb = qtd_bits_dir;

        x.pop_front();
	//a primeira parte deste ou logico pega a primeira parte da parte
	//binaria, e a segunda parte deve pegar a parte posterior do numero
	//que acabou ficando em outro inteiro. Fazendo o ou logico
	//consigo parte binaria
	//cout << "1: " <<  y << " - "<< (y << (qtd_bits_dir)) << endl;
	//cout << "2: " <<  x.front() << " - " <<  (x.front() >> 30) << endl;
	cb = (y << (qtd_bits_dir)) | (x.front() >> (32-qtd_bits_dir));
	pos_cb = 31;
	y = x.front();
    }
    else cb = y >> (deslocamento);

    unsigned int numeroint = pow(2,cu-1) + cb;

    //zerando a parte binaria
    for(int i=pos_cb;i>(pos_cb-qtd_bits_cb);i--) y &= ~(1 << i);

   // cout << " 1: " << y << endl << endl;

    if (qtd_bits_cb>0){
        //verificando se acabou o ultimo buffer tratado pela parte binaria
        if ((pos_cb-qtd_bits_cb)>=0){ 
	   //cout << "Teste 1: " << pos_cb<< " "<< qtd_bits_cb<<" "<<nx<<" "<<y<<endl;
	   //ainda tem bits a serem tratado e que estao em y
	   x.pop_front();
           x.push_front(y);
        }
        else{ 
	    //cout << "Teste 2: " << pos_cb<< " "<< qtd_bits_cb<<" "<<nx<<" "<<y<<x.front()<<endl;
	x.pop_front();
       }
    }else{
	if (pos!=0){ 
	   x.pop_front();
           x.push_front(y);
	}
	else x.pop_front();
    }
    return numeroint;
}

vector<float> frequencia_termo(unordered_map<unsigned int,vector<unsigned int> > lista_docs){
    //em uma lista de de termos eh possivel computar a frequencia deste termo 
    //em cada documento
    vector<float> v;
    v.reserve(lista_docs.size());

    unordered_map<unsigned int,vector<unsigned int> >::iterator it_docs = lista_docs.begin();
    unordered_map<unsigned int,vector<unsigned int> >::iterator it_docs_fim = lista_docs.end();

    while(it_docs!=it_docs_fim){
	float tf = 1 + log(it_docs->second.size());
	v.push_back(tf);
	it_docs++;
    }

    return v;
}

float frequencia_inversa(const int num_docs,unordered_map<unsigned int,vector<unsigned int> > lista_docs){
    //contar a frequencia do termo na colecao
    unordered_map<unsigned int,vector<unsigned int> >::iterator it_docs = lista_docs.begin();
    unordered_map<unsigned int,vector<unsigned int> >::iterator it_docs_fim = lista_docs.end();

    float ni = 0;
    while(it_docs!=it_docs_fim){
	ni += it_docs->second.size();
	it_docs++;
    }

    return log (num_docs/lista_docs.size());
}

unsigned int produto_vetorial(vector<unsigned int> v1,vector<unsigned int> v2){
    unsigned int resultado;

    int n = v1.size();
    for(int i= 0;i<n;i++){
	resultado += v1.at(i)*v2.at(i);
    }

    return resultado;
}

unsigned int norma(vector<unsigned int> v){
    unsigned int resultado = 0;

    vector<unsigned int>::iterator it = v.begin();
    vector<unsigned int>::iterator it_fim = v.end();

    while (it!=it_fim){
	resultado += (*it)*(*it);
	it++;
    }

    return sqrt(resultado);
}
