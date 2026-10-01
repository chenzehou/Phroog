using namespace std;

map<string,char>tri;
set<string>pp;

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
	if(!fs){
		return "";
	}
	string buf((istreambuf_iterator<char>(fs)), istreambuf_iterator<char>());
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
		out.push_back(s[i]);
		i++;
	}
	return out;
}

string dealpp(string s){
	string out;
	int n=s.size();
	int i=0;
	map<string,string>macros;
	map<string,vector<string> >macros_args;
	while(i<n){
		int linest=i;
		while(i<n&&s[i]!='\n'){
			i++;
		}
		int lined=i;
		if(i<n&&s[i]=='\n'){
			i++;
		}
		string line=s.substr(linest,lined-linest);
		int p=0;
		while(p<(int)line.size()&&(line[p]==' '||line[p]=='\t')){
			p++;
		}
		if(p<(int)line.size()&&line[p]=='#'){
			int start=p+1;
			while(start<(int)line.size()&&(line[start]==' '||line[start]=='\t')){
				start++;
			}
			int end=start;
			while(end<(int)line.size()&&(isalpha(line[end])||line[end]=='_')){
				end++;
			}
			string dir=line.substr(start,end-start);
			if(dir=="include"){
				int q=end;
				while(q<(int)line.size()&&(line[q]==' '||line[q]=='\t')){
					q++;
				}
				if(q<(int)line.size()&&line[q]=='"'){
					int q2=q+1;
					while(q2<(int)line.size()&&line[q2]!='"'){
						q2++;
					}
					if(q2<(int)line.size()){
						string fname=line.substr(q+1,q2-q-1);
						string inc=readall(fname.c_str());
						out+=inc;
					}
				}
				continue;
			}
			if(dir=="define"){
				int q=end;
				while(q<(int)line.size()&&(line[q]==' '||line[q]=='\t')){
					q++;
				}
				int mstart=q;
				while(q<(int)line.size()&&(isalnum(line[q])||line[q]=='_')){
					q++;
				}
				string mname=line.substr(mstart,q-mstart);
				vector<string>args;
				if(q<(int)line.size()&&line[q]=='('){
					q++;
					while(q<(int)line.size()&&line[q]!=')'){
						while(q<(int)line.size()&&(line[q]==' '||line[q]=='\t')){
							q++;
						}
						int astart=q;
						while(q<(int)line.size()&&(isalnum(line[q])||line[q]=='_')){
							q++;
						}
						if(q>astart){
							args.push_back(line.substr(astart,q-astart));
						}
						while(q<(int)line.size()&&(line[q]==' '||line[q]=='\t')){
							q++;
						}
						if(q<(int)line.size()&&line[q]==','){
							q++;
						}
					}
					if(q<(int)line.size()&&line[q]==')'){
						q++;
					}
				}
				while(q<(int)line.size()&&(line[q]==' '||line[q]=='\t')){
					q++;
				}
				string mval="";
				while(q<(int)line.size()){
					mval.push_back(line[q]);
					q++;
				}
				if(!args.empty()){
					macros_args[mname]=args;
				}
				macros[mname]=mval;
				continue;
			}
			if(dir=="undef"){
				int q=end;
				while(q<(int)line.size()&&(line[q]==' '||line[q]=='\t')){
					q++;
				}
				int mstart=q;
				while(q<(int)line.size()&&(isalnum(line[q])||line[q]=='_')){
					q++;
				}
				string mname=line.substr(mstart,q-mstart);
				macros.erase(mname);
				macros_args.erase(mname);
				continue;
			}
			if(pp.count(dir)){
				continue;
			}
		}
		int pos=0;
		int inq=0;
		while(pos<(int)line.size()){
			if(line[pos]=='"'){
				inq=!inq;
				out.push_back(line[pos]);
				pos++;
				continue;
			}
			if(inq){
				out.push_back(line[pos]);
				pos++;
				continue;
			}
			if(isalpha(line[pos])||line[pos]=='_'){
				int start=pos;
				while(pos<(int)line.size()&&(isalnum(line[pos])||line[pos]=='_')){
					pos++;
				}
				string ident=line.substr(start,pos-start);
				if(macros.count(ident)&&pos<(int)line.size()&&line[pos]=='('){
					vector<string>args;
					pos++;
					int depth=1;
					string cur="";
					while(pos<(int)line.size()&&depth>0){
						if(line[pos]=='('){
							depth++;
							cur.push_back(line[pos]);
							pos++;
						}else if(line[pos]==')'){
							depth--;
							if(depth==0){
								if(!cur.empty()){
									args.push_back(cur);
								}
								pos++;
								break;
							}else{
								cur.push_back(line[pos]);
								pos++;
							}
						}else if(line[pos]==','&&depth==1){
							args.push_back(cur);
							cur="";
							pos++;
						}else{
							cur.push_back(line[pos]);
							pos++;
						}
					}
					string mval=macros[ident];
					if(macros_args.count(ident)){
						vector<string>params=macros_args[ident];
						if(args.size()>params.size()){
							args.resize(params.size());
						}
						for(int k=0;k<(int)args.size();k++){
							string tmp=args[k];
							int pp=0;
							while((pp=mval.find(params[k],pp))!=(int)string::npos){
								mval.replace(pp,params[k].size(),tmp);
								pp+=tmp.size();
							}
						}
					}
					string tmp;
					int mpos=0;
					while(mpos<(int)mval.size()){
						if(isalpha(mval[mpos])||mval[mpos]=='_'){
							int mstart=mpos;
							while(mpos<(int)mval.size()&&(isalnum(mval[mpos])||mval[mpos]=='_')){
								mpos++;
							}
							string mident=mval.substr(mstart,mpos-mstart);
							if(macros.count(mident)){
								tmp+=macros[mident];
							}else{
								tmp+=mident;
							}
						}else{
							tmp.push_back(mval[mpos]);
							mpos++;
						}
					}
					mval=tmp;
					out+=mval;
					continue;
				}
				if(macros.count(ident)){
					out+=macros[ident];
				}else{
					out+=ident;
				}
			}else{
				out.push_back(line[pos]);
				pos++;
			}
		}
		if(i<n&&s[i-1]=='\n'){
			out.push_back('\n');
		}
	}
	return out;
}

string boogpp(const char *path){
	string s=readall(path);
	s=crlf2lf(s);
	s=t2s(s);
	s=dealtri(s);
	s=librow(s);
	s=delrem(s);
	s=dealpp(s);
	return s;
}