#include "all.h"

Token classifier(char* tok){
    Token t;
    t.value=tok;

    if(strcmp(tok,"|")==0){
        t.type=pipes;
    }
    else if(strcmp(tok,"&")==0){
        t.type=amp;
    }
    else if(strcmp(tok,";")==0){
        t.type=semi_colon;
    }    
    else if(strcmp(tok,">>")==0){
        t.type=append;
    } 
    else if(strcmp(tok,"<")==0){
        t.type=redirect_in;
    }    
    else if(strcmp(tok,">")==0){
        t.type=redirect_out;
    }    
    else{
        t.type=name;
    } 
    return t;   
}