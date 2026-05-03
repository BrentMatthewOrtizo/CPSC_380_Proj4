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
#define MAX_FRAMES 256
#define BACKING_STORE "BACKING_STORE.bin"

// page table entry
typedef struct
{
    int valid;
    int frame_number;
} PageTableEntry;

// tlb entry
typedef struct
{
    int valid;
    int page_number;
    int frame_number;
    int last_used;
} TLBEntry;

void print_usage(char *program_name)
{
    // print expected command format
    fprintf(stderr, "Usage: %s <addresses.txt> -tlb <lru|random> -page <fifo|lru|random> -frames <128|256>\n", program_name);
}

void initialize_page_table(PageTableEntry page_table[])
{
    // mark all page table entries as empty
    for (int i = 0; i < PAGE_TABLE_SIZE; i++)
    {
        page_table[i].valid = 0;
        page_table[i].frame_number = -1;
    }
}

void initialize_tlb(TLBEntry tlb[])
{
    // mark all tlb entries as empty
    for (int i = 0; i < TLB_ENTRIES; i++)
    {
        tlb[i].valid = 0;
        tlb[i].page_number = -1;
        tlb[i].frame_number = -1;
        tlb[i].last_used = 0;
    }
}

int search_tlb(TLBEntry tlb[], int page_number, int current_time)
{
    // search for page number in the tlb
    for (int i = 0; i < TLB_ENTRIES; i++)
    {
        if (tlb[i].valid == 1 && tlb[i].page_number == page_number)
        {
            tlb[i].last_used = current_time;
            return tlb[i].frame_number;
        }
    }

    return -1;
}

void update_tlb(TLBEntry tlb[], int page_number, int frame_number, char *tlb_policy, int current_time)
{
    int index = -1;

    // update existing entry if already present
    for (int i = 0; i < TLB_ENTRIES; i++)
    {
        if (tlb[i].valid == 1 && tlb[i].page_number == page_number)
        {
            tlb[i].frame_number = frame_number;
            tlb[i].last_used = current_time;
            return;
        }
    }

    // use first empty entry
    for (int i = 0; i < TLB_ENTRIES; i++)
    {
        if (tlb[i].valid == 0)
        {
            index = i;
            break;
        }
    }

    // choose replacement entry if tlb is full
    if (index == -1)
    {
        if (strcmp(tlb_policy, "random") == 0)
        {
            index = rand() % TLB_ENTRIES;
        }
        else
        {
            int oldest_time = tlb[0].last_used;
            index = 0;

            for (int i = 1; i < TLB_ENTRIES; i++)
            {
                if (tlb[i].last_used < oldest_time)
                {
                    oldest_time = tlb[i].last_used;
                    index = i;
                }
            }
        }
    }

    // place new entry into tlb
    tlb[index].valid = 1;
    tlb[index].page_number = page_number;
    tlb[index].frame_number = frame_number;
    tlb[index].last_used = current_time;
}

int load_page_from_backing_store(FILE *backing_store, signed char physical_memory[][PAGE_SIZE], int page_number, int frame_number)
{
    // move to page location
    if (fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET) != 0)
    {
        return 0;
    }

    // read one page into memory
    if (fread(physical_memory[frame_number], sizeof(signed char), PAGE_SIZE, backing_store) != PAGE_SIZE)
    {
        return 0;
    }

    return 1;
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

    int frame_number;
    int physical_address;
    signed char value;

    int next_free_frame = 0;
    int total_addresses = 0;
    int page_faults = 0;
    int tlb_hits = 0;
    int current_time = 0;

    char *output_file_name = "results.csv";

    PageTableEntry page_table[PAGE_TABLE_SIZE];
    TLBEntry tlb[TLB_ENTRIES];
    signed char physical_memory[MAX_FRAMES][PAGE_SIZE];

    srand(0);

    initialize_page_table(page_table);
    initialize_tlb(tlb);

    if (argc < 8)
    {
        print_usage(argv[0]);
        return 1;
    }

    filename = argv[1];

    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-tlb") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: missing value after -tlb.\n");
                print_usage(argv[0]);
                return 1;
            }

            tlb_policy = argv[i + 1];

            if (strcmp(tlb_policy, "lru") != 0 && strcmp(tlb_policy, "random") != 0)
            {
                fprintf(stderr, "Error: invalid TLB policy. Valid options are: lru, random\n");
                return 1;
            }

            i++;
        }
        else if (strcmp(argv[i], "-page") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: missing value after -page.\n");
                print_usage(argv[0]);
                return 1;
            }

            page_policy = argv[i + 1];

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
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: missing value after -frames.\n");
                print_usage(argv[0]);
                return 1;
            }

            frames = atoi(argv[i + 1]);

            if (frames != 128 && frames != 256)
            {
                fprintf(stderr, "Error: invalid frame count. Valid options are: 128 or 256\n");
                return 1;
            }

            i++;
        }
        else
        {
            fprintf(stderr, "Error: unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (filename == NULL || tlb_policy == NULL || page_policy == NULL || frames == 0)
    {
        fprintf(stderr, "Error: missing required argument.\n");
        print_usage(argv[0]);
        return 1;
    }

    input_file = fopen(filename, "r");
    if (input_file == NULL)
    {
        fprintf(stderr, "Error: could not open input file %s\n", filename);
        return 1;
    }

    backing_store = fopen(BACKING_STORE, "rb");
    if (backing_store == NULL)
    {
        fprintf(stderr, "Error: could not open backing store file %s\n", BACKING_STORE);
        fclose(input_file);
        return 1;
    }

    output_file = fopen(output_file_name, "w");
    if (output_file == NULL)
    {
        fprintf(stderr, "Error: could not open output file %s\n", output_file_name);
        fclose(input_file);
        fclose(backing_store);
        return 1;
    }

    fprintf(output_file,
            "Logical Address,Page Number,Offset,TLB Hit/Miss,Page Fault,Frame Number,Physical Address,Value,Replaced Page,Replaced Frame\n");

    while (fgets(line, sizeof(line), input_file) != NULL)
    {
        char *endptr;
        int page_fault = 0;
        int tlb_hit = 0;

        errno = 0;
        logical_address = (unsigned int)strtoul(line, &endptr, 10);

        if (errno != 0 || endptr == line)
        {
            fprintf(stderr, "Error: invalid logical address: %s", line);
            fclose(input_file);
            fclose(backing_store);
            fclose(output_file);
            return 1;
        }

        masked_address = logical_address & 0xFFFF;
        page_number = masked_address >> 8;
        offset = masked_address & 0xFF;

        total_addresses++;
        current_time++;

        // check tlb first
        frame_number = search_tlb(tlb, page_number, current_time);

        if (frame_number != -1)
        {
            tlb_hit = 1;
            tlb_hits++;
        }
        else
        {
            // check page table after tlb miss
            if (page_table[page_number].valid == 1)
            {
                frame_number = page_table[page_number].frame_number;
            }
            else
            {
                page_fault = 1;
                page_faults++;

                if (next_free_frame >= frames)
                {
                    fprintf(stderr, "Error: physical memory is full. Page replacement is not implemented yet.\n");
                    fclose(input_file);
                    fclose(backing_store);
                    fclose(output_file);
                    return 1;
                }

                frame_number = next_free_frame;

                if (load_page_from_backing_store(backing_store, physical_memory, page_number, frame_number) == 0)
                {
                    fprintf(stderr, "Error: could not read page %u from backing store.\n", page_number);
                    fclose(input_file);
                    fclose(backing_store);
                    fclose(output_file);
                    return 1;
                }

                page_table[page_number].valid = 1;
                page_table[page_number].frame_number = frame_number;

                next_free_frame++;
            }

            // update tlb after page table hit or page fault
            update_tlb(tlb, page_number, frame_number, tlb_policy, current_time);
        }

        physical_address = frame_number * FRAME_SIZE + offset;
        value = physical_memory[frame_number][offset];

        printf("Logical address: %u Physical address: %d Value: %d\n",
               masked_address,
               physical_address,
               value);

        fprintf(output_file, "%u,%u,%u,%s,%s,%d,%d,%d,%d,%d\n",
                masked_address,
                page_number,
                offset,
                tlb_hit ? "hit" : "miss",
                page_fault ? "yes" : "no",
                frame_number,
                physical_address,
                value,
                -1,
                -1);
    }

    if (total_addresses > 0)
    {
        printf("Number of Translated Addresses = %d\n", total_addresses);
        printf("Page Faults = %d\n", page_faults);
        printf("Page Fault Rate = %.3f\n", (double)page_faults / total_addresses);
        printf("TLB Hits = %d\n", tlb_hits);
        printf("TLB Hit Rate = %.3f\n", (double)tlb_hits / total_addresses);
    }

    fclose(input_file);
    fclose(backing_store);
    fclose(output_file);

    return 0;
}