// 김민수, assignment 1, decomment.c
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <locale.h>
#include <stdarg.h>

// standard output macro
#define PUTCHAR(ch) fprintf(stdout, "%c", (ch))

// State for DFA (named constant enum)
typedef enum {
  STATE_NORMAL,
  STATE_AFTER_SLASH,
  STATE_SINGLE_COMMENT,
  STATE_MULTI_COMMENT,
  STATE_AFTER_STAR,
  STATE_STRING,
  STATE_STRING_ESCAPE,
  STATE_CHAR,
  STATE_CHAR_ESCAPE,
  STATE_SUCCESS,
  STATE_FAILURE
} State;

// function prototypes
int handleEOF(State *state, int *com_line);
void handleNormal(char ch, State *state, int *cur_line);
void handleSlash(char ch, State *state, int *cur_line, int *com_line);
void handleSingleCom(char ch, State *state, int *cur_line);
void handleMultiCom(char ch, State *state, int *cur_line, int *com_line);
void handleAfterStar(char ch, State *state, int *cur_line, int *com_line);
void handleString(char ch, State *state, int *cur_line);
void handleStringEsc(char ch, State *state, int *cur_line);
void handleChar(char ch, State *state, int *cur_line);
void handleCharEsc(char ch, State *state, int *cur_line);
void checkPointers(int count, ...);

/*
* main function of decommenter program
*
* Set locale according to current system
* read source code(.c) from standard input (stdin)
* handle one character at a time by applying DFA (state, transition)
* remove comments from the source code
* write resulting output to standard output (stdout)
* in case of error (locale error, unterminated multi-line comment)
* write error message to standard error (stderr) and return EXIT_FAILURE
*
* Parameter : None
* Return : EXIT_SUCCESS, or EXIT_FAILURE (ERROR)
*/
int main(void)
{
  if (setlocale(LC_CTYPE, "") == NULL) {
    fprintf(stderr, "Locale Setting failed");
    return EXIT_FAILURE;
  }

  int ich;
  char ch;
  int cur_line = 1;
  int com_line = -1;
  State state = STATE_NORMAL;

  while (1) {
    ich = getchar();
    if (ich == EOF) {
      int exit_status = handleEOF(&state, &com_line);
      return exit_status;
    }
    ch = (char)ich;
    switch (state) {
      case STATE_NORMAL:
        handleNormal(ch, &state, &cur_line);
        break;
      case STATE_AFTER_SLASH:
        handleSlash(ch, &state, &cur_line, &com_line);
        break;
      case STATE_SINGLE_COMMENT:
        handleSingleCom(ch, &state, &cur_line);
        break;
      case STATE_MULTI_COMMENT:
        handleMultiCom(ch, &state, &cur_line, &com_line);
        break;
      case STATE_AFTER_STAR:
        handleAfterStar(ch, &state, &cur_line, &com_line);
        break;
      case STATE_STRING:
        handleString(ch, &state, &cur_line);
        break;
      case STATE_STRING_ESCAPE:
        handleStringEsc(ch, &state, &cur_line);
        break;
      case STATE_CHAR:
        handleChar(ch, &state, &cur_line);
        break;
      case STATE_CHAR_ESCAPE:
        handleCharEsc(ch, &state, &cur_line);
        break;
      default:
        assert(0);
        break;
    }
  }
}

/* 
* handle EOF input according to current state 
* Parameters:
*   state    - pointer to the current DFA state.
*   com_line - pointer to the comment start line number.
*
* Return:
*   EXIT_SUCCESS 
*   EXIT_FAILURE (if unterminated comment exists)
*/
int handleEOF(State *state, int *com_line)
{
  checkPointers(2, state, com_line);
  if (*state == STATE_MULTI_COMMENT) {
    fprintf(stderr, "Error: line %d: unterminated comment\n", *com_line);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/* 
* handle input character (Inside Normal State)
* process character outside string, char, or comment
* update State, print input character to stdout 
* increment cur_line if needed
* Parameters:
*   ch       - the input character.
*   state    - pointer to the current DFA state.
*   cur_line - pointer to the current line number.
*
* Return: None.
*/
void handleNormal(char ch, State *state, int *cur_line)
{
  checkPointers(2, state, cur_line);
  switch (ch) {
    case '/':
      *state = STATE_AFTER_SLASH;
      break;
    case '"':
      *state = STATE_STRING;
      PUTCHAR(ch);
      break;
    case '\'':
      *state = STATE_CHAR;
      PUTCHAR(ch);
      break;
    case '\n':
      PUTCHAR(ch);
      (*cur_line)++;
      break;
    default:
      PUTCHAR(ch);
      break;
  }
}

/* 
* handle input character (Inside AFTER_SLASH State)
* process chracter following '/' outside string, char, or comment
* update State, print input character to stdout
* increment cur_line, set com_line if needed
* Parameters:
*   com_line - pointer to the start line number of comment.
*/
void handleSlash(char ch, State *state, int *cur_line, int *com_line)
{
  checkPointers(3, state, cur_line, com_line);
  switch (ch) {
    case '/':
      *state = STATE_SINGLE_COMMENT;
      *com_line = *cur_line;
      PUTCHAR(' ');
      break;
    case '*':
      *state = STATE_MULTI_COMMENT;
      PUTCHAR(' ');
      *com_line = *cur_line;
      break;
    case '"':
      *state = STATE_STRING;
      PUTCHAR('/');
      PUTCHAR(ch);
      break;
    case '\'':
      *state = STATE_CHAR;
      PUTCHAR('/');
      PUTCHAR(ch);
      break;
    case '\n':
      *state = STATE_NORMAL;
      PUTCHAR('/');
      PUTCHAR(ch);
      (*cur_line)++;
      break;
    default:
      *state = STATE_NORMAL;
      PUTCHAR('/');
      PUTCHAR(ch);
      break;
  }
}

/*
* handle input character (Inside Single Comment State)
* process character inside single line comment
* go back to Noraml State, print newline to stdout
* increment cur_line if newline
*/
void handleSingleCom(char ch, State *state, int *cur_line)
{
  checkPointers(2, state, cur_line);
  if (ch == '\n') {
    *state = STATE_NORMAL;
    PUTCHAR(ch);
    (*cur_line)++;
  }
}

/*
* handle input character (Inside Multi Comment State)
* process character inside multi line comment
* go to After Star State, print newline to stdout
* increment cur_line if needed
*/
void handleMultiCom(char ch, State *state, int *cur_line, int *com_line)
{
  checkPointers(3, state, cur_line, com_line);
  switch (ch) {
    case '*':
      *state = STATE_AFTER_STAR;
      break;
    case '\n':
      PUTCHAR(ch);
      (*cur_line)++;
      break;
    default:
      break;
  }
}

/*
* handle input character (Inside After Star State)
* process character following '*' inside multi line comment
* update State, print newline to stdout
* increment cur_line if needed
*/
void handleAfterStar(char ch, State *state, int *cur_line, int *com_line)
{
  checkPointers(3, state, cur_line, com_line);
  switch (ch) {
    case '/':
      *state = STATE_NORMAL;
      break;
    case '\n':
      *state = STATE_MULTI_COMMENT;
      PUTCHAR(ch);
      (*cur_line)++;
      break;
    case '*':
      break;
    default:
      *state = STATE_MULTI_COMMENT;
      break;
  }
}

/*
* handle input character (Inside String State)
* process character inside string
* update State, print input character to stdout
* increment cur_line if needed
*/
void handleString(char ch, State *state, int *cur_line)
{
  checkPointers(2, state, cur_line);
  switch (ch) {
    case '"':
      *state = STATE_NORMAL;
      PUTCHAR(ch);
      break;
    case '\n':
      PUTCHAR(ch);
      (*cur_line)++;
      break;
    case '\\':
      *state = STATE_STRING_ESCAPE;
      PUTCHAR(ch);
      break;
    default:
      PUTCHAR(ch);
      break;
  }
}

/*
* handle input character (Inside String Escape State)
* process character following '\' inside string
* go back to String State, print input character to stdout
* increment cur_line if needed
*/
void handleStringEsc(char ch, State *state, int *cur_line)
{
  checkPointers(2, state, cur_line);
  *state = STATE_STRING;
  PUTCHAR(ch);
  // no need but just in case
  if (ch == '\n') {
    (*cur_line)++;
  }
}

/*
* handle input character (Inside Char State)
* process character inside character constant
* update State, print input character to stdout
* increment cur_line if needed
*/
void handleChar(char ch, State *state, int *cur_line)
{
  checkPointers(2, state, cur_line);
  switch (ch) {
    case '\'':
      *state = STATE_NORMAL;
      PUTCHAR(ch);
      break;
    case '\n':
      PUTCHAR(ch);
      (*cur_line)++;
      break;
    case '\\':
      *state = STATE_CHAR_ESCAPE;
      PUTCHAR(ch);
      break;
    default:
      PUTCHAR(ch);
      break;
  }
}

/*
* handle input character (Inside Char Escape State)
* process character following '\' inside character constant
* go back to Char State, print input character to stdout
* increment cur_line if needed
*/
void handleCharEsc(char ch, State *state, int *cur_line)
{
  checkPointers(2, state, cur_line);
  *state = STATE_CHAR;
  PUTCHAR(ch);
  // no need but just in case
  if (ch == '\n') {
    (*cur_line)++;
  }
}

/*
* check pointer variables used as parameters of other functions
* Parameters:
*   count - number of pointer variables to check
*   ...   - pointer variables to check  
*
* Return: 
*   print error message to stderr
*   terminate program with EXIT_FAILURE 
*   (if there is a NUll pointer variable)
*/
void checkPointers(int count, ...)
{
  va_list args;
  va_start(args, count);
  for (int i = 0; i < count; i++) {
    void *ptr = va_arg(args, void*);
    if (ptr == NULL) {
      fprintf(stderr, "Error: Null pointer encountered\n");
      exit(EXIT_FAILURE);
    }
  }
  va_end(args);
}
