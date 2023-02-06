#include <iostream>
#include <string>
#include <vector>
#include <stack>
using namespace std;

/////////////// LEXICAL ANALYS //////////////////////

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

////////////////////////////////// SYNTAX + SEMANTIC ANALYS /////////////////////////////


template <class T, class T_EL>            // ф-ия для извлечения эл-тов стека 
void from_st ( T & st, T_EL & i ) {
    i = st.top(); st.pop();
}


class Parser {
    Lex          curr_lex;
    type_of_lex  c_type;
    int          c_val;
    Scanner      scan;
    stack < int >           st_int;
    stack < type_of_lex >   st_lex;
    void  P();
    void  D1();
    void  D();
    void  B();
    void  S();
    void  E();
    void  E1();
    void  T();
    void  F();
    void  dec ( type_of_lex type);      // обработка описаний...
    void  check_id ();                  // контроль  контекст условий в выражениях 
    void  check_op ();
    void  check_not ();
    void  eq_type ();
    void  eq_bool ();
    void  check_id_in_read ();
    void  gl () {
        curr_lex  = scan.get_lex ();
        c_type    = curr_lex.get_type ();
        c_val     = curr_lex.get_value ();
    }
public:
    vector <Lex> poliz;
    Parser ( const char *program ) : scan (program) { }
    void  analyze();
};
 
void Parser::analyze () {
    gl ();
    P ();
    if (c_type != LEX_FIN)
        throw curr_lex;
    //for_each( poliz.begin(), poliz.end(), [](Lex l){ cout << l; });
    for ( Lex l : poliz ) 
        cout << l;
    cout << endl << "Yes!!!" << endl;
}
 
void Parser::P () {
    if ( c_type == LEX_PROGRAM ) {
        gl ();
    }
    else 
        throw curr_lex;      
    D1 (); 
    if ( c_type == LEX_SEMICOLON )
        gl ();
    else
        throw curr_lex;
    B ();
}
 
void Parser::D1 () {
    if ( c_type == LEX_VAR ) {
        gl ();
        D ();
        while ( c_type == LEX_COMMA ) {
            gl ();
            D ();
        }
    }
    else
        throw curr_lex;
}
 
void Parser::D () {
    if ( c_type != LEX_ID )
        throw curr_lex;
    else {
        st_int.push ( c_val );
        gl ();
        while ( c_type == LEX_COMMA ) {
            gl ();
            if ( c_type != LEX_ID )
                throw curr_lex;
            else {
                st_int.push ( c_val );
                gl ();
            }
        }
        if ( c_type != LEX_COLON )
            throw curr_lex;
        else {
            gl ();
            if ( c_type == LEX_INT ) {
                dec ( LEX_INT );
                gl ();
            }
            else
                if ( c_type == LEX_BOOL ) {
                    dec ( LEX_BOOL );
                    gl ();
                }
                else 
                    throw curr_lex;
        } 
    }
}
 
void Parser::B () {
    if ( c_type == LEX_BEGIN ) {
        gl ();
        S ();
        while ( c_type == LEX_SEMICOLON ) {
            gl ();
            S ();
        }
        if ( c_type == LEX_END ) {
            gl ();
        }
        else {
            throw curr_lex;
        }
    }
    else
        throw curr_lex;
}
 
void Parser::S () {
    int pl0, pl1, pl2, pl3;
 
    if ( c_type == LEX_IF ) {
        gl ();
        E ();
        eq_bool ();
        pl2 = poliz.size();
        poliz.push_back ( Lex() );
        poliz.push_back ( Lex(POLIZ_FGO) );
        if ( c_type == LEX_THEN ) {
            gl ();
            S ();
            pl3 = poliz.size ();
            poliz.push_back ( Lex () );
 
            poliz.push_back ( Lex ( POLIZ_GO ) );
            poliz[pl2] = Lex ( POLIZ_LABEL, poliz.size() );
 
            if ( c_type == LEX_ELSE ) {
                gl ();
                S ();
                poliz[pl3] = Lex ( POLIZ_LABEL, poliz.size() );
            }
            else
                throw curr_lex;
        }
        else
            throw curr_lex;
    }//end if
    else if ( c_type == LEX_WHILE ) {
        pl0 = poliz.size ();
        gl ();
        E ();
        eq_bool ();
        pl1 = poliz.size (); 
        poliz.push_back ( Lex () );
        poliz.push_back ( Lex (POLIZ_FGO) );
        if ( c_type == LEX_DO ) {
            gl();
            S();
            poliz.push_back ( Lex ( POLIZ_LABEL, pl0 ) );
            poliz.push_back ( Lex ( POLIZ_GO) );
            poliz[pl1] = Lex ( POLIZ_LABEL, poliz.size() );
        }
        else
            throw curr_lex;
        }//end while
        else if ( c_type == LEX_READ ) {
            gl ();
            if ( c_type == LEX_LPAREN ) {
                gl ();
                if ( c_type == LEX_ID ) {
                    check_id_in_read ();
                    poliz.push_back ( Lex( POLIZ_ADDRESS, c_val) );
                    gl();
                }
                else
                    throw curr_lex;
                if ( c_type == LEX_RPAREN ) {
                    gl ();
                    poliz.push_back ( Lex (LEX_READ) );
                }    
                else
                    throw curr_lex;
            }
            else  
                throw curr_lex;
        }//end read
        else if ( c_type == LEX_WRITE ) {
            gl ();
            if ( c_type == LEX_LPAREN ) {
                gl ();
                E ();
                if ( c_type == LEX_RPAREN ) {
                    gl ();
                    poliz.push_back ( Lex ( LEX_WRITE ) );
                }
                else
                    throw curr_lex;
            }
            else
                throw curr_lex;
        }//end write
        else if ( c_type == LEX_ID ) { 
            check_id ();
            poliz.push_back (Lex ( POLIZ_ADDRESS, c_val ) );
            gl();
            if ( c_type == LEX_ASSIGN ) {
                gl ();
                E ();
                eq_type ();
                poliz.push_back ( Lex ( LEX_ASSIGN ) );
            }
            else
                throw curr_lex;
        }//assign-end
        else
            B();
}
 
void Parser::E () {
    E1 ();
    if ( c_type == LEX_EQ  || c_type == LEX_LSS || c_type == LEX_GTR ||
         c_type == LEX_LEQ || c_type == LEX_GEQ || c_type == LEX_NEQ ) {
        st_lex.push ( c_type );
        gl (); 
        E1 (); 
        check_op ();
    }
}
 
void Parser::E1 () {
    T ();
    while ( c_type == LEX_PLUS || c_type == LEX_MINUS || c_type == LEX_OR) {
        st_lex.push ( c_type );
        gl ();
        T ();
        check_op ();
    }
}
 
void Parser::T () {
    F ();
    while ( c_type == LEX_TIMES || c_type == LEX_SLASH || c_type == LEX_AND) {
        st_lex.push ( c_type );
        gl ();
        F ();
        check_op ();
    }
}
 
void Parser::F () {
    if ( c_type == LEX_ID ) {
        check_id ();
        poliz.push_back ( Lex ( LEX_ID, c_val ) );
        gl ();
    }
    else if ( c_type == LEX_NUM ) {
        st_lex.push ( LEX_INT );
        poliz.push_back ( curr_lex );
        gl ();
    }
    else if ( c_type == LEX_TRUE ) {
        st_lex.push ( LEX_BOOL );
        poliz.push_back ( Lex (LEX_TRUE, 1) );
        gl ();
    }
    else if ( c_type == LEX_FALSE) {
        st_lex.push ( LEX_BOOL );
        poliz.push_back ( Lex (LEX_FALSE, 0) );
        gl ();
    }
    else if ( c_type == LEX_NOT ) {
        gl (); 
        F (); 
        check_not ();
    }
    else if ( c_type == LEX_LPAREN ) {
        gl (); 
        E ();
        if ( c_type == LEX_RPAREN)
            gl ();
        else 
            throw curr_lex;
    }
    else 
        throw curr_lex;
}


int main()
{
    
    return 0;
}