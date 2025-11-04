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

/**
 * @file
 * Declaration of a First In First Out replacement policy.
 * The victim is chosen using the timestamp. The oldest entry is always chosen
 * to be evicted, regardless of the amount of times it has been touched.
 */

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_SC_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_SC_RP_HH__

#include "base/types.hh"
#include "mem/cache/replacement_policies/base.hh"

namespace gem5
{

struct SCRPParams;

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

class SC : public Base
{
  public:
    /** Number of SC Ways passed in as parameter */
    int num_sc_ways;

    /** Cache structure info */
    int num_assoc;
    int num_sets;

    /** Shepherd-Cache specific implementation of replacement data. */
    struct SCReplData : ReplacementData
    {
        /** Tick on which the entry was inserted. */
        Tick tickInserted;
        Tick tickAccessed;
        bool valid;
        int my_set;
        int my_way;

        /**
         * Default constructor
         */
        SCReplData() : tickInserted(0), tickAccessed(0), valid(false), my_set(0), my_way(0) {}
    };

    /** Shepherd-Cache specific necessary set data */
    struct SCSetData 
    {
        /** 
         * All shepherd cache related data structure within a single way in a given set
         * Format for each entry:
         * |   bit 7  | bit 6 |bit 5|bit 4|bit 3|bit 2|bit 1|bit 0|
         * |Empty Flag|-----reserved------|------Count Value------| 
         *   0 - Empty 
         *   1 - Full  
         * Number of bits for count value is based on what the total number of associativity
         * is, and thus will use a mask to vary it. For this example, total associativity 
         * including shepherd and main cache ways is 16, such that there are 4 shepherd
         * cache ways total. */
        std::vector<std::vector<uint8_t>> sc_data;

        /**
         * Next Value Counters within each set, vector length is determined by the number
         * of Shepherd Cache ways
         */
        std::vector<uint8_t> sc_set_nvcs; 
        
        /**
         * Vector storing which Shepherd Cache entry maps to which way of the cache,
         * vector length is determined by the number of Shepherd Cache ways.
         * [2, 4, 6, 8] means Shepherd Cache way 0 maps to way 2 within a set, so on
         */
        std::unordered_map<uint8_t, uint8_t> sc_way_ptr_map; 

        SCSetData() : sc_data(0), sc_set_nvcs(0), sc_way_ptr_map(0) {}
        SCSetData(int num_assoc, int num_sc_ways) {
          this->sc_data.resize(num_assoc, std::vector<uint8_t>(num_sc_ways, 0));
          for (int i = 0; i < num_sc_ways; i++) {
            this->sc_way_ptr_map.insert({i, num_assoc - num_sc_ways + i});
          }
          this->sc_set_nvcs.resize(num_sc_ways, 0);
        }
    };
    
    /** Vector of the entire cache's worth of shepherd-cache set data */
    mutable std::vector<SCSetData> set_data;
    
  public:
    PARAMS(SCRP);
    SC(const Params &p);
    ~SC() = default;

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Reset insertion tick to 0.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                                    override;

    /**
     * Touch an entry to update its replacement data.
     * Does not modify the replacement data.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Sets its insertion tick.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Find replacement victim using insertion timestamps.
     *
     * @param cands Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                     override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

} // namespace replacement_policy
} // namespace gem5

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_FIFO_RP_HH__
