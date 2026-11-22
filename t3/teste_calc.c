#include "calc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static void testa_calculo(char const *texto, double esperado)
{
    Str expressao = s_cria(texto);
    Str resultado = calculadora(expressao);

    printf("%s = ", texto);
    s_imprime(resultado);
    putchar('\n');

    assert(s_ch(resultado, 0) != '#');
    assert(s_número(resultado) == esperado);

    s_destroi(resultado);
    s_destroi(expressao);
}

// Verifica se uma expressão retorna uma mensagem de erro.
static void testa_erro(char const *texto)
{
    Str expressao = s_cria(texto);
    Str resultado = calculadora(expressao);
    char *mensagem = s_strc(resultado);

    printf("%s -> %s\n", texto, mensagem);
    assert(strncmp(mensagem, "#ERRO ", 6) == 0);

    free(mensagem);
    s_destroi(resultado);
    s_destroi(expressao);
}

int main(void)
{
    Str expressao = s_cria("12 + 3 * (4 - 1)");
    Lista tokens = tokeniza(expressao);

    l_imprime(tokens);
    assert(l_tam(tokens) == 9);

    l_destroi(tokens);
    s_destroi(expressao);

    testa_calculo("2 + 3", 5);
    testa_calculo("12 + 3 * (4 - 1)", 21);
    testa_calculo("10 - 3 - 2", 5);
    testa_calculo("8 / 2 * 3", 12);
    testa_calculo("2 ^ 3 ^ 2", 512);
    testa_calculo("(2 ^ 3) ^ 2", 64);
    testa_calculo("0.5 + .25", 0.75);

    testa_calculo("x = 10", 10);
    testa_calculo("x + 5", 15);
    testa_calculo("x = 20", 20);
    testa_calculo("x", 20);

    testa_calculo("y = x * 2", 40);
    testa_calculo("x + y", 60);

    testa_calculo("a = b = 7", 7);
    testa_calculo("a + b", 14);

    testa_erro("");
    testa_erro("2 +");
    testa_erro("2 3");
    testa_erro("(2 + 3");
    testa_erro("2 + 3)");
    testa_erro("1 / 0");
    testa_erro("1..2 + 3");
    testa_erro("variavel_inexistente + 1");
    testa_erro("2 = 5");
    testa_erro("2 @ 3");

    puts("Testes da calculadora passaram.");
    return 0;
}