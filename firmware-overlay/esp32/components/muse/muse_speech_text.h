/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* Speech-only derivation. Never modify the caption/chat source string. */
#pragma once
#include <string>
#include <cstdint>
#include <cctype>

namespace muse_speech {
inline uint32_t cp(const std::string &s, size_t &i) {
    unsigned char c=s[i++]; if(c<128)return c;
    unsigned count=c<0xe0?1:c<0xf0?2:3; uint32_t v=c&((1u<<(6-count))-1);
    while(count-- && i<s.size()) {unsigned char t=s[i];if((t&192)!=128)return 0xfffd;++i;v=(v<<6)|(t&63);}
    return v;
}
inline bool emoji(uint32_t c) {
    return (c>=0x1f000&&c<=0x1faff)||(c>=0x2600&&c<=0x27bf)||
        (c>=0x2300&&c<=0x23ff)||(c>=0x25a0&&c<=0x25ff)||c==0x24c2||(c>=0x2b00&&c<=0x2bff)||
        c==0xa9||c==0xae||c==0x203c||c==0x2049||c==0x2122||c==0x2139||
        (c>=0x2194&&c<=0x21ff)||c==0x3030||c==0x303d||c==0x3297||c==0x3299||
        c==0x200d||c==0x20e3||(c>=0xfe00&&c<=0xfe0f)||
        (c>=0xe0000&&c<=0xe01ef);
}
inline bool punct(uint32_t c) {
    return c==','||c=='.'||c==';'||c==':'||c=='!'||c=='?'||c=='-'||
        c==0x3001||c==0x3002||c==0xff0c||c==0xff1b||c==0xff1a||c==0xff01||c==0xff1f;
}
inline bool han(uint32_t c) {return (c>=0x3400&&c<=0x9fff)||(c>=0x20000&&c<=0x3134f);}
inline std::string clean(const char *input, unsigned depth=0) {
    if(depth>8) return {};
    std::string s=input?input:"", plain;
    bool line_start=true;
    for(size_t i=0;i<s.size();) {
        if(s.compare(i,3,"\xee\x88\x80")==0) { // private-use citation delimiters
            size_t end=s.find("\xee\x88\x81",i+3);i=end==std::string::npos?s.size():end+3;continue;
        }
        if(s.compare(i,10,"sandbox://")==0) {
            while(i<s.size()&&!std::isspace(static_cast<unsigned char>(s[i]))&&s[i]!=')')++i;
            if(i<s.size()&&s[i]==')') ++i;
            continue;
        }
        if((s[i]>='0'&&s[i]<='9')||s[i]=='#'||s[i]=='*') {
            size_t next=i+1;uint32_t n=next<s.size()?cp(s,next):0;
            if(n==0xfe0f)n=next<s.size()?cp(s,next):0;
            if(n==0x20e3){i=next;plain+=' ';continue;}
        }
        if(s[i]=='[') {
            size_t close=s.find(']',i+1);
            if(close!=std::string::npos) {
                bool numeric=close>i+1;
                for(size_t n=i+1;n<close;++n)
                    if(!std::isdigit(static_cast<unsigned char>(s[n]))&&s[n]!='^')numeric=false;
                if(numeric){i=close+1;continue;}
                if(close+1<s.size()&&s[close+1]=='[') {
                    size_t end=s.find(']',close+2);
                    if(end!=std::string::npos){plain+=clean(s.substr(i+1,close-i-1).c_str(),depth+1);i=end+1;continue;}
                }
                if(line_start&&close+1<s.size()&&s[close+1]==':') {
                    size_t end=s.find('\n',close+2);i=end==std::string::npos?s.size():end;continue;
                }
            }
        }
        bool image=s[i]=='!'&&i+1<s.size()&&s[i+1]=='[';
        size_t begin=i+(image?1:0);
        if(s[begin]=='[') {
            size_t close=s.find(']',begin+1);
            if(close!=std::string::npos&&close+1<s.size()&&s[close+1]=='(') {
                size_t end=close+2;unsigned nesting=1;
                while(end<s.size()&&nesting) {if(s[end]=='(')++nesting;if(s[end]==')')--nesting;++end;}
                if(!nesting) {
                    bool attachment=s.compare(close+2,10,"sandbox://")==0;
                    if(!image&&!attachment)plain+=clean(s.substr(begin+1,close-begin-1).c_str(),depth+1);
                    i=end;continue;
                }
            }
        }
        if(line_start) {
            size_t n=i;while(n<s.size()&&std::isdigit(static_cast<unsigned char>(s[n])))++n;
            if(n>i&&n+1<s.size()&&(s[n]=='.'||s[n]==')')&&s[n+1]==' ') {i=n+2;continue;}
            if((s[i]=='-'||s[i]=='+'||s[i]=='>')&&i+1<s.size()&&s[i+1]==' ') {i+=2;continue;}
        }
        char c=s[i++];
        if(c=='#'&&!line_start) {while(!plain.empty()&&plain.back()==' ')plain.pop_back();continue;}
        if(c=='*'||c=='_'||c=='`'||c=='#'||c=='~')continue;
        plain+=c;
        if(c=='\n'||c=='\r')line_start=true;
        else if(c!=' '&&c!='\t')line_start=false;
    }
    std::string out;bool gap=false, meaningful=false;uint32_t last=0;
    for(size_t i=0;i<plain.size();) {
        size_t start=i;uint32_t c=cp(plain,i);
        // Digit/#/* plus optional VS16 and keycap is one emoji sequence.
        if((c>='0'&&c<='9')||c=='#'||c=='*') {
            size_t next=i;uint32_t n=next<plain.size()?cp(plain,next):0;
            if(n==0xfe0f)n=next<plain.size()?cp(plain,next):0;
            if(n==0x20e3){i=next;gap=true;continue;}
        }
        if(emoji(c)){gap=true;continue;}
        if(c<=32||c==0xa0||c==0x3000){gap=true;continue;}
        if(gap&&!out.empty()&&!punct(last)&&!punct(c))out+=han(last)&&han(c)?"，":" ";
        gap=false;
        if(punct(c)&&(!meaningful||punct(last)))continue;
        out.append(plain,start,i-start);last=c;
        if(!punct(c))meaningful=true;
    }
    return meaningful?out:std::string();
}
}
