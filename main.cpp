#include <iostream>
#include <time.h>

#define MAX_FILES_PARSED 1000

using namespace std;

struct tokenizer_worker;

enum token_type{
    token_type_undefined  = 0 ,
    token_type_left_curly ,
    token_type_right_curly ,
    token_type_left_brace ,
    token_type_right_brace ,
    token_type_left_bracket ,
    token_type_right_bracket ,
    token_type_literal ,
    token_type_variable ,
    token_type_light_arrow ,
    token_type_decriment ,
    token_type_decriment_assign ,
    token_type_subtraction ,
    token_type_semicolon ,
    token_type_addition ,
    token_type_increment ,
    token_type_increment_assign ,
    token_type_header_initializer,
    token_type_less_than,
    token_type_less_than_or_equal,
    token_type_grater_than,
    token_type_grater_than_or_equal,
    token_type_equal,
    token_type_backslash,
    token_type_comma,
    token_type_dot,
    token_type_colon,
    token_type_assign,
    token_type_lambda,
    token_type_big_arrow,
    token_type_class_accessor,
    token_type_string_singlequoted,
    token_type_string_doublequoted,
    token_type_multiply,
    token_type_multiply_assign,
    token_type_devide,
    token_type_devide_assign,
    token_type_block_comment,
    token_type_comment,
    token_type_boolean_and,
    token_type_boolean_or,
    token_type_binary_and,
    token_type_binary_or,
    token_type_int,
    token_type_hex,
    token_type_float,
    token_type_exclamation_mark,
    token_type_question_mark,
};

char* token_type_names[] = {

        "undefined",
        "left_curly",
        "right_curly",
        "left_brace",
        "right_brace",
        "left_bracket",
        "right_bracket",
        "literal",
        "variable",
        "light_arrow",
        "decriment",
        "decriment_assign",
        "subtraction",
        "semicolon",
        "addition",
        "increment",
        "increment_assign",
        "header_initializer",
        "less_than",
        "less_than_or_equal",
        "grater_than",
        "grater_than_or_equal",
        "equal",
        "backslash",
        "comma",
        "dot",
        "colon",
        "assign",
        "lambda",
        "big_arrow",
        "class_accessor",
        "string_singlequoted",
        "string_doublequoted",
        "multiply",
        "multiply_assign",
        "devide",
        "devide_assign",
        "block_comment",
        "comment",
        "boolean_and",
        "boolean_or",
        "binary_and",
        "binary_or",
        "int",
        "hex",
        "float",
        "exclamation_mark",
        "question_mark",
};

struct Token{
    token_type type;
    char* text;
    int length;
    int column;
    int line;
};

struct SourceFile{
    Token tokens[10240] = {};
    int tokens_count = 0;
    const char* fileName = nullptr;
    char* data = nullptr;
    int length = 0;
};

void parse(SourceFile *file);

void parseFile(const char *fileName);

void tokenize(SourceFile *pFile);

void pushToken(tokenizer_worker *tokenizer, int stringLength, token_type type, SourceFile *file);


void parseLiteral(tokenizer_worker *tokenizer);

bool isLiteral(char character);

bool isNumber(char character);

void printTokens(Token *tokens, int count);

SourceFile files[MAX_FILES_PARSED];
//4GB file limit? change to 64 bit?

int   files_count = 0;


int main(int argc, char** argv) {

    if(argc < 2)
    {
        printf("Needs file input to parse");
        exit(1);
    }

    parseFile(argv[1]);


    return 0;
}

void parseFile(const char *fileName) {
    auto file = fopen(fileName, "rb");
    if(file != nullptr)
    {
        SourceFile& sourceFile = files[files_count++];
        sourceFile.fileName = fileName;
        fseek(file, 0, SEEK_END);
        sourceFile.length = ftell(file);
        fseek(file, 0, SEEK_SET);

        sourceFile.data = (char*)malloc(sourceFile.length +1);
        fread(sourceFile.data, sourceFile.length, 1, file);

        parse(&sourceFile);
    }
    else
    {
        cout << "cound not read file (" << fileName << ")" << endl;
    }
}

void parse(SourceFile *file) {
    cout << "Parsing: " << file->fileName << endl;
    clock_t timeBefore = clock();
    tokenize(file);
    auto elapsedTime = clock() - timeBefore;

    printf("It took %i ticks to tokenize the file\n", elapsedTime);
    printTokens(file->tokens, file->tokens_count);
    cout << "Token count: " << file->tokens_count;
}

void printTokens(Token *tokens, int count) {
    for(int i = 0; i < count; i++)
    {
        Token &token = tokens[i];
        printf(
                "-----\n%.*s\nType: \"%s\" at line: %i column: %i\n",
                token.length,
                token.text,
                token_type_names[token.type],
                token.line,
                token.column
        );
    }
}

struct tokenizer_worker{
    SourceFile * file;
    int column = 1;
    int line = 1;
    char* cursor;
    char* lastPos;

    tokenizer_worker operator++(int)
    {
        tokenizer_worker t(*this);
        if(this->cursor[0] == '\n')
        {
            column = 1;
            line ++;
        }
        else
        {
            column++;
        }
        cursor++;

        return t;
    }

    tokenizer_worker& operator +=(int count)
    {
        for(int i = 0; i < count ;i++)
        {
            (*this)++;
        }
        return *this;
    }

    void eatSpaces()
    {
        if(this->cursor >= this->lastPos)
            return;

        if(*this->cursor == ' ' || *this->cursor == '\t')
        {
            (*this)++;
            this->eatSpaces();
        }
        else if(*this->cursor == '\n')
        {
            (*this)++;
            this->eatSpaces();
        }
        else if(*this->cursor == '\r')
        {
            this->cursor++;
            this->eatSpaces();
        }
    }

    void pushToken(int stringLength, token_type type) {
        Token &token = file->tokens[file->tokens_count++];
        token.type = type;
        token.length = stringLength;
        token.text = this->cursor;
        token.column = this->column;
        token.line = this->line;

        (*this) += stringLength;
    }
};

void tokenize(SourceFile *file) {
    tokenizer_worker tokenizer;
    tokenizer.file = file;
    tokenizer.cursor = file->data;
    tokenizer.lastPos = file->data + file->length -1;

    while(tokenizer.cursor < tokenizer.lastPos)
    {
        tokenizer.eatSpaces();
        if(tokenizer.cursor >= tokenizer.lastPos)
        {
            break;
        }
        else if(*tokenizer.cursor == '{'){ tokenizer.pushToken(1, token_type_left_curly);}
        else if(*tokenizer.cursor == '}'){ tokenizer.pushToken(1, token_type_right_curly);}
        else if(*tokenizer.cursor == '('){ tokenizer.pushToken(1, token_type_left_brace);}
        else if(*tokenizer.cursor == ')'){ tokenizer.pushToken(1, token_type_right_brace);}
        else if(*tokenizer.cursor == '['){ tokenizer.pushToken(1, token_type_left_bracket);}
        else if(*tokenizer.cursor == ']'){ tokenizer.pushToken(1, token_type_right_bracket);}
        else if(*tokenizer.cursor == ';'){ tokenizer.pushToken(1, token_type_semicolon);}
        else if(*tokenizer.cursor == '.'){ tokenizer.pushToken(1, token_type_dot);}
        else if(*tokenizer.cursor == ','){ tokenizer.pushToken(1, token_type_comma);}
        else if(*tokenizer.cursor == '!'){ tokenizer.pushToken(1, token_type_exclamation_mark);}
        else if(*tokenizer.cursor == '?'){ tokenizer.pushToken(1, token_type_question_mark);}
        else if(*tokenizer.cursor == '\\'){ tokenizer.pushToken(1, token_type_backslash);}
        else if(*tokenizer.cursor == '-'){
            if(tokenizer.cursor < tokenizer.lastPos+1)
            {
                if(tokenizer.cursor[1] == '>'){ tokenizer.pushToken(2, token_type_light_arrow);}
                else if(tokenizer.cursor[1] == '-'){ tokenizer.pushToken(2, token_type_decriment);}
                else if(tokenizer.cursor[1] == '='){ tokenizer.pushToken(2, token_type_decriment_assign);}
                else{ tokenizer.pushToken(1, token_type_subtraction); }
            }
            else{ tokenizer.pushToken(1, token_type_subtraction); }
        }
        else if(*tokenizer.cursor == '+'){
            if(tokenizer.cursor < tokenizer.lastPos+1)
            {
                if(tokenizer.cursor[1] == '+'){ tokenizer.pushToken(2, token_type_increment);}
                else if(tokenizer.cursor[1] == '='){ tokenizer.pushToken(2, token_type_increment_assign);}
                else{ tokenizer.pushToken(1, token_type_addition); }
            }
            else{ tokenizer.pushToken(1, token_type_addition); }
        }
        else if(*tokenizer.cursor == '*'){
            if(tokenizer.cursor < tokenizer.lastPos+1)
            {
                if(tokenizer.cursor[1] == '='){ tokenizer.pushToken(2, token_type_multiply_assign);}
                else{ tokenizer.pushToken(1, token_type_multiply); }
            }
            else{ tokenizer.pushToken(1, token_type_multiply); }
        }
        else if(*tokenizer.cursor == '#'){
            char * cursor = tokenizer.cursor;
            cursor++;
            int length = 2;

            while(cursor < tokenizer.lastPos && cursor[0] != '\n')
            {
                cursor++;
                length++;
            }
            length--;//Roll back on the Newline character
            tokenizer.pushToken(length, token_type_comment);
        }
        else if(*tokenizer.cursor == '/'){
            if(tokenizer.cursor < tokenizer.lastPos+1)
            {
                if(tokenizer.cursor[1] == '='){ tokenizer.pushToken(2, token_type_devide_assign);}
                else if( tokenizer.cursor[1] == '/')
                {
                    char * cursor = tokenizer.cursor;
                    cursor++;
                    cursor++;
                    int length = 2;

                    while(cursor < tokenizer.lastPos && cursor[0] != '\n')
                    {
                        cursor++;
                        length++;
                    }
                    length--;//Roll back on the Newline character
                    tokenizer.pushToken(length, token_type_comment);
                }
                else if(tokenizer.cursor[1] == '*')
                {
                    char * cursor = tokenizer.cursor;
                    cursor++;
                    cursor++;
                    int length = 3;

                    while(cursor < tokenizer.lastPos && (cursor[-1] != '*' || cursor[0] != '/'))
                    {
                        cursor++;
                        length++;
                    }
                    tokenizer.pushToken(length, token_type_block_comment);
                }
                else{ tokenizer.pushToken(1, token_type_devide); }
            }
            else{ tokenizer.pushToken(1, token_type_devide); }
        }
        else if(*tokenizer.cursor == '<') {
            if (tokenizer.cursor < tokenizer.lastPos + 1)
            {
                if(tokenizer.cursor[1] == '?'){
                    char * cursor = tokenizer.cursor;
                    cursor++;
                    cursor++;
                    int length = 2;

                    while(cursor < tokenizer.lastPos && isLiteral(*cursor))
                    {
                        cursor++;
                        length++;
                    }
                    tokenizer.pushToken(length, token_type_header_initializer);
                }
                else if(tokenizer.cursor[1] == '='){ tokenizer.pushToken(2, token_type_less_than_or_equal);}
                else{ tokenizer.pushToken(1, token_type_less_than); }
            }
            else
            {
                tokenizer.pushToken(1, token_type_less_than);
            }
        }
        else if(*tokenizer.cursor == '>') {
            if (tokenizer.cursor < tokenizer.lastPos + 1)
            {
                if(tokenizer.cursor[1] == '='){ tokenizer.pushToken(2, token_type_grater_than_or_equal);}
                else{ tokenizer.pushToken(1, token_type_grater_than); }
            }
            else {tokenizer.pushToken(1, token_type_grater_than);}
        }
        else if(*tokenizer.cursor == '&') {
            if (tokenizer.cursor < tokenizer.lastPos + 1)
            {
                if(tokenizer.cursor[1] == '&'){ tokenizer.pushToken(2, token_type_boolean_and);}
                else{ tokenizer.pushToken(1, token_type_binary_and); }
            }
            else {tokenizer.pushToken(1, token_type_binary_and);}
        }
        else if(*tokenizer.cursor == '|') {
            if (tokenizer.cursor < tokenizer.lastPos + 1)
            {
                if(tokenizer.cursor[1] == '|'){ tokenizer.pushToken(2, token_type_boolean_or);}
                else{ tokenizer.pushToken(1, token_type_binary_or); }
            }
            else {tokenizer.pushToken(1, token_type_binary_or);}
        }
        else if(*tokenizer.cursor == '=') {
            if (tokenizer.cursor < tokenizer.lastPos + 1)
            {
                if(tokenizer.cursor[1] == '='){
                    if(tokenizer.cursor < tokenizer.lastPos +2)
                    {
                        if(tokenizer.cursor[2] == '>'){ tokenizer.pushToken(3, token_type_lambda);}
                        else{ tokenizer.pushToken(2, token_type_equal);}
                    }
                    else { tokenizer.pushToken(2, token_type_equal);}

                }
                if(tokenizer.cursor[1] == '>'){ tokenizer.pushToken(2, token_type_big_arrow);}
                else{ tokenizer.pushToken(1, token_type_assign); }
            }
            else
            {
                tokenizer.pushToken(1, token_type_assign);
            }
        }
        else if(isLiteral(*tokenizer.cursor)){ parseLiteral(&tokenizer);}
        else if(*tokenizer.cursor == '$'){ parseLiteral(&tokenizer);}
        else if(*tokenizer.cursor == '\'')
        {
            tokenizer++;//Move the tokenizer past the "'"
            char * cursor = tokenizer.cursor;
            int length = 1;

            while(cursor < tokenizer.lastPos && (cursor[-1] == '\\' || cursor[0] != '\''))
            {
                cursor++;
                length++;
            }
            length--; //Rewind back before the "'"
            tokenizer.pushToken(length, token_type_string_singlequoted);
            tokenizer++;//Move the tokenizer past the "'"
        }
        else if(*tokenizer.cursor == '\"')
        {
            tokenizer++;//Move the tokenizer past the "'"
            char * cursor = tokenizer.cursor;
            int length = 1;

            while(cursor < tokenizer.lastPos && (cursor[-1] == '\\' || cursor[0] != '\"'))
            {
                cursor++;
                length++;
            }
            length--; //Rewind back before the "'"
            tokenizer.pushToken(length, token_type_string_doublequoted);
            tokenizer++;//Move the tokenizer past the "'"
        }
        else if(*tokenizer.cursor == ':')
        {
            if(tokenizer.cursor < tokenizer.lastPos - 1 && tokenizer.cursor[1] == ':'){tokenizer.pushToken(2, token_type_class_accessor);}
            else{tokenizer.pushToken(1, token_type_colon);}
        }
        else if(isNumber(*tokenizer.cursor))
        {
            char * cursor = tokenizer.cursor;
            auto type = token_type_int;
            int length = 1;
            bool hasDot = false;
            cursor++;

            if(cursor[0] == 'x')
            {
                type = token_type_hex;
                cursor++;
                length++;
            }

            while(isNumber(cursor[0]) || (cursor[0] == '.' && !hasDot))
            {
                if(cursor[0] == '.')
                {
                    type = token_type_float;
                    hasDot = true;
                }
                cursor++;
                length++;
            }
            tokenizer.pushToken(length, type);
        }
        else{
            cout << tokenizer.cursor[0] << "(" << (int)tokenizer.cursor[0] << ") at line: " << tokenizer.line << " column: " << tokenizer.column << endl;
            tokenizer++;
        }
    }
}

void parseLiteral(tokenizer_worker *tokenizer) {
    bool valid = true;
    char* cursor = tokenizer->cursor;

    cursor++;
    while(cursor < tokenizer->lastPos && (isLiteral(*cursor) || (cursor > tokenizer->cursor && isNumber(*cursor))))
    {
        cursor++;
    }
    auto type = token_type_literal;

   /* if(*tokenizer->cursor == '$')
    {
        type |= token_type_variable;
    }
    */
    tokenizer->pushToken(cursor - tokenizer->cursor, type);
}

bool isNumber(char character) {
    if(character >= '0' && character <= '9')
        return true;
    return false;
}

bool isLiteral(char character) {
    if(character >= 'A' && character <= 'Z'){ return true; }
    else if(character >= 'a' && character <= 'z'){ return true; }
    else if(character == '_'){ return true; }
    return false;
}

