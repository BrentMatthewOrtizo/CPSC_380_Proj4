// Bintie Kayode and Brent Matthew Ortizo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// system specifications
#define TLB_ENTRIES 16
#define FRAME_SIZE 256
#define PAGE_SIZE 256
#define PAGE_TABLE_SIZE 256
#define BACKING_STORE "BACKING_STORE.bin"

void print_usage(char *program_name)
{
    // expected command format for the assignment
    fprintf(stderr, "Usage: %s <addresses.txt> -tlb <lru|random> -page <fifo|lru|random> -frames <128|256>\n", program_name);
}

int main(int argc, char *argv[])
{
    int frames = 0;
    FILE *input_file = NULL;
    FILE *output_file = NULL;
    FILE *backing_store = NULL;

    char *filename = NULL;
    char *page_policy = NULL;
    char *tlb_policy = NULL;
    char line[100];

    unsigned int logical_address;
    unsigned int masked_address;
    unsigned int page_number;
    unsigned int offset;

    char *output_file_name = "results.csv";

    // check argument count
    if (argc < 8)
    {
        print_usage(argv[0]);
        return 1;
    }

    // get the address file first 
    filename = argv[1];

    // to manually parse command-line arguments so we can support -tlb, -page, and -frames exactly
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-tlb") == 0)
        {
            // to make sure -tlb has a value after it
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: missing value after -tlb.\n");
                print_usage(argv[0]);
                return 1;
            }

            tlb_policy = argv[i + 1];

            // note: assignment requires only lru and random for the tlb
            if (strcmp(tlb_policy, "lru") != 0 && strcmp(tlb_policy, "random") != 0)
            {
                fprintf(stderr, "Error: invalid TLB policy. Valid options are: lru, random\n");
                return 1;
            }

            i++;
        }
        else if (strcmp(argv[i], "-page") == 0)
        {
            // to make sure -page has a value after it
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: missing value after -page.\n");
                print_usage(argv[0]);
                return 1;
            }

            page_policy = argv[i + 1];

            // need fifo, lru, and random for page replacement
            if (strcmp(page_policy, "fifo") != 0 &&
                strcmp(page_policy, "lru") != 0 &&
                strcmp(page_policy, "random") != 0)
            {
                fprintf(stderr, "Error: invalid page replacement policy. Valid options are: fifo, lru, random\n");
                return 1;
            }

            i++;
        }
        else if (strcmp(argv[i], "-frames") == 0)
        {
            // make sure -frames has a value after it
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: missing value after -frames.\n");
                print_usage(argv[0]);
                return 1;
            }

            frames = atoi(argv[i + 1]);

            // use 256 frames first and 128 frames later
            if (frames != 128 && frames != 256)
            {
                fprintf(stderr, "Error: invalid frame count. Valid options are: 128 or 256\n");
                return 1;
            }

            i++;
        }
        else
        {
            // needed to catch unknown flags or extra arguments
            fprintf(stderr, "Error: unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    // make sure required options were provided
    if (filename == NULL || tlb_policy == NULL || page_policy == NULL || frames == 0)
    {
        fprintf(stderr, "Error: missing required argument.\n");
        print_usage(argv[0]);
        return 1;
    }

    // open the input address file
    input_file = fopen(filename, "r");
    if (input_file == NULL)
    {
        fprintf(stderr, "Error: could not open input file %s\n", filename);
        return 1;
    }

    // open the backing store binary file
    backing_store = fopen(BACKING_STORE, "rb");
    if (backing_store == NULL)
    {
        fprintf(stderr, "Error: could not open backing store file %s\n", BACKING_STORE);
        fclose(input_file);
        return 1;
    }

    // open the csv output file
    output_file = fopen(output_file_name, "w");
    if (output_file == NULL)
    {
        fprintf(stderr, "Error: could not open output file %s\n", output_file_name);
        fclose(input_file);
        fclose(backing_store);
        return 1;
    }

    // write the required csv header
    fprintf(output_file,
            "Logical Address,Page Number,Offset,TLB Hit/Miss,Page Fault,Frame Number,Physical Address,Value,Replaced Page,Replaced Frame\n");

    // read every logical address from the address file
    while (fgets(line, sizeof(line), input_file) != NULL)
    {
        char *endptr;

        errno = 0;

        // use strtoul instead of atoi so invalid input can be detected
        logical_address = (unsigned int)strtoul(line, &endptr, 10);

        // validate invalid or overflowing address input
        if (errno != 0 || endptr == line || (*endptr != '\n' && *endptr != '\0'))
        {
            fprintf(stderr, "Error: invalid logical address: %s", line);
            fclose(input_file);
            fclose(backing_store);
            fclose(output_file);
            return 1;
        }

        // mask the rightmost 16 bits because only 16-bit addresses matter
        masked_address = logical_address & 0xFFFF;

        // extract the upper 8 bits as the page number
        page_number = masked_address >> 8;

        // extract the lower 8 bits as the offset
        offset = masked_address & 0xFF;

        // TODO: placeholder row for now so we can confirm csv logging works, work on page table, tlb, and physical memory implementation
        fprintf(output_file, "%u,%u,%u,%s,%s,%d,%d,%d,%d,%d\n",
                masked_address,
                page_number,
                offset,
                "miss",
                "no",
                -1,
                -1,
                0,
                -1,
                -1);
    }

    // close every file exactly once
    fclose(input_file);
    fclose(backing_store);
    fclose(output_file);

    return 0;
}