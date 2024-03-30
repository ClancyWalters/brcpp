
```
Baseline 176260ms
Memory Mapping 120765ms
Removed line string 104446ms
Removed std::stod() 38160ms ± 153ms
Removing .contains() 29096ms ± 29ms
Removing negative ptr advancement branch 28875ms ± 56ms
Replace ternary with if statement 28931ms ± 29ms
Removing removing value negation branch 29185ms ± 146ms
Removing max min branches 29434ms ± 29ms
Using std::string_view 29708ms ± 416ms
absl::flat_hash_map 26320ms ± 158ms
absl::node_hash_map 25974ms ± 78ms
absl::node_hash_map & std::string_vew 29415ms ± 59ms
```