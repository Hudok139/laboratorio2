#include <stdlib.h>
#include <string.h>
#include "grafo.h"

typedef struct Aresta {
    int destino;
    void *dado;
    struct Aresta *next;
} Aresta;

typedef struct {
    void *dado;
    Aresta *arestas;   // as que partem deste nó
} Nó;

struct grafo {
    int tam_nó, tam_aresta;
    int n, cap;
    Nó *nós;
    // estado da consulta de arestas
    int q_nó;          // nó da consulta
    int q_origem;      // (só "chegam") nó de origem sendo varrido
    Aresta *q_atual;   // próxima aresta a devolver
};

static void libera_arestas(Aresta *a) {
    while (a != NULL) {
        Aresta *prox = a->next;
        free(a->dado);
        free(a);
        a = prox;
    }
}

// remove a aresta que vai para 'destino' da lista de n; retorna se removeu
static bool remove_aresta(Nó *n, int destino) {
    Aresta *ant = NULL;
    for (Aresta *a = n->arestas; a != NULL; ant = a, a = a->next) {
        if (a->destino == destino) {
            if (ant == NULL) n->arestas = a->next;
            else ant->next = a->next;
            free(a->dado);
            free(a);
            return true;
        }
    }
    return false;
}

Grafo g_cria(int tam_nó, int tam_aresta) {
    Grafo g = malloc(sizeof(struct grafo));
    g->tam_nó = tam_nó;
    g->tam_aresta = tam_aresta;
    g->n = 0;
    g->cap = 8;
    g->nós = malloc(g->cap * sizeof(Nó));
    g->q_atual = NULL;
    return g;
}

void g_destrói(Grafo self) {
    for (int i = 0; i < self->n; i++) {
        free(self->nós[i].dado);
        libera_arestas(self->nós[i].arestas);
    }
    free(self->nós);
    free(self);
}

int g_insere_nó(Grafo self, void *p_dado) {
    if (self->n == self->cap) {
        self->cap *= 2;
        self->nós = realloc(self->nós, self->cap * sizeof(Nó));
    }
    Nó *novo = &self->nós[self->n];
    novo->dado = malloc(self->tam_nó);
    memcpy(novo->dado, p_dado, self->tam_nó);
    novo->arestas = NULL;
    return self->n++;
}

void g_remove_nó(Grafo self, int nó) {
    free(self->nós[nó].dado);
    libera_arestas(self->nós[nó].arestas);
    // desloca os nós seguintes
    for (int i = nó; i < self->n - 1; i++) {
        self->nós[i] = self->nós[i + 1];
    }
    self->n--;
    // limpa arestas que chegavam em 'nó' e renumera destinos maiores
    for (int i = 0; i < self->n; i++) {
        remove_aresta(&self->nós[i], nó);
        for (Aresta *a = self->nós[i].arestas; a != NULL; a = a->next) {
            if (a->destino > nó) a->destino--;
        }
    }
}

void g_altera_valor_do_nó(Grafo self, int nó, void *p_dado) {
    memcpy(self->nós[nó].dado, p_dado, self->tam_nó);
}

void *g_valor_do_nó(Grafo self, int nó) {
    return self->nós[nó].dado;
}

int g_num_nós(Grafo self) {
    return self->n;
}

void g_altera_valor_da_aresta(Grafo self, int nó_origem, int nó_destino,
                              void *p_dado) {
    Nó *o = &self->nós[nó_origem];
    if (p_dado == NULL) {
        remove_aresta(o, nó_destino);
        return;
    }
    Aresta *a = g_valor_da_aresta(self, nó_origem, nó_destino) == NULL
                    ? NULL : o->arestas;
    // acha a aresta existente (se houver)
    for (a = o->arestas; a != NULL; a = a->next) {
        if (a->destino == nó_destino) break;
    }
    if (a == NULL) {
        a = malloc(sizeof(Aresta));
        a->destino = nó_destino;
        a->dado = malloc(self->tam_aresta);
        a->next = o->arestas;
        o->arestas = a;
    }
    memcpy(a->dado, p_dado, self->tam_aresta);
}

void *g_valor_da_aresta(Grafo self, int nó_origem, int nó_destino) {
    for (Aresta *a = self->nós[nó_origem].arestas; a != NULL; a = a->next) {
        if (a->destino == nó_destino) return a->dado;
    }
    return NULL;
}

void g_arestas_que_partem(Grafo self, int nó_origem) {
    self->q_nó = nó_origem;
    self->q_origem = -1;   // -1 = modo "partem"
    self->q_atual = self->nós[nó_origem].arestas;
}

// modo "chegam": avança q_atual até uma aresta com destino == q_nó
static void acha_chegam(Grafo self) {
    while (self->q_origem < self->n) {
        while (self->q_atual != NULL && self->q_atual->destino != self->q_nó) {
            self->q_atual = self->q_atual->next;
        }
        if (self->q_atual != NULL) return;
        self->q_origem++;
        self->q_atual = self->q_origem < self->n
                            ? self->nós[self->q_origem].arestas : NULL;
    }
}

void g_arestas_que_chegam(Grafo self, int nó_destino) {
    self->q_nó = nó_destino;
    self->q_origem = 0;
    self->q_atual = self->n > 0 ? self->nós[0].arestas : NULL;
    acha_chegam(self);
}

void *g_próxima_aresta(Grafo self, int *p_nó_vizinho) {
    Aresta *a = self->q_atual;
    if (a == NULL) return NULL;
    if (self->q_origem == -1) {            // partem
        if (p_nó_vizinho) *p_nó_vizinho = a->destino;
        self->q_atual = a->next;
    } else {                               // chegam
        if (p_nó_vizinho) *p_nó_vizinho = self->q_origem;
        self->q_atual = a->next;
        acha_chegam(self);
    }
    return a->dado;
}

// Kahn: fila vazia se houver ciclo
Fila g_ordem_topológica(Grafo self) {
    int n = self->n;
    int *grau = calloc(n > 0 ? n : 1, sizeof(int));
    for (int i = 0; i < n; i++) {
        for (Aresta *a = self->nós[i].arestas; a != NULL; a = a->next) {
            grau[a->destino]++;
        }
    }
    Fila trabalho = f_cria(sizeof(int));
    Fila resultado = f_cria(sizeof(int));
    for (int i = 0; i < n; i++) {
        if (grau[i] == 0) f_insere(trabalho, &i);
    }
    int total = 0;
    while (!f_tá_vazia(trabalho)) {
        int v;
        f_remove(trabalho, &v);
        f_insere(resultado, &v);
        total++;
        for (Aresta *a = self->nós[v].arestas; a != NULL; a = a->next) {
            if (--grau[a->destino] == 0) f_insere(trabalho, &a->destino);
        }
    }
    f_destrói(trabalho);
    free(grau);
    if (total < n) {
        f_destrói(resultado);
        resultado = f_cria(sizeof(int));
    }
    return resultado;
}

bool g_tem_ciclo(Grafo self) {
    Fila f = g_ordem_topológica(self);
    bool ciclo = self->n > 0 && f_tá_vazia(f);
    f_destrói(f);
    return ciclo;
}