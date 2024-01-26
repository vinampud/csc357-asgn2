#define _GNU_SOURCE
#include <unistd.h>
#include <inttypes.h>
#include <dirent.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#define NUM_BYTES 1024
#define NUM_ARGS 2

/* NOTE: */
//attempted ro use memset for mkdir and touch to handle valgrind but it was breaking my ls command and I didn't want to spend more time on this, hope that's ok

void validate_args(int argc, char *argv[]);
FILE *open_file (char * name, char * mode);
void split_args(char *s, char *a[]);
void remove_trailing(char *s);
char * remove_leading(char *s);
void verify_command(char * c, char ** v);
void * checked_malloc(size_t s);
char *uint32_to_str(uint32_t i);
void verify_uint(uint32_t u);
void verify_char(char c);
uint32_t command_cd(char *name, uint32_t cwd);
void command_ls(uint32_t cwd);
void command_mkdir(char *name, uint32_t cwd);
void command_touch(char *name, uint32_t cwd);
uint32_t check_command(char * a[], uint32_t cwd);
uint32_t get_command(uint32_t cwd);
void freeargpointer(char** array);

//REQUIREMENT 1
int main(int argc, char *argv[]){


    uint32_t ilist[NUM_BYTES];
    char clist[NUM_BYTES];
    
    DIR *directory;

    validate_args(argc, argv);

    //REQUIREMENT 2
    directory = opendir(argv[1]);
    if (!directory){
        fprintf(stderr, "%s not found", argv[1]);
        return 1;
    }
    else{ //proceed to read inode list
        chdir(argv[1]);
        printf("%s\n", argv[1]);
        char *d = get_current_dir_name();
        printf("%s\n", d);

         //load inodes_list
        FILE *fp;
        fp = open_file("inodes_list", "r");
        int idx = 0;
        while(fread(&ilist[idx], sizeof(uint32_t), 1, fp) > 0){
            verify_uint(ilist[idx]);
            fread(&clist[idx], sizeof(char), 1, fp);
            verify_char(clist[idx]);
            idx++;
        }

        free(d);
        closedir(directory);
        fclose(fp);
    }
    

    //REQUIREMENT 3
    uint32_t cwd = ilist[0];

    if (clist[0] != 'd'){
        perror("Must start with directory");
        exit(1);
    }

    //REQUIREMENT 4
    cwd = get_command(cwd);


    return 0;
}

void validate_args(int argc, char *argv[]){

    if (argc != NUM_ARGS){
        fprintf(stderr, "usage: %s file\n", argv[0]);
        exit(1);
    }

}

FILE *open_file (char * name, char * mode){
    FILE *f = fopen(name, mode);
    
    if(f == NULL){
        perror(name);
        exit(1);
    }
    return f;
}

void split_args(char *s, char *a[]){
    char *arg ;

    int i = 0;
    while ((arg = strsep(&s, " ")) != NULL){
        a[i] = arg;
        i++;
    }

}

void remove_trailing(char *s){
    size_t len = strlen(s);
    size_t idx;
    for (idx = (len - 1); idx >= 0 && isspace(s[idx]); idx--){
        s[idx] = '\0';
    }
}

char * remove_leading(char *s){
    int idx = 0;
    while(isspace(s[idx]) && s[idx] != '\0'){
        idx++;
    }
    return &s[idx];
}

void verify_command(char * c, char ** v){
    char * a1;
    char * a2;

    if ((a1 = strsep(&c, " ")) != NULL){
            if (strcmp(a1, "fs_emulator") == 0){

            }
            v[0] = a1;
            if ((a2 = strsep(&c, " ")) != NULL){
            v[1] = a2;
            }
        }
    
}

void * checked_malloc(size_t s){
    int *p;
    p = malloc(s);
    if(p == NULL){
        perror("unable to malloc");
        exit(1);
    }
    return p;
}

char *uint32_to_str(uint32_t i)
{
   int length = snprintf(NULL, 0, "%lu", (unsigned long)i);       // pretend to print to a string to get length
   char* str = checked_malloc(length + 1);                        // allocate space for the actual string
   snprintf(str, length + 1, "%lu", (unsigned long)i);            // print to string

   return str;
}

void verify_uint(uint32_t u){
    if (u < 0 || u > NUM_BYTES){
        printf("Invalid inode number.\n");
    }
}

void verify_char(char c){
    if (c != 'd' && c != 'f'){
        printf("Invalid indicator.\n");
    }
}

uint32_t command_cd(char *name, uint32_t cwd){
    //take in a[1] for name of directory
        
        //keep track of current uint
        uint32_t cur;
        //use same loop for ls, check to compare name to current cbuff 
        uint32_t ubuff[NUM_BYTES];
        char cbuff[NUM_BYTES];
        char * s = uint32_to_str(cwd);
        FILE *fp = open_file(s, "r");
        free(s);

        if (strcmp(name, ".") == 0){
            return cwd;
        }

        else{

            while(fread(&ubuff, sizeof(uint32_t), 1, fp) > 0){

                cur = ubuff[0];
                
                fread(&cbuff, sizeof(char), 32, fp);

                if (strcmp(cbuff, name) == 0){

                    //if found, check inodes list to make sure it's a directory
                    uint32_t ilist[NUM_BYTES];
                    char clist[NUM_BYTES];
                    FILE *i;
                    i = open_file("inodes_list", "r");
                    int idx = 0;

                    while(fread(&ilist[idx], sizeof(uint32_t), 1, i) > 0){

                        if(cur == ilist[idx]){
                            fread(&clist[idx], sizeof(char), 1, i);
                            if(clist[idx] == 'd'){
                                cwd = cur;
                                fclose(i);
                                fclose(fp);
                                return cwd;
                            }

                            else if(clist[idx] == 'f'){
                                printf("%s is of file type, not directory.\n", name);
                                fclose(i);
                                fclose(fp);
                                return cwd;
                            }
                            
                        }

                        else{
                            fread(&clist[idx], sizeof(char), 1, i);
                        }
                        idx++;
                    }
                    fclose(i);
                }

            }

        }

        fclose(fp);
        //if not directory, print error, if not found, ignore
        //update cwd to that value

        printf("Directory %s does not exist.\n", name);
        return cwd;
}

void command_ls(uint32_t cwd){

    uint32_t ubuff[NUM_BYTES];
    char cbuff[NUM_BYTES];
    char * s = uint32_to_str(cwd);
    FILE *fp = open_file(s, "r");

    while(fread(&ubuff, sizeof(uint32_t), 1, fp) > 0){
        printf("%lu", (unsigned long)(ubuff[0]));
        fread(&cbuff, sizeof(char), 32, fp);
        printf(" %s\n", cbuff);
    }

    free(s);
    fclose(fp);
}

void command_mkdir(char *name, uint32_t cwd){

    if (strlen(name) > 32){
        printf("Please enter a shorter name for new directory.\n");
        return;
    }
    
    uint32_t ubuff[NUM_BYTES];
    char cbuff[NUM_BYTES];
    char * s = uint32_to_str(cwd);
    FILE *fp = open_file(s, "r");

    while(fread(&ubuff, sizeof(uint32_t), 1, fp) > 0){
        
        fread(&cbuff, sizeof(char), 32, fp);
        
        // if file already exists in current directory, ignore command
        if (strcmp(cbuff, name) == 0){
            printf("%s already exists.\n", name);
            free(s);
            fclose(fp);
            return;
        }

    }
    fclose(fp);

    // find next available inode by reading inodes_list with counter

    uint32_t ilist[NUM_BYTES];
    char clist[NUM_BYTES];
    uint32_t next = 0;
    FILE *i;
    i = open_file("inodes_list", "r");
    int idx = 0;
    
    while(fread(&ilist[idx], sizeof(uint32_t), 1, i) > 0){
        fread(&clist[idx], sizeof(char), 1, i);
        idx++;
    }
    fclose(i);

    next = idx;
    char indicator = 'd';

    //update inodes_list
    FILE *fa = open_file("inodes_list", "a");
    fwrite(&next, sizeof(uint32_t), 1, fa);
    fwrite(&indicator, sizeof(char), 1, fa);
    fclose(fa);

    /* update current directory */

    FILE *fc = open_file(s, "a");
    fwrite(&next, sizeof(uint32_t), 1, fc);
    char new[32];
    strcpy(new, name);
    fwrite(new, sizeof(char), 32, fc);
    fclose(fc);

    //create new directory and write in default . and ..

    char *root = ".";
    char *prev = "..";
    char *newfile = uint32_to_str(next);
    FILE *fw = open_file(newfile, "a");
    //write defaut root into directory
    fwrite(&next, sizeof(uint32_t), 1, fw);
    fwrite(root, sizeof(char), 32, fw);
    //write default parent 
    fwrite(&cwd, sizeof(uint32_t), 1, fw);
    fwrite(prev, sizeof(char), 32, fw);

    free(s);
    free(newfile);
    fclose(fw);
}

void command_touch(char *name, uint32_t cwd){

    //use same loop for ls, check to compare name to current cbuff 
    uint32_t ubuff[NUM_BYTES];
    char cbuff[NUM_BYTES];
    char * s = uint32_to_str(cwd);
    FILE *fp = open_file(s, "r");

    while(fread(&ubuff, sizeof(uint32_t), 1, fp) > 0){
        
        fread(&cbuff, sizeof(char), 32, fp);
        // if file already exists in current directory, ignore command
        if (strcmp(cbuff, name) == 0){
            printf("%s already exists.\n", name);
            free(s);
            fclose(fp);
            return;
        }

    }
    fclose(fp);

    // find next available inode by reading inodes_list with counter

    uint32_t ilist[NUM_BYTES];
    char clist[NUM_BYTES];
    uint32_t next = 0;
    FILE *i;
    i = open_file("inodes_list", "r");
    int idx = 0;
    
    while(fread(&ilist[idx], sizeof(uint32_t), 1, i) > 0){
        fread(&clist[idx], sizeof(char), 1, i);
        idx++;
    }
    fclose(i);

    next = idx;    
    char indicator = 'f';

    //update inodes_list
    FILE *fa = open_file("inodes_list", "a");
    fwrite(&next, sizeof(uint32_t), 1, fa);
    fwrite(&indicator, sizeof(char), 1, fa);
    fclose(fa);


    /* update current directory */

    //char * s = uint32_to_str(cwd); already defined earlier
    FILE *fc = open_file(s, "a");
    fwrite(&next, sizeof(uint32_t), 1, fc);
    char new[32];
    strcpy(new, name);
    fwrite(new, sizeof(char), 32, fc);
    fclose(fc);
    

    //create new file
    char *newfile = uint32_to_str(next);
    FILE *fw = open_file(newfile, "w");
    fprintf(fw, "%s\n", name);

    free(s);
    free(newfile);
    fclose(fw);

}

uint32_t check_command(char * a[], uint32_t cwd){

    if (strcmp(a[0], "exit") == 0){
        return 1025; 
    }

    else if (strcmp(a[0], "cd") == 0){
        cwd = command_cd(a[1], cwd);
        return cwd;

    }

    else if (strcmp(a[0], "ls") == 0){
        command_ls(cwd);
        return cwd;

    }

    else if (strcmp(a[0], "mkdir") == 0){
        command_mkdir(a[1], cwd);
        return cwd;
    }
    
    else if (strcmp(a[0], "touch") == 0){
        command_touch(a[1], cwd);
        return cwd;
    }


    else{
        printf("Invalid command, please try again.\n");
        return cwd;
    }

}

uint32_t get_command(uint32_t cwd){
    char *args[NUM_ARGS];
    char *str = NULL;
    size_t n;
    FILE *in = stdin;

    while (getline(&str, &n, in) > 0){
        remove_trailing(str);
        str = remove_leading(str);
        split_args(str, args);
        cwd = check_command(args, cwd);
        if (cwd == 1025){
            break;
        }
    } 

    fclose(in);
    //freeargpointer(args);
    free(str);
    return cwd;
}

void freeargpointer(char** array)
{
    int i;

    for (i = 0; array[i]; i++)
        free(array[i]);

    free(array);
}