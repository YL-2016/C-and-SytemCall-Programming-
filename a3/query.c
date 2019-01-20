#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"

struct worker {
    pid_t pid;
    int fdin;
    int fdout;
    struct worker *next;
    int collected;
};
typedef struct worker Worker; 

int main(int argc, char **argv) {
    
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";

    while((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
            case 'd':
            startdir = optarg;
            break;
            default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }
    // Open the directory provided by the user (or current working directory)
    
    DIR *dirp;
    if((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    } 
    
    /* For each entry in the directory, eliminate . and .., and check
    * to make sure that the entry is a directory, then call run_worker
    * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
    * sequentially through the directories, and will expect to read
    * a word from standard input for each index it checks.
    */

    Worker *head = NULL;
    struct dirent *dp;
    while((dp = readdir(dirp)) != NULL) {

        if(strcmp(dp->d_name, ".") == 0 || 
           strcmp(dp->d_name, "..") == 0 ||
           strcmp(dp->d_name, ".svn") == 0){
            continue;
        }
        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path) - 1);
        strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

        struct stat sbuf;
        if(stat(path, &sbuf) == -1) {
            //This should only fail if we got the path wrong
            // or we don't have permissions on this entry.
            perror("stat");
            exit(1);
        } 

        // Only call run_worker if it is a directory
        // Otherwise ignore it.
        if(S_ISDIR(sbuf.st_mode)) {

            // Create bi direction pipe.
            int m2wfd[2], w2mfd[2];
            if (pipe(m2wfd) != 0 || pipe(w2mfd)) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            // Fork child process.
            pid_t pid = fork();
            if (pid < (pid_t)0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == (pid_t)0) {
                close(m2wfd[1]);
                close(w2mfd[0]);
                run_worker(path, m2wfd[0], w2mfd[1]);
                exit(EXIT_SUCCESS);
            }

            close(m2wfd[0]);
            close(w2mfd[1]);
            // Add to worker list
            Worker *worker = malloc(sizeof(Worker));
            worker->pid = pid;
            worker->fdin = m2wfd[1];
            worker->fdout = w2mfd[0];
            worker->next = head;
            head = worker;
        }
    }

    char *line = NULL;
    char word[MAXWORD];
    size_t size;
    int to_collect;
    int num_record;
    FreqRecord *result = malloc((MAXRECORDS + 1) * sizeof(FreqRecord));
    FreqRecord record;
    while (1) {
        // Get word from standard input
        if (getline(&line, &size, stdin) == -1 || strlen(line) == 1) {
            break;
        }
        size_t length = strlen(line);
        if (length > MAXWORD) {
            length = MAXWORD;
        }
        strcpy(word, line);
        word[length - 1] = 0;
        
        // Write word to workers
        printf("[Master] write word %s to workers.\n", word);
        to_collect = 0;
        Worker *worker = head;
        while (worker != NULL) {
            write(worker->fdin, word, MAXWORD);
            worker->collected = 0;
            to_collect++;
            worker = worker->next;
        }

        // Collect records from workers
        num_record = 0;
        while (to_collect > 0) {
            Worker *worker = head;
            while (worker != NULL) {
                if (worker->collected == 0) {
                    read(worker->fdout, (void *)&record, sizeof(FreqRecord));
                    if (record.freq == 0) {
                        worker->collected = 1;
                        to_collect--;
                    }
                    else {
                        if (num_record < MAXRECORDS) num_record++;
                        insert_freq_record(record, result, num_record);
                    }
                }
                worker = worker->next;
            }   
        }
        result[num_record].freq = 0;
        
        // Print sorted records to standard output
        printf("[Master] collected %d records from workers.\n", num_record);
        sort_freq_records(result, 0, num_record - 1);
        print_freq_records(result);
    }

    free(result);
    
    // Close all pipes to workers
    printf("[Master] close all pipes.\n");
    Worker *worker = head;
    while (worker != NULL) {
        close(worker->fdin);
        close(worker->fdout);
        worker = worker->next;
    }
    
    return 0;
}
