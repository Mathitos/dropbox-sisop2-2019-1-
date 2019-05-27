#include "fila2.h"
#include <stdio.h>
#include <stdlib.h>
#define ERROR 1

int CreateFila2(PFILA2 pFila) {
    //Aloca o espaço de memória para a fila e seta seus ponteiros para NULL
    pFila->it = NULL;
    pFila->first = NULL;
    pFila->last = NULL;
    return 0;
}

int	FirstFila2(PFILA2 pFila) {
    //Testa se a fila existe e se há um first element na fila
    if(pFila == NULL || pFila->first == NULL)
        return 1;
    //Fila existente e não-vazia.
    else {
        pFila->it = pFila->first;
        return 0;
    }
}

int	LastFila2(PFILA2 pFila) {
    //Testa se a fila existe e se há um last element na fila
    if(pFila != NULL && pFila->last != NULL) {
        pFila->it = pFila->last;
        return 0;
    }
    //Fila não existente ou é vazia
    else
        return 1;
}

int	NextFila2(PFILA2 pFila) {
    //Se a fila existir, e o iterador aponta para um nodo
    if(pFila != NULL && pFila->it != NULL){
        //Se não há próximo na fila, retorna 0 e printa que nao há próximo
        if (pFila->it->next != NULL){
            pFila->it = pFila->it->next;
            return 0;
        }
    }
    return ERROR;
}

void *GetAtIteratorFila2(PFILA2 pFila) {
    if(pFila != NULL && pFila->it != NULL) {
        return pFila->it->node;
    }
    else
        return NULL;
}

int	AppendFila2(PFILA2 pFila, void *content){
    PNODE2 novoNodo = (PNODE2)malloc(sizeof(NODE2));
    //Fila existe
    if(pFila != NULL) {
        //Seta os valores do Nodo
        novoNodo->node = content;
        novoNodo->next = NULL;
        //Se é o primeiro
        if(pFila->first == NULL){
            pFila->it = novoNodo;
            pFila->first = novoNodo;
            novoNodo->ant = NULL;
            pFila->last = novoNodo;
        }
        //Se não é
        else {
            novoNodo->ant = pFila->last;
            pFila->last->next = novoNodo;
            pFila->last = novoNodo;
        }
        return 0;
    }
    else
        return ERROR;
}

int	InsertAfterIteratorFila2(PFILA2 pFila, void *content) {
    PNODE2 novoNodo = (PNODE2)malloc(sizeof(NODE2));
    if(pFila != NULL && pFila->it != NULL) {
        novoNodo->node = content;
        novoNodo->next = pFila->it->next;
        novoNodo->ant = pFila->it;
        pFila->it->next = novoNodo;
        novoNodo->next->ant = novoNodo;
        return 0;
        
    }
    else
        return ERROR;
}

int	DeleteAtIteratorFila2(PFILA2 pFila) {
    if(pFila != NULL && pFila->it != NULL){
        if(pFila->first != pFila->last) {
            PNODE2 aux = (PNODE2)malloc(sizeof(NODE2));
            aux = pFila->it;
            if(pFila->first == pFila->it) {
                pFila->first = pFila->it->next;
                pFila->first->ant = NULL;
                pFila->it = aux->next;
            }
            else if(pFila->last == pFila->it){
                pFila->last = pFila->it->ant;
                pFila->last->next = NULL;
                pFila->it = NULL;
            }
            else {
                pFila->it->ant->next = pFila->it->next;
                pFila->it->next->ant = pFila->it->ant;	
                pFila->it = aux->next;
            }
            free(aux);
            aux = NULL;
            return 0;
        }
        else {
            CreateFila2(pFila);
            return 0;
        }
    }
    else
        return 1;
}

