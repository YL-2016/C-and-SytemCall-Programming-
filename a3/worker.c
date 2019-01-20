#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"

/* The function get_word should be added to this file */
void get_word(char *word , Node *head, char **filenames, FreqRecord *result) {
    int i = 0;
    while (head != NULL && result != NULL) {
        if (strcmp(head->word, word) == 0) {
            for (int j = 0; j < MAXFILES; ++j) {
                if (head->freq[j] > 0) {
                    result[i].freq = head->freq[j];
                    strcpy(result[i].filename, filenames[j]);
                    i++;
                }
            }
            break;
        }
        head = head->next;
    }
    result[i].freq = 0;
    result[i].filename[0] = 0;
}


/* Print to standard output the frequency records for a word.
* Used for testing.
*/
void print_freq_records(FreqRecord *frp) {
    int i = 0;
    while(frp != NULL && frp[i].freq != 0) {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

void insert_freq_record(FreqRecord record, FreqRecord *list, int size) {
    int i;
    int min = 0;
    for (i = 0; i < size; ++i) {
        if (list[i].freq < list[min].freq) {
            min = i;
        }
    }
    list[min].freq = record.freq;
    strcpy(list[min].filename, record.filename);
}

void sort_freq_records(FreqRecord *list, int start, int end){
    int i = start, j = end;
    int pivot = list[(start + end) / 2].freq;
    int tmp;
    char buf[PATHLENGTH];

    while (i <= j) {
        while (list[i].freq > pivot) i++;
        while (list[j].freq < pivot) j--;
        if (i <= j) {
            tmp = list[i].freq;
            list[i].freq = list[j].freq;
            list[j].freq = tmp;

            strcpy(buf, list[i].filename);
            strcpy(list[i].filename, list[j].filename);
            strcpy(list[j].filename, buf);

            i++;
            j--;
        }
    };

    if (start < j) sort_freq_records(list, start, j);
    if (i < end) sort_freq_records(list, i, end);
}

/* run_worker
* - load the index found in dirname
* - read a word from the file descriptor "in"
* - find the word in tkhe index list
* - write the frequency records to the file descriptor "out"
*/
void run_worker(char *dirname, int in, int out){
    //printf("Worker %s: initialize worker with input fd %d and output fd %d.\n", dirname, in, out);
    Node *head = NULL;
    char **filenames = init_filenames();
    char listfile[PATHLENGTH];
    char namefile[PATHLENGTH];
    strcpy(listfile, dirname);
    strcat(listfile, "/index");
    strcpy(namefile, dirname);
    strcat(namefile, "/filenames");
    read_list(listfile, namefile, &head, filenames);

    char word[MAXWORD];
    FreqRecord *result = malloc((MAXFILES + 1) * sizeof(FreqRecord));
    while (read(in, word, MAXWORD) > 0) {
        //printf("[Worker %s] read %s\n", dirname, word);
        get_word(word, head, filenames, result);
        int i;
        for (i = 0; i < MAXFILES + 1; ++i) {
            write(out, &result[i], sizeof(FreqRecord));
            if (result[i].freq == 0) break;
        }
    }
    free(result);
    close(in);
    close(out);
}

