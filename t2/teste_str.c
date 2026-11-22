
// teste_str.c
// programa com testes do TAD str

#include "str.h"
#include <stdio.h>
#include <assert.h>

int main()
{
  char *a = "Rábica";
  Str s = s_cria(a);
  printf("Deve escrever [%s] ", a);
  s_imprime(s);
  Str ss = s_cria_substring(s, 3, 2);
  printf("\nDeve escrever [ic] ");
  s_imprime(ss);
  s_substring(ss, s, 1, 2);
  printf("\nDeve escrever [áb] ");
  s_imprime(ss);
  if (s_tam(ss) != 2) printf("\ntamanho de 'áb' (%d) deveria ser 2\n", s_tam(ss));
  s_substitui(ss, -2, 1, NULL);
  s_substitui(s, -7, 3, ss);
  printf("\nDeve escrever [áica] ");
  s_imprime(s);
  assert(s_ch(s, 0) == 0x00E1);  // á
  assert(s_ch(s, -2) == 'a');    // último caractere
  assert(s_ch(s, -1) == UNI_INV);
  assert(s_ch(s, 100) == UNI_INV);
  assert(s_ch(s, -100) == UNI_INV);
  s_destroi(ss);
  s_destroi(s);


  Str teste = s_cria("");

  s_anexa_c(teste, 'A');
  assert(s_tam(teste) == 1);
  assert(s_ch(teste, 0) == 'A');

  s_insere_c(teste, 0, 0x00E1);  // Insere á antes de A.
  assert(s_tam(teste) == 2);
  assert(s_ch(teste, 0) == 0x00E1);
  assert(s_ch(teste, 1) == 'A');

  s_remove(teste, 0, -1);  // Remove todos os caracteres.
  assert(s_tam(teste) == 0);

  s_anexa_c(teste, 'B');
  assert(s_tam(teste) == 1);
  assert(s_ch(teste, 0) == 'B');

  s_destroi(teste);
  puts("\nTestes passaram.");
}
