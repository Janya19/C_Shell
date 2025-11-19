#include "all.h"

char LOG_FILE_PATH[PATH_MAX+1] = "";

void add_to_log(const char* cmd) {

    if (strncmp(cmd, "log", 3) == 0 && (cmd[3] == ' ' || cmd[3] == '\0' || cmd[3] == '\n' || cmd[3] == '\t')) {
        return;
    }

    FILE* f = fopen(LOG_FILE, "r");
    char* lines[MAX_LOG+5];
    int count = 0;
    char buffer[1024];

    if(f){
        while(fgets(buffer, sizeof(buffer), f)) {
            if (count >= MAX_LOG) break;

            buffer[strcspn(buffer, "\n")] = 0;
            lines[count++] = strdup(buffer);
        }   
        fclose(f);
    }

    char cmd_copy[1024];
    strncpy(cmd_copy, cmd, sizeof(cmd_copy));
    cmd_copy[sizeof(cmd_copy)-1] = '\0';
    size_t len = strlen(cmd_copy);
    if(len > 0 && cmd_copy[len-1] == '\n') cmd_copy[len-1] = '\0';

    if (count > 0 && strcmp(cmd_copy, lines[count-1]) == 0) {
        for (int i = 0; i < count; i++) free(lines[i]);
        return;
    }

    // int j = 0;
    // if (count == MAX_LOG) {
    //     free(lines[0]); 
    //     j = 1;
    //     count--;
    // }

    // lines[count++] = strdup(cmd_copy);

    if (count == MAX_LOG) {
        free(lines[0]);
        for (int i = 1; i < MAX_LOG; i++) {
            lines[i-1] = lines[i];
        }
        count = MAX_LOG - 1;
    }

    lines[count++] = strdup(cmd_copy);


    f = fopen(LOG_FILE, "w");
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", lines[i]);
        free(lines[i]);
    }
    fclose(f);
}


void print_log() {
    FILE* f = fopen(LOG_FILE, "r");
    if (!f) return;

    char buffer[1024];
    while(fgets(buffer, sizeof(buffer), f)) {
        buffer[strcspn(buffer, "\n")] = 0;
        printf("%s\n", buffer);
    }
    fclose(f);
}

void purge_log() {
    FILE* f = fopen(LOG_FILE, "w");
    if (f) fclose(f); 
}

void execute_log_index(int index) {
    FILE* f = fopen(LOG_FILE, "r");
    if (!f) return;

    char* lines[MAX_LOG];
    int count = 0;
    char buffer[1024];

    while(fgets(buffer, sizeof(buffer), f)) {
        buffer[strcspn(buffer, "\n")] = 0;
        lines[count++] = strdup(buffer);
    }
    fclose(f);

    if (index < 1 || index > count) {
        printf("Invalid index\n");
        for (int i = 0; i < count; i++) free(lines[i]);
        return;
    }

    int real_idx = count - index; 
    char* cmd_to_execute = lines[real_idx];

    int n = 0;
    char** tokens = tokenizer(cmd_to_execute);
    for (int i = 0; tokens[i] != NULL; i++) {
        final_tokens[n++] = classifier(tokens[i]);
    }
    final_tokens[n].type = eof;

    pos = 0;
    execute_tokens();  // use your existing execution logic

    for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
    free(tokens);

    for (int i = 0; i < count; i++) free(lines[i]);
}

