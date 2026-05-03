# SP26 CPSC 380 Programming Assignment 4 – Virtual Memory Manager

## Contributors
Brent Matthew Ortizo  
Student ID: 2452997  
Email: ortizo@chapman.edu  

Kayode Binitie  
Student ID: 2461327  
Email: binitie@chapman.edu  

---

## Repository

GitHub Repository:
https://github.com/BrentMatthewOrtizo/CPSC_380_Proj4/tree/main

---

## Description

This project implements a virtual memory manager that translates logical addresses into physical addresses using a page table and a Translation Lookaside Buffer (TLB).

Each logical address is masked to 16 bits and split into:

- Page number (upper 8 bits)
- Offset (lower 8 bits)

The program simulates:

- Demand paging
- TLB caching
- Page replacement algorithms

It outputs the physical address and the value stored at that address.

---

## Design Approach

The translation process follows:

1. Extract page number and offset
2. Check TLB
3. If TLB miss → check page table
4. If page not in memory → page fault
5. Load page from backing store
6. Update page table and TLB

Core structures:

- Page table (256 entries)
- TLB (16 entries)
- Physical memory (128 or 256 frames)
- Backing store (BACKING_STORE.bin)

Tracking variables:

- last_used for LRU
- loaded_time for FIFO
- frame_to_page[] for reverse mapping

---

## Policies Implemented

TLB Policies:
- LRU
- Random

Page Replacement Policies:
- FIFO
- LRU
- Random

Policies are selected at runtime via command-line arguments.

---

## Workloads Used

Random (addresses.txt)  
Sequential (0 to 65535, one address per line)  
Looping (small repeating set)  
Stride (every 256 jump)  

---

## Compilation
```
gcc vmmgr.c -o vmmgr
```

---

## Execution
```
./vmmgr addresses.txt -tlb lru -page fifo -frames 128
```

---

## Output
```
Logical address: X Physical address: Y Value: Z
```

---

## CSV Output
Each workload run was saved as a separate CSV file for analysis.

Each run generates:

```
results.csv
```

Fields:
```
Logical Address,Page Number,Offset,TLB Hit/Miss,Page Fault,Frame Number,Physical Address,Value,Replaced Page,Replaced Frame
```

---

## Experimental Results

Random:
- Total: 1000  
- Page Faults: 538  
- Page Fault Rate: 0.538  
- TLB Hits: 55  
- TLB Hit Rate: 0.055  
- Replacements: 411  

Sequential:
- Total: 65536  
- Page Faults: 256  
- Page Fault Rate: 0.004  
- TLB Hits: 65280  
- TLB Hit Rate: 0.996  
- Replacements: 129  

Looping:
- Total: 1000  
- Page Faults: 2  
- Page Fault Rate: 0.002  
- TLB Hits: 998  
- TLB Hit Rate: 0.998  
- Replacements: 1  

Stride:
- Total: 256  
- Page Faults: 256  
- Page Fault Rate: 1.000  
- TLB Hits: 0  
- TLB Hit Rate: 0.000  
- Replacements: 129  

---

## Data Analysis Report

The looping workload performed the best. Because it repeatedly accessed a very small set of pages, those pages stayed in memory and the TLB after being loaded once. This resulted in an extremely low page fault rate (0.002) and a very high TLB hit rate (0.998). This demonstrates strong temporal locality.

The sequential workload also performed very well. Even though it accessed a large range of addresses, it did so in order, allowing for strong spatial locality. The page fault rate remained very low (0.004), and the TLB hit rate was extremely high (0.996).

The random workload showed much worse performance. Since addresses were accessed unpredictably, the system could not take advantage of locality. This resulted in a high page fault rate (0.538) and a low TLB hit rate (0.055). Pages were frequently replaced before reuse.

The stride workload performed the worst. Each access jumped to a different page, preventing reuse entirely. This resulted in a 100% page fault rate and a 0% TLB hit rate, representing a worst-case scenario.

---

## Policy Observations

- Best under locality: LRU  
  Keeps recently used pages, ideal for looping/sequential  

- LRU vs FIFO:  
  LRU performs better when pages are reused frequently  
  FIFO may evict useful pages too early  

- Random performance:  
  Similar to others when access pattern is unpredictable  

- Effect of reduced memory (128 frames):  
  More page faults and replacements occur  
  Memory fills faster, forcing eviction  

---

## Assumptions

- Input file contains valid integers  
- Only lower 16 bits are used  
- BACKING_STORE.bin is correctly formatted  
- Physical memory frames are filled in order  
- One page = 256 bytes  

---

## Files Included

- vmmgr.c — main implementation  
- addresses.txt — random workload  
- sequential.txt — sequential workload  
- looping.txt — looping workload  
- stride.txt — stride workload  
- results_*.csv — output data  
- README.md — documentation

---

## Error Handling

The program handles:

- Missing input file  
- Invalid command-line arguments  
- Invalid policy selection  
- Invalid frame count  

---

## Key Insights

- Locality drastically improves performance  
- LRU is best for realistic workloads  
- Stride patterns break caching completely  
- Memory size directly impacts fault rate  
- TLB effectiveness depends on access pattern  

---

## Conclusion

This project demonstrates how virtual memory systems behave under different workloads and policies. By implementing demand paging, TLB caching, and page replacement strategies, we were able to analyze performance tradeoffs and understand the importance of locality in memory systems.