#include <iostream>
#include <string>
#include <vector>
using namespace std;

/////////////////////////////// LEXICAL ANALYS /////////////////////////////////

enum type_of_lex { // константы типов лексем(токенов)
    LEX_NULL,                                                                                   /* 0*/
    LEX_AND, LEX_BEGIN, LEX_BOOL, LEX_DO, LEX_ELSE, LEX_END, LEX_IF, LEX_FALSE, LEX_INT,        /* 9*/
    LEX_NOT, LEX_OR, LEX_PROGRAM, LEX_READ, LEX_THEN, LEX_TRUE, LEX_VAR, LEX_WHILE, LEX_WRITE,  /*18*/
    LEX_FIN,                                                                                    /*19*/
    LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LSS,   /*27*/
    LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH, LEX_LEQ, LEX_NEQ, LEX_GEQ,              /*35*/
    LEX_NUM,                                                                                    /*36*/
    LEX_ID,                                                                                     /*37*/
    POLIZ_LABEL,                                                                                /*38*/
    POLIZ_ADDRESS,                                                                              /*39*/
    POLIZ_GO,                                                                                   /*40*/
    POLIZ_FGO                                                                                   /*41*/
};


class Lex  // внутреннее представление лексемы
{
    type_of_lex t_lex;
    int v_lex;      // номер строки в таблице TD, TW или TID, либо значение в сл-е конст
public:
    Lex ( type_of_lex t = LEX_NULL, int v = 0 ): t_lex(t), v_lex(v) {}
    type_of_lex get_type () const { return t_lex; }
    int get_value () const { return v_lex; }
    friend ostream & operator << ( ostream &s, Lex l )   // для отладки
    {
        s << '(' << l.t_lex << ',' << l.v_lex << ");";
        return s;
    }
};


class Ident   // внутреннее представление идентификатора
{
    string name;            // имя в исходной программе
    bool declare;           // объявлено ли
    type_of_lex type;       // тип лексемы
    bool assign;            // присвоено ли значение 
    int value;              // значение 
public:
    Ident(): declare(false), assign(false){}
    Ident ( const string n): name(n), declare(false), assign(false ){}
    bool operator == ( const string &s ) const { return name == s; }
    string get_name () const { return name; }
    bool get_declare () const { return declare; }
    void put_declare () { declare = true; }
    type_of_lex get_type() const { return type; }
    void put_type ( type_of_lex t ) { type = t;}
    bool get_assign () const { return assign; }
    void put_asssign () {assign = true; }
    int get_value () const { return value; }
    void put_value (int v) { value = v; }
};


vector <Ident> TID;     // таблица TID -  динамическая 

int put (const string & buf)    // положить идент c именем buf в TID. return номер эл-та в таблице 
{
    vector <Ident>::iterator k;

    if( ( k = find ( TID.begin(), TID.end(), buf) ) != TID.end() ) 
        return k - TID.begin();
    TID.push_back( Ident(buf) );
    return TID.size() - 1;
}


class Scanner   //  Лексический анализатор
{
    FILE *fp;       // указатель на файл с исходной программой
    char c;         
    int look ( const string &, const char ** );     // поиск эл-та в таблице
    void gc() { c = fgetc(fp); }       
public:
    static const char *TW[], *TD[];
    Scanner ( const char * program )     // конструктор, аргумент - имя исходной программы
    {
        if( ( fp = fopen(program, "r") ) == NULL ) throw "NO FILE";
    }
    Lex get_lex();  // считывает очередную лексему и формирует внутр. представление 
    
};

// таблица служебных слов
const char *Scanner::TW[] = 
{ "", "and", "begin", "bool", "do", "else", "end", 
"if", "false", "int", "not", "or", "program", "read", 
"then", "true", "var", "while", "write", NULL
};

// таблица разделителей
const char *Scanner::TD[] = 
{ "@", ";", ",", ":", ":=", "(", ")",
 "=", "<", ">", "+", "-", "*", "/", "<=", "!=", ">=", NULL 
};


int Scanner::look ( const string &buf, const  char **list )
{
    int i = 0;
    while (list[i])
    {
        if ( buf == list[i] ) return i;
        ++i;
    }
    return  0;
}

   
Lex Scanner::get_lex()      // Лексический анализатор (реализован на основе диаграммы состояний) 
{
    enum state {H, IDENT, NUMB, COM, ALE, NEQ};   // состояния автомата
    state CS = H;
    string buf;
    int d, j;
    do
    {
        gc();
        switch(CS)
        {

        case H:   // нач состояние
            if ( c == ' ' || c == '\n' || c == '\r' || c == '\t' );   // пропускаем пробельные символы
            else if ( isalpha(c) ) { buf.push_back(c); CS = IDENT; }  // IDENT
            else if ( isdigit(c) ) { d = c - '0'; CS  = NUMB; }       // NUMB
            else if ( c == '{' ) CS = COM;                            // COM
            else if ( c == ':' || c == '<' || c == '>' ) { buf.push_back(c); CS  = ALE; }   // ALE
            else if ( c == '@' ) return Lex(LEX_FIN);                 // FIN
            else if ( c == '!' ) { buf.push_back(c); CS = NEQ; }      // NEQ
            else                                                   
            {
                buf.push_back(c);
                if ( (j = look(buf, TD)) ){                             // +, -, / , *, , , (, ), =
                    return Lex( (type_of_lex)( j + (int) LEX_FIN ), j );
                }
                else throw c;                                         // ERR
            }
            break;

        case IDENT: // идент
            if ( isalpha(c) || isdigit(c) ) buf.push_back(c);
            else 
            {
                ungetc(c, fp);
                if ( (j = look(buf, TW)) ) return Lex((type_of_lex) j, j);  // служебное слово
                else  { j = put(buf); return Lex(LEX_ID, j); }             // польз имя
            }
            break;

        case NUMB:  // число
            if ( isdigit(c) ) d = d*10 + (c - '0');
            else { ungetc(c, fp); return Lex(LEX_NUM, d); }
            break;

        case COM:  // комментарий
            if ( c == '}' ) CS = H;
            else if ( c == '@' || c ==  '{' ) throw  c;   // ERR
            break;

        case ALE:  // :, <, >
            if ( c == '=') 
            { 
                buf.push_back(c); 
                j = look(buf, TD);
                return Lex( (type_of_lex)(j + (int) LEX_FIN ), j);
            }
            else 
            {
                ungetc(c, fp);
                j = look(buf, TD);
                return Lex( (type_of_lex)(j + (int) LEX_FIN), j);
            }
            break;

        case NEQ:  // !
            if  ( c == '=')
            {
                buf.push_back(c);
                j = look(buf, TD);
                return Lex( LEX_NEQ, j);
            }
            else throw '!';
            break;

        } // end of switch

    }while(1);

}

////////////////////////////////// SYNTAX ANALYS /////////////////////////////

class Parser 
{
    Lex curr_lex;       // текущая лексема
    type_of_lex c_type;
    int c_val;
    Scanner scan;       // сканнер - часть парсера 
    void P();           // Нетерминалы для РС
    void D1(); 
    void D(); 
    void B(); 
    void S(); 
    void E(); 
    void E1(); 
    void T(); 
    void F();
    void gl()
    {
        curr_lex = scan.get_lex();
        c_type = curr_lex.get_type();
    }
public:
    Parser (const char *program): scan(program){}
    void analyze();        // РС
};


void Parser::analyze()
{
    gl();
    P();
    if ( c_type != LEX_FIN ) throw curr_lex;
    cout << endl << "OK" << endl;
}


void Parser::P()
{
    if ( c_type == LEX_PROGRAM ) gl();  // program
    else throw curr_lex;
    D1();
    if ( c_type ==  LEX_SEMICOLON ) gl();  // ;
    else throw curr_lex;
    B();
}


void Parser::D1()   // D1 --> var D {,D}
{
    if ( c_type == LEX_VAR)         // var
    {
        gl();
        D();
        while ( c_type == LEX_COMMA )   // ,
        {
            gl();
            D();
        }
    }
    else throw curr_lex;
}

// остальное пиши сам или см. example 


int main()
{
    Parser P("programModelLang");
    P.analyze();   // парсим прогу - проверяем, что все ок. Иначе выдаст исключение - все просто !)

    return 0;
}