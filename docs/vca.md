### **.vca Files**  

Trace files with a **.vca** extension contain a list of memory access operations. Each file consists of at least **three** required fields, with up to **four** possible fields per entry.  

---

### **File Structure**  
Each entry in the **.vca** file follows this structure:  

1. **Access Type:**  
   - **L**: Read operation  
   - **S**: Write operation  

2. **Memory Address:**  
   - Represented in **hexadecimal** format, always preceded by `0x`.  
   - Example: `0x08000100`  

3. **Instruction/Data Identifier:**  
   - **I**: Instruction address  
   - **D**: Data address  

5. **Write Data (Only for `S` Operations, Optional):**  
   - The **decimal** value to be written to memory.  
   - Only applies to write (`S`) operations.  
   - If it is not provided, a 0 will be written by default.  
   - All stores are **word-sized**.  

---

### **Additional Features**  
- **Comments:**  
  - Lines starting with a `#` are treated as comments and ignored by the simulator.  

- **Breakpoints:**  
  - Any instruction can be preceded by `!` to mark it as a **breakpoint**.  

---

### **Example Trace**  
```
# Loads
L 0x08000040 D
L 0x08000000 I

# Stores
!S 0x080002A0 D			#This will store a 0
S 0x08000124 D 2345
```
