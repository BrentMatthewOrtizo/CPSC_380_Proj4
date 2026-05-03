// Bintie Kayode and Brent Matthew Ortizo

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

// System Specifications:

#define TLB_ENTRIES 16
#define FRAME_SIZE 256
#define PAGE_SIZE  256
#define PAGE_TABLE_SIZE 256
#define BACKING_STORE "BACKING_STORE.bin"

int main(int argc, char *argv[])
{
    int option;
    int frames = 0;
    FILE *input_file;
    FILE *output_file;
    char *page = NULL;
    char *tlb = NULL;
    char line[100];
    unsigned int page_number, offset;
    char *outputFileName = "results.csv";

    // Define long options
    static struct option long_options[] = {
        {"frames", required_argument, 0, 'f'},
        {"page", required_argument, 0, 'p'},
        {"tlb", required_argument, 0, 't'},
        {0, 0, 0, 0}};

    // Parse command-line options
    while ((option = getopt_long(argc, argv, "t:p:f:", long_options, NULL)) != -1)
    {
        switch (option)
        {
        case 'f':
            // Validate that frames is a positive integer, and a power of 2
            frames = atoi(optarg);
            if (frames <= 0 || (frames & (frames - 1)) != 0)
            {
                fprintf(stderr, "Error: invalid frame count. Must be a positive power of 2.\n");
                return 1;
            }
            break;
        case 'p':
            page = optarg;
            if (strcmp(page, "fifo") != 0 && strcmp(page, "random") != 0 && strcmp(page, "lru") != 0)
            {
                fprintf(stderr, "Error: invalid Page replacement policy. Valid options are: fifo, random, and lru\n");
                return 1;
            }
            break;
        case 't':
            tlb = optarg;
            if (strcmp(tlb, "fifo") != 0 && strcmp(tlb, "random") != 0)
            {
                fprintf(stderr, "Error: invalid TLB replacement policy. Valid options are: fifo, random\n");
                return 1;
            }
            break;
        case '?':
            // getopt_long already printed an error message
            break;
        default:
            abort();
        }
    }

    // Validate that all required options were provided
    if (frames == 0 || page == NULL || tlb == NULL)
    {
        fprintf(stderr, "Usage: %s -f <frames> -p <page replacement policy> -t <tlb replacement policy> <input file>\n", argv[0]);
        return 1;
    }
    // Validating that the number of non-option arguments is exactly 1, and that the input file can be opened
    if (optind != argc - 1)
    {
        fprintf(stderr, "Too many arguments: %s\n Only one non-option argument allowed", argv[optind]);
        return 1;
    }
    else
    {
        char *filename = argv[optind];
        input_file = fopen(filename, "r");

        // Check that the files can be opened
        if (input_file == NULL)
        {
            fprintf(stderr, "Error: could not open input file %s\n", filename);
            return 1;
        }

        output_file = fopen(outputFileName, "w");
        if (output_file == NULL)
        {
            fprintf(stderr, "Error: could not open input file %s\n", outputFileName);
            return 1;
        }
        // Write header to the output CSV file
        fprintf(output_file, "Logical Address,\tPage Number,\t" 
                "Offset,\tTLB hit/miss,\tPage fault (yes/no),Frame number\t,"
                "Physical address,\tValue at address,Replaced page/frame");
    }

        #ifdef DEBUG
            printf("Running in DEBUG mode\n");
        #endif
    // Read and print the contents of the input file
    while (fgets(line, sizeof(line), input_file) != NULL)
    {
        // Check that the logical address is a valid 32-bit unsigned integer
        unsigned int logical_address = atoi(line);
        if (logical_address < 0 || logical_address > 0xFFFFFFFF)
        {
            fprintf(stderr, "Error: %s is an invalid logical address. Must be a 32-bit unsigned integer.\n", argv[0]);
            return 1;
        }
        #ifdef DEBUG
            printf("Input from address.txt: %s", line);
        #endif
        // Masking the rightmost 16 bits of the logical address to get the page number, 
        // and masking the rightmost 12 bits to get the offset
        logical_address = logical_address & 0xFFFF; // Mask the rightmost 16 bits
        page_number = logical_address >>  8; // Shift right by 8 bits 
        offset = logical_address & 0xFF; // Mask the rightmost 8 bits 
    }

    fclose(input_file);
    fclose(input_file);
    return 0;
}