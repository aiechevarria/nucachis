## **.ini Files**

All configuration files with a **.ini** extension must include at least a [cpu] and [memory] module representing the **CPU** and **Main Memory**. Also, [cacheN] modules representing **Caches** can be added. 

Each module has different parameters, or keywords, that represent a specific value of that component. Once a module is used, it must have all parameters declared:

---

### **CPU Parameters**
The CPU module includes the following parameters:

- `word_width`: Word size in bits.
- `address_width`: Memory address size in bits.
- `rand_seed`: Random simulation seed. Used for the RAND replacement policy

---

### **Memory Parameters**
The memory module includes the following parameters:

- `size`: Maximum memory size. Can be expressed as an integer followed by a multiplier (**K, M, G**).
  - Example: 2 GiB can be written as **2G**, **2048M**, etc.
- `page_size`: Page size. The maximum number of bytes simulated and displayed on the memory window. Accesses outside of this page cannot be made.
- `page_base_address`: Base address of the page displayed in the memory view.
- `access_time_1`: Access time for individual accesses. Accepts the **m** (1e-3), **u** (1e-6), **n** (1e-9), **p** (1e-12) multipliers.
- `access_time_burst`: Access time for sequential accesses. Also accepts **m, u, n, p** multipliers.

---

### **Cache Parameters**
The simulator supports up to **5 caches** but can be incremented by modifying MAX_CACHE_LEVELS (Misc.h). Each cache is defined using the directive [cacheN], where **N** represents the cache level. A cache module includes the following parameters:

- `line_size`: Cache line size in bytes. Supports **K, M, and G** multipliers.
- `size`: Total cache size. The number of lines is calculated as: number_of_lines = size / line_size. If the cache is split, the size will be distributed equally for both caches Supports **K, M, and G** multipliers.
- `associativity`: Set associativity. Possible values:
  - **F** for fully associative.
  - **1** for direct-mapped.
  - Any power of **2** for set-associative caches.
- `write_policy`: Write policy:
  - **wt** for Write-Through.
  - **wb** for Write-Back.
- `replacement_policy`: Replacement policy options:
  - **lru** (Least Recently Used).
  - **lfu** (Least Frequently Used).
  - **fifo** (First In, First Out).
  - **rand** (Random).
- `separated`: Defines if instruction caches are separate. Possible values:
  - **1** (or `true`, `yes`) for separate instruction and data caches.
  - **0** (or `false`, `no`) for unified caches.
- `access_time`: Cache access time. Accepts **m, u, n, p** multipliers.
