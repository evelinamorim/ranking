/*
 * =====================================================================================
 *
 *       Filename:  le.h
 *
 *    Description:  arquivo header para leitura de arquivos de indice invertido
 *
 *        Version:  1.0
 *        Created:  02/18/14 23:08:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <deque>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>

using namespace std;

#ifndef __LE_H_
#define __LE_H_

class Le{
    protected:
    deque<unsigned int> buffer;
    string nome_arquivo;
    unsigned long int conta_bits;
    streampos tamanho_arquivo;

    unsigned long int tmp_ntripla;

    public:
    Le(string narquivo);
    virtual ~Le(){};
    //metodos de get's e set's
    void inicia_nome_arquivo(string narquivo);
    void inicia_tamanho_arquivo();
    void inicia_conta_bits(unsigned long int cb);
    void inicia_ntripla(unsigned long int x);
    unsigned long int pega_conta_bits();
    unsigned long int pega_ntripla();
    streampos pega_tamanho_arquivo();

    int ler_tripla(deque<unsigned int>& v,int nnum);
    int ler_tripla_pos(deque<unsigned int>& v,unsigned long int pos);

    //metodos virtuais
    virtual int ler_numero() =0;
    virtual void carrega_buffer(ifstream& arquivo,int nnum) =0;
};


class LeNormal:public Le{
    public:
    LeNormal(string narquivo);
    ~LeNormal();
    int ler_numero();
    void carrega_buffer(ifstream& arquivo,int nnum);
};

class LeCompacta: public Le{
    public:
    LeCompacta(string narquivo);
    ~LeCompacta();
    int ler_numero();
    void carrega_buffer(ifstream& arquivo,int nnum);
};
#endif
