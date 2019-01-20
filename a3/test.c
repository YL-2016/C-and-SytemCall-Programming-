#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "freq_list.h"
#include "worker.h"

int main(void)
{
    printf("Testing get_word.\n");
    Node *head = NULL;
    char **filenames = init_filenames();
    read_list("./testing/big/books/index", "./testing/big/books/filenames", &head, filenames);

    FreqRecord *result = malloc((MAXFILES + 1) * sizeof(FreqRecord));

    printf("Search word 'test' in testing/big/books.\n");
    get_word("test", head, filenames, result);
    print_freq_records(result);

    printf("Search word 'well' in testing/big/books.\n");
    get_word("well", head, filenames, result);
    print_freq_records(result);

    free(result);
}