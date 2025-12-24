/**
 * @file confparser.c
 * @brief Parses the main configuration (.ini) file.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "Misc.h"
#include "PolicyReplacement.h"
#include "PolicyWrite.h"
#include "Simulator.h"
#include "ParserConfig.h"

// Valid configuration keys for each simulated element
#define CPU_KEYS 2
#define MEMORY_KEYS 5
#define CACHE_KEYS 7
const char* keysCpu[] =       {"address_width", "rand_seed"};
const char* keysMemory[] =    {"size", "access_time_1","access_time_burst", "page_size", "page_base_address"};
const char* keysCache[] =     {"line_size", "size", "associativity", "write_policy", "replacement_policy", "separated", "access_time"};

/* Wrappers for misc parsing functions */

/**
 * Parse integer configuration field.
 * @param ini dictionary from read ini file
 * @param key which is being parsed
 * @param confVariable return param where the parsed value will be placed
 * @param errors In case of error this variable will have its value incremented by 1.
 */

void parseConfInt(dictionary* ini, const char* key, int* confVariable, int* errors) {
    const char* confString = iniparser_getstring(ini, key, NULL);
    int value = parseInt(confString);

    if (value == -1) {
        fprintf(stderr,"Error: %s value is not valid\n", key);
        (*errors)++;
    } else if (value == -2) {
        fprintf(stderr,"Error: Missing mandatory key %s\n", key);
        (*errors)++;
    } else {
        *confVariable = value;
    }
}

/**
 * Parse long decimal configuration field.
 * @param ini dictionary from read ini file
 * @param key which is being parsed
 * @param confVariable return param where the parsed value will be placed
 * @param errors In case of error this variable will have its value incremented by 1.
 * @param base2 If it should be parsed as base 2 or not.
 */
void parseConfLong(dictionary *ini, const char *key, long int *confVariable, int *errors, bool base2) {
    const char * confString = iniparser_getstring(ini, key, NULL);

    long value = parseLong(confString, base2);

    if (value == -1) {
        fprintf(stderr,"Error: %s value is not valid\n", key);
        (*errors)++;
    } else if (value == -2) {
        fprintf(stderr,"Error: Missing mandatory key %s\n", key);
        (*errors)++;
    } else {
        *confVariable = value;
    }
}

/**
 * Parse double configuration field.
 * @param ini dictionary from read ini file
 * @param key which is being parsed
 * @param confVariable return param where the parsed value will be placed
 * @param errors In case of error this variable will have its value incremented by 1.
 */
void parseConfDouble(dictionary *ini, const char *key, double *confVariable, int *errors) {
    const char * confString = iniparser_getstring(ini, key, NULL);

    double value = parseDouble(confString);

    if (value == -1) {
        fprintf(stderr,"Error: %s value is not valid\n", key);
        (*errors)++;
    } else if (value == -2) {
        fprintf(stderr,"Error: Missing mandatory key %s\n", key);
        (*errors)++;
    } else {
        *confVariable = value;
    }
}

/**
 * parse memory address configuration field.
 * @param ini dictionary from read ini file
 * @param key which is being parsed
 * @param confVariable return param where the parsed value will be placed
 * @param errors In case of error this variable will have its value incremented by 1.
 */
void parseConfAddress(dictionary *ini, const char *key, long int *confVariable, int *errors) {
    const char * confString = iniparser_getstring(ini, key, NULL);

    long value = parseAddress(confString);

    if (value == -1) {
        fprintf(stderr,"Error: %s value is not valid\n", key);
        (*errors)++;
    } else if (value == -2) {
        fprintf(stderr,"Error: Missing mandatory key %s\n", key);
        (*errors)++;
    } else {
        *confVariable = value;
    }
}

/*
 * Checks that all the keys in a section are valid
 */
void checkSectionKeys(dictionary *ini, const char *section, int numberOfValidKeys, char *validKeys[], int *errors) {
   int nkeys = iniparser_getsecnkeys(ini, section);
   const char * keys[nkeys];
   iniparser_getseckeys(ini, section, keys);

   // Make sure that the section does not have unknown keys
   for(int i=0; i<nkeys; i++) {
      // By comparing each one with the known keys of a cache section
      for(int j=0; j<numberOfValidKeys; j++) {
         char key[300];
         snprintf(key, 300, "%s:%s", section, validKeys[j]);
         if (strcmp(keys[i], key)==0) {
            // This key is known. Go to the next one.
            goto nextKey;
         }
      }
      // This key matches none of the known keys of a cache section
      fprintf(stderr,"Error: unknown key %s\n", keys[i]);
      (*errors)++;
      nextKey:;
   }
}

/**
 * Read the simulator configuration file.
 * @param ini_name the file name
 * @param cacheLevels Pointer to a cache level counter. Will return the number of cache levels in the config file.
 */
dictionary *readConfigurationFile(char* iniName, uint8_t* cacheLevels) {
    int errors = 0;
    dictionary *ini;

    if ((ini = iniparser_load(iniName)) == NULL) {
        fprintf(stderr, "Error loading file: %s\n", iniName);
        return NULL;
    }

    int numberSections = iniparser_getnsec(ini);
    int numberCPUs = 0;
    int numberCaches = 0;
    int numberMemories = 0;

    /* Check that all the configuration file sections are correct.
     * No missing sections. No unknown sections. */
    for (int i = 0; i < numberSections; i++) {
        const char* section = iniparser_getsecname(ini, i);

        // if the name of the section is cpu
        if (strcmp(section, "cpu") == 0) {
            // count cpu sections. There can be only one cpu section
            numberCPUs++;
        // If the name of the section is memory
        } else if (strcmp(section, "memory") == 0) {
            // Count memory sections. There can be only one memory section
            numberMemories++;
        // If the name of the section is like "cache..."
        } else if (strncmp(section, "cache", 5) == 0) {
            int correctNum = 1;
            // Get the length of the section name
            int len = strlen(section);
            // Error if there is no characters following 'cache'
            if (len <= 5) {
                fprintf(stderr,"Error: Invalid [cache] section. It must contain the cache level number. [cacheN]\n");
                correctNum=0;
                errors++;
            }

            // Check that the string following 'cache' is a number 
            const char* cacheNumberStr = section+5;
            for (int j=0; cacheNumberStr[j] && correctNum; j++) {
                if (!isdigit(cacheNumberStr[j])) {
                    correctNum = 0;
                    fprintf(stderr,"Error: Invalid cache section name [%s]\n", section);
                    errors++;
                }
            }

            // Parse the contents of the section if the number is correct
            if (correctNum) {
                int cacheNumber = atoi(cacheNumberStr);
                // Remember the highest cache level
                if (cacheNumber > numberCaches) {
                    numberCaches = cacheNumber;
                }
                checkSectionKeys(ini, section, CACHE_KEYS, (char**) keysCache, &errors);
            }
        // If the section name isn't "cpu" or "memory" and section name isn't like "cache..." then error.
        } else {
            fprintf(stderr,"Error: Unknown section name [%s]\n", section);
            errors++;
        }
    }

    // Check the mandatory [cpu] section
    if (numberCPUs == 0) {
        fprintf(stderr,"Error: Missing mandatory section [cpu]\n");
        errors++;
    // Look for unknown keys in [cpu] section
    } else {
        checkSectionKeys(ini, "cpu", CPU_KEYS, (char**) keysCpu, &errors);
    }

    // Check the mandatory [memory] section
    if (numberMemories == 0) {
        fprintf(stderr,"Error: Missing mandatory section [memory]\n");
        errors++;
    // Look for unknown keys in [memory] section
    } else {
        checkSectionKeys(ini, "memory", MEMORY_KEYS, (char**) keysMemory, &errors);
    }

    // Check that the number of cache levels is within range
    if (numberCaches > MAX_CACHE_LEVELS) {
        fprintf(stderr,"Error: The number of caches is excesive.\n");
        errors++;
    }

    // End with error message if there has been any errors
    if (errors > 0) {
        fprintf(stderr,"\nTotal errors: %d\n", errors);
        return NULL;
    }

    *cacheLevels = numberCaches;
    return ini;
}

/**
 * Parses all the configuration in the provided ini file and returns the config the sim should have.
 * @param iniName Path to the ini config file.
 * @param sc Pointer to a SimulatorConfig struct.
 * @return 0 if Ok, -1 if warnings, -2 if fatal errors
 */
int parseConfiguration(char* iniName, SimulatorConfig* sc) {
    int errors = 0;
    int cacheLevels = 0;

    // Read configuration file
    dictionary *ini;

    if((ini = readConfigurationFile(iniName, &sc->miscCacheLevels)) == NULL) {
       return -2;
    }

    // CPU config
    parseConfInt(ini,"cpu:address_width", &sc->cpuAddressWidth, &errors);
    parseConfInt(ini,"cpu:word_width", &sc->cpuWordWidth, &errors);
    parseConfInt(ini,"cpu:rand_seed", &sc->cpuRandSeed, &errors);

    // Check the address and word widths are powers of two
    if (!isPowerOf2(sc->cpuAddressWidth)) {
         fprintf(stderr,"Error: cpu:address_width must be power of 2\n");
         errors++;
    }
    if (!isPowerOf2(sc->cpuWordWidth)) {
         fprintf(stderr,"Error: cpu:word_width must be power of 2\n");
         errors++;
    }

    // Memory config
    parseConfLong(ini, "memory:size", &sc->memSize, &errors, true);
    parseConfDouble(ini, "memory:access_time_1", &sc->memAccessTimeSingle, &errors);
    parseConfDouble(ini, "memory:access_time_burst", &sc->memAccessTimeBurst,  &errors);
    parseConfLong(ini, "memory:page_size", &sc->memPageSize, &errors, true);
    parseConfAddress(ini, "memory:page_base_address", &sc->memPageBaseAddress, &errors);
    long maxMemory = pow(2, sc->cpuAddressWidth);

    if (sc->memSize > maxMemory) {
	    fprintf(stderr,"Warning: memory:size is too big for a %d bits machine.\n", sc->cpuAddressWidth);
        errors++;
    }
    if (sc->memSize % sc->memPageSize != 0) {
	    fprintf(stderr,"Warning: memory:size must be a multiple of memory:page_size\n");
        errors++;
    }
    if (!isPowerOf2(sc->memPageSize)) {
	    fprintf(stderr,"Warning: memory:page_size must be power of 2\n");
        errors++;
    }
    if (sc->memPageBaseAddress % sc->memPageSize != 0) {
	    fprintf(stderr,"Warning: memory:page_base_address is invalid\n");
        errors++;
    }
    if (sc->memPageBaseAddress < 0 || sc->memPageBaseAddress > maxMemory - 1) {
	    fprintf(stderr,"Warning: memory:page_base_address is out of range.\n");
        errors++;
    }

    // Multilevel cache configs
    // Browse the cache array and check the configuration of each cache.
    for (int cacheNumber = 0; cacheNumber < sc->miscCacheLevels; cacheNumber++) {
        // cache:line_size
        char param[50];
        sprintf(param, "cache%d:line_size", cacheNumber + 1);
        parseConfLong(ini, param, &sc->cacheLineSize[cacheNumber],&errors,true);
        if (!isPowerOf2(sc->cacheLineSize[cacheNumber])) {
	        fprintf(stderr,"Warning: cache%d:line_size must be power of 2\n", cacheNumber + 1);
            errors++;
	    }

        // cache:size
        sprintf(param, "cache%d:size", cacheNumber + 1);
        parseConfLong(ini, param, &sc->cacheSize[cacheNumber],&errors, true);
        if ((sc->cacheSize[cacheNumber]) % (sc->cacheLineSize[cacheNumber]) != 0) {
	        fprintf(stderr,"Warning: cache%d:size must be a multiple of cache%d:line_size\n", cacheNumber + 1, cacheNumber + 1);
            errors++;
	    }

        // cache:separated
        sprintf(param, "cache%d:separated", cacheNumber + 1);
        const char* cacheSeparated = iniparser_getstring(ini, param, NULL);
        long long_separated = parseBoolean(cacheSeparated);
        if (long_separated == -1) {
            fprintf(stderr,"Warning: cache%d:separated value is not valid\n", cacheNumber + 1);
            errors++;
        } else if (long_separated == -2) {
            fprintf(stderr,"Warning: Missing value cache%d:separated\n", cacheNumber + 1);
            errors++;
        } else {
            sc->cacheIsSplit[cacheNumber] = long_separated;
        }

        // cache:asocitivity
        sprintf(param, "cache%d:associativity", cacheNumber + 1);
        //this is the number of lines. For error check
        int num_lines=sc->cacheSize[cacheNumber] / sc->cacheLineSize[cacheNumber];

        if (sc->cacheIsSplit[cacheNumber]) {
             num_lines /= 2;
        }

        const char* cache_asociativity = iniparser_getstring(ini, param, NULL);
        // si es F es de compleatamente asociativa. Un solo set. Tantas lines/set como lines totales.
        if (cache_asociativity != NULL&&strcmp(cache_asociativity, "F") == 0) {
            sc->cacheAssoc[cacheNumber] = sc->cacheSize[cacheNumber] / sc->cacheLineSize[cacheNumber];
        } else {
            long long_asociativity = parseInt(cache_asociativity);
            if (long_asociativity == -1) {
                fprintf(stderr,"Warning: cache%d:associativity value is not valid\n", cacheNumber + 1);
                errors++;
            } else if (long_asociativity == -2) {
                fprintf(stderr,"Warning: Missing value cache%d:associativity\n", cacheNumber + 1);
                errors++;
            } else if (!isPowerOf2(long_asociativity)) {
                fprintf(stderr,"Warning: The value of cache%d:associativity must be power of 2\n", cacheNumber + 1);
                errors++;
            } else if (long_asociativity>num_lines) {
                fprintf(stderr,"Warning: The value of cache%d:associativity can't be bigger than the number of lines\n", cacheNumber + 1);
                errors++;
            }else {
                sc->cacheAssoc[cacheNumber] = long_asociativity;
            }
        }

        // cache:write_policy
        sprintf(param, "cache%d:write_policy", cacheNumber + 1);
        const char* cache_write_policy = iniparser_getstring(ini, param, NULL);
        long long_write_policy = parseWritePolicy(cache_write_policy);
        if (long_write_policy == -1) {
            fprintf(stderr,"Warning: cache%d:write_policy value is not valid\n", cacheNumber + 1);
            errors++;
        } else if (long_write_policy==-2) {
            fprintf(stderr,"Warning: Missing value cache%d:write_policy\n", cacheNumber + 1);
            errors++;
        } else {
            sc->cachePolicyWrite[cacheNumber] = (PolicyWrite) long_write_policy;
        }

        // reading key cache:replacement_policy
        sprintf(param, "cache%d:replacement_policy", cacheNumber + 1);
        const char* cache_replacement = iniparser_getstring(ini, param, NULL);
        long long_replacement = parseReplacementPolicy(cache_replacement);
        if (long_replacement == -1) {
            fprintf(stderr,"Warning: replacement_policy value for cache%d is not valid.\n", cacheNumber + 1);
            errors++;
        } else if (long_replacement == -2) {
            fprintf(stderr,"Warning: Missing replacement_policy value for cache%d.\n", cacheNumber + 1);
            errors++;
        } else {
            sc->cachePolicyReplacement[cacheNumber] = (PolicyReplacement) long_replacement;
        }

        // reading key cache:access_time
        sprintf(param, "cache%d:access_time", cacheNumber + 1);
        parseConfDouble(ini, param, &sc->cacheAccessTime[cacheNumber], &errors);
    }

    // ALL caches MUST have the same line size
    if (sc->miscCacheLevels > 0) {
       int64_t previous = sc->cacheLineSize[0];
       for(int cacheNumber = 1; cacheNumber < sc->miscCacheLevels; cacheNumber++) {
          if (sc->cacheLineSize[cacheNumber] != previous) {
             fprintf(stderr,"Warning: All the caches must have the same line_size.\n");
             errors++;
             break;
          }
       }
    }

    if (errors > 0) {
        fprintf(stderr,"\nTotal warnings: %d\n", errors);
        return -1;
    }

    return 0;
}

