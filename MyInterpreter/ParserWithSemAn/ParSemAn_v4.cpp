#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
using namespace std;

////////////////////////////////////////////// LEXICAL ANALISYS ///////////////////////////////////////////////////////

enum type_of_lex {   // константы типов лексем
    LEX_NULL,                                                                                   
    LEX_AND, LEX_BREAK, LEX_BOOL, LEX_ELSE, LEX_FOR, LEX_FALSE, LEX_GOTO, LEX_IF, LEX_INT, 
    LEX_NOT, LEX_OR, LEX_PROG, LEX_READ, LEX_STR, LEX_STRUC, LEX_TRUE, LEX_WHILE, LEX_WRITE,                      
    LEX_FIN,                                                                                  
    LEX_FIGL, LEX_FIGR, LEX_SEMICOLON, LEX_COMMA, LEX_ASSIGN, LEX_PLUS, LEX_MINUS, LEX_QUO,     
    LEX_LPAREN, LEX_RPAREN, LEX_MUL, LEX_DIV, LEX_GTR, LEX_LSS, LEX_LEQ, LEX_GEQ, LEX_EQ,      
    LEX_NEQ, LEX_COlON, LEX_DOT,                                                                                   
    LEX_NUM,                                                                                   
    LEX_LINE,                                                                                   
    LEX_ID,                                                                                    
    POLIZ_LABEL,                                                                                
    POLIZ_ADDRESS,                                                                              
    POLIZ_GO,                                                                                  
    POLIZ_FGO                                                                                   
};


class Lex                   // внутреннее представление Лексемы
{
    type_of_lex t_lex;
    int         v_lex;      // номер строки в таблице TD, TW или TID, либо значение в сл-е конст
    string     v_slex;      // значение в случае строковой константы  
public:
    Lex ( type_of_lex t = LEX_NULL, int v = 0, string s = "" ) : t_lex(t), v_lex(v), v_slex(s) {}
    type_of_lex get_type () const { return t_lex; }
    int get_value () const { return v_lex; }
    string get_svalue () const { return v_slex; }
    friend ostream & operator << ( ostream &s, Lex l )  
    {
        if ( l.v_slex == "" ) s << '<' << l.t_lex << ',' << l.v_lex << ">;" << endl;
        else s << '<' << l.t_lex << ',' << l.v_slex << ">;" << endl;
        return s;
    }
    void printLex() const;
};


class Ident                      // внутреннее представление Идентификатора
{
    string      name;            // имя в исходной программе
    bool     declare;            // объявлено ли
    bool    declareS;            // объявлено ли в качестве имени структуры
    type_of_lex type;            // тип идентификатора
    string     typeS;            // имя составного типа ( "0" если не является составным типом )
    bool      assign;            // присвоено ли значение 
    int        value;            // значение 
    string    svalue;            // значение в случае строки 
public:
    Ident() : declare(false), assign(false) {}
    Ident ( const string n ) : name(n), declare(false), assign(false ) {}
    bool operator == ( const string &s ) const { return name == s; }
    string get_name  () const { return name; }
    bool get_declare () const { return declare; }
    void put_declare () { declare = true; }
    type_of_lex get_type () const { return type; }
    void put_type ( type_of_lex t ) { type = t; }
    bool get_assign  () const { return assign; }
    void put_asssign () { assign = true; }
    int get_value    () const { return value; }
    void put_value (int v) { value = v; }
    string get_svalue () const { return svalue; }
    void put_svalue ( const string n ) { svalue =  n; }
    bool get_declareS () const { return declareS; }
    void put_declareS () { declareS = true; }
    string get_typeS () const { return typeS; }
    void put_typeS ( const string n ) { typeS =  n; }
};


map < string, vector < Ident > > TIDL;  // словарь из пространств имен ( таблиц TID ) 
string ns = "-1";                       // имя текущего namesapce


int put ( const string & buf)           // положить идент c именем buf в TID. return номер эл-та в таблице 
{
    vector <Ident>::iterator k;
    if( ( k = find ( TIDL[ns].begin(), TIDL[ns].end(), buf) ) != TIDL[ns].end() ) 
        return k - TIDL[ns].begin();
    TIDL[ns].push_back( Ident(buf) );
    return TIDL[ns].size() - 1;
}


class Scanner                                       //  Лексический анализатор
{
    FILE *fp;                                       // указатель на файл с исходной программой
    char   c;         
    int look ( const string &, const char ** );     // поиск эл-та в таблице
    void gc() { c = fgetc(fp); }       
public:
    static const char *TW[], *TD[];
    Scanner ( const char * program )                // конструктор, аргумент - имя исходной программы
    {
        if ( ( fp = fopen(program, "r") ) == NULL ) throw "NO FILE";
    }
    Lex get_lex ();                                 // считывает очередную лексему и формирует внутр. представление 
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
")", "*", "/", ">", "<", "<=", ">=", "==", "!=", ":", ".", NULL 
};


int Scanner::look ( const string &buf, const  char **list )
{
    int i = 0;
    while ( list[i] )
    {
        if ( buf == list[i] ) return i;
        ++i;
    }
    return  0;
}

   
Lex Scanner::get_lex ()      // Лексический анализатор (реализован на основе диаграммы состояний) 
{
    enum state {H, IDENT, NUMB, COM, ALE, NEQ, CSTR, DELIM};        // состояния автомата
    state CS = H;
    string   buf;
    int     d, j;
    do
    {
        gc(); 
        switch(CS)
        {

        case H:                                                       // нач состояние
            if ( c == ' ' || c == '\n' || c == '\r' || c == '\t' );   // пропускаем пробельные символы
            else if ( c == '"') CS = CSTR;
            else if ( isalpha(c) ) { buf.push_back(c); CS = IDENT; }  // IDENT
            else if ( isdigit(c) ) { d = c - '0'; CS  = NUMB; }       // NUMB
            else if ( c == '/' )                                      // COM
            {
                gc();
                if ( c == '*') CS = COM;
                else { ungetc(c, fp); c = '/'; buf.push_back(c); CS = DELIM; }
            }
            else if ( c == '=' || c == '<' || c == '>' ) { buf.push_back(c); CS  = ALE; }   // ALE
            else if ( c == EOF ) return Lex(LEX_FIN);                                       // FIN
            else if ( c == '!' ) { buf.push_back(c); CS = NEQ; }                            // NEQ
            else                                                                            // DELIM                             
            {
                buf.push_back(c);                                                           // {,},;,,,+,-,(,),*,/,:,.
                CS = DELIM;
            }
            break;

        case CSTR:                                                                          // строка константа
            if ( c == '"' ) return Lex(LEX_LINE, 0, buf);        
            else if ( c  == EOF ) throw c; 
            else buf.push_back(c);
            break;

        case DELIM:                                                                         // разделитель 
            ungetc(c, fp);
            if ( (j = look(buf, TD)) )  
                return Lex( (type_of_lex)( j + (int) LEX_FIN ), j );
            else throw c;                                                                   // ERR
            break;     

        case IDENT:                                                                         // идент
            if ( isalpha(c) || isdigit(c) ) buf.push_back(c);
            else 
            {
                ungetc(c, fp);
                if ( (j = look(buf, TW)) ) return Lex((type_of_lex) j, j);                  // служебное слово
                else  { j = put(buf); return Lex(LEX_ID, j); }                              // польз имя
            }
            break;

        case NUMB:                                                                          // число
            if ( isdigit(c) ) d = d*10 + (c - '0');
            else { ungetc(c, fp); return Lex(LEX_NUM, d); }
            break;

        case COM:                                                                           // комментарий
            if ( c == '*')
            {
                gc();
                if( c == '/') CS = H;
                else ungetc(c, fp);
            }
            else if ( c == '/')
            {   
                gc();
                if( c == '*') throw c;
                else ungetc(c, fp);
            }
            else if ( c == EOF) throw c;
            break;

        case ALE:                                                                           // =, <, >
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

        case NEQ:                                                                           // !
            if ( c == '=')
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
    else if ( ((int)(t_lex) < 40) ) cout << Scanner::TD[v_lex] << endl;
    else if ( ((int)(t_lex) == 40) ) cout << "Num Const: " << v_lex << endl;
    else if ( ((int)(t_lex) == 41) ) cout << "String Const: " << v_slex << endl;
    else if ( ((int)(t_lex) == 42) ) cout << "Ident: " << TIDL[ns][v_lex].get_name() << endl;
}   

////////////////// SYNTAX + SEMANTIC ANALYS //////////////////////////

template < class T, class T_EL >   // извлечение эл-та стека в x
void from_st ( T &t, T_EL &x)
{
    x = t.top();
    t.pop();
}


class Parser {
    Lex        curr_lex;                    // текущая лексема 
    type_of_lex  c_type;                    // ее тип
    int           c_val;                    // ее значение 
    Scanner        scan;
    bool       flag = 1;
    int     incycle = 0;                    // конртоль - в цикле ли мы
    int          st_int;                    // переменная для контроля описаний
    stack < type_of_lex >   st_lex;         // семантический стек 
    vector <int>          goto_vec;         // вектор меток из goto 
    stack  <int>           st_lexS;
    void dec       ( type_of_lex );         // глобальное пр-во имен
    void decS                   ();         // пр-во имен структур
    void eq_type                (); 
    void eq_typeS               ();
    void check_id               ();
    void check_op               ();
    void check_not              ();
    void check_minus            ();
    void eq_bool                ();
    void check_id_in_read       ();
    void dup                    ();
    void check_S                ();
    void putStype (   string s   );
    void P  ();                             // Нетерминалы 
    void S  ();      // ++
    void O  ();      // ++
    void C  ();      // +-
    void Op ();      // +-
    void Ops();      // ++
    void F  ();      // ++
    void A  ();      // -+
    void B  ();      // ++
    void B1 ();      // ++
    void B2 ();      // ++
    void B3 ();      // ++
    void B4 ();      // ++
    void B5 ();      // ++
    void gl ()                              // запрос новой лексемы
    {                           
        curr_lex  =       scan.get_lex ();
        c_type    =  curr_lex.get_type ();
        c_val     = curr_lex.get_value ();
    }
public:
    Parser ( const char *program ) : scan (program) { }
    void  analyze();
};


//////////////////////////////////////////////////////// RD ///////////////////////////////////////////////////////////////////


void Parser::analyze ()
{

    P();

    int i = 0;
    while ( i < goto_vec.size() )                  // проверяем goto
    {
        c_val = goto_vec[i];
        check_id();
        st_lex.pop();
        ++i;
    }

    if ( c_type != LEX_FIN) throw curr_lex;
    cout << "\nOK\n";
}

void Parser::P ()                                   // P --> program {S O Op}
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


void Parser::O ()   // O --> {{ [[int|string|bool|struc LEX_ID]] [[I|I=C]] {{ , [[I|I=C]] }}; }}   (I = LEX_ID)
{
    int i = 0;
    do 
    {
        if ( i ) gl();
        if ( c_type != LEX_INT && c_type != LEX_STR && c_type != LEX_BOOL && c_type != LEX_STRUC ) return;

        type_of_lex type = c_type;
        string typeS = "0";

        if ( c_type == LEX_STRUC )              // обработка случая структуры
        {
            gl();
            if ( c_type != LEX_ID ) throw curr_lex;
            check_S();
            typeS = TIDL[ns][c_val].get_name();
        }

        do 
        {
            gl();
            if ( c_type != LEX_ID ) throw curr_lex;

            dec(type);                          // объявляем LEX_ID 
            putStype(typeS);                    // имя составного типа 
            check_id();                         // заносим тип LEX_ID в стек st_lex для контроля инициализации 

            gl();
            if ( c_type == LEX_ASSIGN )
            {
                gl();
                C(); 
                eq_type();                      // контроль типов в инициализации 
                gl();
            } 
            else st_lex.pop();                  // если  инициализации не было, то чистим стек 

        } while ( c_type == LEX_COMMA ); if ( !i ) i = 1;

    } while ( c_type == LEX_SEMICOLON );
    
}


void Parser::S ()    // S --> {{ struct LEX_ID { O }; }}
{
    do
    {
        if ( c_type != LEX_STRUC )        return;
        gl();
        if ( c_type != LEX_ID )   throw curr_lex;
        decS();

        string nsp = ns;                                           // запоминаем имя старого пространство имен 
        ns = TIDL[ns][ c_val ].get_name();                         // переходим в новое пространство имен с именем структуры                       
        TIDL[ns] = { TIDL[nsp].begin(), TIDL[nsp].end() };         // скопируем старый namespace в новый, тк он - вложенный 

        gl();
        if ( c_type != LEX_FIGL ) throw curr_lex;

        gl();
        O();

        ns = nsp;                                                   // возвращаем старое простравнсто имен  

        if ( c_type != LEX_FIGR ) throw curr_lex;
        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        gl();
    } while (1);
}


void Parser::C ()   // C --> [+|-] LEX_NUM | LEX_LINE | LEX_TRUE | LEX_FALSE 
{
    if ( c_type == LEX_PLUS || c_type == LEX_MINUS ) gl();
    if( c_type != LEX_NUM && c_type != LEX_LINE && c_type != LEX_TRUE && c_type != LEX_FALSE ) throw curr_lex;
    if ( c_type == LEX_TRUE || c_type == LEX_FALSE ) st_lex.push ( LEX_BOOL );
    else if ( c_type == LEX_NUM ) st_lex.push ( LEX_INT );
    else st_lex.push ( LEX_STR );
}
 

void Parser::Op ()                                                  // Op -->
{
    switch( c_type )
    {

    case LEX_IF:                                                    // | if ( A ) Op else Op
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        A();
        eq_bool();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();
        gl();
        if ( c_type != LEX_ELSE ) throw curr_lex;
        gl();
        Op();
        break;

    case LEX_FOR:                                                   // | for ( [ A ]; [ A ]; [ A ] ) Op
        ++incycle;
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        for (int i = 0; i < 3; ++i)
        {
            gl();
            if ( c_type == LEX_SEMICOLON && i == 1) throw "expression is not boolean";
            if ( (c_type != LEX_SEMICOLON && (i < 2)) || (c_type != LEX_RPAREN && (i == 2)) )
            {
                A();
                ( i != 1 ) ? st_lex.pop() : eq_bool();
                if ( c_type != LEX_SEMICOLON && i < 2 ) throw curr_lex;
            } 
        }
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();
        --incycle;
        break;

    case LEX_WHILE:                                                 // | while ( A ) Op
        ++incycle;
        gl     ();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        A ();
        eq_bool();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();
        --incycle;
        break;

    case LEX_BREAK:                                                 // | break;
        if ( !incycle ) throw "break must be in cylce ";
        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        break;

    case LEX_GOTO:                                                  // | goto LEX_ID;
        gl();
        if ( c_type != LEX_ID ) throw  curr_lex;
        goto_vec.push_back(c_val);
        gl();
        if ( c_type != LEX_SEMICOLON ) throw  curr_lex;
        break;

    case LEX_READ:                                                  // | read ( LEX_ID );
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        if ( c_type != LEX_ID ) throw curr_lex;
        check_id_in_read();
        gl();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        break;

    case LEX_WRITE:                                                 // | write ( A {{ , A }} );
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        do
        {
            gl();
            A ();
            st_lex.pop();
        }while ( c_type == LEX_COMMA );
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        break;

    case LEX_FIGL:                                                  //  | {  {{ Op }}  }
        while(1)
        {
            gl();
            if ( c_type == LEX_FIGR ) break;    
            Op();
        }
        break;

    case LEX_ID:  
        st_int = c_val;
        B();                                          
        if( c_type == LEX_SEMICOLON )                 
        {
            st_lex.pop();
            break;
        }
        if( c_type == LEX_COlON )                                   // | LEX_ID : Op
        {
            if ( TIDL[ns][st_int].get_type() != LEX_GOTO ) 
                throw curr_lex;
            gl();
            Op();
        }
        else if ( c_type == LEX_ASSIGN )                            // | LEX_ID = A
        {
            gl();
            A();
            eq_type(); 
            eq_typeS();
            if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        }
        else throw curr_lex;
        break;

    default:                                                        //  | B;
        B();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex; 
    }
}


void Parser::Ops ()
{
    do
    {
        Op();
        gl();
    } while ( c_type != LEX_FIGR);
}


void Parser::F ()                                                   // F --> LEX_ID {{ . LEX_ID }}   
{
    if ( c_type != LEX_ID ) throw curr_lex;
    string structname = TIDL[ns][c_val].get_typeS();
    gl();
    if ( c_type != LEX_DOT ) return;
    if ( !flag ) st_lex.pop();
    if ( flag ) flag = 0;
    if ( structname == "0" ) throw  "not Struct type";
    string nsp = ns;
    ns = structname;                                                // меняем ns на ns структуры, чтобы проверить, объявлено ли там имя 
    gl();
    if ( c_type != LEX_ID ) throw curr_lex;
    st_lexS.push(c_val);
    check_id();
    F();
    ns = nsp;                                                       // возвращаем прежний ns
}


void Parser::A ()                                                   // A --> LEX_ID = A | B
{

    flag = 0;

    B();                                                           


    if ( c_type == LEX_ASSIGN )
    {
        if ( !flag ) throw curr_lex;
        gl();
        A();
        
        type_of_lex x;
        from_st(st_lex, x);         
        dup();
        st_lex.push(x);

        eq_type(); 
        
    } 
}


void Parser::B ()                                                   // B --> B1 {{ or B1 }}
{   
    B1();
    while ( c_type == LEX_OR )
    {
        st_lex.push(c_type);
        gl();
        B1();
        check_op();
    }    
}


void Parser::B1 ()                                                  // B1  --> B2 {{ and B2 }}
{
    B2();
    while ( c_type == LEX_AND )
    {
        st_lex.push(c_type);
        gl();
        B2();
        check_op();
    }    
}


void Parser::B2 ()                                                  // B2  --> B3 [[ <|>|<=|>=|==|!= ]] B3 | B3
{
    B3();
    if( c_type == LEX_LSS || c_type == LEX_GTR || c_type == LEX_LEQ
    || c_type == LEX_GEQ || c_type == LEX_EQ || c_type == LEX_NEQ )
    {
        st_lex.push(c_type);
        gl();
        B3();
        check_op();
    } 

}


void Parser::B3 ()                                                  // B3  --> B4 {{ [[ +|- ]] B4 }}
{
    B4();
    while ( c_type == LEX_PLUS || c_type == LEX_MINUS )
    {
        st_lex.push(c_type);
        gl();
        B4();
        check_op();
    }
}


void Parser::B4 ()                                                  // B4  --> B5 {{ [[ *|/ ]] B5 }}
{
    B5();
    if ( c_type == LEX_SEMICOLON ) return;
    while ( c_type == LEX_MUL || c_type == LEX_DIV )
    {
        st_lex.push(c_type);
        gl();
        B5();
        check_op();
    }
}


void Parser::B5 ()                                                  // B5  --> C | F | not B5 | -B5 | ( B )
{
    if ( c_type == LEX_ID)
    {
        flag = 1;
        int val = c_val;     
        F();
        if ( c_type == LEX_COlON && flag != 1 ) throw curr_lex;     // LEX_ID.LEX_ID..... :
        if ( c_type == LEX_COlON && flag == 1)                      // LEX_ID :
        {
            c_val = val;
            dec(LEX_GOTO);
            check_id();
        }
        else if ( flag == 1 )
        {
            c_val = val;
            st_lexS.push(c_val);
            check_id();
        }
        flag = 1;

        return;
    }
    else if ( c_type == LEX_NOT )
    {
        gl();
        B5();
        check_not();
        //gl();
    }
    else if ( c_type == LEX_MINUS )
    {
        gl();
        B5();
        check_minus();
        //gl();
    }
    else if ( c_type == LEX_LPAREN )
    {
        gl();
        A ();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
    }
    else if ( c_type == LEX_SEMICOLON )                                  // пустой оператор
    {
        st_lex.push(LEX_NULL);
        return;
    }
    else
    { 
        C ();
        gl();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Parser::dec ( type_of_lex type )
{
    int i = c_val;
    if ( TIDL[ns][i].get_declare () ) throw "twice";
    else 
    {
        TIDL[ns][i].put_declare    ();
        TIDL[ns][i].put_type ( type );
    }
}


void Parser::putStype ( string s ) { TIDL[ns][c_val].put_typeS(s); }


void Parser::decS ( )
{
    int i = c_val;
    if ( TIDL[ns][i].get_declareS () ) throw "twice";
    else TIDL[ns][i].put_declareS    ();
}


void Parser::check_S ( ) { 
    if ( !TIDL[ns][c_val].get_declareS() ) throw "Name of struct isn't declared"; 
}


void Parser::eq_type ()
{
    type_of_lex t;
    from_st ( st_lex, t );
    if ( t != st_lex.top () ) throw "wrong types are in =";
    st_lex.pop();
}


void Parser::eq_typeS ()
{
    if ( st_lexS.size() < 2 ) return ;
    int t1, t2;
    string ts1, ts2;
    from_st ( st_lexS, t1 );
    from_st ( st_lexS, t2 );
    ts1 = TIDL[ns][t1].get_typeS();
    ts2 = TIDL[ns][t2].get_typeS();
    if ( ts1 != ts2 ) throw "wrong types are in =";
}


void Parser::check_id ()
{
    if ( TIDL[ns][c_val].get_declare() ) st_lex.push ( TIDL[ns][c_val].get_type () );
    else throw "not declared";
}


void Parser::check_op ()
{
    type_of_lex t1, t2, op, t = LEX_INT, r = LEX_BOOL;
 
    from_st ( st_lex, t2 );
    from_st ( st_lex, op );
    from_st ( st_lex, t1 );
 
    if ( op == LEX_AND || op == LEX_OR ) t = LEX_BOOL;
    else if ( op == LEX_MINUS || op == LEX_MUL || op == LEX_DIV ) r = LEX_INT;
    else if ( op != LEX_LEQ && op != LEX_GEQ )  // операции, общие для int и string
    {
        if ( t1 == LEX_STR ) t = LEX_STR;
        if (   op == LEX_PLUS   )  r = t;
        else r = LEX_BOOL;
    }
    if ( t1 == t2  &&  t1 == t ) st_lex.push (r);
    else throw "wrong types are in operation";
}


void Parser::check_not () 
{
    if (st_lex.top() != LEX_BOOL) throw "wrong type is in not";
}


void Parser::check_minus ()
{
    if (st_lex.top() != LEX_INT) throw "wrong type is in unary minus";
}


void Parser::eq_bool () 
{
    if ( st_lex.top () != LEX_BOOL ) throw "expression is not boolean";
    st_lex.pop ();
}


void Parser::check_id_in_read () 
{
    if ( !TIDL[ns][c_val].get_declare() ) throw "not declared";
}

void Parser::dup() { st_lex.push(st_lex.top()); }


int main(int argc, char **argv)
{
    try
    {
        Parser P(argv[1]);
        P.analyze();   
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