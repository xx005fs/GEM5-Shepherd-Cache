echo Shepherd Cache Miss Rates: 
grep -r "system.l2.overallMissRate" m5out_sc*
grep -r "system.l2.overallAccesses" m5out_sc*
grep -r "system.cpu.numCycles" m5out_sc*
echo LRU Miss Rates: 
grep -r "system.l2.overallMissRate" m5out_lru*
grep -r "system.l2.overallAccesses" m5out_lru*
grep -r "system.cpu.numCycles" m5out_lru*