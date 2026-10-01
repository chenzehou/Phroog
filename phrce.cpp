#include<bits/stdc++.h>
using namespace std;
struct tken{
	enum toktype{
	    IDENT,NUMBER,STRING,CCHAR,
		/*标识符 数字 字符串 字符 */
	    KEYWORD,
	    BELONG,								 /* :: */
	    DOT,ARROW,							 /* . -> */
	    NOT,BITNOT,PLUS,MINUS,               /* ! ~ + - */
	    INC,DEC,                             /* ++ -- */
	    ADDR,DEREF,                          /* $&取地址 @*解引用 */
	    STAR,SLASH,PERCENT,                  /* * / % */
	    LSHIFT,RSHIFT,                       /* << >> */
	    LT,LE,GT,GE,                      	 /* < <= > >= */
	    EQ,NE,                               /* == != */
	    AAND,CARET,OOR,                		 /* & ^ | */
	    AND,OR,                              /* && / || */
	    ASSIGN,                              /* = */
	    ADD_ASSIGN,SUB_ASSIGN,               /* +=,=+ -=,=- */
	    MUL_ASSIGN,DIV_ASSIGN, MOD_ASSIGN,   /* *=,=* /=,=/ %=,=% */
	    LSHIFT_ASSIGN,RSHIFT_ASSIGN,		 /* <<=,=<< >>=,=>> */
	    AAND_ASSIGN,OOR_ASSIGN,CARET_ASSIGN, /* &=,=& |=,=| ^=,=^ */
	    BITNOT_ASSIGN,						 /* ~=,=~ */
	    LPAREN,RPAREN,LBRACE,RBRACE,      	 /* ( ) { } */
	    LBRACK,RBRACK,SEMI,COLON,COMMA,  	 /* [ ] ; : , */
	    PP,                                  /* 预处理命令 */
	    EEOF,                                /* 文件结束 */
	    PPIECE,DOUPPIECE,					 /* # ## */
	    LOG,UPSORT,DOWNSORT,MATRIXINV,
	    PICIRC,CEILOP,FLOOROP,
	    UNK
	}tok;
	string txt;
	int col,row;
};
enum tpkind{
	TPWORD,TPINT,TPDOUBLE,TPFLOAT,TPCHAR,TPBOOL,TPPIECE,TPWCHAR,
	TPVOID,TPSTRUCT,TPUNION,TPALIAS,TPPTR,TPENUM,TPARR,TPFUNC
};
struct tpinfo{
    tpkind kind;
    string name;                	   /*类型名 struct/enum/typedef用*/
    struct tpinfo *base;               /*指针/数组的基础类型*/
    int arrlen;              		   /*数组长度(0表示动态)*/
    vector<struct tpinfo*>members;     /*struct/union成员*/
    vector<string>mebnames;
    vector<struct tpinfo*>argtypes;	   /* 函数参数 */
    struct tpinfo *rettype;            /* 函数返回类型 */
};
struct symbol{
	tpkind kind;
	string name;
	tpinfo *type;
    int storage;    /*0=auto, 1=static, 2=extrn, 3=register*/
    bool isconst;
    bool isexport;
    vector<symbol*>args;
    map<string,symbol*>children;
};
struct scopi{
    map<string,symbol*>syms;
    scopi *par;
    string name;
};
class symtable{
	public:
		scopi *curscope;
    	scopi *gloscope;
    	void enterscp(string name="");
	    void exitscp();
	    void addsym(symbol *sym);
	    symbol* lookup(string name);
	    symbol* lookupcur(string name);
	    symtable(){
			gloscope=new scopi();
			gloscope->par=nullptr;
			gloscope->name="_Boog_Global";
			curscope=gloscope;
		}
		~symtable(){
			/*占位*/
		}
};
void symtable::enterscp(string name){}
void symtable::exitscp(){}
void symtable::addsym(symbol *sym){}
symbol* symtable::lookup(string name){
	return NULL;
}
symbol* symtable::lookupcur(string name){
	return NULL;
}
map<string,char>tri;
set<string>keyw;
set<string>pp;
string code0,code1;
void initkeyw(void){
	/*关键字*/
	keyw.insert("auto");keyw.insert("register");keyw.insert("extrn");
	keyw.insert("static");keyw.insert("int");keyw.insert("float");
	keyw.insert("double");keyw.insert("word");keyw.insert("wchar");
	keyw.insert("any");keyw.insert("_Char");keyw.insert("_Bool");
	keyw.insert("bool");keyw.insert("piece");keyw.insert("import");
	keyw.insert("export");keyw.insert("scope");keyw.insert("defer");
	keyw.insert("return");keyw.insert("func");keyw.insert("struct");
	keyw.insert("union");keyw.insert("enum");keyw.insert("delete");
	keyw.insert("new");keyw.insert("while");keyw.insert("if");
	keyw.insert("else");keyw.insert("switch");keyw.insert("until");
	keyw.insert("goto");keyw.insert("continue");keyw.insert("break");
	keyw.insert("for");keyw.insert("do");keyw.insert("default");
	keyw.insert("sizeof");keyw.insert("typeof");keyw.insert("extern");
	/*保留关键字*/
	keyw.insert("char");keyw.insert("short");keyw.insert("dim");
	keyw.insert("let");keyw.insert("long");keyw.insert("alignof");
}
void initpp(void){
	pp.insert("include");pp.insert("define");
	pp.insert("if");pp.insert("else");pp.insert("endif");
	pp.insert("ifdef");pp.insert("ifndef");pp.insert("error");
	pp.insert("line");pp.insert("warning");pp.insert("");
	pp.insert("rem");pp.insert("program");
}
void inittri(void){
	tri["??="]='#';tri["??("]='[';tri["??)"]=']';
	tri["??<"]='{';tri["??>"]='}';tri["??/"]='\\';
	tri["??!"]='|';tri["??'"]='^';tri["??-"]='~';
}
string readall(const char *path){
    ifstream fs(path,std::ios::binary);
    if(!fs)
		return"";
    string buf(
		(istreambuf_iterator<char>(fs)),
		istreambuf_iterator<char>()
	);
    return buf;
}
string crlf2lf(string s){
    string out;
    int n=s.size();
    int i=0;
    while(i<n){
        if(i+1<n && s[i]=='\r' && s[i+1]=='\n'){
            out.push_back('\n');
            i+=2;
        }else{
            out.push_back(s[i]);
            i++;
        }
    }
    return out;
}
string t2s(string s){
    string out;
    for(char c:s){
        if(c=='\t'){
            out.push_back(' ');
        }else{
            out.push_back(c);
        }
    }
    return out;
}
string dealtri(string s){
	string out;
    int n=s.size();
    int i=0;
    while(i<n){
        if(i+2<n){
            string sub=s.substr(i,3);
            if(tri.find(sub)!=tri.end()){
                out.push_back(tri[sub]);
                i+=3;
                continue;
            }
        }
        out.push_back(s[i]);
        i++;
    }
    return out;
}
string librow(string s){
    string out;
    int n=s.size();
    int i=0;
    while(i<n){
        if(s[i]=='\\'){
            int j=i+1;
            while(j<n && (s[j]==' ' || s[j]=='\t')){
                j++;
            }
            if(j<n && s[j]=='\n'){
                i=j+1;
                continue;
            }
        }
        out.push_back(s[i]);
        i++;
    }
    return out;
}
string delrem(string s){
	string out;
    int n=s.size();
    int i=0;
    while(i<n){
        if(i+1<n && s[i]=='/' && s[i+1]=='*'){
            out.push_back(' ');
            i+=2;
            while(i+1<n && !(s[i]=='*' && s[i+1]=='/')){
                i++;
            }
            if(i+1>=n){
            	break;
			}
            i+=2;
            continue;
        }
        if(s[i]=='\n'){
            out.push_back(s[i]);
            i++;
            int j=i;
            while(j<n && (s[j]==' '||s[j]=='\t')){
                j++;
            }
            if(j+4 <= n && s[j]=='#' && s[j+1]=='r' && s[j+2]=='e' && s[j+3]=='m'){
                while(j<n && s[j]!='\n'){
                    j++;
                }
                i=j;
                continue;
            }
        }
        out.push_back(s[i]);
        i++;
    }
    return out;
}
string predeal(string s){
	s=crlf2lf(s);
	s=t2s(s);
	s=dealtri(s);
	s=librow(s);
	s=delrem(s);
	return s;
}
char peek(const string &s,int pos){
	if(pos>=(int)s.size()){
		return 0;
	}
	return s[pos];
}
size_t utf8len(unsigned char c){
    if(c<0x80){
    	return 1;
	}if((c&0xE0)==0xC0){
		return 2;
	}if((c&0xF0)==0xE0){
		return 3;
	}if((c&0xF8)==0xF0){
    	return 4;
	}return -1;
}
uint32_t peekutf8(const string &s,int pos){
    unsigned char c=s[pos];
    if(c<0x80){
    	return c;
	}
    int len=utf8len(c);
    if(len==-1){
    	printf("Error: UIP.\n");
    	exit(0);
	}
    uint32_t ch;
    if(len==2){
    	ch=c&0x1F;
	}else if(len==3){
		ch=c&0x0F;
	}else if(len==4){
		ch=c&0x07;
	}
    for(int i=1;i<len;i++){
        ch=(ch<<6)|(s[pos+i]&0x3F);
    }
    return ch;
}
char nextch(const string &s,int &pos){
	if(pos>=(int)s.size()){
		return 0;
	}
	return s[pos++];
}
uint32_t nextchutf8(const string &s,int &pos){
    if(pos>=(int)s.size()){
    	return 0;
	}
    unsigned char c=s[pos];
    if(c<0x80){
        pos++;
        return c;
    }
    int len=utf8len(c);
    uint32_t ch;
    if(len==2){
    	ch=c&0x1F;
	}else if(len==3){
		ch=c&0x0F;
	}else if(len==4){
		ch=c&0x07;
	}
    for(int i=1;i<len;i++){
        ch=(ch<<6)|(s[pos+i]&0x3F);
    }
    pos+=len;
    return ch;
}
bool consume(const string &s,int &pos,char c){
	if(peek(s,pos)==c){
		nextch(s,pos);
		return true;
	}
	return false;
}
void skipws(const string &s,int &pos){
	while(pos<(int)s.size()){
		char ch=peek(s,pos);
		if(ch==' '||ch=='\t'||ch=='\n'||ch=='\r'){
			nextch(s,pos);
		}else{
			break;
		}
	}
}
static bool is_ws(char c){
	return c==' '||c=='\t'||c=='\n'||c=='\r';
}
inline bool isalpha()
tken nxttok(const string &s,int &pos,int &row,int &col){
	tken tok;
	tok.txt="";
	tok.row=row;
	tok.col=col;
	while(1){
		char c=peek(s,pos);
		if(c==' '||c=='\t'){
			nextch(s,pos);
			col++;
			continue;
		}
		if(c=='\n'){
			nextch(s,pos);
			row++;
			col=1;
			continue;
		}
		if(c=='\r'){
			nextch(s,pos);
			continue;
		}
		break;
	}
	char c=nextch(s,pos);
	if(c==0){
		tok.tok=tken::EEOF;
		return tok;
	}
	col++;
	if(isalpha(c)||c=='_'){
		string id;
		id.push_back(c);
		while(1){
			char p=peek(s,pos);
			if(isalnum(p)||p=='_'){
				id.push_back(nextch(s,pos));
				col++;
			}else{
				break;
			}
		}
		tok.txt=id;
		if(keyw.count(id)){
			tok.tok=tken::KEYWORD;
		}else{
			tok.tok=tken::IDENT;
		}
		return tok;
	}
	if(isdigit(c)){
		string num;
		num.push_back(c);
		while(1){
			char p=peek(s,pos);
			if(isdigit(p) || p=='.'){
				num.push_back(nextch(s,pos));
				col++;
			}else{
				break;
			}
		}
		tok.txt=num;
		tok.tok=tken::NUMBER;
		return tok;
	}
	if(c=='"'){
		string str;
		while(1){
			char p=nextch(s,pos);
			col++;
			if(p==0||p=='"'){
				break;
			}
			if(p=='\\'){
				str.push_back(p);
				p=nextch(s,pos);
				col++;
				str.push_back(p);
			}else{
				str.push_back(p);
			}
		}
		tok.txt=str;
		tok.tok=tken::STRING;
		return tok;
	}
	if(c=='\''){
		string ch;
		char p=nextch(s,pos);
		col++;
		if(p=='\\'){
			ch.push_back(p);
			p=nextch(s,pos);
			col++;
			ch.push_back(p);
			p=nextch(s,pos);
			col++;
		}else{
			ch.push_back(p);
			p=nextch(s,pos);
			col++;
		}
		if(ch.size()<1 || ch.size()>4){
			tok.tok=tken::UNK;
			tok.txt=ch;
			return tok;
		}
		tok.txt=ch;
		tok.tok=tken::CCHAR;
		return tok;
	}
	tok.txt.push_back(c);
	switch(c){
		case '.':{
			if(peek(s,pos)=='-' && peek(s,pos+1)=='>'){
				nextch(s,pos);col++;
				nextch(s,pos);col++;
				tok.txt="->";
				tok.tok=tken::ARROW;
			}else{
				tok.tok=tken::DOT;
			}
			break;
		}
		case '$':tok.tok=tken::ADDR;break;
		case '@':tok.tok=tken::DEREF;break;
		case '#':{
			if(peek(s,pos)=='#'){
				nextch(s,pos);col++;
				tok.txt="##";
				tok.tok=tken::DOUPPIECE;
			}else{
				tok.tok=tken::PPIECE;
			}
			break;
		}
		case '~':{
			if(peek(s,pos+1)=='='){
				nextch(s,pos);col++;
				tok.txt="~=";
				tok.tok=tken::CARET_ASSIGN;
			}else{
				tok.tok=tken::BITNOT;
			}
			break;
		}
		case '+':{
			if(peek(s,pos)=='+'){
				nextch(s,pos);col++;
				tok.txt="++";
				tok.tok=tken::INC;
			}else if(peek(s,pos)=='='){
				nextch(s,pos);col++;
				tok.txt="+=";
				tok.tok=tken::ADD_ASSIGN;
			}else{
				tok.tok=tken::PLUS;
			}
			break;
		}
		case '-':{
			if(peek(s,pos)=='-'){
				nextch(s,pos);col++;
				tok.txt="--";
				tok.tok=tken::DEC;
			}else if(peek(s,pos)=='='){
				nextch(s,pos);col++;
				tok.txt="-=";
				tok.tok=tken::SUB_ASSIGN;
			}else{
				tok.tok=tken::MINUS;
			}
			break;
		}
		case '*':{
			if(peek(s,pos)=='='){
				nextch(s,pos);
				col++;
				tok.txt="*=";
				tok.tok=tken::MUL_ASSIGN;
			}else{
				tok.tok=tken::STAR;
			}
			break;
		}
		case '/':{
			if(peek(s,pos)=='='){
				nextch(s,pos);
				col++;
				tok.txt="/=";
				tok.tok=tken::DIV_ASSIGN;
			}else{
				tok.tok=tken::SLASH;
			}
			break;
		}
		case '%':{
			if(peek(s,pos)=='='){
				nextch(s,pos);col++;
				tok.txt="%=";
				tok.tok=tken::MOD_ASSIGN;
			}else{
				tok.tok=tken::PERCENT;
			}
			break;
		}
		case '=':{
			auto tete=[&](int off)->char{
			    size_t p=pos+off;
			    if(p>=s.size()){
			    	return 0;
				}
			    return s[p-1];
			};
			if(tete(1)=='='){
				nextch(s,pos);col++;
				tok.txt="==";
				tok.tok=tken::EQ;
				break;
			}
			char c1=tete(1);
			char c2=tete(2);
			char c3=tete(3);
			bool matched=false;
			if(c1=='+'||c1=='-'||c1=='*'||c1=='/'||c1=='%'||c1=='&'||c1=='|'||c1=='~'||c1=='!'||c1=='^'){
				if(is_ws(c2)){
				    nextch(s,pos);col++;
				    nextch(s,pos);col++;
				    switch(c1){
				    case '+':tok.tok=tken::ADD_ASSIGN;tok.txt="=+";break;
				    case '-':tok.tok=tken::SUB_ASSIGN;tok.txt="=-";break;
				    case '*':tok.tok=tken::MUL_ASSIGN;tok.txt="=*";break;
				    case '/':tok.tok=tken::DIV_ASSIGN;tok.txt="=/";break;
				    case '%':tok.tok=tken::MOD_ASSIGN;tok.txt="=%";break;
				    case '&':tok.tok=tken::AAND_ASSIGN;tok.txt="=&";break;
				    case '|':tok.tok=tken::OOR_ASSIGN;tok.txt="=|";break;
				    case '~':tok.tok=tken::BITNOT_ASSIGN;tok.txt="=~";break;
				    case '^':tok.tok=tken::CARET_ASSIGN;tok.txt="=^";break;
				    }
				    matched=true;
				}
			}
			if(!matched && c1=='<' && c2=='<' && is_ws(c3)){
				nextch(s,pos);col++;
				nextch(s,pos);col++;
				tok.tok=tken::LSHIFT_ASSIGN;
				tok.txt="=<<";
				matched=true;
			}
			if(!matched && c1=='>' && c2=='>' && is_ws(c3)){
				nextch(s,pos);col++;
				nextch(s,pos);col++;
				tok.tok=tken::RSHIFT_ASSIGN;
				tok.txt="=>>";
				matched=true;
			}
			if(matched){
				break;
			}
			tok.tok=tken::ASSIGN;
			tok.txt="=";
			break;
		}
		case '&':{
			if(peek(s,pos)=='&'){
				nextch(s,pos);col++;
				tok.txt="&&";
				tok.tok=tken::AND;
			}else if(peek(s,pos)=='='){
				nextch(s,pos);col++;
				tok.txt="&=";
				tok.tok=tken::AAND_ASSIGN;
			}else{
				tok.tok=tken::AAND;
			}
			break;
		}
		case '|':{
			if(peek(s,pos)=='|'){
				nextch(s,pos);col++;
				tok.txt="||";
				tok.tok=tken::OR;
			}else if(peek(s,pos)=='='){
				nextch(s,pos);col++;
				tok.txt="|=";
				tok.tok=tken::OOR_ASSIGN;
			}else{
				tok.tok=tken::OOR;
			}
			break;
		}
		case '^':{
			if(peek(s,pos)=='='){
				nextch(s,pos);col++;
				tok.txt="^=";
				tok.tok=tken::CARET_ASSIGN;
			}else{
				tok.tok=tken::CARET;
			}
			break;
		}
		case '<':{
			if(peek(s,pos)=='='){
				nextch(s,pos);col++;
				tok.txt="<=";
				tok.tok=tken::LE;
			}else if(peek(s,pos)=='<'){
				nextch(s,pos);col++;
				if(peek(s,pos)=='='){
					nextch(s,pos);col++;
					tok.txt="<<=";
					tok.tok=tken::LSHIFT_ASSIGN;
				}else{
					tok.txt="<<";
					tok.tok=tken::LSHIFT;
				}
			}else{
				tok.tok=tken::LT;
			}
			break;
		}
		case '>':{
			if(peek(s,pos)=='='){
				nextch(s,pos);col++;
				tok.txt=">=";
				tok.tok=tken::GE;
			}else if(peek(s,pos)=='>'){
				nextch(s,pos);col++;
				if(peek(s,pos)=='='){
					nextch(s,pos);col++;
					tok.txt=">>=";
					tok.tok=tken::RSHIFT_ASSIGN;
				}else{
					tok.txt=">>";
					tok.tok=tken::RSHIFT;
				}
			}else{
				tok.tok=tken::GT;
			}
			break;
		}
		case '?':{
            if(peek(s,pos)=='?' && peek(s,pos+1)!=0){
                nextch(s,pos);
                col++;
                char sig=nextch(s,pos);
                col++;
                switch(sig){
                    case 'u':
                        tok.tok=tken::UPSORT;
                        tok.txt="⍋";
                        break;
                    case 'd':
                        tok.tok=tken::DOWNSORT;
                        tok.txt="⍒";
                        break;
                    case 'l':
                        tok.tok=tken::LOG;
                        tok.txt="⍟";
                        break;
                    case 'q':
                        tok.tok = tken::MATRIXINV;
                        tok.txt = "⌹";
                        break;
                    case 'p':
                        tok.tok = tken::PICIRC;
                        tok.txt = "○";
                        break;
                    case 'c':
                        tok.tok = tken::CEILOP;
                        tok.txt = "⌈";
                        break;
                    case 'f':
                        tok.tok = tken::FLOOROP;
                        tok.txt = "⌊";
                        break;
                    default:{
                        pos-=2;
                        col-=2;
                        tok.tok=tken::UNK;
                        tok.txt="?";
                        break;
                    }
                }
                return tok;
            }
            tok.tok=tken::UNK;
            tok.txt="?";
            break;
        }
		case '(':tok.tok=tken::LPAREN;break;
		case ')':tok.tok=tken::RPAREN;break;
		case '{':tok.tok=tken::LBRACE;break;
		case '}':tok.tok=tken::RBRACE;break;
		case '[':tok.tok=tken::LBRACK;break;
		case ']':tok.tok=tken::RBRACK;break;
		case ';':tok.tok=tken::SEMI;break;
		case ':':{
			if(peek(s,pos)==':'){
				nextch(s,pos);col++;
				tok.tok=tken::BELONG;
				tok.txt="::";
			}else{
				tok.tok=tken::COLON;
				tok.txt=":";
			}
			break;
		}
		case ',':tok.tok=tken::COMMA;break;
		default:{
			tok.tok=tken::UNK;
			break;
		}
	}
	return tok;
}
enum astkind{
    AST_MODULE,        /* 整个文件 */
    AST_VAR_DECL,      /* auto a:int=10; */
    AST_FUNC_DEF,      /* func add(x:int,y:int)->int{ ... } */
    AST_PARAM,         /* 函参 */
    AST_SCOPE,         /* scope name{ } */
    AST_CONSTEXPR_DEF, /* constexpr word a=0; */
    AST_BLOCK,         /* { ... } */
    AST_IF,            /* if(cond){ ... }else{ ... } */
    AST_WHILE,         /* while(cond){ ... } */
    AST_UNTIL,         /* until(cond){ ... } */
    AST_RETURN,        /* return expr; */
    AST_DEFER,         /* defer{ ... } */
    AST_GOTO,          /* goto label; */
    AST_LABEL,         /* label: */
    AST_BREAK,         /* break; */
    AST_CONTINUE,      /* continue; */
    AST_COMPIF,        /* compif(expr){ } */
    AST_IDENT,         /* 变量名 */
    AST_NUMBER,        /* 123 */
    AST_STRING,        /* "hello" */
    AST_CHAR,          /* 'A' */
    AST_BINARY_OP,     /* a+b */
    AST_UNARY_OP,      /* -a, !a */
    AST_ADDR,          /* $a */
    AST_DEREF,         /* @p */
    AST_CALL,          /* func(args) */
    AST_ASSIGN,        /* a=b, a=+ b, a+=b, ... */
    AST_TYPE,          /* int, word, float ... */
    AST_EMPTY,         /* 空节点 */
};
struct astnode{
    astkind kind;
    astnode *next;       /* 同兄弟链表 */
    astnode *child;      /* 子节点链表 */
    union{
        char* sval;      /* 标识符/字符串/类型名 */
        long long ival;  /* 整数字面量 / 字符 */
        double fval;     /* 浮点字面量 */
        int optok;       /* 运算符tok id, BINOP/UNOP/ASSIGN使用 */
    }u;
    int line;
    int col;
};
astnode* astnew(astkind k){
    astnode *n=new astnode();
    n->next=nullptr;
    n->child=nullptr;
    n->line=n->col=0;
    n->kind=k;
    return n;
}
astnode* astnident(const std::string& s){
    astnode *n=astnew(AST_IDENT);
    n->u.sval=strdup(s.c_str());
    return n;
}
astnode* astnnum(long long v){
    astnode *n=astnew(AST_NUMBER);
    n->u.ival=v;
    return n;
}
astnode* astnchar(char ch){
    astnode *n=astnew(AST_CHAR);
    n->u.ival=static_cast<long long>(ch);
    return n;
}
astnode* astnstr(const std::string& s){
    astnode *n=astnew(AST_STRING);
    n->u.sval=strdup(s.c_str());
    return n;
}
astnode* astntype(const std::string& s){
    astnode *n=astnew(AST_TYPE);
    n->u.sval=strdup(s.c_str());
    return n;
}
astnode* astnbinop(int op,astnode* l,astnode* r){
    astnode *n=astnew(AST_BINARY_OP);
    n->u.optok=op;
    n->child=l;
    if(l){
        l->next=r;
    }
    return n;
}
astnode* astnunop(int op,astnode* sub){
    astnode *n=astnew(AST_UNARY_OP);
    n->u.optok=op;
    n->child=sub;
    return n;
}
astnode* astnassign(int op, astnode* lhs,astnode* rhs){
    astnode *n=astnew(AST_ASSIGN);
    n->u.optok=op;
    n->child=lhs;
    if(lhs){
        lhs->next=rhs;
    }
    return n;
}
void astfree(astnode* n){
    if(!n){
        return;
    }
    astfree(n->child);
    astfree(n->next);
    if(n->kind==AST_IDENT || n->kind==AST_STRING || n->kind==AST_TYPE){
        free(n->u.sval);
    }
    delete n;
}
static vector<tken>tokens;
static int tokid=0;
static tken now;
void nxttok(){
    if(tokid<(int)tokens.size()){
        now=tokens[tokid++];
    }
}
tken peektok(){
    if(tokid<(int)tokens.size()){
        return tokens[tokid];
    }
    tken eof;
    eof.tok=tken::EEOF;
    return eof;
}
bool match(int want){
    if(now.tok==want){
        nxttok();
        return 1;
    }
    return 0;
}
/*
void expect(int want){
    if(now.tok!=want){
        printf("Error: Expected %d at %d:%d\n",want,now.row,row.col);
        exit(1);
    }
    nxttok();
}
astnode* parcode(){
    astnode *mod=astnew(AST_MODULE);
    mod->line=mod->col=1;
    nxttok();
    while(now.tok!=tken::EEOF){
        astnode *decl=pargdecl();
        if(decl){
            if(!mod->child){
                mod->child=decl;
            }else{
                astnode *p=mod->child;
                while(p->next){
                	p=p->next;
				}
                p->next=decl;
            }
        }
    }
    return mod;
}
astnode* pargdecl(){
    switch(now.tok){
        case tken::KEYWORD:{
            string kw=now.txt;
            if(kw=="func"){
            	return parfuncdef();
			}if(kw=="scope"){
            	return parscope();
			}if(kw=="import"){
				return parimport();
			}if(kw=="export"){
				return parexport();
			}if(kw=="constexpr"){
				return parconstexpr();
			}
            return parvardef();
        }
        case tken::IDENT:{
        	if(isglobalscope()){
			    if(peektok().tok==tken::SEMI){
			        return parse_extrn_decl();
			    }else if(peektok().tok==tken::LPAREN){
			        return parfuncdef();
			    }
			}else{
			    return parcall();
			}
			break;
		}
        default:
            printf("Error: unexpected token '%s' at %d:%d\n",
                   now.txt.c_str(), now.row, now.col);
            exit(1);
    }
}
*/
int main(int argc,char *argv[]){
	if(argc<2){
		printf("Usage: boogc file\n");
		return 1;
	}
	inittri();initkeyw();initpp();
	string src=string(argv[1])+".boog";
	code0=readall(src.c_str());
	code1=predeal(code0);
	FILE *ftmp;
	char fmt[1001];
	string tmpname=string(argv[1])+".tmp";
	ftmp=fopen(tmpname.c_str(),"wb");
	if(ftmp!=NULL){
		fwrite(code1.data(),1,code1.size(),ftmp);
		fclose(ftmp);
	}
	sprintf(fmt,"gcc -E -P -trigraphs -I./include -x c %s -o %s.i",tmpname.c_str(),argv[1]);
	system(fmt);
	string code;
	FILE *fgcc=fopen((string(argv[1])+".i").c_str(),"rb");
	if(fgcc){
		fseek(fgcc,0,SEEK_END);
		size_t sz=ftell(fgcc);
		fseek(fgcc,0,SEEK_SET);
		code.resize(sz);
		fread(&code[0],1,sz,fgcc);
		fclose(fgcc);
	}
	printf("Predeal code: \n%s\n\n----------------------\n",code1.c_str());
	printf("GCC preprocess output:\n%s\n",code.c_str());
	remove(tmpname.c_str());

	int pos=0;
	int row=1;
	int col=1;
	auto token_name=[](tken::toktype t)->const char*{
		switch(t){
		case tken::BELONG:return "BELONG";
		case tken::IDENT:return "IDENT";
		case tken::NUMBER:return "NUMBER";
		case tken::STRING:return "STRING";
		case tken::CCHAR:return "CCHAR";
		case tken::KEYWORD:return "KEYWORD";
		case tken::DOT:return "DOT";
		case tken::ARROW:return "ARROW";
		case tken::NOT:return "NOT";
		case tken::BITNOT:return "BITNOT";
		case tken::PLUS:return "PLUS";
		case tken::MINUS:return "MINUS";
		case tken::INC:return "INC";
		case tken::DEC:return "DEC";
		case tken::ADDR:return "ADDR";
		case tken::DEREF:return "DEREF";
		case tken::STAR:return "STAR";
		case tken::SLASH:return "SLASH";
		case tken::PERCENT:return "PERCENT";
		case tken::LSHIFT:return "LSHIFT";
		case tken::RSHIFT:return "RSHIFT";
		case tken::LT:return "LT";
		case tken::LE:return "LE";
		case tken::GT:return "GT";
		case tken::GE:return "GE";
		case tken::EQ:return "EQ";
		case tken::NE:return "NE";
		case tken::AAND:return "AAND";
		case tken::CARET:return "CARET";
		case tken::OOR:return "OOR";
		case tken::AND:return "AND";
		case tken::OR:return "OR";
		case tken::ASSIGN:return "ASSIGN";
		case tken::ADD_ASSIGN:return "ADD_ASSIGN";
		case tken::SUB_ASSIGN:return "SUB_ASSIGN";
		case tken::MUL_ASSIGN:return "MUL_ASSIGN";
		case tken::DIV_ASSIGN:return "DIV_ASSIGN";
		case tken::MOD_ASSIGN:return "MOD_ASSIGN";
		case tken::LSHIFT_ASSIGN:return "LSHIFT_ASSIGN";
		case tken::RSHIFT_ASSIGN:return "RSHIFT_ASSIGN";
		case tken::AAND_ASSIGN:return "AAND_ASSIGN";
		case tken::OOR_ASSIGN:return "OOR_ASSIGN";
		case tken::CARET_ASSIGN:return "CARET_ASSIGN";
		case tken::LPAREN:return "LPAREN";
		case tken::RPAREN:return "RPAREN";
		case tken::LBRACE:return "LBRACE";
		case tken::RBRACE:return "RBRACE";
		case tken::LBRACK:return "LBRACK";
		case tken::RBRACK:return "RBRACK";
		case tken::SEMI:return "SEMI";
		case tken::COLON:return "COLON";
		case tken::COMMA:return "COMMA";
		case tken::PPIECE:return "PPIECE";
		case tken::DOUPPIECE:return "DOUPPIECE";
		case tken::EEOF:return "EEOF";
		case tken::LOG:return "LOG";
		case tken::UNK:return "UNK";
		default:return "???";
		}
	};
	printf("\n==== Token stream ====\n");
	for(;;){
		tken tok=nxttok(code,pos,row,col);
		printf("[%2d,%2d] %-16s |%s|\n",
			tok.row,tok.col,token_name(tok.tok),
			tok.txt.c_str()
		);
		if(tok.tok==tken::EEOF){
			break;
		}
	}
	return 0;
}