#include "calc.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
  char *nome_ent = argc > 1 ? argv[1] : "entrada.txt";
  char *nome_saí = argc > 2 ? argv[2] : "saida.txt";

  Str texto = s_cria_de_arquivo(nome_ent);
  Str fim_de_linha = s_cria("\n");
  Lista linhas = l_cria_separando(texto, fim_de_linha);
  Lista saída = l_cria();

  while (!l_vazia(linhas)) {
    Str linha = l_remove(linhas);
    l_insere(saída, calculadora(linha));
    s_destroi(linha);
  }

  Str resultado = s_cria_unindo(saída, fim_de_linha);
  if (!l_vazia(saída)) s_anexa(resultado, fim_de_linha);
  s_grava_arquivo(resultado, nome_saí);

  s_destroi(resultado);
  s_destroi(fim_de_linha);
  s_destroi(texto);
  l_destroi(linhas);
  l_destroi(saída);
  return 0;
}
