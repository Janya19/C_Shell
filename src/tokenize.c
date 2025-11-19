#include "all.h"

// char** tokenizer(char* input){
//     char** tokens=malloc(sizeof(char*) * MAX_BUFFER); //array of strings which will hold our tokens
//     int tokenIndex=0;

//     char tempBuffer[MAX_BUFFER]; //its our temporary buffer to use while building the token
//     int bufferIndex=0;

//     for(int i=0;input[i]!='\0';i++){
//         char ch=input[i];
//         if(isspace(ch)!=0){ // char is a space
//             if(bufferIndex>0){ //we just finsihed reading a word or smthing
//                 //add word to token array
//                 tempBuffer[bufferIndex++]='\0';
//                 tokens[tokenIndex++]=strdup(tempBuffer);
//                 bufferIndex=0;
//             }
//             else{ //ignore the white space
//                 continue;
//             }
//         }
//         else if(ch=='|' || ch=='&' || ch=='<' || ch=='>' || ch==';'){
//             if(bufferIndex>0){
//                 tempBuffer[bufferIndex++]='\0';
//                 tokens[tokenIndex++]=strdup(tempBuffer);
//                 bufferIndex=0;
//             }
//             if(ch=='>' && input[i+1]=='>'){
//                 tempBuffer[0]=ch;
//                 tempBuffer[1]=input[i+1];
//                 tempBuffer[2]='\0';
//                 tokens[tokenIndex++]=strdup(tempBuffer);
//                 bufferIndex=0;
//                 i++;
//             }
//             else{
//                 tempBuffer[0]=ch;
//                 tempBuffer[1]='\0';
//                 tokens[tokenIndex++]=strdup(tempBuffer);
//                 bufferIndex=0;
//             }
//             continue;
//         }
//         else if(bufferIndex<MAX_BUFFER-1){
//             tempBuffer[bufferIndex++] = ch;      
//         }
//     }
//     if(bufferIndex>0){
//         tempBuffer[bufferIndex++]='\0';
//         tokens[tokenIndex++]=strdup(tempBuffer);
//         bufferIndex=0;
//     }
//     tokens[tokenIndex] = NULL;
//     return tokens;
// }



char** tokenizer(char* input){
    /* allocate pointer array using MAX_TOKENS and guard token count */
    char** tokens = malloc(sizeof(char*) * MAX_TOKENS);
    if (!tokens) return NULL;
    int tokenIndex = 0;

    char tempBuffer[MAX_BUFFER];
    int bufferIndex = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        char ch = input[i];
        if (isspace((unsigned char)ch)) {
            if (bufferIndex > 0) {
                tempBuffer[bufferIndex] = '\0';
                if (tokenIndex < MAX_TOKENS - 1) tokens[tokenIndex++] = strdup(tempBuffer);
                bufferIndex = 0;
            } else {
                continue;
            }
        }
        else if (ch == '|' || ch == '&' || ch == '<' || ch == '>' || ch == ';') {
            if (bufferIndex > 0) {
                tempBuffer[bufferIndex] = '\0';
                if (tokenIndex < MAX_TOKENS - 1) tokens[tokenIndex++] = strdup(tempBuffer);
                bufferIndex = 0;
            }
            if (ch == '>' && input[i+1] == '>') {
                tempBuffer[0] = ch;
                tempBuffer[1] = input[i+1];
                tempBuffer[2] = '\0';
                if (tokenIndex < MAX_TOKENS - 1) tokens[tokenIndex++] = strdup(tempBuffer);
                i++;
            } else {
                tempBuffer[0] = ch;
                tempBuffer[1] = '\0';
                if (tokenIndex < MAX_TOKENS - 1) tokens[tokenIndex++] = strdup(tempBuffer);
            }
            continue;
        }
        else if (bufferIndex < MAX_BUFFER - 1) {
            tempBuffer[bufferIndex++] = ch;
        }
    }

    if (bufferIndex > 0) {
        tempBuffer[bufferIndex] = '\0';
        if (tokenIndex < MAX_TOKENS - 1) tokens[tokenIndex++] = strdup(tempBuffer);
    }
    tokens[tokenIndex] = NULL;
    return tokens;
}