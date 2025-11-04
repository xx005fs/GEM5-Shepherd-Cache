/**
 * Copyright (c) 2018-2020 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/cache/replacement_policies/sc_rp.hh"

#include <cassert>
#include <memory>

#include "params/SCRP.hh" // Change to SCRP.hh later
#include "sim/cur_tick.hh"

#define DBPRINTSCDATA(my_set)                                                                     \
    do                                                                                            \
    {                                                                                             \
        for (int i = 0; i < num_assoc; i++)                                                       \
        {                                                                                         \
            printf("SC_Set_Data for cache way%2d, ", i);                                          \
            for (int j = 0; j < num_sc_ways; j++)                                                 \
            {                                                                                     \
                if ((set_data[my_set].sc_data[i][j] & 0x80) == 0x00)                              \
                {                                                                                 \
                    printf("SC-way%d:  e ", j);                                                    \
                }                                                                                 \
                else                                                                              \
                {                                                                                 \
                    printf("SC-way%d: %2d ", j, set_data[my_set].sc_data[i][j] & (num_assoc - 1)); \
                }                                                                                 \
            }                                                                                     \
            printf("\n");                                                                         \
        }                                                                                         \
    } while (0);

#define DBPRINTSCPTRS(my_set)                                   \
    do                                                          \
    {                                                           \
        printf("SC Pointers: ");                                \
        for (int i = 0; i < num_sc_ways; i++)                   \
        {                                                       \
            printf("%d, ", set_data[my_set].sc_way_ptr_map[i]); \
        }                                                       \
        printf("\n");                                           \
    } while (0);

#define DBPRINTSCNVCS(my_set)                                \
    do                                                       \
    {                                                        \
        printf("SC NVC for Set %d: ", my_set);               \
        for (int i = 0; i < num_sc_ways; i++)                \
        {                                                    \
            printf("%d, ", set_data[my_set].sc_set_nvcs[i]); \
        }                                                    \
        printf("\n");                                        \
    } while (0);

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

static uint32_t count_LRU = 0;
static uint32_t count_SC = 0;
static uint32_t debug_flag = 0;
static int debug_set = -1;
static int debug_way = -1;

SC::SC(const Params &p)
  : Base(p), num_sc_ways(p.num_sc_ways)
{
    fatal_if(!!(num_sc_ways & (num_sc_ways - 1)) || num_sc_ways < 0 || num_sc_ways > 8, 
        "Number of Shepherd Cache ways has to be a power of 2 and between [0, 8]\n");
}

void // NOTE: NOT DONE YET!!! Placeholder for now
SC::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    std::shared_ptr<SCReplData> casted_replacement_data =
        std::static_pointer_cast<SCReplData>(replacement_data);
    // Reset insertion tick
    casted_replacement_data->tickInserted = Tick(0);
    // Reset LRU touch tick
    casted_replacement_data->tickAccessed = Tick(0);

    // Clear Shepherd Cache Counter and set it to empty for that set
    // Need to think through what else to write for this
    // casted_replacement_data->valid = false;
    if (debug_flag && debug_set == casted_replacement_data->my_set && debug_way == casted_replacement_data->my_way) {
        printf("Called Invalidate function with set: %d, way: %d\n", casted_replacement_data->my_set, casted_replacement_data->my_way);
    }
}

void
SC::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // At touch (cache hit), the count list of a certain sc-entry will be updated
    // if that sc-entry is valid and empty
    std::shared_ptr<SCReplData> casted_replacement_data =
        std::static_pointer_cast<SCReplData>(replacement_data);
    // Touched the entry, LRU tick needs update
    casted_replacement_data->tickAccessed = curTick();
    
    // Every time a touch happens, the counter and NVC need to be updated accordingly
    int my_set_idx = casted_replacement_data->my_set;
    int my_way_idx = casted_replacement_data->my_way;

    if (debug_flag && debug_set == my_set_idx && debug_way == my_way_idx) {
        printf("Before calling touch\n");
        DBPRINTSCDATA(my_set_idx);
        DBPRINTSCPTRS(my_set_idx);
        DBPRINTSCNVCS(my_set_idx);
    }

    /**
     * Search through all the SC entries and find which SC-way's counter and NVC
     * needs to be updated. If the entry is empty then copy previous NVC value to
     * the counter and increment NVC by 1, otherwise preserve the value and don't
     * increment NVC
     */
    for (int i = 0; i < num_sc_ways; i++) {
        if (!!(set_data[my_set_idx].sc_data[my_way_idx][i] & 0x80) == 0) {
            set_data[my_set_idx].sc_data[my_way_idx][i] = 0x80 | (set_data[my_set_idx].sc_set_nvcs[i] & (num_assoc - 1));
            set_data[my_set_idx].sc_set_nvcs[i] += 1;
        }
    }

    if (debug_flag && debug_set == my_set_idx && debug_way == my_way_idx) {
        printf("Called touch function with set: %d, way: %d\n", casted_replacement_data->my_set, casted_replacement_data->my_way);
        DBPRINTSCDATA(my_set_idx);
        DBPRINTSCPTRS(my_set_idx);
        DBPRINTSCNVCS(my_set_idx);
        debug_flag = 0;
    }
}

void
SC::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Set insertion tick
    std::shared_ptr<SCReplData> casted_replacement_data =
        std::static_pointer_cast<SCReplData>(replacement_data);
    casted_replacement_data->tickInserted = curTick();
    // LRU Tick also needs update
    casted_replacement_data->tickAccessed = curTick();
    casted_replacement_data->valid = true;

    int my_set_idx = casted_replacement_data->my_set;
    int my_way_idx = casted_replacement_data->my_way;
    /**
     * When reset is called, this means that we are inserting a new entry
     * into the cache, whether if it's through replacement or just populating
     * the cache initially. We clear whichever SC-way's sc_data to all empty 
     * and set all the counters along the way to 0
     */
    // First find the SC entry 
    int sc_way_idx = 0;
    int found_sc_way = 0;
    for (int i = 0; i < num_sc_ways; i++) {
        if (my_way_idx == set_data[my_set_idx].sc_way_ptr_map[i]) {
            // Found the SC entry, set all counter along that way to 0
            sc_way_idx = i;
            found_sc_way = 1;
            break;
        }
    }

    if (found_sc_way) { // Only do it if it found an SC way, otherwise do nothing
        std::fill(set_data[my_set_idx].sc_data[my_way_idx].begin(), set_data[my_set_idx].sc_data[my_way_idx].end(), 0x80); 
        for (int i = 0; i < num_assoc; i++) {
            set_data[my_set_idx].sc_data[i][sc_way_idx] &= 0x00;
        }
        set_data[my_set_idx].sc_set_nvcs[sc_way_idx] = 0;
    }
    if (debug_flag && debug_set == my_set_idx && debug_way == my_way_idx) {
        printf("Called reset function with set: %d, way: %d\n", casted_replacement_data->my_set, casted_replacement_data->my_way);
        printf("After Reset Clears SC Data\n");
        DBPRINTSCDATA(my_set_idx);
        DBPRINTSCPTRS(my_set_idx);
        DBPRINTSCNVCS(my_set_idx);
    }
}

ReplaceableEntry*
SC::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    int set_num = candidates[0]->getSet();

    /** 
     * Handle compulsory misses by finding the first invalid block 
     */
    for (const auto& candidate : candidates) {  
        if (std::static_pointer_cast<SCReplData>(candidate->replacementData)->valid == false) {
            // printf("Compulsory Miss, candidate set: %d, way: %d\n", candidate->getSet(), candidate->getWay());
            return candidate;
        }
    }
        
    /** 
     * There are no empty blocks remaining, check within shepherd cache
     * to find the right victim to replace, use FIFO to determine which
     * shepherd cache entry will be used to replace things 
     */
    int sc_way_num = 0;
    ReplaceableEntry* sc_candidate = candidates[set_data[set_num].sc_way_ptr_map[0]];
    ReplaceableEntry* sc_victim = sc_candidate;
    auto sc_candidate_rep_ptr = std::static_pointer_cast<SCReplData>(sc_candidate->replacementData);
    auto sc_victim_rep_ptr = std::static_pointer_cast<SCReplData>(sc_victim->replacementData);
    for (int i = 1; i < num_sc_ways; i++) {
        sc_candidate = candidates[set_data[set_num].sc_way_ptr_map[i]];
        sc_candidate_rep_ptr = std::static_pointer_cast<SCReplData>(sc_candidate->replacementData);
        if (sc_candidate_rep_ptr->tickInserted < sc_victim_rep_ptr->tickInserted) {
            sc_victim = sc_candidate;
            sc_victim_rep_ptr = std::static_pointer_cast<SCReplData>(sc_victim->replacementData);
            sc_way_num = i;
        }
    }
    
    /** 
     * Use the found Shepherd Cache entry to perform replacement on the rest
     * of the cache block, using LRU within the empty items if all counters 
     * aren't full and otherwise find the largest count and return that as 
     * the candidate of replacement 
     */
    int use_LRU = 0;
    int max_ct = -1;
    int max_way = 0;
    uint8_t sc_val = 0;
    ReplaceableEntry* victim = NULL;
    std::shared_ptr<SCReplData> candidate_rep_ptr = NULL;
    std::shared_ptr<SCReplData> victim_rep_ptr = NULL;
    for (int j = 0; j < num_assoc; j++) {
        sc_val = set_data[set_num].sc_data[j][sc_way_num];
        if (!!(sc_val & 0x80) == 0) {
            // Detected an empty flag, use LRU replacement and
            // return the right candidate
            use_LRU = 1;
            // Perform LRU also
            candidate_rep_ptr = std::static_pointer_cast<SCReplData>(candidates[j]->replacementData);
            if (victim == NULL) {
                victim = candidates[j];
                victim_rep_ptr = std::static_pointer_cast<SCReplData>(candidates[j]->replacementData);
            } else if (victim_rep_ptr != NULL && candidate_rep_ptr->tickAccessed < victim_rep_ptr->tickAccessed) {
                victim = candidates[j];
                victim_rep_ptr = std::static_pointer_cast<SCReplData>(candidates[j]->replacementData);
            }
        } else {
            // Find the way with the maximum count and save its index
            if ((sc_val & (num_assoc - 1)) > max_ct) {
                max_ct = sc_val & (num_assoc - 1);
                max_way = j;
            }
        }
    }
    
    /**
     * If we use LRU to replace, we look through every entry that's not a
     * shephered cache entry and find the least recently used item to kick out
     * Upon replacement, we also swap the pointer of the SC-candidate and MC-
     * candidate, demoting the SC-candidate to MC and setting newest item to
     * replace the old one as SC-candidate's SC entry, swap sc_data entries also
     */
    if (count_LRU % 50000 == 0) {
        printf("LRU Count: %d, SC Count: %d\n", count_LRU, count_SC);
    }
    int old_sc_way_idx = set_data[set_num].sc_way_ptr_map[sc_way_num];
    if (use_LRU) {
        count_LRU++;
        // DBPRINTSCDATA()
        set_data[set_num].sc_way_ptr_map[sc_way_num] = victim->getWay(); 
        return victim;
    } else {
        // debug_flag = 1;
        count_SC++;
        debug_way = candidates[max_way]->getWay();
        debug_set = candidates[max_way]->getSet();
        if (debug_flag) {
            printf("Found Shepherd Cache Way: %1d, it's way in the set is: %2d\n", sc_way_num, set_data[set_num].sc_way_ptr_map[sc_way_num]);
            printf("Shepherd Cache Generated Victim, candidate set: %d, way: %d\n", candidates[max_way]->getSet(), candidates[max_way]->getWay());
            printf("Before Shepherd Cache swaps the pointer\n");
            DBPRINTSCDATA(set_num);
            DBPRINTSCPTRS(set_num);
        }
        set_data[set_num].sc_way_ptr_map[sc_way_num] = candidates[max_way]->getWay();
        if (debug_flag) {
            printf("After Shepherd Cache swaps the pointer\n");
            DBPRINTSCPTRS(set_num);
        }
        return candidates[max_way];
    }
}

std::shared_ptr<ReplacementData>
SC::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new SCReplData());
}

} // namespace replacement_policy
} // namespace gem5
