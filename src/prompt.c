#include "all.h"

//used to store the home directory and previous directory of our shell
char home_dir[PATH_MAX+1]=""; 
char prev_dir[PATH_MAX+1]="";
char curr_dir[PATH_MAX+1];

void shell_prompt(){ 
    struct passwd *pw = getpwuid(getuid()); //returns a pointer to a struct passwd that contains detailed information about that user
    
    char host_name[HOST_NAME_MAX+1];
    gethostname(host_name, sizeof(host_name)); 
    
    getcwd(curr_dir, sizeof(curr_dir));
    
    printf("<%s@%s:", pw->pw_name, host_name);
    
    // strcpy(curr_dir,"randomDir/anotherRandomDir"); //for checking if ~ and normal both are printed properly
    //figuring out if we need to put ~ or need to print whole name
    if(strncmp(curr_dir, home_dir, strlen(home_dir)) == 0) { // inside the shell's "home"
        printf("~%s> ", curr_dir + strlen(home_dir));
    } 
    else{ // outside
        printf("%s> ", curr_dir);
    }
    fflush(stdout);
}
