#include "lista.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    Lista pilha = l_cria();
    assert(l_vazia(pilha));

    l_empilha(pilha, s_cria("10"));
    l_empilha(pilha, s_cria("20"));
    assert(l_tam(pilha) == 2);

    Str retirada = l_desempilha(pilha);
    assert(s_número(retirada) == 20);
    s_destroi(retirada);

    assert(l_tam(pilha) == 1);
    assert(s_número(l_topo(pilha)) == 10);

    l_destroi(pilha);

    Lista palavras = l_cria();
    l_insere_fim(palavras, s_cria("oi"));
    l_insere_fim(palavras, s_cria("mundo"));

    Str separador = s_cria(", ");
    Str unida = s_cria_unindo(palavras, separador);
    Str esperado = s_cria("oi, mundo");

    assert(s_igual(unida, esperado));

    s_destroi(unida);
    s_destroi(esperado);
    s_destroi(separador);
    l_destroi(palavras);

    puts("Teste da pilha passou.");
    return 0;
}