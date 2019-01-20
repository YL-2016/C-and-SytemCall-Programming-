#define PATHLENGTH 128
#define MAXRECORDS 100

// This data structure is used by the workers to prepare the output
// to be sent to the master process.

typedef struct {
	int freq;
	char filename[PATHLENGTH];
} FreqRecord;

void get_word(char *word , Node *head, char **filenames, FreqRecord *result);
void insert_freq_record(FreqRecord record, FreqRecord *list, int size);
void sort_freq_records(FreqRecord *list, int start, int end);
void print_freq_records(FreqRecord *frp);
void run_worker(char *dirname, int in, int out);
