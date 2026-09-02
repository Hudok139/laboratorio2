// includes, constantes e declarações {{{1
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MIN_ALLOC 8    // alocação mínima

struct str {
  byte *dados; //bytes UTF-8
  int nbytes;
  int nchar; //n unicode
  int alocado; 
};

// Rica = 4 bytes - 4 caracteres
// Áica = 6 bytes - 4 caracteres

// A memória para conter os bytes de uma string deve ser alocada e/ou
//   realocada conforme a necessidade, cuidando para que a quantidade
//   de memória alocada seja sempre:
//   - nula (não alocada) se a string for vazia, ou
//   - não inferior ao necessário para armazenar os bytes da codificação utf8;
//   - não inferior à alocação mínima;
//   - não superior ao triplo do número de bytes necessários
//     (exceto quando for o mínimo);
//   - uma potência de 2.

// funções auxiliares {{{1


// Se o tamanho for 0, retorna, senão multiplica por 2 até ser maior que o nbytes da string
// OBS: ainda não conhecia essa estratégia de alocação dinâmica

static int s_capacidade(int nbytes){
  if(nbytes == 0){
    return 0;
  }

  int capacidade = MIN_ALLOC;

  while(capacidade < nbytes ){
    capacidade *= 2;
  }

  return capacidade;
}

// verifica se a string cad está de acordo com a especificação
// aborta o programa com assert se não tiver
// OBS: ainda não conhecia assert
static void s_ok(Str_c s)
{
  assert(s != NULL);

  if(s->nchar == 0){
    assert(s->nbytes == 0);
    assert(s->dados == NULL);
    assert(s->alocado == 0);
    
    return;
  }

  assert(s->dados != NULL);
  assert(s->nbytes > 0);
  assert(s->nchar > 0);
  assert(s->alocado >= s->nbytes);
  assert(s->alocado >= MIN_ALLOC);

  assert((s->alocado & (s->alocado -1)) == 0);

  assert(u8_conta_unichar_nos_bytes(s->nbytes, s->dados) == s->nchar);

}

static void s_reserva(Str s, int nbytes){
  int new_capacity;

  if(nbytes == 0){
    free(s->dados);
    s->dados = NULL;
    s->nbytes = 0;
    s->nchar = 0;
    s->alocado = 0;
    return;
  }

  new_capacity - s_capacidade(nbytes);

  if(new_capacity != s->alocado){
    byte *novo = realloc(s->dados, new_capacity);
    assert(novo != NULL);

    s->dados = novo;
    s->alocado = new_capacity;
  }
}

static int s_corrige_pos(Str_c s, int pos){
  int p;

  if(pos < 0){
    p = s->nchar + pos + 1;
  }else {
    p = pos;
  }

  if(p < 0){
    p = 0;
  }

  if(p > s->nchar){
    p = s->nchar;
  }

  return p;
}

static byte *s_ptr_pos(Str_c s, int pos){
  return u8_avanca_unichar(s->dados, pos);
}

static int s_nbytes_nchar(Str_c s, int n)
{
  if (n <= 0)
    return 0;

  byte *p = u8_avanca_unichar(s->dados, n);

  return (int)(p - s->dados);
}

static bool s_contem_c(Str_c sb, unichar c)
{
  for (int i = 0; i < sb->nchar; i++) {
    if (s_ch(sb, i) == c)
      return true;
  }

  return false;
}

//...

// operações de criação e destruição {{{1

Str s_cria(char const *strC)
{
  Str s = malloc(sizeof(*s));
  assert(s != NULL);

  s->dados = NULL;
  s->nbytes = 0;
  s->nchar = 0;
  s->alocado = 0;

  if (strC == NULL)
    return s;

  int nbytes = strlen(strC);

  if (nbytes == 0)
    return s;

  int nchar = u8_conta_unichar_nos_bytes(
      nbytes, (byte *)strC);

  if (nchar < 0)
    return s;

  s->dados = malloc(s_capacidade(nbytes));
  assert(s->dados != NULL);

  memcpy(s->dados, strC, nbytes);

  s->nbytes = nbytes;
  s->nchar = nchar;
  s->alocado = s_capacidade(nbytes);

  s_ok(s);

  return s;
}


void s_destroi(Str s)
{
  if (s == NULL){
    return;
  }
  s_ok(s);

  free(s->dados);
  free(s);
}

Str s_cria_substring(Str_c s, int pos, int tam)
{
   Str nova = s_cria("");
   s_substring(nova, s, pos, tam);
   return nova;
}

Str s_cria_cópia(Str_c s)
{
   return s_cria_substring(s, 0, -1);
}

// Retorna uma nova string com o conteúdo do arquivo chamado nome.
// Retorna uma string vazia em caso de erro.
Str s_cria_de_arquivo(char *nome)
{
  Str s = s_cria("");

  FILE *f = fopen(nome, "rb");

  if (f == NULL)
    return s;

  fseek(f, 0, SEEK_END);
  long tamanho = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (tamanho <= 0) {
    fclose(f);
    return s;
  }

  byte *dados = malloc(tamanho);
  assert(dados != NULL);

  size_t lidos = fread(dados, 1, tamanho, f);

  fclose(f);

  int nchar = u8_conta_unichar_nos_bytes(
      lidos, dados);

  if (nchar < 0) {
    free(dados);
    return s;
  }

  s->dados = malloc(s_capacidade(lidos));
  assert(s->dados != NULL);

  memcpy(s->dados, dados, lidos);

  s->nbytes = lidos;
  s->nchar = nchar;
  s->alocado = s_capacidade(lidos);

  free(dados);

  s_ok(s);

  return s;
}

// operações de acesso {{{1

int s_tam(Str_c s)
{
  s_ok(s);

  return s->nchar;
}

char *s_strc(Str_c s)
{
  s_ok(s);

  char *resultado = malloc(s->nbytes + 1);
  assert(resultado != NULL);

  if (s->nbytes > 0){
    memcpy(resultado, s->dados, s->nbytes);
  }
  
  resultado[s->nbytes] = '\0';

  return resultado;
}

unichar s_ch(Str_c s, int pos)
{
  s_ok(s);

  int p = s_corrige_pos(s, pos);

  if (p < 0 || p >= s->nchar)
    return UNI_INV;

  byte *ptr = u8_avanca_unichar(s->dados, p);

  unichar c;

  int nbytes = u8_unichar_nos_bytes(
      s->nbytes - (ptr - s->dados),
      ptr,
      &c
  );

  if (nbytes < 0)
    return UNI_INV;

  return c;
}


// operações de busca e comparação {{{1

bool s_igual(Str_c s, Str_c sb)
{
  s_ok(s);
  s_ok(sb);

  if (s->nchar != sb->nchar)
    return false;

  if (s->nbytes != sb->nbytes)
    return false;

  return memcmp(s->dados, sb->dados, s->nbytes) == 0;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);

  if (pos < 0){
    pos = s->nchar + pos + 1;
  }

  if (pos < 0){
    pos = 0;
  }

  if (pos > s->nchar){
    pos = s->nchar;
  }
  
  for (int i = pos; i < s->nchar; i++) {
    unichar c = s_ch(s, i);

    for (int j = 0; j < sb->nchar; j++) {
      if (c == s_ch(sb, j))
        return i;
    }
  }

  return -1;
}

int s_busca_nc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);

  if (pos < 0){
    pos = s->nchar + pos + 1;
  }

  if (pos < 0){
    pos = 0;
  }

  if (pos > s->nchar){
    pos = s->nchar;
  }
  
  for (int i = pos; i < s->nchar; i++) {
    unichar c = s_ch(s, i);
    bool encontrado = false;

    for (int j = 0; j < sb->nchar; j++) {
      if (c == s_ch(sb, j)) {
        encontrado = true;
        break;
      }
    }

    if (!encontrado)
      return i;
  }

  return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);

  if (pos < 0){
    pos = s->nchar + pos + 1;
  }

  if (pos < 0){
    pos = 0;
  }

  if (pos > s->nchar){
    pos = s->nchar;
  }
  
  for (int i = pos - 1; i >= 0; i--) {
    unichar c = s_ch(s, i);

    for (int j = 0; j < sb->nchar; j++) {
      if (c == s_ch(sb, j))
        return i;
    }
  }

  return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);

  if (pos < 0){
    pos = s->nchar + pos + 1;
  }

  if (pos < 0){
    pos = 0;
  }

  if (pos > s->nchar){
    pos = s->nchar;
  }
  
  for (int i = pos - 1; i >= 0; i--) {
    unichar c = s_ch(s, i);
    bool encontrado = false;

    for (int j = 0; j < sb->nchar; j++) {
      if (c == s_ch(sb, j)) {
        encontrado = true;
        break;
      }
    }

    if (!encontrado)
      return i;
  }

  return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
  s_ok(s);
  s_ok(buscada);

  int inicio = s_corrige_pos(s, pos);

  if (buscada->nchar == 0)
    return inicio;

  if (buscada->nchar > s->nchar - inicio)
    return -1;

  for (int i = inicio;
       i <= s->nchar - buscada->nchar;
       i++) {

    bool igual = true;

    for (int j = 0; j < buscada->nchar; j++) {
      if (s_ch(s, i + j) != s_ch(buscada, j)) {
        igual = false;
        break;
      }
    }

    if (igual)
      return i;
  }

  return -1;
}


// operações de alteração {{{1

void s_substitui(Str s, int pos, int tam, Str_c sb)
{
  s_ok(s);

  if (sb != NULL)
    s_ok(sb);

  if (pos < 0){
    pos = s->nchar + pos + 1;
  }

  if (pos < 0){
    pos = 0;
  }

  if (pos > s->nchar){
    pos = s->nchar;
  }
  
  int fim;

  if (tam < 0)
    fim = s->nchar;
  else
    fim = pos + tam;

  if (fim > s->nchar)
    fim = s->nchar;

  if (fim < pos)
    fim = pos;

  byte *inicio_ptr = u8_avanca_unichar(s->dados, pos);
  byte *fim_ptr = u8_avanca_unichar(s->dados, fim);

  int bytes_inicio = inicio_ptr - s->dados;
  int bytes_fim = fim_ptr - s->dados;

  int bytes_removidos = bytes_fim - bytes_inicio;

  int bytes_inseridos = 0;
  int chars_inseridos = 0;

  if (sb != NULL) {
    bytes_inseridos = sb->nbytes;
    chars_inseridos = sb->nchar;
  }

  int novo_nbytes =
      s->nbytes - bytes_removidos + bytes_inseridos;

  int novo_nchar =
      s->nchar - (fim - pos) + chars_inseridos;

  byte *novo = NULL;

  if (novo_nbytes > 0) {
    novo = malloc(novo_nbytes);
    assert(novo != NULL);

    memcpy(
        novo,
        s->dados,
        bytes_inicio
    );

    if (sb != NULL && sb->nbytes > 0) {
      memcpy(
          novo + bytes_inicio,
          sb->dados,
          sb->nbytes
      );
    }

    memcpy(
        novo + bytes_inicio + bytes_inseridos,
        s->dados + bytes_fim,
        s->nbytes - bytes_fim
    );
  }

  free(s->dados);

  s->dados = NULL;
  s->nbytes = 0;
  s->nchar = 0;
  s->alocado = 0;

  if (novo_nbytes > 0) {

    int capacidade = MIN_ALLOC;

    while (capacidade < novo_nbytes)
      capacidade *= 2;

    s->dados = malloc(capacidade);
    assert(s->dados != NULL);

    memcpy(
        s->dados,
        novo,
        novo_nbytes
    );

    s->nbytes = novo_nbytes;
    s->nchar = novo_nchar;
    s->alocado = capacidade;
  }

  free(novo);

  s_ok(s);
}

void s_substring(Str s, Str_c sb, int pos, int tam)
{
  s_ok(s);
  s_ok(sb);

  if (pos < 0)
    pos = sb->nchar + pos + 1;

  if (pos < 0)
    pos = 0;

  if (pos > sb->nchar)
    pos = sb->nchar;

  int fim;

  if (tam < 0)
    fim = sb->nchar;
  else
    fim = pos + tam;

  if (fim > sb->nchar)
    fim = sb->nchar;

  if (fim < pos)
    fim = pos;

  byte *inicio_ptr = u8_avanca_unichar(sb->dados, pos);
  byte *fim_ptr = u8_avanca_unichar(sb->dados, fim);

  int bytes = fim_ptr - inicio_ptr;
  int chars = fim - pos;

  byte *novo = NULL;

  if (bytes > 0) {
    novo = malloc(bytes);
    assert(novo != NULL);

    memcpy(novo, inicio_ptr, bytes);
  }

  free(s->dados);

  s->dados = NULL;
  s->nbytes = 0;
  s->nchar = 0;
  s->alocado = 0;

  if (bytes > 0) {

    int capacidade = MIN_ALLOC;

    while (capacidade < bytes)
      capacidade *= 2;

    s->dados = malloc(capacidade);
    assert(s->dados != NULL);

    memcpy(s->dados, novo, bytes);

    s->nbytes = bytes;
    s->nchar = chars;
    s->alocado = capacidade;
  }

  free(novo);

  s_ok(s);
}

void s_copia(Str s, Str_c sb)
{
  s_substring(s, sb, 0, -1);
}

void s_insere(Str s, int pos, Str_c sb)
{
  s_substitui(s, pos, 0, sb);
}

void s_insere_c(Str s, int pos, unichar c)
{
  s_ok(s);

  byte utf8[4];

  int nbytes = u8_converte_pra_utf8(c, utf8);

  if (nbytes < 0)
    return;

  if (pos < 0)
    pos = s->nchar + pos + 1;

  if (pos < 0)
    pos = 0;

  if (pos > s->nchar)
    pos = s->nchar;

  byte *ptr = u8_avanca_unichar(s->dados, pos);

  int byte_pos = ptr - s->dados;

  int novo_nbytes = s->nbytes + nbytes;

  byte *novo = malloc(novo_nbytes);
  assert(novo != NULL);

  memcpy(
      novo,
      s->dados,
      byte_pos
  );

  memcpy(
      novo + byte_pos,
      utf8,
      nbytes
  );

  memcpy(
      novo + byte_pos + nbytes,
      s->dados + byte_pos,
      s->nbytes - byte_pos
  );

  free(s->dados);

  int capacidade = MIN_ALLOC;

  while (capacidade < novo_nbytes)
    capacidade *= 2;

  s->dados = malloc(capacidade);
  assert(s->dados != NULL);

  memcpy(s->dados, novo, novo_nbytes);

  s->nbytes = novo_nbytes;
  s->nchar++;
  s->alocado = capacidade;

  free(novo);

  s_ok(s);
}

void s_anexa(Str s, Str_c sb)
{
  s_substitui(s, -1, 0, sb);
}

void s_anexa_c(Str s, unichar c)
{
  s_insere_c(s, -1, c);
}

void s_remove(Str s, int pos, int tam)
{
  s_substitui(s, pos, tam, NULL);
}

void s_apara(Str s, Str_c sobras)
{
  s_ok(s);
  s_ok(sobras);

  int inicio = 0;

  while (inicio < s->nchar) {

    unichar c = s_ch(s, inicio);

    if (!s_contem_c(sobras, c))
      break;

    inicio++;
  }

  int fim = s->nchar;

  while (fim > inicio) {

    unichar c = s_ch(s, fim - 1);

    if (!s_contem_c(sobras, c))
      break;

    fim--;
  }

  int bytes_inicio =
      u8_avanca_unichar(s->dados, inicio) - s->dados;

  int bytes_fim =
      u8_avanca_unichar(s->dados, fim) - s->dados;

  int novo_nbytes = bytes_fim - bytes_inicio;
  int novo_nchar = fim - inicio;

  byte *novo = NULL;

  if (novo_nbytes > 0) {
    novo = malloc(novo_nbytes);
    assert(novo != NULL);

    memcpy(
        novo,
        s->dados + bytes_inicio,
        novo_nbytes
    );
  }

  free(s->dados);

  s->dados = NULL;
  s->nbytes = 0;
  s->nchar = 0;
  s->alocado = 0;

  if (novo_nbytes > 0) {

    int capacidade = MIN_ALLOC;

    while (capacidade < novo_nbytes)
      capacidade *= 2;

    s->dados = malloc(capacidade);
    assert(s->dados != NULL);

    memcpy(s->dados, novo, novo_nbytes);

    s->nbytes = novo_nbytes;
    s->nchar = novo_nchar;
    s->alocado = capacidade;
  }

  free(novo);

  s_ok(s);
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{
  s_ok(s);

  fwrite(
      s->dados,
      1,
      s->nbytes,
      stdout
  );
}

void s_grava_arquivo(Str_c s, char *nome)
{
  s_ok(s);

  if (nome == NULL)
    return;

  FILE *f = fopen(nome, "wb");

  if (f == NULL)
    return;

  fwrite(
      s->dados,
      1,
      s->nbytes,
      f
  );

  fclose(f);
}


// vim: foldmethod=marker shiftwidth=2

