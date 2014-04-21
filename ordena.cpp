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
#include <ctime>

using namespace std;

int teste_conta[60];
int ntriplas_global;
int nrun_global;

Ordena::Ordena(string narquivo,bool compacta){
    nome_arquivo = narquivo;
    run = new tripla_t[TAMANHO_RUN];


    if (compacta){
	escrita = new EscreveCompacta(narquivo+"tmp");
	leitura_run = new LeCompacta(narquivo+"tmp");
	escrita_ordenada = new EscreveCompacta(narquivo+".ord");
	leitura = new LeCompacta(narquivo);
    }else{
	escrita = new EscreveNormal(narquivo+"tmp");
	leitura_run = new LeNormal(narquivo+"tmp");
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
    delete leitura_run;
    if (run !=NULL)  delete[] run;
}

int Ordena::carrega_run(int& pos_arquivo){

    unsigned long int conta_bits_leitura = leitura->pega_conta_bits();
    conta_bits.push_back(leitura->pega_conta_bits());
    deque<unsigned int> v;
    int i;
    leitura->inicia_ntripla(nrun_global);
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
    streampos pos_arquivo;
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

void Ordena::ordena_todas_runs(unsigned long int* pos_prox){

     int pos_arquivo = 1;
     int i = 0;
     unsigned long int old_pos_prox = 0;

     while (pos_arquivo > 0){
	 nrun_global = i;
         int tam_run = carrega_run(pos_arquivo);
         //cout<<"RUN "<<i<<"  "<<tam_run<<" "<<pos_arquivo<<" "<<conta_triplas<<" "<<conta_bits[i]<<endl;
	 ordena_run(tam_run);
	 //Acho que esta ok aqui. Visto que as triplas estao ordenadas por run
	 /* for (int j=0;j<tam_run;j++){
	 if (run[j].lex>=3726028 && run[j].lex<=3726055){
	     cout << ">>> " << run[j].lex << " " << run[j].doc << " " << run[j].pos <<" "<<j<<endl; }
	 }*/

	 pos_prox[i] = old_pos_prox;

	 for(int j = 0;j<MIN(tam_run,TAM_BUFFER_ORD);j++){

	     buffer_ordenacao[i].push_back(run[j]);


	     unsigned long int y;
	     unsigned int ny;
            //pos prox tem que considerar o offset anterior!
            if(dynamic_cast<EscreveCompacta*>(escrita) != 0){
		para_codigo_gamma(run[j].lex,y,ny);
		pos_prox[i] += ny;
		para_codigo_gamma(run[j].doc,y,ny);
		pos_prox[i] += ny;
		para_codigo_gamma(run[j].pos,y,ny);
		pos_prox[i] += ny;
	    }else{
		pos_prox[i] += 96;
	    }
	 }

	 escrita->inicia_ntripla(i);
	 escreve_run(tam_run);
	//cout<<"RUN "<<i<<" termina em "<< escrita->pega_conta_bits_global()<<" "<<pos_prox[i]<<endl;
	 old_pos_prox = escrita->pega_conta_bits_global();
         i++;
     }
	 escrita->inicia_ntripla(0);


    if(dynamic_cast<EscreveCompacta*>(escrita) != 0){
        if (escrita->pega_excedente()!=0)
          escrita->escreve_excedente();
    }
    escrita->fecha_arquivo();
     conta_bits.push_back(escrita->pega_conta_bits_global());
   leitura_run->inicia_tamanho_arquivo();

}


void Ordena::atualiza_buffer_ordenacao(int pos,unsigned long int* pos_prox,vector<unsigned long int> limites){

    deque<unsigned int> v;

    if (conta_bits[pos] < limites[pos+1]){
        int tamanho_buffer = buffer_ordenacao[pos].size();
        if (tamanho_buffer <= 1){

              leitura_run->inicia_ntripla(pos);
	      buffer_ordenacao[pos].clear();
	      /*if (pos==60 && teste_conta[pos] >= 1000000){
		  cout<<"1 >>> "<<conta_bits[pos]<<" "<<limites[pos+1]<<" "<<leitura_run->pega_conta_bits()<<endl;
	      }*/
              leitura_run->inicia_conta_bits(conta_bits[pos]);
              int pos_arquivo = leitura_run->ler_tripla(v,3*TAM_BUFFER_ORD);
	      /*if (pos==60 && teste_conta[pos] >= 1000000){
		  cout<<"2 >>> "<<conta_bits[pos]<<" "<<limites[pos+1]<<" "<<leitura_run->pega_conta_bits()<<endl;
	      }*/


	      int n = v.size()/3;
	      tripla_t tt;
	      for(int i=0;i<n;i++){
                  tt.lex = v.at(3*i);
                  tt.doc = v.at(3*i+1);
                  tt.pos = v.at(3*i+2);
	          buffer_ordenacao[pos].push_back(tt);
	      }
	      
	      
            pos_prox[pos] = leitura_run->pega_conta_bits();
	    conta_bits[pos] = pos_prox[pos];
	      /*if (pos==60 && teste_conta[pos] >= 1000000){
		  cout<<"3 >>> "<<conta_bits[pos]<<" "<<limites[pos+1]<<" "<<leitura_run->pega_conta_bits()<<endl;
	      }*/

	}else{
	         buffer_ordenacao[pos].pop_front();
        }
    }else{
	if (buffer_ordenacao[pos].size()!=0) buffer_ordenacao[pos].pop_front();
    }
}

void Ordena::executa(Colecao& col){
    

    //executa a ordenacao do indice em disco e atualiza 
    //a posicao da lista de termos em disco

    buffer_ordenacao = new deque<tripla_t>[QTD_RUNS]();
    unsigned long int pos_prox[QTD_RUNS];

    clock_t t;
    t = clock();
    ordena_todas_runs(pos_prox);
    int num_conta_bits = conta_bits.size()-1;
    t = clock() -t;
    cout << "Tempo Ordena::ordena_todas_runs: "<< ((float)t/CLOCKS_PER_SEC) << "s" << endl;

    leitura->inicia_nome_arquivo(escrita->pega_nome_arquivo());

    tripla_t min;
    vector<unsigned long int> limites(conta_bits);

    //atualizar conta_bits com pos_prox
    for (int k = 0;k<conta_bits.size();k++){
	conta_bits[k] = pos_prox[k];
    }

    int min_conta_bits = 0;
    deque<unsigned int> v;
    vector<unsigned int> tmp_v;

    tmp_v.reserve(TAM_BUFFER_ORD);


    int old_lex,old_doc,old_pos,tamanho_voc;
    int ntriplas = 0;
    ntriplas_global = ntriplas;
    int atualiza_conta_bits = escrita_ordenada->pega_conta_bits();
    int termino_arquivo = 1;

    tamanho_voc = col.pega_tamanho_vocabulario();
    int* flag_lex_visitado = new int[tamanho_voc+1];

    memset(flag_lex_visitado,0,sizeof(int)*(tamanho_voc+1));

    cout << "NUMERO DE TRIPLAS: " << conta_triplas << " " << col.pega_tamanho_vocabulario()<< " "<<col.pega_tamanho_vocabulario_invertido() << endl;

    t = clock();


    int num_pos = 0;
    for(int k = 0;k<60;k++)
           teste_conta[k] = 0;
    while(ntriplas<conta_triplas){


	if (ntriplas > 0 ){
	   old_doc = min.doc;
	   old_lex = min.lex;
	   old_pos = min.pos;
	}else{
	    old_lex = -1;
	    old_doc = -1;
	    old_pos = -1;
	}

         min.lex = UINT_MAX;
         min.doc = UINT_MAX;
         min.pos = UINT_MAX;
        
        for(int i=0;i<num_conta_bits;i++){
	    //verificar se uma run nao 
	    //tem mais elemento
	    //Todos estao vazios: porque?ver como esta atualizando mesmo
	    //buffer 3 foi o ultimo
	    if (buffer_ordenacao[i].empty()){ 
		continue;
	    }

	    //cout<<"buffer_ordenacao["<<i<<"] " << buffer_ordenacao[i].front().lex <<" "<<buffer_ordenacao[i].front().doc<<" "<<buffer_ordenacao[i].front().pos<<endl;

	    if (compara_triplas(buffer_ordenacao[i].front(),min)<0){
		min.lex = buffer_ordenacao[i].front().lex;
		min.doc = buffer_ordenacao[i].front().doc;
		min.pos = buffer_ordenacao[i].front().pos;
		min_conta_bits = i;
	    }
        }

	/* if (teste_conta[17]>=9900000){
	    cout << "Minimo escolhido: (" <<ntriplas<<") "<< min.lex << " " << min.doc << " " << min.pos << " " << flag_lex_visitado[min.lex] << old_lex<< endl;
	    cout<<"Frente (17) "<<buffer_ordenacao[17].front().lex<<" "<<buffer_ordenacao[17].front().doc<<" "<<buffer_ordenacao[17].front().pos<<" "<<teste_conta[17]<<endl;
	}*/





        if (tamanho_voc<min.lex){
	    //e nem entrou aqui! porque esta acontecendo isso?
	    cout <<"OUCH: "<<col.pega_tamanho_vocabulario()<<" "<<min.lex<<" "<<ntriplas<<" buffer no.: "<<min_conta_bits<<" "<<buffer_ordenacao[min_conta_bits].size()<<endl;
	}

        teste_conta[min_conta_bits]++;

	//se esta no final de conta bits tbm
	    //na ultima tripla nao precisa acessar o proximo
	 atualiza_buffer_ordenacao(min_conta_bits,pos_prox,limites);

	 /* if (min.lex>=3726028 && min.lex<=3726055)
	 cout << "Minimo escolhido: (" <<ntriplas<<") "<< min.lex << " " << min.doc << " " << min.pos << " " << flag_lex_visitado[min.lex] << old_lex<< endl;
	 */

	  

	//primeira vez que esbarro neste lexico
	if (flag_lex_visitado[min.lex] == 0){
	    if (old_lex == -1 && old_doc == -1){
		old_lex = min.lex;
		old_doc = min.doc;
		old_pos = min.pos;
	    }
	}


	//se trocar de documento. Acumular no vetor 
	//junto com o tamanho
	//TODO: Vai dar errado isso aqui. rever
	    //Escrever apenas quando mudar de lexico
        if (old_lex!=min.lex){
	       v.push_front(num_pos);
	       v.push_front(old_doc);
	       copy(v.begin(), v.end(), back_inserter(tmp_v));

               escrita_ordenada->escreve_tripla(tmp_v);
	       tmp_v.clear();
	       v.clear();
	       num_pos = 0;
	    
	}else{
	     if (old_doc!=min.doc){
	         v.push_front(num_pos);
	         v.push_front(old_doc);
	         copy(v.begin(), v.end(), back_inserter(tmp_v));
	         if (tmp_v.size()>TAM_BUFFER_ORD){
                     escrita_ordenada->escreve_tripla(tmp_v);
		     tmp_v.clear();
	         }
	         v.clear();

	         num_pos = 0;
	    }
	}

	//primeira vez que esbarro neste lexico
	if (flag_lex_visitado[min.lex] == 0){
	    //acho que este conta_bits nao presta mais porque vou escrever 
	    //em um novo arquivo
	    col.atualiza_vocabulario(min.lex,escrita_ordenada->pega_conta_bits_global());
	    flag_lex_visitado[min.lex] = 1;
	    //cout << "Inicio Buffer do minimo: " <<col.pega_lexico(56092)<<" "<< min.lex << " " << min.doc << " " << min.pos << " " << escrita_ordenada->pega_conta_bits_global()<<" "<<old_lex<<" "<<ntriplas<< endl;
	}

	v.push_back(min.pos);
	num_pos++;
    
	//cout << " " << escrita_ordenada.pega_conta_bits_global() << endl;

        //cout << ">> " << ntriplas << " " << conta_triplas << endl;
       //cout << "Ponta do Laco que escolhe menor tripla " << conta_triplas << " "<< ntriplas << endl;

	ntriplas++;
	ntriplas_global = ntriplas;
    }

    //cout<<"Teste do buffer vazio"<<endl;
    //for(int k = 0;k<num_conta_bits;k++){
	//cout<<"-->"<<k<<" "<<tese_conta[k]<<" "<<buffer_ordenacao[k].size()<<endl;
    //}

    t = clock() -t;
    cout << "Tempo Ordena::executa "<< ((float)t/CLOCKS_PER_SEC) << "s" << endl;

   // cout << "Fim do laco " << conta_triplas << endl;

    //sera que no compactado anda escrevendo lixo?
    if (v.size()!=0){
	v.push_front(num_pos);
	v.push_front(old_doc);
	vector<unsigned int> tmp_v;
	copy(v.begin(), v.end(), back_inserter(tmp_v));
        escrita_ordenada->escreve_tripla(tmp_v);
    }

   // cout << "Escreveu ultima tripla " << endl;

    if(dynamic_cast<EscreveCompacta*>(escrita) != 0 ){
        if (escrita_ordenada->pega_excedente()!=0)
         escrita_ordenada->escreve_excedente();
    }
    escrita_ordenada->fecha_arquivo();


     //as runs ordenadas estao em um arquivo ordenado
     //entao vou pegar o conteudo do arquivo temporario e 
     //sobrecrever o arquivo original
     char narquivo[100]; 
     memset(narquivo,0,100);
     strcpy(narquivo,nome_arquivo.c_str());

     char narquivo_run[100]; 
     memset(narquivo_run,0,100);
     strcpy(narquivo_run,escrita->pega_nome_arquivo().c_str());

     char narquivo_ordenado[100]; 
     memset(narquivo_ordenado,0,100);
     strcpy(narquivo_ordenado,escrita_ordenada->pega_nome_arquivo().c_str());

     if (narquivo_run!=NULL) remove(narquivo_run);
     if (narquivo_ordenado!=NULL && narquivo!=NULL) rename(narquivo_ordenado,narquivo);

     if (flag_lex_visitado!=NULL) delete[] flag_lex_visitado;
     if (buffer_ordenacao!=NULL) delete[] buffer_ordenacao;

}

