

vector<double> Colecao::computa_info_links(string dirEntrada,string nomeIndice){
    //computa o pr das paginas
    ifstream arquivo_link(dirEntrada+nomeIndice,ios::in);
    vector<vector<int> > back_links;
    char* link_tmp = new char [MAIOR_LINK+2];
    vector<double> pr;

    if (arquivo_link.is_open()){
	//inicia matriz de backlinks
	while(!arquivo_link.eof()){
	    //...
	}

	vector<double> old_pr(pr);

	int ii = 0;
	//a partir dos backlinks calcular o pagerank
	while(ii< ITER_PR){//calcular x vezes para cad a pagina
	     int jj = 0;
	     //iterando sobre cada pagina
	     while (jj<back_links.size()){
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

    if (link_tmp!=NULL) delete[] link_tmp;

    return pr;
}



