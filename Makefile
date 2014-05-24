ricode = /Users/evelinamorim/Dropbox/UFMG/2014-1/RI/tp1/code/src/
urlcode = oneurl-master/include/
ridata = /Users/evelinamorim/Documents/Ufmg/2014-1/RI/
riindex = index.txt

LIB = -L oneurl-master/output/lib/ -L/usr/local/opt/icu4c/lib -loneurl -licuuc -licudata  -rdynamic  -ldl -lz -lhtmlcxx -ggdb

ziplib : $(ricode) 
	g++ -O0 -c -o $(ricode)/CollectionReader.o $(ricode)/CollectionReader.cpp -ggdb
	g++ -O0  -c -o $(ricode)/CollectionWriter.o $(ricode)/CollectionWriter.cpp -ggdb
	g++ -O0  -c -o $(ricode)/Document.o $(ricode)/Document.cpp -ggdb

pesquisa: $(ricode)
	g++ -O0  -c -o pesquisa.o pesquisa.cpp -I $(ricode)  -g
	g++ -O0  -c -o colecao.o colecao.cpp -I $(ricode) -I $(urlcode) -g
	g++ -O0  -c -o util.o util.cpp -I $(ricode)  -g
	g++ -O0  -c -o le.o le.cpp -I $(ricode)  -g
	g++ -O0  -c -o escreve.o escreve.cpp -I $(ricode)  -g
	g++ -O0  -c -o ordena.o ordena.cpp -I $(ricode)  -g
	g++ -O0  -c -o ranking.o ranking.cpp -I $(ricode)  -g
	g++ -O0  -o pesquisa pesquisa.o colecao.o util.o le.o escreve.o ordena.o ranking.o $(ricode)/CollectionReader.o $(ricode)/Document.o $(LIB)

#compila ricode
index : $(ricode)
	g++ -O0 -c -o index.o index.cpp -I $(ricode) -ggdb
	g++ -O0  -c -o colecao.o colecao.cpp -I $(ricode) -I $(urlcode) -ggdb
	g++ -O0  -c -o util.o util.cpp -I $(ricode) -ggdb
	g++ -O0  -c -o le.o le.cpp -I $(ricode) -ggdb
	g++ -O0  -c -o escreve.o escreve.cpp -I $(ricode) -ggdb
	g++ -O0  -c -o ordena.o ordena.cpp -I $(ricode)  -ggdb
	g++ -O0  -o index index.o colecao.o util.o le.o escreve.o ordena.o $(ricode)/CollectionReader.o $(ricode)/Document.o $(LIB)

clean :
	rm *.o
	rm pesquisa
	rm index
	rm index.bin*
	rm voc.txt

all : ziplib index pesquisa

teste : $(ridata) 
	./pesquisa $(ridata) $(riindex) -c < consultas.txt
