/*
 * =====================================================================================
 *
 *       Filename:  util.h
 *
 *    Description:  biblioteca de funcoes uteis
 *
 *        Version:  1.0
 *        Created:  02/10/14 14:20:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  evelin amorim, 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <time.h>
#include <algorithm>
#include <string>
#include <deque>
#include <vector>
#include <sys/stat.h>
#include <cmath>
#include <unordered_map>

#ifndef __UTIL_H_
#define __UTIL_H_

#define MAIOR_PALAVRA 20
#define MAIOR_LINK 200
#define TAM_VOC 20000000
//TODO: depois mudar este tamanho. No momento eh soh para teste
#define TAMANHO_RUN 10000000
#define QTD_RUNS 5000
#define TAM_COL 945642

#define TAM_BUFFER_ORD 100000

//constante dumping factor do pagerank
#define D_FACTOR 0.85
//constante definida como numero de iteracoes que o pagerank deve executar
#define ITER_PR 50

using namespace std;

void imprime_string(string s);

void converteParaMinusculo_char(char* s);
void converteParaMinusculo(string& s);

void tokenizar(string s,vector<string>& v);

bool ehPontuacao(char c);

bool arquivoExiste(const string nomeArquivo);

int para_codigo_unario(unsigned int x);

void para_codigo_gamma(unsigned int x,unsigned long int& y,unsigned int &ny);


unsigned int unario_para_int(deque<unsigned int>& x,int pos);

unsigned int gamma_para_int(deque<unsigned int>& x,unsigned int& nx,int pos);

/* funcoes para operacao de ranking */
vector<float> frequencia_termo(unordered_map<unsigned int,vector<unsigned int> > lista_docs);
float frequencia_inversa(const int num_docs,unordered_map<unsigned int,vector<unsigned int> > lista_docs);

/* funcoes para operacao em vetor */
unsigned int produto_vetorial(vector<unsigned int> v1,vector<unsigned int> v2);
unsigned int norma(vector<unsigned int> v);

#endif
