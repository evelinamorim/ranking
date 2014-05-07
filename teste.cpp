/*
 * =====================================================================================
 *
 *       Filename:  teste.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/06/14 21:42:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <string>
#include <algorithm>
#include <cstdio>

using namespace std;

#define TAM 4

int main(){
    string t= "evelin eh legal";
    char *s = new char[5];

    memset(s,'\0',sizeof(char)*5);
    //copy(t.begin(),t+4,s);
    sprintf(s,"%.4s",t.c_str());
    cout<<s<<endl;
}
