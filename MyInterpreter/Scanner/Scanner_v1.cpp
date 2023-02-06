#include <iostream>
#include <string>
#include <vector>
using namespace std;

// LEXICAL ANALISYS 

enum type_of_lex { // константы типов лексем(токенов)
    LEX_NULL,                                                                                   
    LEX_AND, LEX_BREAK, LEX_BOOL, LEX_ELSE, LEX_FOR, LEX_FALSE, LEX_GOTO, LEX_IF, LEX_INT, 
    LEX_NOT, LEX_OR, LEX_PROG, LEX_READ, LEX_STR, LEX_STRUC, LEX_TRUE, LEX_WHILE, LEX_WRITE,                      
    LEX_FIN,                                                                                  
    LEX_FIGL, LEX_FIGR, LEX_SEMICOLON, LEX_COMMA, LEX_ASSIGN, LEX_PLUS, LEX_MINUS, LEX_QUO,     
    LEX_LPAREN, LEX_RPAREN, LEX_MUL, LEX_DIV, LEX_GTR, LEX_LSS, LEX_LEQ, LEX_GEQ, LEX_EQ,      
    LEX_NEQ, LEX_COlON,                                                                                    
    LEX_NUM,                                                                                   
    LEX_LINE,                                                                                   
    LEX_ID,                                                                                    
    POLIZ_LABEL,                                                                                
    POLIZ_ADDRESS,                                                                              
    POLIZ_GO,                                                                                  
    POLIZ_FGO                                                                                   
};


class Lex  // внутреннее представление лексемы
{
    type_of_lex t_lex;
    int v_lex;      // номер строки в таблице TD, TW или TID, либо значение в сл-е конст
    string v_slex;  // значение в случае строковой константы  
public:
    Lex ( type_of_lex t = LEX_NULL, int v = 0, string s = "" ): t_lex(t), v_lex(v), v_slex(s) {}
    type_of_lex get_type () const { return t_lex; }
    int get_value () const { return v_lex; }
    string get_svalue () const { return v_slex; }
    friend ostream & operator << ( ostream &s, Lex l )   // для отладки
    {
        if( l.v_slex == "" ) s << '<' << l.t_lex << ',' << l.v_lex << ">;" << endl;
        else s << '<' << l.t_lex << ',' << l.v_slex << ">;" << endl;
        return s;
    }
    void printLex () const;     // для демонстрации - выводит токен в исходном виде
};


class Ident   // внутреннее представление идентификатора
{
    string name;            // имя в исходной программе
    bool declare;           // объявлено ли
    type_of_lex type;       // тип лексемы
    bool assign;            // присвоено ли значение 
    int value;              // значение 
    string svalue;          // значение в случае строки 
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
    string get_svalue () const { return svalue; }
    void put_svalue ( const string n ) { svalue =  n; }
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
    vector<Lex> LexAnalyze ();  // последовательность всех токенов во внутреннем представлении 
};

// таблица служебных слов
const char *Scanner::TW[] = 
{ "", "and", "break", "bool", "else", "for", "false", 
"goto", "if", "int", "not", "or", "program", "read", 
"string", "struct", "true", "while", "write", NULL
};

// таблица разделителей
const char *Scanner::TD[] = 
{ "@", "{", "}", ";", ",", "=", "+", "-", "\"", "(",
")", "*", "/", ">", "<", "<=", ">=", "==", "!=", ":", NULL 
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
    enum state {H, IDENT, NUMB, COM, ALE, NEQ, CSTR, DELIM};   // состояния автомата
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
            else if ( c == '"') CS = CSTR;
            else if ( isalpha(c) ) { buf.push_back(c); CS = IDENT; }  // IDENT
            else if ( isdigit(c) ) { d = c - '0'; CS  = NUMB; }       // NUMB
            else if ( c == '/' )                                      // COM
            {
                gc();
                if ( c == '*') CS = COM;
                else { ungetc(c, fp); c = '/'; buf.push_back(c); CS = DELIM;}
            }
            else if ( c == '=' || c == '<' || c == '>' ) { buf.push_back(c); CS  = ALE; }   // ALE
            else if ( c == EOF ) return Lex(LEX_FIN);                 // FIN
            else if ( c == '!' ) { buf.push_back(c); CS = NEQ; }      // NEQ
            else                                                      // DELIM                             
            {
                buf.push_back(c);                                     // {,},;,,,+,-,(,),*,/,:
                CS = DELIM;
            }
            break;

        case CSTR:  // строка константа
            if ( c == '"' ) return Lex(LEX_LINE, 0, buf);        
            else if ( c  == EOF ) throw c; 
            else buf.push_back(c);
            break;

        case DELIM:  // разделитель 
            ungetc(c, fp);
            if ( (j = look(buf, TD)) )  return Lex( (type_of_lex)( j + (int) LEX_FIN ), j );
            else throw c;     // ERR
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
            if( c == '*')
            {
                gc();
                if( c == '/') CS = H;
                else ungetc(c, fp);
            }
            else if( c == '/')
            {   
                gc();
                if( c == '*') throw c;
                else ungetc(c, fp);
            }
            else if( c == EOF) throw c;
            break;

        case ALE:  // =, <, >
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

vector<Lex> Scanner::LexAnalyze()
{
    vector<Lex> tokns;
    Lex l;
    while ( (l = get_lex()).get_type() != LEX_FIN ) tokns.push_back(l);
    tokns.push_back(l);
    return tokns;
}


void Lex::printLex () const 
{
    if ( ((int)(t_lex) < 19) ) cout << Scanner::TW[v_lex] << endl;
    else if ( ((int)(t_lex) < 39) ) cout << Scanner::TD[v_lex] << endl;
    else if ( ((int)(t_lex) == 39) ) cout << "Num Const: " << v_lex << endl;
    else if ( ((int)(t_lex) == 40) ) cout << "String Const: " << v_slex << endl;
    else if ( ((int)(t_lex) == 41) ) cout << "Ident: " << TID[v_lex].get_name() << endl;
}   

int main(int argc, char *argv[])
{
    try
    {
        Scanner S(argv[1]);                  // открываем файл с прогой
        vector<Lex> lex = S.LexAnalyze();       // проводим Лекс Анализ и получаем вектор токенов + TID
        vector<Lex>::iterator i = lex.begin();
        cout << "\n\n\n/////////////// INTERNAL VIEW OF TOKENS ///////////\n\n";
        while ( i != lex.end())
        {
            cout << *i;
            i++;
        }
        cout << "\n/////////////////////////////////////\n\n\n";
        cout << "//////////// EXTERNAL VIEW OF TOKENS ///////////////\n\n";
        i = lex.begin();
        while ( i != lex.end())
        {
            i->printLex();
            i++;
        }
        cout << "\n/////////////////////////////////////\n\n\n";
        cout << "//////////////  TID /////////////////\n\n";
        vector<Ident>::iterator k = TID.begin();
        while ( k != TID.end())
        {
            cout << k->get_name() << endl;
            k++;
        }
        cout << "\n/////////////////////////////////////\n\n\n";
        return 0;
    }
    catch(char x)
    {
        cout << x;
    }
}