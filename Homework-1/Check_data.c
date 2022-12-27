#include <stdio.h>
#include <math.h>
#include <intrin.h>
#include <windows.h>

#include "Cache_research.c"

    /* Analysis of the received data */
    int TLB = 0;                                // last analyzed level - TLB
    for (int cache_level = 1;;cache_level++) {

        /* Cache access size and time */
        int cache_size = allsegs[0].data.size_r, step_count = 0;
        if (!cache_size) break;                 // Highest level (main memory)

        int less=0, same=0, more=0;             // Determine the median "on the go"

        /* For all tried step sizes */
        for(segments * cursegs = allsegs; cursegs -> step_size_bytes; cursegs++) {
            segment * next = cursegs -> data.next;  // Next cache level
            if(!next) {                             // Bump into the main memory
                printf("Missing results for L%d! Step size %d, array size %f mb\n",
                       cache_level, cursegs -> step_size_bytes, (double)cursegs -> data.size_l / mb);
                cache_size = 0;
                break;
            }
            /* If the next level is not much different - combine */
            while(fabs(cursegs -> data.level-next -> level) < cursegs -> data.level*.2) {
                cursegs -> data.size_r = next -> size_r;
                cursegs -> data.total += next -> total;
                cursegs -> data.width += next -> width;
                cursegs -> data.level = cursegs -> data.total / cursegs -> data.width;
                cursegs -> data.next = next -> next;
                free(next);
                next = cursegs -> data.next;
                /* Reinitialization */
                if (cursegs == allsegs) {
                    cache_size = cursegs -> data.size_r;
                    if (!TLB) latency = cursegs -> data.level;
                }
            }
            /* If the next step coincided with the calculated one */
            if (cursegs -> data.size_r == cache_size)
                same++;
            /* If the next step differs from the calculated one */
            else if (cursegs -> data.size_r == step(cache_size))
                more++;
            else if (step(cursegs -> data.size_r) == cache_size)
                less++;
            /* If the step is much to the left of the calculated one: TLB effect */
            else if (cursegs -> data.size_r < cache_size / 2) {
                /* The size measured is not real */
                cache_size = cursegs -> data.size_r;
                more = less = 0; same = 1;
                /* Add fake steps with the same level */
                for(segments * prevsegs = allsegs; prevsegs < cursegs; prevsegs++) {
                    segment * second = (segment*)malloc(sizeof(segment));
                    *second = prevsegs -> data;
                    prevsegs -> data.next = second;
                    prevsegs -> data.size_r = second -> size_l = cache_size;
                }
            }
            /* If there are more different steps than the calculated ones */
            if(less > same) {
                cache_size = cursegs -> data.size_r;
                more = same; same = less; less = 0;
            } else if (more > same) {
                cache_size = cursegs -> data.size_r;
                less = same; same = more; more = 0;
            }
            if (!TLB && fabs(latency-cursegs -> data.level) < latency*.1) {
                total += cursegs -> data.level;
                latency = total / ++step_count;
            }
        }
        if(!cache_size) break;      // Cache size measure failed

        /* Associativity and parameters TLB */
        int min_way_size = 0, max_way_size = 0, next_step_at = 2 * cache_size;
        /* Overdue. Added by TLB access time */
        double additional = (allsegs[0].data.next -> level - latency) / 2;
        if (additional < 0) additional = 0;         // Within the margin of error
        TLB = 1;                                    // Count for TLB until convinced otherwise
        for(segments * cursegs = allsegs; cursegs -> step_size_bytes; cursegs++) {
            segment * next = cursegs -> data.next;  // Next cache level
                                                    // If all the steps are filled, the left sides of the steps coincide
            if(cursegs -> data.size_r <= cache_size) {
                if(max_way_size && (max_way_size != next -> size_l - cache_size)) {
                    printf("Inconsistent results for L%d! Step size %d, array size %f mb\n",
                           cache_level, cursegs -> step_size_bytes, (double)next -> size_l / mb);
                }
                min_way_size = cursegs -> step_size_bytes;        // Step is small
                max_way_size = next -> size_l - cache_size;       // Size - width of the step
                /* If the step is not vertical, then the exact size is known */
                if (next -> size_l > step(cache_size)) min_way_size = max_way_size;
                /* If not extension, the step is shifted twice to the right */
            } else if (cursegs -> data.size_r > step(cache_size)) {
                if(cursegs -> data.size_r != next_step_at)
                    printf("Inconsistent results for L%d! Step size %d, array size %f mb\n",
                           cache_level, cursegs -> step_size_bytes, (double)cursegs -> data.size_r/mb);
                if (!max_way_size)
                    max_way_size = cursegs -> step_size_bytes / 2;  // At least doubling the step
                next_step_at *= 2;                                  // Next step should be twice as far to the right
            }

            /* Similar to TLB if the extra overdue is doubled when the step is doubled */
            double new_additional = cursegs -> data.next -> level - latency;
            if ((fabs(new_additional - additional * 2) < new_additional*.1) || (additional < latency*.1))
                additional = new_additional;
            else                            // Not look like TLB
                TLB = 0;

            /* Finished with the first segment */
            cursegs -> data = *next;
            free(next);
        }
        if (TLB)
            printf("TLB size: %d, latency: %.2f cycles (%.2f ns)\n"
                   "    way size: min. %d, max. %d\n",
                   cache_size/4096, additional/2, (additional/2)/MHz*1000,
                   min_way_size/4096, max_way_size/4096);
        else
            printf("L%d size: %d KB, latency: %.2f cycles (%.2f ns)\n"
                   "   way size: min. %d KB, max. %d KB\n",
                   cache_level, cache_size/1024, latency, latency/MHz*1000,
                   min_way_size/1024, max_way_size/1024);
    }
