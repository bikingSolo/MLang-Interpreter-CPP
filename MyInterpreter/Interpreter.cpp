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
    POLIZ_FGO,
    POLIZ_UNMIN,            // переобозначим унарный минус   
    POLIZ_STRPLUS,
    POLIZ_STRLSS,
    POLIZ_STRGTR,
    POLIZ_STREQ,
    POLIZ_STRNEQ                                                                        
};


class Lex                   // внутреннее представление Лексемы
{
    type_of_lex t_lex;
    int         v_lex;      // номер строки в таблице TD, TW или TID, либо значение в сл-е конст
    string     v_slex;      // значение в случае строковой константы  
    int         space;      // номер ns ( нужно для интерпретации )
public:
    Lex ( type_of_lex t = LEX_NULL, int v = 0, string s = "", int n=0 ) : t_lex(t), v_lex(v), v_slex(s), space(n)  {}
    type_of_lex get_type () const { return t_lex; }
    int get_value () const { return v_lex; }
    string get_svalue () const { return v_slex; }
    int get_space() const { return space; }
    friend ostream & operator << ( ostream &s, Lex l )  
    {
        if ( l.v_slex == "" ) s << '<' << l.t_lex << ',' << l.v_lex << ">;" << endl;
        else s << '<' << l.t_lex << ',' << l.v_slex << ">;" << endl;
        return s;
    }
    //void printLex() const;
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
    int     label=-1;            // значение метки
    int    tidvNum=0;            // номер ns в TIDL ( нужно для интерпретации )
public:
    Ident() : declare(false), assign(false), declareS(false) {}
    Ident ( const string n ) : name(n), declare(false), assign(false), declareS(false) {}
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
    string get_typeS  () const { return typeS; }
    void put_typeS ( const string n ) { typeS =  n; }
    int get_label  () const { return label; }
    void put_label ( int n ) { label =  n; }
    void put_tidv  (int n )  { tidvNum = n;}
    int get_tidv   ()     { return tidvNum; }
};


map < string, vector < Ident > > TIDL;      // словарь из пространств имен ( таблиц TID ) 
string ns = "-1";                           // имя текущего namesapce
vector < vector <Ident > > TIDV = {{}};     // вектор их пространств имен ( нулевого глобального и объектов структур )
int Number = 0;                             // номер текущего ns 
  


int put ( const string & buf)               // положить идент c именем buf в TID. return номер эл-та в таблице 
{
    vector <Ident>::iterator k;
    if( ( k = find ( TIDL[ns].begin(), TIDL[ns].end(), buf) ) != TIDL[ns].end() ) 
        return k - TIDL[ns].begin();
    TIDL[ns].push_back( Ident(buf) );
    return TIDL[ns].size() - 1;
}


class Scanner                                       // Лексический анализатор
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


int Scanner::look ( const string &buf, const  char **list )         // возвращает номер эл-та buf в таблице list
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

                                                                         

//////////////////////////////////////////////////// SYNTAX + SEMANTIC ANALYS + POLIZ //////////////////////////////////////////////////////////

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
    bool       flag = 1;                    // вспомогательный флажок 
    bool          flag2;
    int     incycle = 0;                    // конртоль - в цикле ли мы
    int          st_int;                    // переменная для контроля описаний
    type_of_lex    type;
    string        typeS;
    string   structname;                    // для контроля типа структур
    stack < type_of_lex >   st_lex;         // семантический стек 
    vector <int>          goto_vec;         // вектор меток из goto 
    vector <int>         break_vec;         // вектор меток для выхода из объемлющего цикла
    stack  <string>        st_lexS;         // стек для контроля присваиваний структур
    void dec       ( type_of_lex );         // объявление в локальном или глобальном пр-во имен
    void decS                   ();         // объявление в пр-ве имен структур
    void eq_type                (); 
    void eq_typeS               ();
    void check_id               ();
    void check_op               ();
    void check_not              ();
    void check_minus            ();
    void eq_bool                ();
    void check_id_in_read       ();
    void dup                    ();
    void dup2                   ();
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
    vector < Lex > poliz;                   // ПОЛИЗ
    Parser ( const char *program ) : scan (program) { }
    void  analyze();
};


//////////////////////////////////////////////////////// RD ///////////////////////////////////////////////////////////////////


void Parser::analyze ()
{

    P();

    int i = 0;
    int pl;
    while ( i < goto_vec.size() )                  // проверяем goto и выставляем значения меток 
    {
        c_val = goto_vec[i];
        check_id();
        st_lex.pop();
        ++i;
        pl = goto_vec[i];   
        poliz[pl] = Lex( POLIZ_LABEL, TIDL[ns][c_val].get_label());
        ++i;
    }
    if ( c_type != LEX_FIN) throw curr_lex;
    
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


void Parser::O ()   // O --> {{ [[int|string|bool|struct LEX_ID]] [[I|I=C]] {{ , [[I|I=C]] }}; }}   (I = LEX_ID)
{
    int i = 0;
    do 
    {
        if ( flag ) { flag = 0; goto M; }
        if ( i ) gl();
        if ( c_type != LEX_INT && c_type != LEX_STR && c_type != LEX_BOOL && c_type != LEX_STRUC ) return;

        type = c_type;
        typeS = "0";

        if ( c_type == LEX_STRUC )                     // обработка случая структуры
        {
            gl();
            if ( c_type != LEX_ID ) throw curr_lex;
            check_S();
            typeS = TIDL[ns][c_val].get_name();
        }

        do 
        {
            gl();
            M:if ( c_type != LEX_ID ) throw curr_lex;

            dec(type);                                  // объявляем LEX_ID 
            putStype(typeS);                            // имя составного типа 
            check_id();                                 // заносим тип LEX_ID в стек st_lex для контроля инициализации 
            st_int = c_val;                             // запоминаем номер в TID для инициализации 
            

            gl();
            if ( c_type == LEX_ASSIGN )
            {
                gl();
                C(); 

                if ( poliz[poliz.size() - 1].get_type() == POLIZ_UNMIN ) 
                {
                    poliz.pop_back();
                    TIDL[ns][st_int].put_value( poliz[poliz.size() - 1].get_value()*(-1) );
                } 
                else TIDL[ns][st_int].put_value( poliz[poliz.size() - 1].get_value());
                TIDL[ns][st_int].put_svalue( poliz[poliz.size() - 1].get_svalue());
                TIDL[ns][st_int].put_asssign();
                poliz.pop_back();

                eq_type();                              // контроль типов в инициализации 
                gl();
            } 
            else st_lex.pop();                          // если  инициализации не было, то чистим стек 

        } while ( c_type == LEX_COMMA ); if ( !i ) i = 1;

    } while ( c_type == LEX_SEMICOLON );
    
}


void Parser::S ()    // S --> {{ struct LEX_ID { O }; }}   struct A a 
{
    do
    {
        flag =  0;
        if ( c_type != LEX_STRUC )        return;

        gl();
        if ( c_type != LEX_ID )   throw curr_lex;
        st_int = c_val;

        gl();
        if ( c_type != LEX_FIGL )                                 // мб это поисание объекта типа структуры и мы в чужой области
        {
            if ( c_type != LEX_ID ) throw curr_lex;
            flag = 1;
            type = LEX_STRUC;
            typeS = TIDL[ns][st_int].get_name();
            return;
        }
        c_val = st_int;

        decS();

        string nsp = ns;                                           // запоминаем имя старого пространство имен 
        ns = TIDL[ns][ c_val ].get_name();                         // переходим в новое пространство имен с именем структуры                       
        TIDL[ns] = { TIDL[nsp].begin(), TIDL[nsp].end() };         // скопируем старый namespace в новый, тк он - вложенный (там только имена структур)

    

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
    int f,g=f=0;  if ( c_type == LEX_MINUS ) f++;
    if ( c_type == LEX_PLUS || c_type == LEX_MINUS ) { gl(); g++; }
    if ( c_type != LEX_NUM && c_type != LEX_LINE && c_type != LEX_TRUE && c_type != LEX_FALSE ) throw curr_lex;
    if ( c_type != LEX_NUM && g) throw curr_lex;
    if ( c_type == LEX_TRUE || c_type == LEX_FALSE )
    {
        st_lex.push ( LEX_BOOL );
        c_type == LEX_TRUE ? poliz.push_back( Lex( LEX_NUM, 1 ) ) : poliz.push_back( Lex( LEX_NUM, 0 ) );
    }
    else 
    {
        c_type == LEX_NUM ? st_lex.push ( LEX_INT ) : st_lex.push ( LEX_STR );
        poliz.push_back( curr_lex );

    } 
    if (f) poliz.push_back( Lex( POLIZ_UNMIN ) );
}
 

void Parser::Op ()                                                  // Op -->
{
    switch( c_type )
    {

    case LEX_IF:                                                    // | if ( A ) Op else Op

        int pl2, pl3;

        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        A();
        if ( !st_lexS.empty() ) st_lexS.pop();
        eq_bool();

        pl2 = poliz.size();
        poliz.push_back(Lex()); 
        poliz.push_back(Lex(POLIZ_FGO));

        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();

        pl3 = poliz.size();
        poliz.push_back(Lex()); 
        poliz.push_back(Lex(POLIZ_GO)); 
        poliz[pl2] = Lex(POLIZ_LABEL, poliz.size());

        gl();
        if ( c_type != LEX_ELSE ) throw curr_lex;
        gl();
        Op();

        poliz[pl3] = Lex(POLIZ_LABEL, poliz.size());

        break;

    case LEX_FOR:                                                   // | for ( [ A ]; [ A ]; [ A ] ) Op
        ++incycle;
        flag  = 0;
        flag2 = 0;
        int plf0, plf1, plf2, plf;
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        for (int i = 0; i < 3; ++i)
        {
            gl();
            if ( c_type == LEX_SEMICOLON && i == 1) throw "expression is not boolean";
            if ( (c_type != LEX_SEMICOLON && (i < 2)) || (c_type != LEX_RPAREN && (i == 2)) )
            {
                A();
                if ( !st_lexS.empty() ) st_lexS.pop();
                ( i != 1 ) ? st_lex.pop() : eq_bool();
                if ( c_type != LEX_SEMICOLON && i < 2 ) throw curr_lex;
            } 
            else if ( c_type == LEX_SEMICOLON && i == 0 ) flag = 1;
            else if ( c_type == LEX_RPAREN  &&  i == 2) flag2 = 1;

            if ( i == 0)
            { 
                if ( !flag ) poliz.push_back(Lex(LEX_SEMICOLON));
                plf0 = poliz.size();
            }
            if ( i == 1)
            {
                plf1 = poliz.size(); 
                poliz.push_back(Lex()); 
                poliz.push_back(Lex(POLIZ_FGO)); 
                plf2 = poliz.size(); 
                poliz.push_back(Lex()); 
                poliz.push_back(Lex(POLIZ_GO));

            }
            if ( i == 2)
            {
                if( !flag2 ) poliz.push_back(Lex(LEX_SEMICOLON)); 
                poliz.push_back(Lex(POLIZ_LABEL, plf0)); 
                poliz.push_back(Lex(POLIZ_GO)); 
                poliz[plf2] = Lex(POLIZ_LABEL, poliz.size());
            }

        }
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();

        poliz.push_back(Lex(POLIZ_LABEL, plf2+2)); 
        poliz.push_back(Lex(POLIZ_GO)); 
        poliz[plf1] = Lex(POLIZ_LABEL, poliz.size());

        if ( !break_vec.empty() ) 
        {
            plf = break_vec[0];
            break_vec.pop_back();
            poliz[plf] = Lex(POLIZ_LABEL, poliz.size());
        }

        --incycle;
        break;

    case LEX_WHILE:                                                 // | while ( A ) Op
        int pl0, pl1, pl;
        ++incycle;
        gl     ();
        if ( c_type != LEX_LPAREN ) throw curr_lex;

        pl0 = poliz.size();

        gl();
        A ();
        if ( !st_lexS.empty() ) st_lexS.pop();
        eq_bool();

        pl1 = poliz.size(); 
        poliz.push_back(Lex()); 
        poliz.push_back(Lex(POLIZ_FGO));

        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        Op();

        poliz.push_back(Lex(POLIZ_LABEL, pl0));
        poliz.push_back(Lex(POLIZ_GO)); 
        poliz[pl1] = Lex(POLIZ_LABEL, poliz.size());

        if ( !break_vec.empty() ) 
        {
            pl = break_vec[0];
            break_vec.pop_back();
            poliz[pl] = Lex(POLIZ_LABEL, poliz.size());
        }
    
        --incycle;
        break;

    case LEX_BREAK:                                                 // | break;
        if ( !incycle ) throw "break must be in cylce ";

        break_vec.push_back(poliz.size());
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_GO));

        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;
        break;

    case LEX_GOTO:                                                  // | goto LEX_ID;
        gl();
        if ( c_type != LEX_ID ) throw  curr_lex;

        goto_vec.push_back(c_val);                                  // чтобы потом проверить описана ли метка
        goto_vec.push_back(poliz.size());                           // запоминаем размер стека poliz чтобы вствить в это место адрес метки
        poliz.push_back(Lex());                                     // вставляем пустой адрес
        poliz.push_back(Lex(POLIZ_GO));

        gl();
        if ( c_type != LEX_SEMICOLON ) throw  curr_lex;
        break;

    case LEX_READ:                                                  // | read ( F );
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        gl();
        flag = 1;
        F();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex;

        poliz.push_back(Lex(LEX_READ));

        break;

    case LEX_WRITE:                                                 // | write ( A {{ , A }} );
        gl();
        if ( c_type != LEX_LPAREN ) throw curr_lex;
        do
        {
            gl();
            A ();

            poliz.push_back(Lex(LEX_WRITE));                        // write(a,b,c) ~ write(a);write(b);write(c);

            st_lex.pop();
            if ( !st_lexS.empty() ) st_lexS.pop();
            
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
            poliz.push_back( Lex( LEX_SEMICOLON ) );
            break;
        }
        if( c_type == LEX_COlON )                                   // | LEX_ID : Op
        {
            if ( TIDL[ns][st_int].get_type() != LEX_GOTO ) 
                throw curr_lex;
            gl();
            Op();
        }
        else if ( c_type == LEX_ASSIGN )                            // | F = A
        {
            gl();
            A();
            poliz.push_back( Lex( LEX_ASSIGN ) );
            eq_type(); 
            if ( !st_lexS.empty() ) eq_typeS();
            if ( c_type != LEX_SEMICOLON ) throw curr_lex;
            poliz.push_back( Lex( LEX_SEMICOLON ) );
        }
        else throw curr_lex;
        break;

    default:                                                        //  | B;
        B();
        if ( c_type != LEX_SEMICOLON ) throw curr_lex; 
        poliz.push_back( Lex( LEX_SEMICOLON ) );
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
    if ( flag ) poliz.push_back( Lex( POLIZ_ADDRESS, c_val ) );
    structname = TIDL[ns][c_val].get_typeS();
    gl();
    if ( c_type != LEX_DOT ) return;
    if ( !flag ) { st_lex.pop(); st_lexS.pop(); }
    else flag = 0;
    if ( structname == "0" ) throw  "not Struct type";
    string nsp = ns;
    ns = structname;                                                // меняем ns на ns структуры, чтобы проверить, объявлено ли там имя 
    gl();
    if ( c_type != LEX_ID ) throw curr_lex;
    poliz.push_back( Lex( POLIZ_ADDRESS, c_val ) );
    poliz.push_back( Lex( LEX_DOT ) );
    st_lexS.push(TIDL[ns][c_val].get_typeS());
    check_id();
    F();
    ns = nsp;                                                       // возвращаем прежний ns
}


void Parser::A ()                                                   // A --> F = A | B
{

    flag = 0;

    B();                                                           


    if ( c_type == LEX_ASSIGN )
    {
        if ( !flag ) throw curr_lex;
        gl();
        A();
        poliz.push_back( Lex( LEX_ASSIGN ) );

        type_of_lex x;
        from_st(st_lex, x);        
        dup();
        st_lex.push(x);

        if ( st_lexS.size() > 1 )
        {
            string y;
            from_st(st_lexS, y);
            dup2();
            st_lexS.push(y);
            eq_typeS();
        }
        else if ( st_lexS.size() == 1 ) st_lexS.pop();

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
    if ( c_type == LEX_ID)                                          // F      
    {
        flag = 1;
        int val = c_val;     
        F();
        if ( c_type == LEX_COlON && flag != 1 ) throw curr_lex;     // LEX_ID.LEX_ID..... :
        if ( c_type == LEX_COlON && flag == 1)                      // LEX_ID :
        {
            poliz.pop_back();
            c_val = val;
            dec(LEX_GOTO);
            check_id();

            TIDL[ns][c_val].put_label(poliz.size());
        }
        else if ( flag == 1 )                                       // точек небыло  
        {
            c_val = val;
            st_lexS.push(structname);
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
    }
    else if ( c_type == LEX_MINUS )
    {
        gl();
        B5();
        check_minus();
    }
    else if ( c_type == LEX_LPAREN )
    {
        gl();
        A ();
        if ( c_type != LEX_RPAREN ) throw curr_lex;
        gl();
    }
    else if ( c_type == LEX_SEMICOLON )                                  // пустой оператор (!?)
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


void Parser::putStype ( string s ) 
{
    TIDL[ns][c_val].put_typeS(s); 

    if ( TIDL[ns][c_val].get_type() == LEX_STRUC )                                // под каждый объект типа структуры s создаем простравнство имен, 
    {                                                                             // равное пространству имен соответсвующей структуры
        Number++;                                                                 // чтобы потом изменять ТАМ значения полей 
        TIDV.push_back ( { TIDL[s].begin(), TIDL[s].end() } );                    // номер этого пространства имен будет хранится в поле таблицы TID глобального пространства имен 
        TIDL[ns][c_val].put_tidv(Number);
    }

}


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
    string t1, t2;
    from_st ( st_lexS, t1 );
    from_st ( st_lexS, t2 );
    if ( t1 != t2 ) throw "wrong types are in =";
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
    if ( t == LEX_STR )
    {
        if ( op == LEX_PLUS ) poliz.push_back(  Lex( POLIZ_STRPLUS ));
        if ( op == LEX_LSS  ) poliz.push_back(  Lex( POLIZ_STRLSS  ));
        if ( op == LEX_GTR  ) poliz.push_back(  Lex( POLIZ_STRGTR  ));
        if ( op == LEX_EQ   ) poliz.push_back(  Lex( POLIZ_STREQ   ));
        if ( op == LEX_NEQ  ) poliz.push_back(  Lex( POLIZ_STRNEQ  ));

    }
    else poliz.push_back( Lex( op ) );

}


void Parser::check_not () 
{
    if (st_lex.top() != LEX_BOOL) throw "wrong type in not";
    poliz.push_back( Lex( LEX_NOT ) );
}


void Parser::check_minus ()
{
    if (st_lex.top() != LEX_INT) throw "wrong type in unary minus";
    poliz.push_back( Lex( POLIZ_UNMIN ) );
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


void Parser::dup2() { st_lexS.push(st_lexS.top()); }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class Executer
{
public:
    void execute ( vector<Lex> & poliz );

};


void Executer::execute ( vector<Lex> & poliz )
{
    TIDV[0] = {TIDL[ns].begin(), TIDL[ns].end()};
    int ns = 0, ns1;
    Lex pc_el;                                                                                      // текущий эл-т, который интерпретируем
    stack < Lex > args;                                                                             // вспомогательный стек
    Lex i,j;
    string s1,s2;
    int l, k, i1, index = 0, size = poliz.size();
    poliz.push_back(LEX_SEMICOLON);

    while ( index < size ) 
    {
        pc_el = poliz [ index ];
        switch ( pc_el.get_type () ) 
        {

        case LEX_NUM: case POLIZ_LABEL:
            args.push ( Lex( LEX_NUM, pc_el.get_value() ) );
            break;

        case POLIZ_ADDRESS: case LEX_LINE:
            args.push ( pc_el );
            break;

        case LEX_SEMICOLON:
            args.pop();
            break;
        
        case LEX_MUL:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k * l ) );
            break; 

        case LEX_DIV:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            if (l==0) throw "Division by zero";

            args.push( Lex( LEX_NUM, k / l ) );
            break;

        case LEX_LEQ:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k <= l ) );
            break;

        case LEX_GEQ:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k >= l ) );
            break;

        case LEX_AND:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k && l ) );
            break;

        case LEX_OR:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k || l ) );
            break;

        case LEX_PLUS:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k + l ) );
            break;

        case LEX_MINUS:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k - l ) );
            break;

        case LEX_LSS:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k < l ) );
            break;

        case LEX_GTR:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k > l ) );
            break;

        case LEX_EQ:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k == l ) );
            break;

        case LEX_NEQ:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, k != l ) );
            break;

        case POLIZ_STRPLUS:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_LINE ) s1 = i.get_svalue();
            else if ( TIDV[ns][i.get_value()].get_assign() ) s1 = TIDV[ns][i.get_value()].get_svalue();
            else throw " indefinite identifier ";

            ns = j.get_space();
            if ( j.get_type() == LEX_LINE ) s2 = j.get_svalue();
            else if ( TIDV[ns][j.get_value()].get_assign() ) s2 = TIDV[ns][j.get_value()].get_svalue();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_LINE, 0, s2.append(s1) ) );
            break;

        case POLIZ_STRLSS:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_LINE ) s1 = i.get_svalue();
            else if ( TIDV[ns][i.get_value()].get_assign() ) s1 = TIDV[ns][i.get_value()].get_svalue();
            else throw " indefinite identifier ";
            
            ns = j.get_space();
            if ( j.get_type() == LEX_LINE ) s2 = j.get_svalue();
            else if ( TIDV[ns][j.get_value()].get_assign() ) s2 = TIDV[ns][j.get_value()].get_svalue();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, s2 > s1 ) );
            break;

        case POLIZ_STRGTR:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_LINE ) s1 = i.get_svalue();
            else if ( TIDV[ns][i.get_value()].get_assign() ) s1 = TIDV[ns][i.get_value()].get_svalue();
            else throw " indefinite identifier ";
            
            ns = j.get_space();
            if ( j.get_type() == LEX_LINE ) s2 = j.get_svalue();
            else if ( TIDV[ns][j.get_value()].get_assign() ) s2 = TIDV[ns][j.get_value()].get_svalue();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, s2 < s1 ) );
            break;

        case POLIZ_STREQ:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_LINE ) s1 = i.get_svalue();
            else if ( TIDV[ns][i.get_value()].get_assign() ) s1 = TIDV[ns][i.get_value()].get_svalue();
            else throw " indefinite identifier ";
            
            ns = j.get_space();
            if ( j.get_type() == LEX_LINE ) s2 = j.get_svalue();
            else if ( TIDV[ns][j.get_value()].get_assign() ) s2 = TIDV[ns][j.get_value()].get_svalue();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, s2 == s1 ) );
            break;

        case POLIZ_STRNEQ:
            from_st ( args, i );
            from_st ( args, j );

            ns = i.get_space();
            if ( i.get_type() == LEX_LINE ) s1 = i.get_svalue();
            else if ( TIDV[ns][i.get_value()].get_assign() ) s1 = TIDV[ns][i.get_value()].get_svalue();
            else throw " indefinite identifier ";
            
            ns = j.get_space();
            if ( j.get_type() == LEX_LINE ) s2 = j.get_svalue();
            else if ( TIDV[ns][j.get_value()].get_assign() ) s2 = TIDV[ns][j.get_value()].get_svalue();
            else throw " indefinite identifier ";

            args.push( Lex( LEX_NUM, s2 != s1 ) );
            break;

        case POLIZ_GO:
            from_st ( args, i );
            index = i.get_value() - 1;
            break;

        case LEX_NOT:
            from_st ( args, i );

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";
            args.push( Lex( LEX_NUM, !l ) );
            break;

 
        case POLIZ_FGO:
            from_st ( args, i );
            from_st ( args, j );

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) k = j.get_value();
            else if ( TIDV[ns][j.get_value()].get_assign() ) k = TIDV[ns][j.get_value()].get_value();
            else throw " indefinite identifier ";
            if ( !k ) index = i.get_value() - 1;
            break;

        case LEX_WRITE:
            from_st ( args, j );

            ns = j.get_space();
            if ( j.get_type() == LEX_NUM ) cout << j.get_value() << endl;
            else if ( j.get_type() == LEX_LINE ) cout << j.get_svalue() << endl;
            else if ( !TIDV[ns][j.get_value()].get_assign() && TIDV[ns][j.get_value()].get_type() != LEX_STRUC ) 
                throw " indefinite identifier ";
            else if ( TIDV[ns][j.get_value()].get_type() == LEX_STR ) 
                cout << TIDV[ns][j.get_value()].get_svalue() << endl;
            else if ( TIDV[ns][j.get_value()].get_type() != LEX_STRUC ) 
                cout << TIDV[ns][j.get_value()].get_value() << endl;
            else // вывод структуры 
            {
    
                ns = TIDV[ns][j.get_value()].get_tidv();

                for ( Ident l : TIDV[ns] )      // выводим поля ( имена из пр-ва имен соотв структуры )
                {
                    if ( l.get_declareS() ) continue;
                    if ( !l.get_assign() ) throw " indefinite identifier ";
                    if ( l.get_type() == LEX_STR ) cout << l.get_name() << " : " << l.get_svalue() << endl;
                    else cout << l.get_name() << " : " << l.get_value() << endl;
                }

            }
                
            break;

        case LEX_READ:
            int k;
            from_st( args, i);
            i1 = i.get_value();
            ns = i.get_space();

            if ( TIDV[ns][i1].get_type() == LEX_INT )
            {
                cout << "Input int value for " << TIDV[ns][i1].get_name () << endl;
                cin >> k;
                TIDV[ns][i1].put_value(k);
            }
            else if ( TIDV[ns][i1].get_type() == LEX_BOOL )
            {
                string j;

                while (1) 
                {
                    cout << "Input boolean value (true or false) for " << TIDV[ns][i1].get_name() << endl;
                    cin >> j;
                    if ( j != "true" && j != "false" ) {
                        cout << "Error in input:true/false" << endl;
                        continue;
                    }
                    k = ( j == "true" ) ? 1 : 0;
                    TIDV[ns][i1].put_value(k);
                    break;
                }
            }
            else if ( TIDV[ns][i1].get_type() == LEX_STR )
            {
                string j;
                cout << "Input string for " << TIDV[ns][i1].get_name () << endl;
                cin >> j;
                TIDV[ns][i1].put_svalue(j);
            }
            else ///////////////////////////////////// ввод структуры //////////////////////////////////////////////////////
            {   
                ns = TIDV[ns][i1].get_tidv();
                int it = 0;

                for ( Ident l : TIDV[ns] )                                              // вводим поля ( имена из пр-ва имен соотв структуры )
                {
                    if ( l.get_declareS() ) { ++it; continue; }
                    if ( l.get_type() == LEX_INT )
                    {
                        cout << "Input int value for " << l.get_name () << endl;
                        cin >> k;
                        TIDV[ns][it].put_value(k);
                    }
                    else if ( l.get_type() == LEX_BOOL )
                    {
                        string j;
                        while (1) 
                        {
                            cout << "Input boolean value (true or false) for " << l.get_name() << endl;
                            cin >> j;
                            if ( j != "true" && j != "false" ) {
                                cout << "Error in input:true/false" << endl;
                                continue;
                            }
                            k = ( j == "true" ) ? 1 : 0;
                            TIDV[ns][it].put_value(k);
                            break;
                        }
                    }
                    else if ( l.get_type() == LEX_STR )
                    {
                        string j;
                        cout << "Input string for " << l.get_name () << endl;
                        cin >> j;
                        TIDV[ns][it].put_svalue(j);
                    }

                    TIDV[ns][it].put_asssign();
                    ++it;
                }

                break;
            }//////////////////////////////////////////////////////////////////////////////////////////////////////////////

            TIDV[ns][i1].put_asssign();
            break;

        case POLIZ_UNMIN:
            from_st ( args, i );

            ns = i.get_space( );
            if ( i.get_type() == LEX_NUM ) l = i.get_value();
            else if ( TIDV[ns][i.get_value()].get_assign() ) l = TIDV[ns][i.get_value()].get_value();
            else throw " indefinite identifier ";
            args.push( Lex( LEX_NUM, -l ) );
            break;

        case LEX_DOT:
            from_st ( args, i );   
            from_st ( args, j );   

            ns = j.get_space();
            ns = TIDV[ns][j.get_value()].get_tidv();
            args.push ( Lex( POLIZ_ADDRESS, i.get_value(), "", ns ) );
            break;            

        case LEX_ASSIGN:
            from_st ( args, i );   
            from_st ( args, j );    // j  --> j = i  
            

            ns = i.get_space();
            if ( i.get_type() == LEX_NUM )
            {
                l = i.get_value();
                ns = j.get_space();
                TIDV[ns][j.get_value()].put_value(l);
                TIDV[ns][j.get_value()].put_asssign();
                args.push( Lex ( LEX_NUM, l) );
            }
            else if ( i.get_type() == LEX_LINE )
            {
                s1 = i.get_svalue();
                ns = j.get_space();
                TIDV[ns][j.get_value()].put_svalue(s1);
                TIDV[ns][j.get_value()].put_asssign();
                args.push( Lex ( LEX_LINE, 0, s1) );
            }
            else if ( !TIDV[ns][i.get_value()].get_assign() && TIDV[ns][i.get_value()].get_type()!= LEX_STRUC ) 
                throw " indefinite identifier ";
            else if ( TIDV[ns][i.get_value()].get_type() == LEX_STR )
            { 
                s1 = TIDV[ns][i.get_value()].get_svalue();
                ns = j.get_space();
                TIDV[ns][j.get_value()].put_svalue(s1);
                TIDV[ns][j.get_value()].put_asssign();
                args.push( Lex ( LEX_LINE, 0, s1) );
            }
            else if ( TIDV[ns][i.get_value()].get_type() != LEX_STRUC )
            {
                l = TIDV[ns][i.get_value()].get_value();
                ns = j.get_space();
                TIDV[ns][j.get_value()].put_value(l);
                TIDV[ns][j.get_value()].put_asssign();
                args.push( Lex ( LEX_NUM, l) );
            }
            else   // присваивание структуры ( меняем нэймспейс )
            {
                
                ns1 = TIDV[ns][i.get_value()].get_tidv();
                ns = j.get_space();
                ns = TIDV[ns][j.get_value()].get_tidv();

                for ( Ident l : TIDV[ns1] )
                {
                    if ( l.get_declareS() ) continue;
                    if ( !l.get_assign() ) throw " indefinite identifier ";
                }

                TIDV[ns] = { TIDV[ns1].begin(), TIDV[ns1].end() };
                args.push( Lex( POLIZ_ADDRESS, i.get_value(), "", i.get_space()) );
                
            }
            break;

        default:
            throw "POLIZ: unexpected elem";
        
        }
        ++index;
    }

}



class Interpreter
{
    Parser pars;
    Executer E;
public:
    Interpreter( const char * program ) : pars( program ) {};
    void interpretation();
};


void Interpreter::interpretation()
{
    pars.analyze();
    E.execute( pars.poliz );

}


int main(int argc, char **argv)
{
    try {
        Interpreter I ( argv[1] );
        I.interpretation ();
        return 0;
    }
    catch ( char c ) 
    {
        cout << "unexpected symbol " << c << endl;
        return 1;
    }
    catch ( Lex l ) 
    {
        cout << "unexpected lexeme" << l << endl;
        return 1;
    }
    catch ( const char *source ) 
    {
        cout << source << endl;
        return 1;
    }
}