#include <string.h>
#include <stdbool.h>
#include "fila.h"
#include <malloc.h>

typedef struct No {
    void *dado;
    struct No *next;
    struct No *prev;
} No;

struct fila
{
    int bytes;
    No* begin;
    No* end;
    No* current_pos; //Próximo nó
    int current_dir; // +1 vai pro fim, -1 vai pro início
};

Fila f_cria(int tam_do_dado){
    Fila new = malloc(sizeof(struct fila));
    new->begin = NULL;
    new->end = NULL;
    new->bytes = tam_do_dado;
    new->current_pos = NULL;
    new->current_dir = 1;
    return new;
}


void f_destrói(Fila self){
    if (self == NULL) return;

    No* aux = self->begin;
    No* next;

    // aqui percorre até o final da fila pra começar a destruição
    while (aux != NULL) 
    {
        next = aux->next;
    
        free(aux->dado);
        free(aux);

        aux = next;
    }
    
    free(self);
}


bool f_tá_vazia(Fila self){
    return self->begin == NULL;
}

void f_remove(Fila self, void *pdado){
    if(self == NULL || self->begin == NULL) return;
    
    No* aux = self->begin;
    if(pdado != NULL){
        memcpy(pdado, aux->dado, self->bytes);
    }

    self->begin = aux->next;
    if(self->begin == NULL){
        self->end = NULL;
    }else {
        self->begin->prev = NULL;
    }

    free(aux->dado);
    free(aux);
}

void f_insere(Fila self, void *pdado) {
    No *novo = malloc(sizeof(No));
    novo->dado = malloc(self->bytes);
    memcpy(novo->dado, pdado, self->bytes);
    novo->next = NULL;
    novo->prev = self->end;

    if (self->end == NULL) {
        self->begin = novo;
    } else {
        self->end->next = novo;
    }
    self->end = novo;
}

void f_inicia_percurso(Fila self, int pos_inical){
    if(pos_inical >= 0){
        self->current_dir = 1;
        self->current_pos = self->begin;
        for(int i = 0; i < pos_inical && self->current_pos != NULL; i++){
            self->current_pos = self->current_pos->next;
        }
    } else {
        self->current_dir = -1;
        self->current_pos = self->end;
        for(int i = 0; i < -pos_inical -1 && self->current_pos != NULL; i++){
            self->current_pos = self->current_pos->prev;
        }
    }
}

bool f_próximo(Fila self, void *pdado){
    if(self->current_pos == NULL) return false;

    if(pdado != NULL){
        memcpy(pdado, self->current_pos->dado, self->bytes);
    }

    if(self->current_dir == 1){
        self->current_pos = self->current_pos->next;
    }else{
        self->current_pos = self->current_pos->prev;
    }

    return true;
}

