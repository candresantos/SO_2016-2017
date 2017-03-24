#Grupo 35

SRC=src/
BIN=bin/
OBJ=obj/
INC=include/
IN=testes/in/
OUT=testes/out/
LOG=testes/log/

OBJECTS=$(OBJ)assistente.o $(OBJ)cliente.o $(OBJ)controlo.o $(OBJ)escalonador.o $(OBJ)ficheiro.o $(OBJ)loja.o $(OBJ)main.o $(OBJ)memoria.o $(OBJ)prodcons.o $(OBJ)tempo.o $(OBJ)so.o
LIB=-lpthread -lrt
XFLAGS=$(LIB) -o

ASSISTENTE=$(SRC)assistente.c $(INC)assistente.h $(INC)main.h $(INC)memoria.h
CLIENTE=$(SRC)cliente.c $(INC)main.h $(INC)memoria.h $(INC)tempo.h $(INC)cliente.h $(INC)ficheiro.h
CONTROLO=$(SRC)controlo.c $(INC)main.h $(INC)so.h $(INC)memoria.h $(INC)prodcons.h $(INC)controlo.h
ESCALONADOR=$(SRC)escalonador.c $(INC)main.h $(INC)so.h  $(INC)escalonador.h
FICHEIRO=$(SRC)ficheiro.c $(INC)main.h $(INC)so.h $(INC)memoria.h $(INC)escalonador.h $(INC)ficheiro.h $(INC)prodcons.h $(INC)tempo.h
LOJA=$(SRC)loja.c $(INC)loja.h $(INC)main.h $(INC)memoria.h $(INC)tempo.h
MAIN=$(SRC)main.c $(INC)assistente.h $(INC)loja.h $(INC)main.h $(INC)cliente.h $(INC)memoria.h $(INC)prodcons.h $(INC)controlo.h $(INC)ficheiro.h $(INC)tempo.h $(INC)so.h
MEMORIA=$(SRC)memoria.c $(INC)main.h $(INC)so.h $(INC)memoria.h $(INC)prodcons.h $(INC)controlo.h $(INC)escalonador.h $(INC)ficheiro.h $(INC)tempo.h
PRODCONS=$(SRC)prodcons.c $(INC)main.h $(INC)so.h $(INC)controlo.h $(INC)prodcons.h
TEMPO=$(SRC)tempo.c $(INC)main.h $(INC)so.h $(INC)tempo.h
OFLAGS=-c -Iinclude -o

$(BIN)sostore: $(OBJECTS)
	gcc $^ $(XFLAGS) $@
	
$(OBJ)assistente.o:	$(ASSISTENTE)
	gcc $< $(OFLAGS) $@
	
$(OBJ)cliente.o: $(CLIENTE)
	gcc $< $(OFLAGS) $@
	
$(OBJ)controlo.o: $(CONTROLO)
	gcc $< $(OFLAGS) $@
	
$(OBJ)escalonador.o: $(ESCALONADOR)
	gcc $< $(OFLAGS) $@
	
$(OBJ)ficheiro.o: $(FICHEIRO)
	gcc $< $(OFLAGS) $@
	
$(OBJ)loja.o: $(LOJA)
	gcc $< $(OFLAGS) $@
	
$(OBJ)main.o: $(MAIN)
	gcc $< $(OFLAGS) $@
	
$(OBJ)memoria.o: $(MEMORIA)
	gcc $< $(OFLAGS) $@
	
$(OBJ)prodcons.o: $(PRODCONS)
	gcc $< $(OFLAGS) $@
	
$(OBJ)tempo.o: $(TEMPO)
	gcc $< $(OFLAGS) $@
	
clean:
	rm -f $(OBJ)???*.o
	rm -f $(BIN)sostore
	rm -f $(OUT)*
	rm -f $(LOG)*

teste:
	./soG35.sh $(IN)cenario1 $(IN)cenario2 $(IN)cenario3 $(IN)cenario4
