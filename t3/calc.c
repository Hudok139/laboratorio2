#include "calc.h"
#include "dicionario.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static bool é_espaço(unichar c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
static bool é_dígito(unichar c) { return c >= '0' && c <= '9'; }
static bool é_letra(unichar c)  { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static bool é_operador_c(unichar c) { return c != 0 && strchr("+-*/^()=", (int)c) != NULL && c < 128; }
static bool inicia_número(unichar c) { return é_dígito(c) || c == '.'; }
static bool inicia_nome(unichar c)   { return é_letra(c) || c == '_' || c == '$'; }
static bool continua_nome(unichar c) { return inicia_nome(c) || é_dígito(c); }

typedef struct calc {
  Dicionário variáveis;
} *Calc;

static bool str_igual(chave_t a, chave_t b) { return s_igual(a, b); }

static bool str_menor(chave_t a, chave_t b)
{
  int na = s_tam(a), nb = s_tam(b);
  for (int i = 0; i < na && i < nb; i++) {
    unichar ca = s_ch(a, i), cb = s_ch(b, i);
    if (ca != cb) return ca < cb;
  }
  return na < nb;
}

static void destrói_tupla(chave_t ch, valor_t v)
{
  s_destroi(ch);
  s_destroi(v);
}

static Calc calc_cria(void)
{
  Calc c = malloc(sizeof(*c));
  c->variáveis = dic_cria(str_menor, str_igual);
  return c;
}

static void calc_destrói(Calc c)
{
  if (c == NULL) return;
  dic_para_todos(c->variáveis, destrói_tupla);
  dic_destrói(c->variáveis);
  free(c);
}

static Calc a_calc = NULL;

static void calc_finaliza(void)
{
  calc_destrói(a_calc);
  a_calc = NULL;
}

static Calc calc_obtém(void)
{
  if (a_calc == NULL) {
    a_calc = calc_cria();
    atexit(calc_finaliza);
  }
  return a_calc;
}

static Str calc_busca_var(Calc c, Str nome)
{
  return dic_busca(c->variáveis, nome);
}

static void calc_atribui_var(Calc c, Str nome, Str valor)
{
  Str v = s_cria_cópia(valor);
  Str antigo = dic_busca(c->variáveis, nome);
  if (antigo != NULL) {

    dic_insere(c->variáveis, nome, v);
    s_destroi(antigo);
  } else {
    dic_insere(c->variáveis, s_cria_cópia(nome), v);
  }
}

Lista tokeniza(Str txt)
{
  Lista l = l_cria();
  int n = s_tam(txt);
  int i = 0;
  while (i < n) {
    unichar c = s_ch(txt, i);
    if (é_espaço(c)) { i++; continue; }
    int ini = i++;
    if (inicia_número(c)) {
      while (i < n && inicia_número(s_ch(txt, i))) i++;
    } else if (inicia_nome(c)) {
      while (i < n && continua_nome(s_ch(txt, i))) i++;
    }

    l_insere_fim(l, s_cria_substring(txt, ini, i - ini));
  }
  return l;
}

typedef enum { T_OPERADOR, T_NÚMERO, T_NOME, T_ERRO } tipo_token;

static bool é_número_válido(Str_c t)
{
  int n = s_tam(t), pontos = 0, dígitos = 0;
  for (int i = 0; i < n; i++) {
    unichar c = s_ch(t, i);
    if (c == '.') pontos++;
    else if (é_dígito(c)) dígitos++;
    else return false;
  }
  return pontos <= 1 && dígitos > 0;
}

static tipo_token classifica(Str_c t)
{
  unichar c = s_ch(t, 0);
  if (s_tam(t) == 1 && é_operador_c(c)) return T_OPERADOR;
  if (inicia_número(c)) return é_número_válido(t) ? T_NÚMERO : T_ERRO;
  if (inicia_nome(c)) return T_NOME;
  return T_ERRO;
}

static Str erro(char const *msg, Str_c detalhe)
{
  Str e = s_cria("#ERRO ");
  Str m = s_cria(msg);
  s_anexa(e, m);
  s_destroi(m);
  if (detalhe != NULL) {
    Str q = s_cria(" '");
    s_anexa(e, q);
    s_anexa(e, detalhe);
    s_anexa_c(e, '\'');
    s_destroi(q);
  }
  return e;
}

static Str valor_operando(Calc c, Str_c op, double *v)
{
  Str_c texto = op;
  if (inicia_nome(s_ch(op, 0))) {
    texto = calc_busca_var(c, (Str)op);
    if (texto == NULL) return erro("variável não definida", op);
  }
  *v = s_número(texto);
  return NULL;
}

static Str opera(Calc c, unichar op, Lista operandos)
{
  if (l_tam(operandos) < 2) return erro("faltam operandos", NULL);
  Str b = l_desempilha(operandos);
  Str a = l_desempilha(operandos);
  Str e = NULL;
  double va = 0, vb = 0, r = 0;

  if (op == '=') {
    if (!inicia_nome(s_ch(a, 0))) {
      e = erro("atribuição a algo que não é variável:", a);
    } else if ((e = valor_operando(c, b, &vb)) == NULL) {
      Str valor = s_cria_número(vb);
      calc_atribui_var(c, a, valor);
      l_empilha(operandos, valor);
    }
  } else if ((e = valor_operando(c, a, &va)) == NULL
          && (e = valor_operando(c, b, &vb)) == NULL) {
    switch (op) {
      case '+': r = va + vb; break;
      case '-': r = va - vb; break;
      case '*': r = va * vb; break;
      case '/':
        if (vb == 0) e = erro("divisão por zero", NULL);
        else r = va / vb;
        break;
      case '^': r = pow(va, vb); break;
    }
    if (e == NULL && !isfinite(r)) e = erro("resultado inválido", NULL);
    if (e == NULL) l_empilha(operandos, s_cria_número(r));
  }
  s_destroi(a);
  s_destroi(b);
  return e;
}

typedef enum { EMPILHA, OPERA, DESCARTA, TERMINA, FALTA_ABRE, FALTA_FECHA } ação;
#define E  EMPILHA
#define O  OPERA
#define D  DESCARTA
#define T  TERMINA
#define Ea FALTA_ABRE
#define Ef FALTA_FECHA

static ação const tabela[6][7] = {

   {T,  E,  E,  E,  E,  Ea, E},
   {O,  O,  E,  E,  E,  O,  E},
   {O,  O,  O,  E,  E,  O,  E},
   {O,  O,  O,  E,  E,  O,  E},
   {Ef, E,  E,  E,  E,  D,  E},
   {O,  E,  E,  E,  E,  O,  E},
};
#undef E
#undef O
#undef D
#undef T
#undef Ea
#undef Ef

static int linha(unichar op)
{
  switch (op) {
    case '+': case '-': return 1;
    case '*': case '/': return 2;
    case '^': return 3;
    case '(': return 4;
    case '=': return 5;
  }
  return 0;
}

static int coluna(unichar op)
{
  switch (op) {
    case '+': case '-': return 1;
    case '*': case '/': return 2;
    case '^': return 3;
    case '(': return 4;
    case ')': return 5;
    case '=': return 6;
  }
  return 0;
}

Str calculadora(Str expressão)
{
  Calc c = calc_obtém();
  Lista tokens = tokeniza(expressão);
  Lista operandos = l_cria();
  Lista operadores = l_cria();
  Str resultado = NULL;
  bool terminou = false;

  while (!terminou && resultado == NULL) {

    Str tok = l_vazia(tokens) ? NULL : l_remove(tokens);
    unichar op_e = 0;
    if (tok != NULL) {
      tipo_token tipo = classifica(tok);
      if (tipo == T_ERRO) {
        resultado = erro("token inválido", tok);
        s_destroi(tok);
        break;
      }
      if (tipo != T_OPERADOR) {
        l_empilha(operandos, tok);
        continue;
      }
      op_e = s_ch(tok, 0);
    }

    bool próximo = false;
    while (!próximo && resultado == NULL) {
      unichar op_p = l_vazia(operadores) ? 0 : s_ch(l_topo(operadores), 0);
      switch (tabela[linha(op_p)][coluna(op_e)]) {
        case EMPILHA:
          l_empilha(operadores, tok);
          tok = NULL;
          próximo = true;
          break;
        case DESCARTA:
          s_destroi(l_desempilha(operadores));
          próximo = true;
          break;
        case OPERA: {
          Str op = l_desempilha(operadores);
          resultado = opera(c, s_ch(op, 0), operandos);
          s_destroi(op);
          break;
        }
        case TERMINA:
          terminou = true;
          próximo = true;
          break;
        case FALTA_ABRE:
          resultado = erro("falta (", NULL);
          break;
        case FALTA_FECHA:
          resultado = erro("falta )", NULL);
          break;
      }
    }
    s_destroi(tok);
  }

  if (resultado == NULL) {
    if (l_tam(operandos) != 1) {
      resultado = erro(l_vazia(operandos) ? "expressão vazia"
                                          : "sobraram operandos", NULL);
    } else {
      double v;
      resultado = valor_operando(c, l_topo(operandos), &v);
      if (resultado == NULL) resultado = s_cria_número(v);
    }
  }

  l_destroi(tokens);
  l_destroi(operandos);
  l_destroi(operadores);
  return resultado;
}
