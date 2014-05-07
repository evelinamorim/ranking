/*
 * =====================================================================================
 *
 *       Filename:  avaliacao.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/28/14 16:08:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "avaliacao.h"
Avaliacao::Avaliacao(int nr){
    num_respostas = nr;
}
void Avaliacao::inicia_em_comum(vector<double> ec){
    em_comum = ec;
}

vector<string> Avaliacao::ler_arquivo_links(ifstream& arquivo){
    string link;
    vector<string> v;

    link.reserve(TAM_LINK);

    while (getline(arquivo,link)){
	v.push_back(link);
    }

    return v;
}

vector<double>  Avaliacao::intersecao(vector<string> relevantes,vector<string> recuperados){
    vector<string>::iterator it_relevantes = relevantes.begin();
    vector<string>::iterator it_relevantes_fim = relevantes.end();

    vector<string>::iterator it_recuperados;
    vector<string>::iterator it_recuperados_fim = recuperados.end();

    vector<double> inter;
    double x;
    bool achou = false;
    while (it_relevantes!=it_relevantes_fim){
	x = 0;
	it_recuperados = recuperados.begin();
	while (it_recuperados!=it_recuperados_fim){
	    x++;
	    if (*it_relevantes == *it_recuperados){
		achou = true;
		break;
	    }
	    it_recuperados++;
	}
	//it_recuperados = find(recuperados.begin(),recuperados.end(),*it_relevantes);
	if (achou){
	    //cout<<"+++> " <<x<<endl;
	    pos_rel.push_back(x);
	    achou = false;
	}	
	//cout<<">> "<<ii<<endl;
	it_relevantes++;
    }

    sort(pos_rel.begin(),pos_rel.end());

    int limite = pos_rel.back();
    int ii = 0;
    int k = 0;
    int conta_em_comum = 1;
    while (ii<num_respostas){
	//cout<<"--> "<<pos_rel.size()<<" "<<endl;
	if (k<pos_rel.size() && pos_rel.at(k) == (ii+1)){
	    inter.push_back(conta_em_comum);
	    conta_em_comum++;
	    k++;
	}else{
	    inter.push_back(conta_em_comum);
	}
	ii++;
    }

    return inter;
}

vector<double>  Avaliacao::precisao(vector<string> relevantes,vector<string> recuperados){
    vector<string>::iterator it_relevantes = relevantes.begin();
    vector<string>::iterator it_relevantes_fim = relevantes.end();

    vector<double> v;
    double p;
    int ii = 0;
    int tam =  em_comum.size();
    int tam_pr = pos_rel.size();
    while(ii<num_respostas){
        int pr;
        if (ii<tam_pr){
	   pr = pos_rel.at(ii);
	   if ((pr-1)<tam){
	      // cout<<"==> "<<ii<<" "<<pos_rel.size()<<endl;
	      // cout<<"--> "<<pr<<" "<<em_comum.size()<<endl;
	      p = em_comum.at(pr-1)/pr;
	      // cout<<"--> "<<p<<" "<<em_comum.at(pr-1)<<" "<<pr<<endl;
	   }else{
	      p = 0;
	  }
	}else p =0;
	v.push_back(p);
	//cout<<"--> "<<p<<endl;
	ii++;
        //cout<<"==> "<<ii<<" "<<num_respostas<<" "<<tam<<endl;
    }
    return v;
}

vector<double>  Avaliacao::revocacao(vector<string> relevantes,vector<string> recuperados){
    vector<string>::iterator it_relevantes = relevantes.begin();
    vector<string>::iterator it_relevantes_fim = relevantes.end();

    vector<double> v;
    double r;
    int ii = 0;
    //rever isso, pois preciso pegar numero anterior
    double tam_relevantes = relevantes.size();
    double tam_pr = pos_rel.size();
    int tam_em_comum = em_comum.size();
    while(ii<num_respostas){
	if (ii<tam_pr){
	    int pr = pos_rel.at(ii);
	    if ((pr-1)<tam_em_comum){
	        r = em_comum.at(pr-1)/num_respostas;
		//cout<<"==> "<<r<<" "<<em_comum.at(pr-1)<<" "<<tam_relevantes<<endl;
	    }
	    else 
		r = r+0.1;;
	}else{
	    r = r+0.1;
	}
	v.push_back(r);
	ii++;
    }
    return v;
}

int main(int argc,char** argv){
    string nome_doc_arq = argv[1];
    string nome_relevantes_arq = argv[2];
    //string nome_doc = argv[3];
    //string saida = "grafos_vet/" + nome_doc + ".txt";
    Avaliacao av = Avaliacao(10);

    ifstream doc_arq(nome_doc_arq,ios::in);
    ifstream relevantes_arq(nome_relevantes_arq,ios::in);
    //ofstream nome_doc_fd(nome_doc,ios::out);

    vector<string> docs = av.ler_arquivo_links(doc_arq);
    vector<string> relevantes = av.ler_arquivo_links(relevantes_arq);

    //cout<< "Leu"<<endl;
    vector<double> inter = av.intersecao(relevantes,docs);
    //cout<<"Intersecao "<<endl;
    av.inicia_em_comum(inter);

    //cout<<"Precisao"<<endl;
    vector<double> p = av.precisao(relevantes,docs);
    //cout<<"Revocacao"<<endl;
    vector<double> r = av.revocacao(relevantes,docs);

    //cout<<"Saida"<<endl;
    int tam = p.size()<r.size()?p.size():r.size();
    int ii = 0;
    //cout<<"Recall Precision"<<endl;
    //nome_doc_fd<<"Recall Precision"<<endl;
    r.insert(r.begin(),0);
    double primeira_precisao = p.front();
    p.insert(p.begin(),primeira_precisao);
    while(ii<=tam){
	cout<<r.at(ii)<<" "<<p.at(ii)<<endl;
	//nome_doc_fd<<r.at(ii)<<" "<<p.at(ii)<<endl;
	ii++;
    }

    //nome_doc_fd.close();

    doc_arq.close();
    relevantes_arq.close();
    return 0;
}
