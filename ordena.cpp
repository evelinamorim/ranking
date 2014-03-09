/*
 * =====================================================================================
 *
 *       Filename:  ordena.cpp
 *
 *    Description:  esta classe tem o papel de ordenar um arquivo de indice em disco
 *
 *        Version:  1.0
 *        Created:  02/22/14 00:19:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#define MIN(a,b)  ({ __typeof__ (a) _a = (a);  __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

#include "ordena.h"

#include <iostream>
#include <deque>
#include <cstdlib>
#include <climits>
#include <cstdio>
#include <unordered_map>

using namespace std;

Ordena::Ordena(string narquivo,bool compacta){
    nome_arquivo = narquivo;
    run = new tripla_t[TAMANHO_RUN];


    if (compacta){
	escrita = new EscreveCompacta(narquivo+"tmp");
	escrita_ordenada = new EscreveCompacta(narquivo+".ord");
	leitura = new LeCompacta(narquivo);
    }else{
	escrita = new EscreveNormal(narquivo+"tmp");
	escrita_ordenada = new EscreveNormal(narquivo+".ord");
	leitura = new LeNormal(narquivo);
    }

    ifstream arquivo(nome_arquivo,ios::in|ios::binary|ios::ate);
    if (arquivo.is_open()){
	tamanho_arquivo = arquivo.tellg();
	arquivo.close();
    }else{
	cout << "Ordena::carrega_run::Nao foi possivel abrir o arquivo.";
    }

    conta_triplas = 0;
}

Ordena::~Ordena(){
    delete escrita;
    delete escrita_ordenada;
    delete leitura;
    if (run !=NULL)  delete[] run;
}

int Ordena::carrega_run(int& pos_arquivo){

    conta_bits.push_back(leitura->pega_conta_bits());
    deque<unsigned int> v;
    int i;
   // cout << "RUN" << endl;
   //TODO: ver como tirar  leitura  daqui. Visto que aqui eu sei o numero de 
   //triplas a serem lidas. Acho que vou ter que mudar a funcao de leitura
    pos_arquivo = leitura->ler_tripla(v,3*TAMANHO_RUN);
    int n = (v.size()/3);

    for(i=0;i<n;i++){


	run[i].lex = v.at(3*i);
	run[i].doc = v.at(3*i+1);
	run[i].pos = v.at(3*i+2);

	conta_triplas++;
    }
    //cout << endl;

    return (v.size()/3);
}

int Ordena::compara_triplas(tripla_t a,tripla_t b){

    if (a.lex < b.lex) return -1;
    if (a.lex > b.lex) return 1;
    if (a.lex == b.lex){
	if (a.doc < b.doc) return -1;
	if (a.doc > b.doc) return 1;
	return 0;
    }
    return 0;
}

void Ordena::merge_run(int i,int m,int f){
    int n1 = m-i;
    int n2 = f-m+1;

    tripla_t *esq,*dir;

    esq = new tripla_t[n1+1];
    dir = new tripla_t[n2+1];

    for(int k=0;k<n1;k++){
	esq[k].lex = run[i+k].lex;
	esq[k].doc = run[i+k].doc;
	esq[k].pos = run[i+k].pos;
    }

    esq[n1].lex = UINT_MAX;
    esq[n1].doc = UINT_MAX;
    esq[n1].pos = UINT_MAX;

    for(int k=0;k<n2;k++){
	dir[k].lex = run[m+k].lex;
	dir[k].doc = run[m+k].doc;
	dir[k].pos = run[m+k].pos;
    }

    dir[n2].lex = UINT_MAX;
    dir[n2].doc = UINT_MAX;
    dir[n2].pos = UINT_MAX;

    int j,l;
    j = l = 0;
    for(int k=i;k<=f;k++){
	if (compara_triplas(esq[j],dir[l])<=0){
	    run[k].lex = esq[j].lex;
	    run[k].doc = esq[j].doc;
	    run[k].pos = esq[j].pos;
	    j++;
	}else{
	    run[k].lex = dir[l].lex;
	    run[k].doc = dir[l].doc;
	    run[k].pos = dir[l].pos;
	    l++;
	}
    } 
    if (dir!=NULL) delete[] dir;
    if (esq!=NULL) delete[] esq;

}

void Ordena::ordena_run(int tam_run){
    //mergesort
    int m = 1;
    while(m<tam_run){
	int i = 0;
	while (i<(tam_run-m)){
	    merge_run(i,i+m,MIN(i+2*m-1,tam_run-1));
	    i = i + 2*m;
	}
	m = m*2;
    }
} 

 int Ordena::escreve_run(int tam_run){
    int inicio = conta_bits.back();
    int conta_bits_geral = inicio;
    int pos_arquivo;
    vector<unsigned int> v;
    //escrita.inicia_conta_bits(inicio);

   for(int i=0;i<tam_run;i++){

       v.push_back(run[i].lex);
       v.push_back(run[i].doc);
       v.push_back(run[i].pos);
       
   }

   //TODO: isso estava dentro do laco de repeticao
   pos_arquivo = escrita->escreve_tripla(v);

    return pos_arquivo;
}

void Ordena::ordena_todas_runs(){

     int pos_arquivo = 0;
     int i = 0;
     while (pos_arquivo >= 0){
	 cout << "Carregando run" << endl;
         int tam_run = carrega_run(pos_arquivo);
	 cout << "POS_ARQUIVO: " << pos_arquivo << " " << i << endl;
	 ordena_run(tam_run);
	 //Acho que esta ok aqui. Visto que as triplas estao ordenadas por run
	 /*for (int i=0;i<tam_run;i++){
	     cout << ">>> " << run[i].lex << " " << run[i].doc << " " << run[i].pos <<endl; 
	 }*/
         i++;
	 escreve_run(tam_run);
     }

    if(dynamic_cast<EscreveCompacta*>(escrita) != 0){
        if (escrita->pega_excedente()!=0)
          escrita->escreve_excedente();
    }


}

void Ordena::carrega_buffer_ordenacao(int* pos_prox){

    int num_conta_bits = conta_bits.size();
    deque<unsigned int> v;
    int termino_arquivo;

    for(int i=0;i<num_conta_bits;i++){
       
          leitura->inicia_conta_bits(conta_bits[i]);

          termino_arquivo = leitura->ler_tripla(v,3);
          buffer_ordenacao[i].pos = v.back();
          v.pop_back();
          buffer_ordenacao[i].doc = v.back();
          v.pop_back();
          buffer_ordenacao[i].lex = v.back();
          v.pop_back();

          if (termino_arquivo >= 0)
                  pos_prox[i] = leitura->pega_conta_bits();
          else pos_prox[i] = -leitura->pega_conta_bits();
     }
}

void Ordena::atualiza_buffer_ordenacao(int pos,int* pos_prox){

    deque<unsigned int> v;

    leitura->inicia_conta_bits(conta_bits[pos]);

    int pos_arquivo = leitura->ler_tripla(v,3);
    buffer_ordenacao[pos].pos = v.back();
    v.pop_back();
    buffer_ordenacao[pos].doc = v.back();
    v.pop_back();
    buffer_ordenacao[pos].lex = v.back();
    v.pop_back();

    if (pos_arquivo >0 )
       pos_prox[pos] = leitura->pega_conta_bits();
    else pos_prox[pos] = pos_arquivo;

}

void Ordena::executa(Colecao& col){
    

    //executa a ordenacao do indice em disco e atualiza 
    //a posicao da lista de termos em disco
    
    ordena_todas_runs();

    leitura->inicia_nome_arquivo(escrita->pega_nome_arquivo());

    tripla_t min;
    vector<int> limites(conta_bits);
    int min_conta_bits = 0;
    vector<unsigned int> v;

    int num_conta_bits = conta_bits.size();

    int old_lex,old_doc;
    int ntriplas = 0;
    int atualiza_conta_bits = escrita_ordenada->pega_conta_bits();
    int termino_arquivo = 1;
    int* flag_lex_visitado = new int[col.pega_tamanho_vocabulario()+1]();

    //TODO: parece ter mais lexico que o que pegou no vocabulario?
    cout << "NUMERO DE TRIPLAS: " << conta_triplas << " " << col.pega_tamanho_vocabulario()<< endl;

    buffer_ordenacao = new tripla_t[num_conta_bits]();
    int* pos_prox = new int[num_conta_bits]();

    carrega_buffer_ordenacao(pos_prox);

    //TODO: algum problema neste laco
    while(ntriplas<conta_triplas){
       //cout << "Inicio do Laco que escolhe menor tripla " << conta_triplas << " "<< ntriplas << endl;


	if (ntriplas > 0 ){
	   old_doc = min.doc;
	   old_lex = min.lex;
	}else{
	    old_lex = -1;
	    old_doc = -1;
	}

         min.lex = UINT_MAX;
         min.doc = UINT_MAX;
         min.pos = UINT_MAX;


        for(int i=0;i<num_conta_bits;i++){
	    //verificar se uma run nao 
	    //tem mais elemento
	    if (i!=(num_conta_bits-1)){
	       if (conta_bits[i]>=limites[i+1]){ 
		  // cout << "RUN " << i << " pulou de " << num_conta_bits << " RUNS "<< endl;
		   continue;
	       }
	    }else{
		if (conta_bits[i]<0) continue;
	    } 	
	
	    if (compara_triplas(buffer_ordenacao[i],min)<0){
		min.lex = buffer_ordenacao[i].lex;
		min.doc = buffer_ordenacao[i].doc;
		min.pos = buffer_ordenacao[i].pos;
		min_conta_bits = i;
	    }
        }


	//cout << "Minimo escolhido: " << min.lex << " " << min.doc << " " << min.pos << " " << flag_lex_visitado[min.lex] << endl;
	//se esta no final de conta bits tbm
	if (ntriplas != (conta_triplas-1) && (min_conta_bits < num_conta_bits)){
	    //na ultima tripla nao precisa acessar o proximo
	   conta_bits[min_conta_bits] = pos_prox[min_conta_bits];
	   atualiza_buffer_ordenacao(min_conta_bits,pos_prox);
	}
	


	//primeira vez que esbarro neste lexico
	if (flag_lex_visitado[min.lex] == 0){
	    if (old_lex == -1 && old_doc == -1){
		old_lex = min.lex;
		old_doc = min.doc;
	    }
	}


	//se trocar de documento.Escreve
	if (old_doc!=min.doc){
	    int num_pos = v.size();
	    v.insert(v.begin(),num_pos);
	    v.insert(v.begin(),old_doc);
            escrita_ordenada->escreve_tripla(v);
	    v.clear();
	}else{
	    //se nao trocou de documento, mas trocou de lexico. Escreve
	    if (old_lex!=min.lex){
	       int num_pos = v.size();
	       v.insert(v.begin(),num_pos);
	       v.insert(v.begin(),old_doc);
               escrita_ordenada->escreve_tripla(v);
	       v.clear();
	    }
	}

	//primeira vez que esbarro neste lexico
	if (flag_lex_visitado[min.lex] == 0){
	    //acho que este conta_bits nao presta mais porque vou escrever 
	    //em um novo arquivo
	    //cout << "Inicio Buffer do minimo: " << min.lex << " " << min.doc << " " << min.pos << " " << escrita_ordenada->pega_conta_bits_global()<< endl;
	    col.atualiza_vocabulario(min.lex,escrita_ordenada->pega_conta_bits_global());
	    flag_lex_visitado[min.lex] = 1;
	}

	v.push_back(min.pos);
    
	//cout << " " << escrita_ordenada.pega_conta_bits_global() << endl;

   //  cout << ">> " << ntriplas << " " << conta_triplas << endl;
     //  cout << "Ponta do Laco que escolhe menor tripla " << conta_triplas << " "<< ntriplas << endl;

	ntriplas++;
    }

   // cout << "Fim do laco " << conta_triplas << endl;

    //sera que no compactado anda escrevendo lixo?
    if (v.size()!=0){
	int num_pos = v.size();
	v.insert(v.begin(),num_pos);
	v.insert(v.begin(),old_doc);
        escrita_ordenada->escreve_tripla(v);
    }

   // cout << "Escreveu ultima tripla " << endl;

    if(dynamic_cast<EscreveCompacta*>(escrita) != 0 ){
        if (escrita_ordenada->pega_excedente()!=0)
         escrita_ordenada->escreve_excedente();
    }


     //as runs ordenadas estao em um arquivo ordenado
     //entao vou pegar o conteudo do arquivo temporario e 
     //sobrecrever o arquivo original
    const char* narquivo = nome_arquivo.c_str();
     const char* narquivo_run = escrita->pega_nome_arquivo().c_str();
     const char* narquivo_ordenado = escrita_ordenada->pega_nome_arquivo().c_str();

     remove(narquivo_run);
     rename(narquivo_ordenado,narquivo);

     if (flag_lex_visitado!=NULL) delete[] flag_lex_visitado;
     if (buffer_ordenacao!=NULL) delete[] buffer_ordenacao;
     if (pos_prox!=NULL) delete[] pos_prox;

}

/* int main(){
    //TODO: apagar arquivos temporarios
    Ordena o = Ordena("teste_comp4.bin");
    o.executa();
    return 0;
}*/



