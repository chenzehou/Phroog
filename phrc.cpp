#include<bits/stdc++.h>
using namespace std;
struct tken{
	enum toktype{
		IDENT,NUMBER,STRING,CCHAR,
		/*标识符 数字 字符串 字符 */
		KEYWORD,
		BELONG,								 /* :: */
		DOT,ARROW,							 /* . -> */
		NOT,BITNOT,PLUS,MINUS,			   /* ! ~ + - */
		INC,DEC,							 /* ++ -- */
		ADDR,DEREF,						  /* $&取地址 @*解引用 */
		STAR,SLASH,PERCENT,				  /* * / % */
		LSHIFT,RSHIFT,					   /* << >> */
		LT,LE,GT,GE,					  	 /* < <= > >= */
		EQ,NE,							   /* == != */
		AAND,CARET,OOR,						 /* & ^ | */
		AND,OR,							  /* && / || */
		ASSIGN,							  /* = */
		ADD_ASSIGN,SUB_ASSIGN,			   /* +=,=+ -=,=- */
		MUL_ASSIGN,DIV_ASSIGN, MOD_ASSIGN,   /* *=,=* /=,=/ %=,=% */
		LSHIFT_ASSIGN,RSHIFT_ASSIGN,		 /* <<=,=<< >>=,=>> */
		AAND_ASSIGN,OOR_ASSIGN,CARET_ASSIGN, /* &=,=& |=,=| ^=,=^ */
		BITNOT_ASSIGN,						 /* ~=,=~ */
		LPAREN,RPAREN,LBRACE,RBRACE,	  	 /* ( ) { } */
		LBRACK,RBRACK,SEMI,COLON,COMMA,  	 /* [ ] ; : , */
		RIGHTPOS,							/* -> */
		EEOF,								/* 文件结束 */
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
struct astnode;
struct tpinfo{
	tpkind kind;
	string name;					   /*类型名 struct/enum/typedef用*/
	struct tpinfo *base;			   /*指针/数组的基础类型*/
	int arrlen;			  		   /*数组长度(0表示动态)*/
	vector<struct tpinfo*>members;	 /*struct/union成员*/
	vector<string>mebnames;
	vector<struct tpinfo*>argtypes;	   /* 函数参数 */
	struct tpinfo *rettype;			/* 函数返回类型 */
};
struct symbol{
	tpkind kind;
	string name;
	tpinfo *type;
	int storage;	/*0=auto, 1=static, 2=extrn, 3=register*/
	bool isconst;
	bool isexport;
	vector<symbol*>args;
	map<string,symbol*>children;
	vector<string>groups;
	int line=0,col=0;
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
			gloscope->name="_Phr_Global";
			curscope=gloscope;
		}
		~symtable(){
			while(curscope->par!=nullptr){
				scopi *old=curscope;
				curscope=curscope->par;
				old->syms.clear();
				delete old;
			}
			gloscope->syms.clear();
			delete gloscope;
		}
};
static symtable gsym;
void symtable::enterscp(string name){
	scopi *s=new scopi();
	s->par=curscope;
	s->name=name.empty()?"<anonymous>":name;
	curscope=s;
}
void symtable::exitscp(){
	if(curscope->par==nullptr)
		return;
	scopi *old=curscope;
	curscope=curscope->par;
	old->syms.clear();
	delete old;
}
void symtable::addsym(symbol *sym){
	if(curscope->syms.count(sym->name)){
		printf("Error: redefinition of '%s' at %d:%d\n",sym->name.c_str(),sym->line,sym->col);
        exit(1);
    }
    curscope->syms[sym->name]=sym;
}
symbol* symtable::lookup(string name){
	scopi *s=curscope;
	while(s!=nullptr){
		auto it=s->syms.find(name);
		if(it!=s->syms.end()){
			return it->second;
		}
		s=s->par;
	}
	return nullptr;
}
symbol* symtable::lookupcur(string name){
	auto it=curscope->syms.find(name);
	if(it!=curscope->syms.end()){
		return it->second;
	}
	return nullptr;
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
	keyw.insert("any");keyw.insert("char");keyw.insert("_Bool");
	keyw.insert("bool");keyw.insert("piece");keyw.insert("import");
	keyw.insert("export");keyw.insert("scope");keyw.insert("defer");
	keyw.insert("return");keyw.insert("func");keyw.insert("struct");
	keyw.insert("union");keyw.insert("enum");keyw.insert("delete");
	keyw.insert("new");keyw.insert("while");keyw.insert("if");
	keyw.insert("else");keyw.insert("switch");keyw.insert("until");
	keyw.insert("goto");keyw.insert("continue");keyw.insert("break");
	keyw.insert("for");keyw.insert("do");keyw.insert("default");
	keyw.insert("sizeof");keyw.insert("typeof");keyw.insert("extern");
	keyw.insert("surprise");keyw.insert("strict");
	/*保留关键字*/
	keyw.insert("short");keyw.insert("dim");keyw.insert("let");
	keyw.insert("long");keyw.insert("alignof");
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
inline bool isident(uint32_t ch){
	if(isalpha(ch)||ch=='_'){
		return 1;
	}if(ch>=0x4E00&&ch<=0x9FFF){
		return 1;
	}if(ch>=0x3400&&ch<=0x4DBF){
		return 1;
	}if(ch>=0x20000&&ch<=0x2A6DF){
		return 1;
	}if(ch>=0x2A700&&ch<=0x2B734){
		return 1;
	}if(ch>=0x2B740&&ch<=0x2B81F){
		return 1;
	}if(ch>=0x2B820&&ch<=0x2CEAF){
		return 1;
	}if(ch>=0x2CEB0&&ch<=0x2EBEF){
		return 1;
	}if(ch>=0x30000&&ch<=0x3134F){
		return 1;
	}if(ch>=0x31350&&ch<=0x323AF){
		return 1;
	}if(ch>=0x2EBF0&&ch<=0x2EE5F){
		return 1;
	}if(ch>=0xA000&&ch<=0xA48F){
		return 1;
	}if(ch>=0x181A&&ch<=0x18AF){
		return 1;
	}if(ch>=0x0400&&ch<=0x04FF){
		return 1;
	}if(ch>=0x0370&&ch<=0x03FF){
		return 1;
	}if(ch==0x0F0B||ch>=0x0F40&&ch<=0x0FFF){
		return 1;
	}if(ch>=0x3D000&&ch<=0x3FC3F){
		return 1;
	}return 0;
}
inline bool isnum(uint32_t ch){
	if(isdigit(ch)){
		return 1;
	}if(ch>=0x3021&&ch<=0x3029){
		return 1;
	}if(ch>=0x1810&&ch<=0x1819){
		return 1;
	}if(ch>=0x0F20&&ch<=0x0F29){
		return 1;
	}if(ch>=0x19D0&&ch<=0x19D9){
		return 1;
	}return 0;
}
inline bool isallnum(uint32_t ch){
	if(isdigit(ch)){
		return 1;
	}if(ch>=0x3021&&ch<=0x3029){
		return 1;
	}if(ch==0x3007){
		return 1;
	}if(ch>=0x1810&&ch<=0x1819){
		return 1;
	}if(ch>=0x0F20&&ch<=0x0F29){
		return 1;
	}if(ch>=0x19D0&&ch<=0x19D9){
		return 1;
	}if(ch==0x4E00||ch==0x4E8C||ch==0x4E09||ch==0x56DB||ch==0x4E94||
		ch==0x516D||ch==0x4E03||ch==0x516B||ch==0x4E5D
	   ){
	   	return 1;
	}return 0;
}
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
	if(isident(c)){
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
	if(isnum(c)){
		string num;
		num.push_back(c);
		while(1){
			char p=peek(s,pos);
			if(isallnum(p)||p=='.'){
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
			}else if(peek(s,pos)=='>'){
				nextch(s,pos);col++;
				tok.txt="->";
				tok.tok=tken::RIGHTPOS;
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
	AST_MODULE,		/* 整个文件 */
	AST_VAR_DECL,	  /* auto a:int=10; */
	AST_FUNC_DEF,	  /* func add(x:int,y:int)->int{ ... } */
	AST_PARAM,		 /* 函参 */
	AST_SCOPE,		 /* scope name{ } */
	AST_CONSTEXPR_DEF, /* constexpr word a=0; */
	AST_BLOCK,		 /* { ... } */
	AST_IF,			/* if(cond){ ... }else{ ... } */
	AST_WHILE,		 /* while(cond){ ... } */
	AST_UNTIL,		 /* until(cond){ ... } */
	AST_RETURN,		/* return expr; */
	AST_DEFER,		 /* defer{ ... } */
	AST_GOTO,		  /* goto label; */
	AST_LABEL,		 /* label: */
	AST_BREAK,		 /* break; */
	AST_CONTINUE,	  /* continue; */
	AST_COMPIF,		/* compif(expr){ } */
	AST_IDENT,		 /* 变量名 */
	AST_NUMBER,		/* 123 */
	AST_STRING,		/* "hello" */
	AST_CHAR,		  /* 'A' */
	AST_BINARY_OP,	 /* a+b */
	AST_UNARY_OP,	  /* -a, !a */
	AST_ADDR,		  /* $a */
	AST_DEREF,		 /* @p */
	AST_CALL,		  /* func(args) */
	AST_ASSIGN,		/* a=b, a=+ b, a+=b, ... */
	AST_TYPE,		  /* int, word, float ... */
	AST_EMPTY,		 /* 空节点 */
};
struct astnode{
	astkind kind;
	astnode *next;	   /* 同兄弟链表 */
	astnode *child;	  /* 子节点链表 */
	union{
		char* sval;	  /* 标识符/字符串/类型名 */
		long long ival;  /* 整数字面量 / 字符 */
		double fval;	 /* 浮点字面量 */
		int optok;	   /* 运算符tok id, BINOP/UNOP/ASSIGN使用 */
	}u;
	int line;
	int col;
};
symbol* mksymvar(astnode *var){
    symbol *sym=new symbol();
    sym->name=var->u.sval;
    //sym->kind=/* 从类型推断 */;
    sym->type=nullptr;
    sym->storage=0;     	/* 从 storage 字符串转 */
    sym->isconst=false;
    sym->isexport=false;
    sym->line=var->line;
    sym->col=var->col;
    return sym;
}
symbol* mksymfunc(astnode *func){
    symbol *sym=new symbol();
    sym->name=func->u.sval;
    //sym->kind=/* FUNC */;
    sym->type=nullptr;
    sym->storage=2;
    sym->isconst = false;
    sym->isexport = false;
    sym->line = func->line;
    sym->col = func->col;
    return sym;
}
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
astnode* astnassign(int op,astnode* lhs,astnode* rhs){
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
static int tokid;
static tken now;
void gettok(){
	if(tokid<tokens.size()){
		now=tokens[tokid++];
	}else{
		now.tok=tken::EEOF;
		now.txt="";
	}
}
tken peektok(){
	if(tokid<tokens.size()){
		return tokens[tokid];
	}
	tken eof;
	eof.tok=tken::EEOF;
	eof.row=now.row;
	eof.col=now.col;
	return eof;
}
bool match(int want){
	if(now.tok==want){
		gettok();
		return 1;
	}
	return 0;
}
void expect(int want){
	if(now.tok!=want){
		printf("Error: expected token %d at %d:%d, got %d '%s'\n",
			   want,now.row,now.col,now.tok,now.txt.c_str());
		exit(1);
	}
	gettok();
}
astnode* pardecl();
astnode* parfunc();
astnode* parargs();
astnode* parbody();
astnode* pardefer();
astnode* parvardef();
astnode* parstmt();
astnode* parexpr();
astnode* partype();
astnode* parblock();
astnode* parif();
astnode* pardo();
astnode* parwhile();
astnode* paruntil();
astnode* pargoto();
astnode* parfor();
astnode* parbreak();
astnode* parcontinue();
astnode* parmodule(){
	astnode *mod=astnew(AST_MODULE);
	astnode **tail=&mod->child;
	while(now.tok!=tken::EEOF){
		astnode *decl=pardecl();
		if(decl){
			*tail=decl;
			while(*tail)
				tail=&(*tail)->next;
		}
	}
	return mod;
}
astnode* parfunc(){
	expect(tken::KEYWORD);
	if(now.tok!=tken::IDENT){
		printf("Error: expected function name at %d:%d, got '%s'\n",
			   now.row,now.col,now.txt.c_str());
		exit(1);
	}
	string fname=now.txt;
	int line=now.row,col=now.col;
	gettok();
	astnode *func=astnew(AST_FUNC_DEF);
	func->u.sval=strdup(fname.c_str());
	func->line=line;
	func->col=col;
	{	symbol *fsym=new symbol();
		fsym->name=fname;
		fsym->kind=TPFUNC;
		fsym->storage=2;
		fsym->line=line;
		fsym->col=col;
		gsym.addsym(fsym);
	}
	gsym.enterscp(fname);
	func->child=parargs();
	astnode *rettype=nullptr;
	if(now.tok==tken::RIGHTPOS||now.tok==tken::ARROW){
		gettok();
		rettype=partype();
	}
	astnode *body=parbody();
	gsym.exitscp();
	astnode *p=func->child;
	if(p){
		while(p->next)
			p=p->next;
		p->next=rettype;
	}else{
		func->child=rettype;
	}
	if(rettype){
		rettype->next=body;
	}else{
		astnode *q=func->child;
		while(q->next)
			q=q->next;
		q->next=body;
	}
	return func;
}
astnode* parargs(){
	expect(tken::LPAREN);
	astnode *head=nullptr;
	astnode **tail=&head;
	while(now.tok!=tken::RPAREN&&now.tok!=tken::EEOF){
		if(now.tok!=tken::IDENT){
			printf("Error: expected parameter name at %d:%d, got '%s'\n",
					now.row,now.col,now.txt.c_str());
			exit(1);
		}
		astnode *param=astnew(AST_PARAM);
		param->u.sval=strdup(now.txt.c_str());
		param->line=now.row;
		param->col=now.col;
		{	symbol *psym=new symbol();
			psym->name=now.txt;
			psym->kind=TPWORD;
			psym->storage=0;
			psym->line=now.row;
			psym->col=now.col;
			gsym.addsym(psym);
		}
		gettok();
		if(now.tok==tken::COLON){
			gettok();
			param->child=partype();
		}
		*tail=param;
		tail=&param->next;
		if(now.tok==tken::COMMA)
			gettok();
	}
	expect(tken::RPAREN);
	return head;
}
astnode* parbody(){
	expect(tken::LBRACE);
	astnode *blk=astnew(AST_BLOCK);
	astnode **tail=&blk->child;
	while(now.tok!=tken::RBRACE&&now.tok!=tken::EEOF){
		if(now.tok==tken::KEYWORD&&now.txt=="defer"){
			gettok();
			astnode *d=astnew(AST_DEFER);
			expect(tken::LBRACE);
			d->child=pardefer();
			expect(tken::RBRACE);
			*tail=d;
			tail=&d->next;
			continue;
		}
		if(now.tok==tken::KEYWORD&&now.txt=="return"){
			gettok();
			astnode *r=astnew(AST_RETURN);
			if(now.tok!=tken::SEMI)
				r->child=parexpr();
			expect(tken::SEMI);
			*tail=r;
			tail=&r->next;
			continue;
		}
		if(now.tok == tken::KEYWORD &&
		   (now.txt == "auto" || now.txt == "static" ||
			now.txt == "extrn" || now.txt == "register")){
			astnode *v = parvardef();
			*tail = v;
			tail = &v->next;
			continue;
		}
		astnode *stmt=parstmt();
		if(stmt){
			*tail=stmt;
			while(*tail)
				tail=&(*tail)->next;
		}
	}
	expect(tken::RBRACE);
	return blk;
}
astnode* pardefer(){
	astnode *blk=astnew(AST_BLOCK);
	astnode **tail=&blk->child;
	while(now.tok!=tken::RBRACE&&now.tok!=tken::EEOF){
		if(now.tok==tken::KEYWORD&&now.txt=="return"){
			printf("Error: return not allowed inside defer at %d:%d\n",
					now.row,now.col);
			exit(1);
		}
		if(now.tok==tken::KEYWORD&&now.txt=="defer"){
			printf("Error: nested defer not allowed at %d:%d\n",
					now.row,now.col);
			exit(1);
		}
		if(now.tok==tken::KEYWORD&&
		   (now.txt=="auto"||now.txt=="static"||
			now.txt=="extrn"||now.txt=="register")){
			astnode *v=parvardef();
			*tail=v;
			tail=&v->next;
			continue;
		}
		astnode *stmt=parstmt();
		if(stmt){
			*tail=stmt;
			while(*tail)
				tail=&(*tail)->next;
		}
	}
	return blk;
}
astnode* parvardef(){
	string storage=now.txt;
	gettok();
	astnode *decl=astnew(AST_VAR_DECL);
	decl->u.sval=strdup(storage.c_str());
	astnode **tail=&decl->child;
	while(1){
		if(now.tok!=tken::IDENT){
			printf("Error: expected variable name at %d:%d, got '%s'\n",
					now.row,now.col,now.txt.c_str());
			exit(1);
		}
		astnode *var=astnident(now.txt);
		var->line=now.row;
		var->col=now.col;
		{	symbol *vsym=new symbol();
			vsym->name=now.txt;
			vsym->kind=TPWORD;
			vsym->storage=(storage=="auto")?0:(storage=="static")?1:
						  (storage=="extrn")?2:(storage=="register")?3:0;
		    vsym->line=now.row;
		    vsym->col=now.col;
		    gsym.addsym(vsym);
		}
		gettok();
		if(now.tok==tken::COLON){
			gettok();
			var->next=partype();
		}
		if(now.tok==tken::ASSIGN){
			gettok();
			astnode *init=parexpr();
			astnode *p=var;
			while(p->next)
				p=p->next;
			p->next=init;
		}
		*tail=var;
		while(*tail)
			tail=&(*tail)->next;
		if(now.tok==tken::COMMA){
			gettok();
			continue;
		}
		break;
	}
	expect(tken::SEMI);
	return decl;
}
astnode* parstmt(){
	switch(now.tok){
		case tken::RBRACE:
			return nullptr;
		case tken::LBRACE:
			return parblock();
		case tken::KEYWORD:{
			string kw=now.txt;
			if(kw=="if")
				return parif();
			if(kw=="while")
				return parwhile();
			if(kw=="until")
				return paruntil();
			if(kw=="goto")
				return pargoto();
			if(kw=="break")
				return parbreak();
			if(kw=="continue")
				return parcontinue();
			if(kw=="auto"||kw=="static"||kw=="extrn"||kw=="register")
				return parvardef();
			break;
		}
		case tken::SEMI:
			gettok();
			return astnew(AST_EMPTY);
	}
	astnode *expr=parexpr();
	expect(tken::SEMI);
	return expr;
}
astnode* parblock(){
	expect(tken::LBRACE);
	gsym.enterscp();
	astnode *blk=astnew(AST_BLOCK);
	astnode **tail=&blk->child;
	while(now.tok!=tken::RBRACE&&now.tok!=tken::EEOF){
		if(now.tok==tken::KEYWORD &&(now.txt=="auto"||now.txt=="static"||
			now.txt=="extrn"||now.txt=="register")){
			astnode *v=parvardef();
			*tail=v;
			tail=&v->next;
			continue;
		}
		astnode *stmt=parstmt();
		if(stmt){
			*tail=stmt;
			while(*tail)
				tail=&(*tail)->next;
		}
	}
	gsym.exitscp();  
	expect(tken::RBRACE);
	return blk;
}
astnode* parif(){
	gettok();
	expect(tken::LPAREN);
	astnode *node=astnew(AST_IF);
	node->child=parexpr();
	expect(tken::RPAREN);
	node->child->next=parstmt();
	if(now.tok==tken::KEYWORD&&now.txt=="else"){
		gettok();
		astnode *p=node->child;
		while(p->next)
			p=p->next;
		p->next=parstmt();
	}
	return node;
}
astnode* parwhile(){
	gettok();
	expect(tken::LPAREN);
	astnode *node=astnew(AST_WHILE);
	node->child=parexpr();
	expect(tken::RPAREN);
	node->child->next=parstmt();
	return node;
}
astnode* paruntil(){
	gettok();
	expect(tken::LPAREN);
	astnode *node=astnew(AST_UNTIL);
	node->child=parexpr();
	expect(tken::RPAREN);
	node->child->next=parstmt();
	return node;
}
astnode* pargoto(){
	gettok();
	astnode *node=astnew(AST_GOTO);
	if(now.tok==tken::IDENT){
		node->u.sval=strdup(now.txt.c_str());
		gettok();
	}
	expect(tken::SEMI);
	return node;
}
astnode* parbreak(){
	gettok();
	expect(tken::SEMI);
	return astnew(AST_BREAK);
}
astnode* parcontinue(){
	gettok();
	expect(tken::SEMI);
	return astnew(AST_CONTINUE);
}
int getprec(int tok){
	switch(tok){
		case tken::ASSIGN:
		case tken::ADD_ASSIGN:
		case tken::SUB_ASSIGN:
		case tken::MUL_ASSIGN:
		case tken::DIV_ASSIGN:
		case tken::MOD_ASSIGN: return 1;
		case tken::OR: return 2;
		case tken::AND: return 3;
		case tken::OOR: return 4;
		case tken::CARET: return 5;
		case tken::AAND: return 6;
		case tken::EQ:
		case tken::NE: return 7;
		case tken::LT:
		case tken::LE:
		case tken::GT:
		case tken::GE: return 8;
		case tken::LSHIFT:
		case tken::RSHIFT: return 9;
		case tken::PLUS:
		case tken::MINUS: return 10;
		case tken::STAR:
		case tken::SLASH:
		case tken::PERCENT: return 11;
		default: return 0;
	}
}

astnode* parprimary(){
	switch(now.tok){
		case tken::IDENT:{
			astnode *n = astnident(now.txt);
			n->line = now.row;
			n->col = now.col;
			gettok();
			if(now.tok == tken::LPAREN){
				gettok();
				astnode *call = astnew(AST_CALL);
				call->child = n;
				astnode **tail = &call->child;
				while(*tail) tail = &(*tail)->next;
				while(now.tok != tken::RPAREN && now.tok != tken::EEOF){
					astnode *arg = parexpr();
					*tail = arg;
					while(*tail) tail = &(*tail)->next;
					if(now.tok == tken::COMMA) gettok();
				}
				expect(tken::RPAREN);
				return call;
			}
			return n;
		}
		case tken::NUMBER:{
			astnode *n = astnnum(atoll(now.txt.c_str()));
			gettok();
			return n;
		}
		case tken::STRING:{
			astnode *n = astnstr(now.txt);
			gettok();
			return n;
		}
		case tken::CCHAR:{
			astnode *n = astnchar(now.txt[0]);
			gettok();
			return n;
		}
		case tken::LPAREN:{
			gettok();
			astnode *n = parexpr();
			expect(tken::RPAREN);
			return n;
		}
		case tken::ADDR:
		case tken::DEREF:
		case tken::PLUS:
		case tken::MINUS:
		case tken::NOT:
		case tken::BITNOT:
		case tken::INC:
		case tken::DEC:{
			int op = now.tok;
			gettok();
			return astnunop(op, parprimary());
		}
	}
	printf("Error: expected expression at %d:%d, got '%s'\n",
		   now.row, now.col, now.txt.c_str());
	exit(1);
}
astnode* parexprprec(int minprec){
	astnode *left=parprimary();
	while(1){
		int tok=now.tok;
		int prec=getprec(tok);
		if(prec<=minprec)
			break;
		if(tok==tken::SEMI)
			break;
		if(tok>=tken::ASSIGN&&tok<=tken::MOD_ASSIGN){
			gettok();
			astnode *right=parexprprec(prec-1);
			left=astnassign(tok,left,right);
			continue;
		}
		gettok();
		astnode *right=parexprprec(prec+1);
		left=astnbinop(tok,left,right);
	}
	return left;
}

astnode* parexpr(){
	return parexprprec(0);
}
astnode* partype(){
	astnode *t=nullptr;
	if(now.tok==tken::KEYWORD){
		string kw=now.txt;
		if(kw=="int"||kw=="float"||kw=="double"||kw=="char"||kw=="word"||kw=="bool"||kw=="void"||
			kw=="any"||kw=="wchar"||kw=="piece"||kw=="long"||kw=="short"){
			t=astntype(kw);
			gettok();
		}
	}
	if(!t&&now.tok==tken::DEREF){
		gettok();
		astnode *ptr=astnew(AST_TYPE);
		ptr->u.sval=strdup("pointer");
		ptr->child=partype();
		return ptr;
	}
	if(!t){
		printf("Error: expected type at %d:%d, got '%s'\n",now.row,now.col,now.txt.c_str());
		exit(1);
	}
	while(now.tok==tken::LBRACK){
		gettok();
		astnode *arr=astnew(AST_TYPE);
		arr->u.sval=strdup("array");
		arr->child=t;
		if(now.tok==tken::NUMBER){
			arr->next=astnnum(atoll(now.txt.c_str()));
			gettok();
		}else{
			arr->next=astnnum(0);
		}
		expect(tken::RBRACK);
		t=arr;
	}
	return t;
}
bool sur=1;
astnode* parsur(){
	int rd=rand()%10000;
	if(rd==9999){
		rd=rand()%2;
		if(sur&&rd){
			printf("\n天\a下\a爲\a公\n");
			sur=0;
		}else
			printf("\nToo bad, you were just about to hit the ultimate luck.\n");
	}else if(rd>=9950){
		printf("\nPhroog is the oppo\asite of Rust.\n");
	}else if(rd>=9500){
		printf("\nObjects are just sy\antactic sugar.\n");
	}else if(rd>=8000){
		printf("\nMemory safety should be ensured by the programmer, \anot relied on the compiler.\n");
	}else if(rd>=6000){
		printf("\nCongrats, you triggered a hidden easter egg, but you \adidn't get any information.\n");
	}else if(rd>=3000){
		printf("\nLook forward to Phroog27! It'll be providing more modern \agrammar...\n");
	}else{
		printf("\nYou don't have to worry about old programs becoming obsolete; \aPhroog always ensures backward compatibility.\n");
	}
	return (astnode*)NULL;
}
astnode* pardecl(){
	switch(now.tok){
		case tken::KEYWORD:{
			string kw=now.txt;
			if(kw=="func")
				return parfunc();
			if(kw=="surprise"){
				return parsur();
			}
//			if(kw=="import")
//				return parimport();
//			if(kw=="export")
//				return parexport();
//			if(kw=="scope")
//				return parscope();
//			if(kw=="struct")
//				return parstruct();
//			if(kw=="union")
//				return parunion();
//			if(kw=="enum")
//				return parenum();
			if(kw=="auto"||kw=="static"||kw=="extrn"||kw=="register")
				return parvardef();
			break;
		}
		case tken::IDENT:{
			break;
		}
	}
	printf("Error: unexpected token '%s' at %d:%d\n",now.txt.c_str(),now.row,now.col);
	exit(1);
}
const char* astkindname(astkind k){
	switch(k){
		case AST_MODULE:return "MODULE";
		case AST_VAR_DECL:return "VAR_DECL";
		case AST_FUNC_DEF:return "FUNC_DEF";
		case AST_PARAM:return "PARAM";
		case AST_SCOPE:return "SCOPE";
		case AST_CONSTEXPR_DEF:return "CONSTEXPR_DEF";
		case AST_BLOCK:return "BLOCK";
		case AST_IF:return "IF";
		case AST_WHILE:return "WHILE";
		case AST_UNTIL:return "UNTIL";
		case AST_RETURN:return "RETURN";
		case AST_DEFER:return "DEFER";
		case AST_GOTO:return "GOTO";
		case AST_LABEL:return "LABEL";
		case AST_BREAK:return "BREAK";
		case AST_CONTINUE:return "CONTINUE";
		case AST_COMPIF:return "COMPIF";
		case AST_IDENT:return "IDENT";
		case AST_NUMBER:return "NUMBER";
		case AST_STRING:return "STRING";
		case AST_CHAR:return "CHAR";
		case AST_BINARY_OP:return "BINARY_OP";
		case AST_UNARY_OP:return "UNARY_OP";
		case AST_ADDR:return "ADDR";
		case AST_DEREF:return "DEREF";
		case AST_CALL:return "CALL";
		case AST_ASSIGN:return "ASSIGN";
		case AST_TYPE:return "TYPE";
		case AST_EMPTY:return "EMPTY";
		default:return "UNKNOWN";
	}
}
const char* tokkindname(int t){
	static char buf[32];
	switch(t){
		case tken::PLUS:  return "+";
		case tken::MINUS: return "-";
		case tken::STAR:  return "*";
		case tken::SLASH: return "/";
		case tken::PERCENT:return "%";
		case tken::LT:	return "<";
		case tken::LE:	return "<=";
		case tken::GT:	return ">";
		case tken::GE:	return ">=";
		case tken::EQ:	return "==";
		case tken::NE:	return "!=";
		case tken::AAND:  return "&";
		case tken::OOR:   return "|";
		case tken::CARET: return "^";
		case tken::AND:   return "&&";
		case tken::OR:	return "||";
		case tken::LSHIFT:return "<<";
		case tken::RSHIFT:return ">>";
		case tken::ASSIGN:return "=";
		case tken::ADD_ASSIGN:return "+=";
		case tken::SUB_ASSIGN:return "-=";
		case tken::MUL_ASSIGN:return "*=";
		case tken::DIV_ASSIGN:return "/=";
		case tken::MOD_ASSIGN:return "%=";
		case tken::NOT:   return "!";
		case tken::BITNOT:return "~";
		case tken::INC:   return "++";
		case tken::DEC:   return "--";
		default:
			sprintf(buf, "tok%d", t);
			return buf;
	}
}
void astprint(astnode *n,int indent){
	for(astnode *cur=n;cur;cur=cur->next){
		for(int i=0;i<indent;i++)
			printf("  ");
		printf("%s",astkindname(cur->kind));
		switch(cur->kind){
			case AST_IDENT:
			case AST_TYPE:
				printf(" \"%s\"",cur->u.sval?cur->u.sval:"");
				break;
			case AST_STRING:
				printf(" \"%s\"",cur->u.sval?cur->u.sval:"");
				break;
			case AST_NUMBER:
				printf(" %lld",cur->u.ival);
				break;
			case AST_CHAR:
				printf(" '%c'",cur->u.ival);
				break;
			case AST_BINARY_OP:
			case AST_UNARY_OP:
			case AST_ASSIGN:
				printf(" %s", tokkindname(cur->u.optok));
				break;
			default:
				break;
		}
		printf("\n");
		if(cur->child)
			astprint(cur->child,indent+1);
	}
}
void version(){
	printf("Official Phroog Compiler version 1.0.0\n");
	printf("Copyright (c) CZH. All rights reserved.\n");
	printf("\n");
	printf("This compiler is provided as part of the Phroog Toolchain,\n");
	printf("  but only supports language versions up to Phroog 26.\n");
	printf("For compilers that support newer versions of the Phroog\n");
	printf("  programming language, see https://codeberg.org/chenzehou/Phroog\n\n");
}
void visit(astnode *n){
	if(!n)
		return;
	switch(n->kind){
		case AST_FUNC_DEF:{
			string fname=n->u.sval;
			gsym.enterscp(fname);
			astnode *p=n->child;
			while(p&&p->kind==AST_PARAM){
				p=p->next;
			}
			if(p&&p->kind==AST_TYPE){
				p=p->next;
			}
			if(p&&p->kind==AST_BLOCK){
				visit(p);
			}
			gsym.exitscp();
			return;
		}
		case AST_IDENT:{
			symbol *s=gsym.lookup(n->u.sval);
			if(!s){
				printf("Error: undefined symbol '%s' at %d:%d\n",
						n->u.sval,n->line,n->col);
				exit(1);
			}
			return;
		}
		case AST_VAR_DECL:{
			astnode *var=n->child;
			while(var){
				if(var->kind==AST_IDENT){
					symbol *vsym=new symbol();
					vsym->name=var->u.sval;
					vsym->kind=TPWORD;
					vsym->storage=0;
					vsym->line=var->line;
					vsym->col=var->col;
					gsym.addsym(vsym);
				}
				var=var->next;
			}
			return;
		}
		case AST_BLOCK:{
			gsym.enterscp();
			astnode *p=n->child;
			while(p){
				visit(p);
				p=p->next;
			}
			gsym.exitscp();
			return;
		}
		default:
			break;
	}
	visit(n->child);
	visit(n->next);
}
int main(int argc,char *argv[]){
	srand((unsigned)time(NULL));
	version();
	if(argc<2){
		printf("Usage: boogc file\n");
		return 1;
	}
	inittri();initkeyw();initpp();
	string src=string(argv[1])+".phr";
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
	#ifdef DEBUG
	printf("Predeal code: \n%s\n\n----------------------\n",code1.c_str());
	printf("GCC preprocess output:\n%s\n",code.c_str());
	#endif
	remove(tmpname.c_str());
	tokens.clear();
	tokid=0;
	{   int p=0,r=1,c=1;
		for(;;){
			tken t=nxttok(code,p,r,c);
			tokens.push_back(t);
			if(t.tok==tken::EEOF){
				break;
			}
		}
	}
	gettok();
	astnode *mod=parmodule();
	#ifdef DEBUG
	astprint(mod,0);
	#endif
	visit(mod);
	astfree(mod);
	return 0;
}