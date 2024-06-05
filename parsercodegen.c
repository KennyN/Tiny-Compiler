/* COP 3402
  Kenny Nguyen
  Jared Zayas
*/

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOL_TABLE_SIZE = 500;

// Define enum to represent different types of tokens
typedef enum {
  ERRORSYM = 0,
  oddsym = 1,
  identsym,
  numbersym,
  plussym,
  minussym,
  multsym,
  slashsym,
  xorsym,
  eqlsym,
  neqsym,
  lessym,
  leqsym,
  gtrsym,
  geqsym,
  lparentsym,
  rparentsym,
  commasym,
  semicolonsym,
  periodsym,
  becomessym,
  beginsym,
  endsym,
  ifsym,
  thensym,
  whilesym,
  dosym,
  callsym,
  constsym,
  varsym,
  procsym,
  writesym,
  readsym,
  elsesym,
  EOFSYM
} token_type;

// Struct representing a single token
typedef struct {
  token_type type;
  char *lexeme;
  int is_error;
} Token;

// Struct representing instructions
typedef struct {
  int op;
  int m;
  int l;
} instruction;

typedef struct {
  int kind;      // const = 1, var = 2, proc = 3
  char name[10]; // name up to 11 chars
  int val;       // number (ASCII value)
  int level;     // L level
  int addr;      // M address
  int mark;      // to indicate unavailable or deleted
} symbol;

instruction *code;
symbol *table;
Token *tokenArray;
int tokenArrayIndex = 0;
int tableIndex;
int codeIndex = 0;

// Function Declarations
void block();
void program();
void constDeclar();
int varDeclar();
void statement();
void condition();
void expression();
void term();
void factor();
int symbolTableCheck(char *search);
void tableInsert(int kind, char name[], int val, int level, int addr, int mark);

// Function to create a new token
Token *createToken(token_type type, const char *lexeme, int is_error) {
  Token *token = (Token *)malloc(sizeof(Token));
  token->type = type;
  token->lexeme = strdup(lexeme);
  token->is_error = is_error;
  return token;
}

// Function to free a token's memory
void freeToken(Token *token) {
  // free(token->lexeme);
  // free(token);
}

// Function to check if a lexeme is a keyword (Used later in Token List)
bool isKeyword(const char *lexeme) {
  const char *keywords[] = {"const", "var",   "procedure", "call",  "begin",
                            "end",   "if",    "then",      "while", "do",
                            "odd",   "write", "read",      "else",  "xor"};

  for (int i = 0; i < sizeof(keywords) / sizeof(char *); i++) {
    if (strcmp(lexeme, keywords[i]) == 0) {
      return true;
    }
  }
  return false;
}

// Function to get the next token from a file
Token *getNextToken(FILE *file) {
  typedef struct {
    const char *op;
    token_type type;
  } multi_char_operator;

  multi_char_operator multi_char_operators[] = {
      {"<=", leqsym},
      {">=", geqsym},
      {"<>", neqsym},
  };

  int c = fgetc(file);

  // Ignore whitespace and comments
  while (isspace(c)) {
    c = fgetc(file);
  }

  if (c == '/') {
    char next_char = fgetc(file);
    if (next_char == '*') {
      // Comment detected, ignore until end of comment
      char prev_char = 0;
      while (true) {
        next_char = fgetc(file);
        if (prev_char == '*' && next_char == '/') {
          break;
        }
        prev_char = next_char;
      }
      // Continue reading the next character
      c = fgetc(file);
    } else {
      // '/' found, but it is not a start of a comment
      ungetc(next_char, file);
    }
  }

  while (isspace(c))
    c = fgetc(file);

  // Handle end of file
  if (c == EOF)
    return createToken(EOFSYM, "", 0);

  // Handle single-character operators
  if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' ||
      c == ',' || c == ';' || c == ':' || c == '.' || c == '<' || c == '>' ||
      c == '=') {
    char symbol[3] = {c, '\0', '\0'};
    token_type symbol_type;

    // Handle identifiers and keywords
    switch (c) {
    case '+':
      symbol_type = plussym;
      break;
    case '-':
      symbol_type = minussym;
      break;
    case '*':
      symbol_type = multsym;
      break;
    case '/':
      symbol_type = slashsym;
      break;
    case '(':
      symbol_type = lparentsym;
      break;
    case ')':
      symbol_type = rparentsym;
      break;
    case ',':
      symbol_type = commasym;
      break;
    case ';':
      symbol_type = semicolonsym;
      break;
    case '.':
      symbol_type = periodsym;
      break;
    case '<':
      symbol_type = lessym;
      break;
    case '>':
      symbol_type = gtrsym;
      break;
    case '=':
      symbol_type = eqlsym;
      break;
    }

    if (c == ':' || c == '<' || c == '>' || c == '/') {
      c = fgetc(file);
      if (c == '=') {
        symbol[1] = c;
        if (symbol[0] == ':') {
          symbol_type = becomessym;
        } else if (symbol[0] == '<') {
          symbol_type = leqsym;
        } else if (symbol[0] == '>') {
          symbol_type = geqsym;
        }
        return createToken(symbol_type, symbol, 0);

      } else if (c == '>') {
        symbol[1] = c;
        symbol_type = neqsym;
        return createToken(symbol_type, symbol, 0);
      }
      ungetc(c, file);
    }

    return createToken(symbol_type, symbol, 0);
  }

  // Error handling
  if (isalpha(c)) {
    char lexeme[256];
    int i = 0;
    lexeme[i++] = c;

    while ((c = fgetc(file)) != EOF && isalnum(c))
      lexeme[i++] = c;

    lexeme[i] = '\0';

    ungetc(c, file);

    if (i > 12) {
      fprintf(stderr, "Error : Identifier names cannot exceed 11 characters\n");
      return createToken(ERRORSYM, "Error", 1);
    }

    token_type keyword_type = identsym;

    if (strcmp(lexeme, "const") == 0)
      keyword_type = constsym;
    else if (strcmp(lexeme, "var") == 0)
      keyword_type = varsym;
    else if (strcmp(lexeme, "procedure") == 0)
      keyword_type = procsym;
    else if (strcmp(lexeme, "call") == 0)
      keyword_type = callsym;
    else if (strcmp(lexeme, "begin") == 0)
      keyword_type = beginsym;
    else if (strcmp(lexeme, "end") == 0)
      keyword_type = endsym;
    else if (strcmp(lexeme, "if") == 0)
      keyword_type = ifsym;
    else if (strcmp(lexeme, "then") == 0)
      keyword_type = thensym;
    else if (strcmp(lexeme, "while") == 0)
      keyword_type = whilesym;
    else if (strcmp(lexeme, "do") == 0)
      keyword_type = dosym;
    else if (strcmp(lexeme, "odd") == 0)
      keyword_type = oddsym;
    else if (strcmp(lexeme, "xor") == 0)
      keyword_type = xorsym;
    else if (strcmp(lexeme, "write") == 0)
      keyword_type = writesym;
    else if (strcmp(lexeme, "read") == 0)
      keyword_type = readsym;
    else if (strcmp(lexeme, "else") == 0)
      keyword_type = elsesym;

    return createToken(keyword_type, lexeme, 0);
  }

  if (isdigit(c)) {
    char lexeme[256];
    int i = 0;
    lexeme[i++] = c;

    while ((c = fgetc(file)) != EOF && isalnum(c)) {
      lexeme[i++] = c;
      if (isalpha(c)) {
        fprintf(stderr, "Error : Identifier starts with a digit\n");
        return createToken(ERRORSYM, "Error", 1);
      }
    }

    lexeme[i] = '\0';

    ungetc(c, file);

    if (i > 6) {
      fprintf(stderr, "Error : Numbers cannot exceed 5 digits\n");
      return createToken(ERRORSYM, "Error", 1);
    }

    return createToken(numbersym, lexeme, c);
  }

  fprintf(stderr, "Error : Invalid Symbol\n");
  return createToken(ERRORSYM, "Error", 1);
}

// Print Errors
void printErrors(int error) {
  switch (error) {
  case 1:
    printf("program must end with period\n");
    break;
  case 2:
    printf("const, var, and read keywords must be followed by identifier\n");
    break;
  case 3:
    printf("symbol name has already been declared\n");
    break;
  case 4:
    printf("constants must be assigned with =\n");
    break;
  case 5:
    printf("constants must be assigned an integer value\n");
    break;
  case 6:
    printf(
        "constant and variable declarations must be followed by a semicolon\n");
    break;
  case 7:
    printf("undeclared identifier\n");
    break;
  case 8:
    printf("only variable values may be altered\n");
    break;
  case 9:
    printf("assignment statements must use :=\n");
    break;
  case 10:
    printf("begin must be followed by end\n");
    break;
  case 11:
    printf("if must be followed by then\n");
    break;
  case 12:
    printf("while must be followed by do\n");
    break;
  case 13:
    printf("condition must contain comparison operator\n");
    break;
  case 14:
    printf("right parenthesis must follow left parenthesis\n");
    break;
  case 15:
    printf("arithmetic equations must contain operands, parentheses, numbers, "
           "or symbols\n");
    break;
  }
  // free(code);
  // free(table);
  exit(0);
}

// SYMBOLTABLECHECK FUNCTION
int symbolTableCheck(char *search) {
  int i;
  // printf("TABLE INDEX: %d\n", tableIndex);
  for (i = 0; i < tableIndex; i++) {
    // printf("I:= %d\n", i);
    // printf("SEARCH:= %s\n", search);
    // printf("%s\n", table[i].name);
    if (strcmp(table[i].name, search) == 0) {
      return i; // Symbol found, return its index
    }
  }
  return -1; // Symbol not found
}

// Emit Function
void emit(char *instructName, int L, int M) {
  printf("\n  %d \t %s \t %d \t %d \n", codeIndex, instructName, L, M);

  int opCodeValue = 0;
  if (strcmp(instructName, "LIT") == 0) {
    opCodeValue = 1;
  } else if (strcmp(instructName, "OPR") == 0 ||
             strcmp(instructName, "RTN") == 0 ||
             strcmp(instructName, "ADD") == 0 ||
             strcmp(instructName, "SUB") == 0 ||
             strcmp(instructName, "MUL") == 0 ||
             strcmp(instructName, "DIV") == 0 ||
             strcmp(instructName, "EQL") == 0 ||
             strcmp(instructName, "NEQ") == 0 ||
             strcmp(instructName, "LSS") == 0 ||
             strcmp(instructName, "LEQ") == 0 ||
             strcmp(instructName, "GTR") == 0 ||
             strcmp(instructName, "GEQ") == 0) {
    opCodeValue = 2;
  } else if (strcmp(instructName, "LOD") == 0) {
    opCodeValue = 3;
  } else if (strcmp(instructName, "STO") == 0) {
    opCodeValue = 4;
  } else if (strcmp(instructName, "CAL") == 0) {
    opCodeValue = 5;
  } else if (strcmp(instructName, "INC") == 0) {
    opCodeValue = 6;
  } else if (strcmp(instructName, "JMP") == 0) {
    opCodeValue = 7;
  } else if (strcmp(instructName, "JPC") == 0) {
    opCodeValue = 8;
  } else if (strcmp(instructName, "SYS") == 0) {
    opCodeValue = 9;
  }
  code[codeIndex].op = opCodeValue;
  code[codeIndex].l = L;
  code[codeIndex].m = M;
  codeIndex++;
}

// Add to Symbol Table Function
void tableInsert(int kind, char name[], int val, int level, int addr,
                 int mark) {
  table[tableIndex].kind = kind;
  strcpy(table[tableIndex].name, name);
  table[tableIndex].val = val;
  table[tableIndex].level = level;
  table[tableIndex].addr = addr;
  table[tableIndex].mark = mark;
  tableIndex++;
}

// VAR-DECLARATION Function
int varDeclare() {
  int numVars = 0;
  if (tokenArray[tokenArrayIndex].type == varsym) {

    do {
      numVars++;
      tokenArrayIndex++;
      if (tokenArray[tokenArrayIndex].type != identsym)
        printErrors(
            2); // const, var, and read keywords must be followed by identifier
      if (symbolTableCheck(tokenArray[tokenArrayIndex].lexeme) != -1)
        printErrors(3); // symbol name has already been declared

      // add to symbol table (kind 2, ident, 0, 0, var# +2)
      int newVars = numVars + 2;
      // printf("LEXEME: %s", tokenArray[tokenArrayIndex].lexeme);
      tableInsert(2, tokenArray[tokenArrayIndex].lexeme, 0, 0, newVars, 0);
      tokenArrayIndex++;
    } while (tokenArray[tokenArrayIndex].type == commasym);
    if (tokenArray[tokenArrayIndex].type != semicolonsym) {
      printErrors(6); // constant and variable declarations must be followed by
                      // a semicolon
    }
    tokenArrayIndex++;
  }

  return numVars;
}

// Block Function
void block() {
  constDeclar();
  int numVars = varDeclare();
  emit("INC", 0, 3 + numVars);
  statement();
}

// Parse/Program Function
void program(Token *tokenArray) {
  codeIndex = 0;
  tokenArrayIndex = 0;
  tableIndex = 0;
  code = malloc(sizeof(instruction) * 500);
  table = malloc(sizeof(symbol) * 500);
  printf("\n\nAssembly Code:\n");
  printf("Line \t OP \t L \t M");
  emit("JMP", 0, 3);
  // Exec Block Function
  block();
  emit("SYS", 0, 3); // success reached end of program!
}

// CONST-DECLARATION Function
void constDeclar() {
  if (tokenArray[tokenArrayIndex].type == constsym) {

    do {
      tokenArrayIndex++;
      if (tokenArray[tokenArrayIndex].type != identsym)
        printErrors(
            2); // const, var, and read keywords must be followed by identifier
      if (symbolTableCheck(tokenArray[tokenArrayIndex].lexeme) != -1)
        printErrors(3); // symbol name has already been declared

      // Save ident name
      char *name;
      // strcpy(name, tokenArray[tokenArrayIndex].lexeme);
      name = strdup(tokenArray[tokenArrayIndex].lexeme);

      tokenArrayIndex++;
      if (tokenArray[tokenArrayIndex].type != eqlsym)
        printErrors(4); // constants must be assigned with =

      tokenArrayIndex++;
      if (tokenArray[tokenArrayIndex].type != numbersym)
        printErrors(5); // constants must be assigned an integer value

      // Add to symbol table (kind 1, saved name, number, 0, 0)
      tableInsert(1, name, atoi(tokenArray[tokenArrayIndex].lexeme), 0, 0, 0);

      tokenArrayIndex++;

    } while (tokenArray[tokenArrayIndex].type == commasym);

    if (tokenArray[tokenArrayIndex].type != semicolonsym) {
      printErrors(6); // constant and variable declarations must be followed by
                      // a semicolon
    }
    tokenArrayIndex++;
  }
}

// Statement Function
void statement() {
  int jpcIdx;
  int loopIdx;
  int jmpIdx;

  if (tokenArray[tokenArrayIndex].type == identsym) {
    int symIdx = symbolTableCheck(tokenArray[tokenArrayIndex].lexeme);
    // printf("FIRST: %s %d\n", tokenArray[tokenArrayIndex].lexeme, symIdx);
    if (symIdx == -1)
      // undeclared identifier
      printErrors(7);
    if (table[symIdx].kind != 2)
      printErrors(8); // only variable values may be altered

    tokenArrayIndex++;
    if (tokenArray[tokenArrayIndex].type != becomessym)
      printErrors(9); // assignment statements must use :=

    tokenArrayIndex++;
    expression();
    // printf("EMIT STO ONE");
    emit("STO", 0, atoi(tokenArray[tokenArrayIndex].lexeme));

    return;
  }

  if (tokenArray[tokenArrayIndex].type == beginsym) {

    do {

      tokenArrayIndex++;
      statement();

    } while (tokenArray[tokenArrayIndex].type == semicolonsym);

    if (tokenArray[tokenArrayIndex].type != endsym)
      printErrors(10); // begin must be followed by end
    tokenArrayIndex++;

    return;
  }

  if (tokenArray[tokenArrayIndex].type == ifsym) {

    tokenArrayIndex++;
    condition();
    jpcIdx = codeIndex;
    emit("JPC", 0, 0);

    if (tokenArray[tokenArrayIndex].type != thensym)
      printErrors(11); // if must be followed by then

    tokenArrayIndex++;
    statement();
    code[jpcIdx].m = codeIndex;
    return;
  }

  if (tokenArray[tokenArrayIndex].type == xorsym) {

    tokenArrayIndex++;
    condition();
    jpcIdx = codeIndex;
    emit("JPC", 0, 0);

    if (tokenArray[tokenArrayIndex].type != thensym)
      printErrors(11);
    statement();

    if (tokenArray[tokenArrayIndex].type != semicolonsym) {
      printErrors(6);
    } else
      tokenArrayIndex++;

    if (tokenArray[tokenArrayIndex].type != elsesym) {
      printf("Else must follow if");
      exit(0);
    }

    jmpIdx = codeIndex;

    emit("JMP", 0, jmpIdx); // where are we jumping to
    tokenArrayIndex++;
    code[jpcIdx].m = codeIndex;
    statement();
    code[jmpIdx].m = codeIndex;
  }

  if (tokenArray[tokenArrayIndex].type == whilesym) {

    tokenArrayIndex++;
    int loopIdx = codeIndex;
    condition();

    if (tokenArray[tokenArrayIndex].type != dosym)
      printErrors(12); // while must be followed by do

    tokenArrayIndex++;
    jpcIdx = codeIndex;
    emit("JPC", 0, 0);
    statement();
    emit("JMP", 0, loopIdx);
    code[jpcIdx].m = codeIndex;

    return;
  }

  if (tokenArray[tokenArrayIndex].type == readsym) {

    tokenArrayIndex++;
    if (tokenArray[tokenArrayIndex].type != identsym)
      printErrors(
          2); // const, var, and read keywords must be followed by identifier

    int symIdx = symbolTableCheck(tokenArray[tokenArrayIndex].lexeme);
    // printf("SECOND: %s %d\n", tokenArray[tokenArrayIndex].lexeme, symIdx);
    if (symIdx == -1)
      printErrors(7); // undeclared identifier
    if (table[symIdx].kind != 2)
      printErrors(8); // only variable values may be altered

    tokenArrayIndex++;
    emit("SYS", 0, 2);
    // printf("EMIT STO TWO");
    emit("STO", 0, table[symIdx].addr);

    return;
  }

  if (tokenArray[tokenArrayIndex].type == writesym) {
    tokenArrayIndex++;
    expression();
    emit("SYS", 0, 1);
    return;
  }
}

// Condition Function
void condition() {
  if (tokenArray[tokenArrayIndex].type == oddsym) {
    tokenArrayIndex++;
    expression();
    emit("ODD", 0, 4);
  } else {
    expression();
    if (tokenArray[tokenArrayIndex].type == eqlsym) {
      tokenArrayIndex++;
      expression();
      emit("EQL", 0, 5);
    } else if (tokenArray[tokenArrayIndex].type == neqsym) {
      tokenArrayIndex++;
      expression();
      emit("NEQ", 0, 6);
    } else if (tokenArray[tokenArrayIndex].type == lessym) {
      tokenArrayIndex++;
      expression();
      emit("LSS", 0, 7);
    } else if (tokenArray[tokenArrayIndex].type == leqsym) {
      tokenArrayIndex++;
      expression();
      emit("LEQ", 0, 8);
    } else if (tokenArray[tokenArrayIndex].type == gtrsym) {
      tokenArrayIndex++;
      expression();
      emit("GTR", 0, 9);
    } else if (tokenArray[tokenArrayIndex].type == geqsym) {
      tokenArrayIndex++;
      expression();
      emit("GEQ", 0, 10);
    } else {
      // condition must contain comparison operator
      printErrors(13);
      exit(0);
    }
  }
}

// EXPRESSION Function
void expression() {
  term();
  while (tokenArray[tokenArrayIndex].type == plussym ||
         tokenArray[tokenArrayIndex].type == minussym) {
    if (tokenArray[tokenArrayIndex].type == plussym) {
      tokenArrayIndex++;
      term();
      emit("ADD", 0, 1);
    } else {
      tokenArrayIndex++;
      term();
      emit("SUB", 0, 2);
    }
  }
}

// TERM Function
void term() {
  factor();
  while (tokenArray[tokenArrayIndex].type == multsym ||
         tokenArray[tokenArrayIndex].type == slashsym) {
    if (tokenArray[tokenArrayIndex].type == multsym) {
      tokenArrayIndex++;
      factor();
      emit("MUL", 0, 3);
    } else if (tokenArray[tokenArrayIndex].type == slashsym) {
      tokenArrayIndex++;
      factor();
      emit("DIV", 0, 4);
    } else {
      tokenArrayIndex++;
      factor();
      emit("MOD", 0, 3);
    }
  }
}

// FACTOR Function
void factor() {
  if (tokenArray[tokenArrayIndex].type == identsym) {
    int symIdx = symbolTableCheck(tokenArray[tokenArrayIndex].lexeme);
    // printf("THIRD: %s %d\n", tokenArray[tokenArrayIndex].lexeme, symIdx);
    if (symIdx == -1) {
      // undeclared identifier
      printErrors(7);
      exit(0);
    }
    if (table[symIdx].kind == 1) {
      emit("LIT", 0, table[symIdx].val);
    } else {
      emit("LOD", 0, table[symIdx].addr);
    }
    tokenArrayIndex++;
  } else if (tokenArray[tokenArrayIndex].type == numbersym) {
    emit("LIT", 0, atoi(tokenArray[tokenArrayIndex].lexeme));
    tokenArrayIndex++;
  } else if (tokenArray[tokenArrayIndex].type == lparentsym) {
    tokenArrayIndex++;
    expression();
    if (tokenArray[tokenArrayIndex].type != rparentsym) {
      // right parenthesis must follow left parenthesis
      printErrors(14);
      exit(0);
    }
    tokenArrayIndex++;
  } else {
    // arithmetic equations must contain operands, parentheses, numbers, or
    // symbols
    printErrors(15);
    exit(0);
  }
}

// Function to export assembly code to a file (used for hw1)
void exportAssembly() {
  FILE *file = fopen("elf.txt", "w");
  if (file == NULL) {
    printf("Failed to open output file.\n");
    return;
  }
  for (int i = 0; i < codeIndex; i++) {
    fprintf(file, "%d\t%d\t%d\n", code[i].op, code[i].l, code[i].m);
  }

  fclose(file);
}

// Main function to open input file and to produce output
int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: ./program_name input_file\n");
    return 1;
  }

  char *inputFileName = argv[1];

  FILE *file = fopen(inputFileName, "r");
  FILE *outfile = fopen("token_list.txt", "w"); // output file

  if (file == NULL) {
    printf("Failed to open file: %s\n", inputFileName);
    return 1;
  }

  // printf("Source Program:\n");
  char c = fgetc(file);
  while (c != EOF) {
    // printf("%c", c);
    c = fgetc(file);
  }

  rewind(file);

  // printf("\n\nLexeme Table:\n\n");
  //  printf("lexeme\t\ttoken type\n");

  Token *token;
  tokenArray = malloc(sizeof(Token) * 100);
  while ((token = getNextToken(file))->type != EOFSYM) {
    if (token->type != ERRORSYM)
      // printf("%s\t\t%d\n", token->lexeme, token->type);
      tokenArray[tokenArrayIndex].type = token->type;
    tokenArray[tokenArrayIndex].lexeme = strdup(token->lexeme);
    tokenArray[tokenArrayIndex].is_error = token->is_error;
    tokenArrayIndex++;
  }

  rewind(file);
  if (tokenArray[tokenArrayIndex - 1].type !=
      periodsym) { // Checks if last token value is ".", if not it's an error
    printErrors(1);
    return 0;
  }

  // printf("\nToken List:\n");
  while ((token = getNextToken(file))->type != EOFSYM) {
    if (!token->is_error) {
      if (token->type == identsym && !isKeyword(token->lexeme)) {
        // printf("%d %s ", token->type, token->lexeme);
        fprintf(outfile, "%d %s ", token->type,
                token->lexeme); // write to output file
      } else {
        // printf("%d ", token->type);
        fprintf(outfile, "%d ", token->type); // write to output file
      }
    }
  }
  program(tokenArray);
  printf("\n");

  // free(tokenArray);
  // free(code);
  // free(table);

  fclose(file);
  exportAssembly();
  fclose(outfile); // close output file
  return 0;
}
