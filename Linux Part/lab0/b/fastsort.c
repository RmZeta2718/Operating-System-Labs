#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sort.h"
#include <sys/types.h>
#include <sys/stat.h>

int infile = -1, outfile = -1;
rec_t *arr = NULL;

void cleanup(int exit_code) {
    if (infile >= 0) close(infile);
    if (outfile >= 0) close(outfile);
    if (arr != NULL) free(arr);
    exit(exit_code);
}

void usageErr(char *prog) {
    fprintf(stderr, "Usage: %s inputfile outputfile\n", prog);
    cleanup(1);
}

void openErr(char *file_path) {
    fprintf(stderr, "Error: Cannot open file %s\n", file_path);
    cleanup(1);
}

void allocErr(unsigned sz) {
    fprintf(stderr, "Error: Cannot allocate %u byte(s)\n", sz);
    cleanup(1);
}

void readErr() {
    fprintf(stderr, "Error: Inputfile is not well formatted\n");
    cleanup(1);
}

int cmp(const void *p1, const void *p2) {
    const rec_t *r1, *r2;
    r1 = (const rec_t*)p1;
    r2 = (const rec_t*)p2;
    if (r1->key < r2->key) return -1;
    if (r1->key > r2->key) return 1;
    return 0;
}

int main(int argc, char *argv[]) {
    // arguments
	if (argc != 3) usageErr(argv[0]);
    char *inpath = argv[1];
    char *outpath = argv[2];

    // open and create output file
    infile = open(inpath, O_RDONLY);
    if (infile < 0) openErr(inpath);
    outfile = open(outpath, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
    if (outfile < 0) openErr(outpath);

    // allocate mem according to file size
    struct stat filestat; 
    fstat(infile, &filestat);
    arr = (rec_t*)malloc(filestat.st_size);
    if (!arr) allocErr(filestat.st_size);
    unsigned sz = filestat.st_size / sizeof(rec_t);

    // read & dump data
    rec_t r;
    for (int i = 0; i < sz; ++i) {	
		int rc;
		rc = read(infile, &r, sizeof(rec_t));
		if (rc <= 0) readErr();
        arr[i] = r;
		// printf("key: %u rec:", r.key);
		// int j;
		// for (j = 0; j < NUMRECS; j++) 
		// 	printf("%u ", r.record[j]);
		// printf("\n");
    }
    // EOF expected
    if (read(infile, &r, sizeof(rec_t)) != 0) readErr();

    // fast sorting
    qsort(arr, sz, sizeof(rec_t), cmp);

    // output sorted data
    for (int i = 0; i < sz; ++i) write(outfile, &arr[i], sizeof(rec_t));

    cleanup(0);
}

