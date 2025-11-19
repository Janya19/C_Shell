#include "all.h"

int atomic_parser(){
    if(final_tokens[pos].type!=name){
        return 0;
    }
    else{
        pos++;
    }

    while (1) {
        if(final_tokens[pos].type == name) {
            pos++;
        } 
        else if(final_tokens[pos].type == redirect_in) {
            pos++; 
            if (final_tokens[pos].type != name) {
                return 0; 
            }
            pos++;
        } 
        else if(final_tokens[pos].type == redirect_out || final_tokens[pos].type == append) {
            pos++; 
            if (final_tokens[pos].type != name) {
                return 0; 
            }
            pos++;
        } 
        else {
            break; 
        }
    }
    return 1;
}

int command_group_parser(){
    if(atomic_parser()==0){
        return 0;
    }
    while(final_tokens[pos].type==pipes){
        pos++;
        if(atomic_parser()==0){
            return 0;
        }
    }
    return 1;
}

int shell_command_parser() {
    if (command_group_parser() == 0) {
        return 0;
    }

    while (final_tokens[pos].type == amp || final_tokens[pos].type == semi_colon) {
        /* If we see an amp and it's the last token (i.e., next token is EOF),
           treat it as the trailing optional '&' (do not try to parse another cmd_group). */
        if (final_tokens[pos].type == amp && final_tokens[pos + 1].type == eof) {
            break;
        }

        /* otherwise consume the operator and expect another cmd_group */
        pos++;
        if (command_group_parser() == 0) {
            return 0;
        }
    }

    /* If there's a trailing amp (the optional &?), consume it */
    if (final_tokens[pos].type == amp) {
        pos++;
    }
    return 1;
}
