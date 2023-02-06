/*
    Измененное правило вывода для оператора присваивания на правую рекурсию
*/

#include <iostream>
#include <string>
#include <vector>
#include <stack>
using namespace std;

/////////////////// LEXICAL ANALISYS //////////////////////

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
        if ( l.v_slex == "" ) s << '<' << l.t_lex << ',' << l.v_lex << ">;" << endl;
        else s << '<' << l.t_lex << ',' << l.v_slex << ">;" << endl;
        return s;
    }
    void printLex() const;
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

void Lex::printLex () const 
{
    if ( ((int)(t_lex) < 19) ) cout << Scanner::TW[v_lex] << endl;
    else if ( ((int)(t_lex) < 39) ) cout << Scanner::TD[v_lex] << endl;
    else if ( ((int)(t_lex) == 39) ) cout << "Num Const: " << v_lex << endl;
    else if ( ((int)(t_lex) == 40) ) cout << "String Const: " << v_slex << endl;
    else if ( ((int)(t_lex) == 41) ) cout << "Ident: " << TID[v_lex].get_name() << endl;
}   

////////////////// SYNTAX ANALYS //////////////////////////

/*
    {{X}} - итерация {X^n|n>=0}
    [[a|b]] - обязательно должен быть либо a либо b 
    [a] - a либо есть либо нет {a^n| 0 <= n <= 1}
    {X} - скобки языка
*/


class Parser {
    Lex          curr_lex;     // текущая лексема 
    type_of_lex  c_type;       // ее тип
    int          c_val;        // ее значение 
    Scanner      scan;
    void P  ();      // Нетерминалы 
    void S  ();      // ++
    void O  ();      // ++
    void C  ();      // +-
    void Op ();      // +-
    void Ops();      // ++
    void A  ();
    void B  ();      // ++
    void B1 ();      // ++
    void B2 ();      // ++
    void B3 ();      // ++
    void B4 ();      // ++
    void B5 ();      // ++
    void gl () 
    {                           // запрос новой лексемы
        curr_lex  = scan.get_lex ();
        c_type    = curr_lex.get_type ();
        c_val     = curr_lex.get_value ();
    }
public:
    Parser ( const char *program ) : scan (program) { }
    void  analyze();
};

void Parser::analyze()
{
    P();
    if ( c_type != LEX_FIN) throw curr_lex;
    cout << "\nOK\n";
}

void Parser::P()  // P --> program {S O Op}
{
    gl();
    if ( c_type != LEX_PROG ) throw curr_lex;
    gl();
    if ( c_type != LEX_FIGL ) throw curr_lex;
    gl();
    S();
    O();
    Ops();
    if ( c_type != LEX_FIGR ) throw curr_lex;
    gl();
}


void Parser::O()   // O --> {{ [[int|string|bool]] [[I|I=C]] {{ , [[I|I=C]] }}; }}   (I = LEX_ID)
{
    if ( c_type != LEX_INT && c_type != LEX_STR && c_type != LEX_BOOL ) return;

    // первый проход по  внутренней части
    gl();
    if ( c_type != LEX_ID ) throw curr_lex;
    gl();
    if( c_type == LEX_ASSIGN )
    {
        gl();
        C();
        gl();
    } 

    // итерация по внутренней части 
    while ( c_type == LEX_COMMA )
    {
        gl();
        if (c_type != LEX_ID) throw curr_lex;
        gl();
        if( c_type == LEX_ASSIGN )
        {
            gl();
            C();
            gl();
        } 
    }

    // итерация по внешней части
    while ( c_type == LEX_SEMICOLON )
    {
        gl();
        if ( c_type != LEX_INT && c_type != LEX_STR && c_type != LEX_BOOL ) return; // корректный выход здесь

        gl();
        if ( c_type != LEX_ID ) throw curr_lex;
        gl();
        if( c_type == LEX_ASSIGN )
        {
            gl();
            C();
            gl();
        } 
        while ( c_type == LEX_COMMA )
        {
            gl();
            if (c_type != LEX_ID) throw curr_lex;
            gl();
            if( c_type == LEX_ASSIGN )
            {
                gl();
                C();
                gl();
            } 
        }  
    }
    throw curr_lex; // нету ;
}


void Parser::C()   // C --> [+|-] LEX_NUM | LEX_LINE | LEX_TRUE | LEX_FALSE 
{
    if ( c_type == LEX_PLUS || c_type == LEX_MINUS ) gl();
    if( c_type != LEX_NUM && c_type != LEX_LINE && c_type != LEX_TRUE && c_type != LEX_FALSE ) throw curr_lex;
}
 

void Parser::Op()   // Op -->
{
    switch( c_type )
    {

    case LEX_IF:   // | if ( B ) Op else Op
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        B();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();
        gl();
        if ( c_type != LEX_ELSE ) throw curr_lex;
        gl();
        Op();
        break;

    case LEX_FOR:  // | for ( [ B ]; [ B ]; [ B ]; ) Op
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        for (int i = 0; i < 3; ++i)
        {
            gl();
            if ( c_type != LEX_SEMICOLON )
            {
                B();
                if ( c_type != LEX_SEMICOLON ) throw curr_lex;
            } 
        }
        gl();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();
        break;

    case LEX_WHILE:  // | while ( B ) Op
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        B();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();
        break;

    case LEX_BREAK:  // | break;
        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        break;

    case LEX_GOTO:  // | goto LEX_ID;
        gl();
        if ( c_type != LEX_ID ) throw  curr_lex;
        gl();
        if ( c_type != LEX_SEMICOLON ) throw  curr_lex;
        break;

    case LEX_READ:  // | read ( LEX_ID );
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        if ( c_type != LEX_ID ) throw curr_lex;
        gl();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        break;

    case LEX_WRITE:  // | write ( B {{ , B }} );
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        B();
        while ( c_type == LEX_COMMA )
        {
            gl();
            B();
        }
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        break;

    case LEX_FIGL:  //  | {  {{ Op }}  }
        while(1)
        {
            gl();
            if ( c_type == LEX_FIGR ) break;    // операторы кончились 
            Op();
        }
        break;

    case LEX_ID:  
        B();                                          // мб выражение, начинающееся на идентификатор 
        if( c_type == LEX_SEMICOLON ) break;          // тогда после B будет ;, иначе - смотрим дальше
        if( c_type == LEX_COlON )   // | LEX_ID : Op
        {
            gl();
            Op();
        }
        else if ( c_type == LEX_ASSIGN )   // | LEX_ID = A
        {
            A();
            if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        }
        else throw curr_lex;
        break;

    default:      //  | B;
        B();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex; 
    }
}


void Parser::A()  // A --> LEX_ID = A | B;
{
    gl();
    B();
    if ( c_type == LEX_SEMICOLON ) return;
    if ( c_type != LEX_ASSIGN ) throw curr_lex;
    A();
}


void Parser::Ops()
{
    Op();
    gl();
    while ( c_type != LEX_FIGR)
    {
        Op();
        gl();
    }
}


void Parser::B()  // B --> B1 {{ or B1 }}
{   
    B1();
    while ( c_type == LEX_OR )
    {
        gl();
        B1();
    }    
}


void Parser::B1()  // B1  --> B2 {{ and B2 }}
{
    B2();
    while ( c_type == LEX_AND )
    {
        gl();
        B2();
    }    
}


void Parser::B2()   // B2  --> B3 [[ <|>|<=|>=|==|!= ]] B3 | B3
{
    B3();
    if( c_type == LEX_LSS || c_type == LEX_GTR || c_type == LEX_LEQ
    || c_type == LEX_GEQ || c_type == LEX_EQ || c_type == LEX_NEQ )
    {
        gl();
        B3();
    } 

}


void Parser::B3()  // B3  --> B4 {{ [[ +|- ]] B4 }}
{
    B4();
    while ( c_type == LEX_PLUS || c_type == LEX_MINUS )
    {
        gl();
        B4();
    }
}


void Parser::B4()  // B4  --> B5 {{ [[ *|/ ]] B5 }}
{
    B5();
    gl();
    while ( c_type == LEX_MUL || c_type == LEX_DIV )
    {
        gl();
        B5();
        gl();
    }
}


void Parser::B5()   // B5  --> C | LEX_ID | not B5 | -B5 | ( B )
{
    if ( c_type == LEX_ID) return;
    else if ( c_type == LEX_NOT )
    {
        gl();
        B5();
    }
    else if ( c_type == LEX_MINUS )
    {
        gl();
        B5();
    }
    else if ( c_type == LEX_LPAREN )
    {
        gl();
        B();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
    }
    else C();
}


void Parser::S()    // S --> struct LEX_ID { O }
{
    if ( c_type != LEX_STRUC ) return;
    gl();
    if ( c_type != LEX_ID ) throw curr_lex;
    gl();
    if ( c_type != LEX_FIGL ) throw curr_lex;
    gl();
    O();
    if ( c_type != LEX_FIGR ) throw curr_lex;
    gl();
}


int main(int argc, char **argv)
{
    try
    {
        Parser P(argv[1]);
        P.analyze();   // парсим прогу - проверяем, что все ок. Иначе выдаст исключение - все просто !)
        return 0;
    }
    catch(Lex x)
    {
        x.printLex();
    }
    catch(const char * s)
    {
        cout << s << endl;
    }
    catch(char c)
    {
        cout << c << endl;
    }
}