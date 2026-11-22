#include "lista.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct nó {
  dado_t dado;
  struct nó *ant;
  struct nó *prox;
} Nó;

struct lista {
  Nó sentinela;
  int tam;
};

static Nó *l_nó_pos(Lista l, int pos)
{
  assert(pos >= 0 && pos < l->tam);
  Nó *n;
  if (pos < l->tam / 2) {
    n = l->sentinela.prox;
    for (int i = 0; i < pos; i++) n = n->prox;
  } else {
    n = l->sentinela.ant;
    for (int i = l->tam - 1; i > pos; i--) n = n->ant;
  }
  return n;
}

static void l_insere_após(Lista l, Nó *ant, dado_t d)
{
  Nó *novo = malloc(sizeof(*novo));
  assert(novo != NULL);
  novo->dado = d;
  novo->ant = ant;
  novo->prox = ant->prox;
  ant->prox->ant = novo;
  ant->prox = novo;
  l->tam++;
}

static dado_t l_retira_nó(Lista l, Nó *n)
{
  assert(n != &l->sentinela);
  n->ant->prox = n->prox;
  n->prox->ant = n->ant;
  dado_t d = n->dado;
  free(n);
  l->tam--;
  return d;
}

Lista l_cria()
{
  Lista l = malloc(sizeof(*l));
  assert(l != NULL);
  l->sentinela.dado = NULL;
  l->sentinela.prox = &l->sentinela;
  l->sentinela.ant = &l->sentinela;
  l->tam = 0;
  return l;
}

Lista l_cria_separando(Str s, Str sep)
{
  Lista l = l_cria();
  int pos = 0;
  while (true) {
    int ini = s_busca_nc(s, pos, sep);
    if (ini == -1) break;
    int fim = s_busca_c(s, ini, sep);
    if (fim == -1) fim = s_tam(s);
    l_insere_fim(l, s_cria_substring(s, ini, fim - ini));
    pos = fim;
  }
  return l;
}

void l_destroi(Lista l)
{
  if (l == NULL) return;
  while (!l_vazia(l)) {
    s_destroi(l_remove_inicio(l));
  }
  free(l);
}

int l_tam(Lista l)
{
  return l->tam;
}

bool l_cheia(Lista l)
{
  (void)l;
  return false;
}

bool l_vazia(Lista l)
{
  return l->tam == 0;
}

void l_imprime(Lista l)
{
  printf("[");
  for (Nó *n = l->sentinela.prox; n != &l->sentinela; n = n->prox) {
    printf(" \"");
    s_imprime(n->dado);
    printf("\"");
  }
  printf(" ]\n");
}

dado_t l_dado_inicio(Lista l)
{
  assert(!l_vazia(l));
  return l->sentinela.prox->dado;
}

dado_t l_dado_fim(Lista l)
{
  assert(!l_vazia(l));
  return l->sentinela.ant->dado;
}

dado_t l_dado_pos(Lista l, int pos)
{
  return l_nó_pos(l, pos)->dado;
}

void l_insere_inicio(Lista l, dado_t d)
{
  l_insere_após(l, &l->sentinela, d);
}

void l_insere_fim(Lista l, dado_t d)
{
  l_insere_após(l, l->sentinela.ant, d);
}

void l_insere_pos(Lista l, dado_t d, int p)
{
  if (p <= 0) {
    l_insere_inicio(l, d);
  } else if (p >= l->tam) {
    l_insere_fim(l, d);
  } else {
    l_insere_após(l, l_nó_pos(l, p - 1), d);
  }
}

dado_t l_remove_inicio(Lista l)
{
  assert(!l_vazia(l));
  return l_retira_nó(l, l->sentinela.prox);
}

dado_t l_remove_fim(Lista l)
{
  assert(!l_vazia(l));
  return l_retira_nó(l, l->sentinela.ant);
}

dado_t l_remove_pos(Lista l, int pos)
{
  return l_retira_nó(l, l_nó_pos(l, pos));
}

dado_t l_primeiro(Lista l)
{
  return l_dado_inicio(l);
}

void l_insere(Lista l, dado_t d)
{
  l_insere_fim(l, d);
}

dado_t l_remove(Lista l)
{
  return l_remove_inicio(l);
}

dado_t l_topo(Lista l)
{
  return l_dado_fim(l);
}

void l_empilha(Lista l, dado_t d)
{
  l_insere_fim(l, d);
}

dado_t l_desempilha(Lista l)
{
  return l_remove_fim(l);
}
