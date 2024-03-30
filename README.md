
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
unordered_dense::map 30760ms ± 400ms
absl::node_hash_map & unordered_dense::hash 26141ms ± 732ms
unordered_dense::map & absl::hash 31331ms ± 3039ms
absl::node_hash_map & std::hash 30354ms ± 3369ms
single threaded blocked 33629ms ± 403ms
multi threaded blocked 6885ms ± 454ms
```