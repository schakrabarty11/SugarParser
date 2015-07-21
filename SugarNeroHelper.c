
//
//  SugarNeroHelper.c
//  SugarNeroC
//
//  Created by Yuk Lai Suen on 4/27/15.
//  Copyright (c) 2015 Yuk Lai. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "SugarNeroHelper.h"
#include "PointerStack.h"
#include "PointerStack.c"

SUGAR_BUILD_FLAVOR lastFlavor = SUGAR_BUILD_FLAVOR_NONE;
Stack cStack;               //Used to store all nested conditions.

PointerStack *cPointers;    //Used to store all memory allocations.




SUGAR_BUILD_RESULT copyFileContentToBuffer(const char *filePath, char **outputBuffer, size_t *outputSize) {
    char *buffer;
    size_t size;
    
    FILE *file = fopen(filePath, "r");
    if (!file) {
        return SUGAR_BUILD_RESULT_FAIL;
    }
    
    // get file size;
    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    
    
    buffer = psmalloc(cPointers, size + 1);
    //buffer = emalloc(size + 1);
    
    fread(buffer, size, 1, file);
    buffer[size] = 0;
    
    fclose(file);
    
    *outputBuffer = buffer;
    *outputSize = size;
    
    
    return SUGAR_BUILD_RESULT_PASS;
}

size_t lineCountInString(const char *buffer) {
    size_t bufferLen = strlen(buffer);
    size_t endOfLineCount = 0;
    for (size_t i = 0; i < bufferLen; i++) {
        if (buffer[i] == '\n') {
            endOfLineCount++;
        }
    }
    return endOfLineCount + 1;
}

// convert a string to an array. Each element of the array is a line in the string.
// E.g. "Hello\nWorld" would have two elements, "Hello" and "World"
// But "Hello\n\nWorld" would have three elements, "Hello", "", and "World"
SUGAR_BUILD_RESULT stringToArrayOfLines(const char *buffer, char ***arrayOutput, size_t *countOutput) {
    const char *start = buffer;
    const char *end = buffer;
    size_t lineLength;
    char *currentLine = 0;
    size_t lineIndex = 0;
    char **array = 0;
    size_t arrayCount;
    
    if (!arrayOutput || !countOutput) { return SUGAR_BUILD_RESULT_FAIL; }
    
    arrayCount = lineCountInString(buffer);
    
    array = psmalloc(cPointers, sizeof(char*)*arrayCount);
    //array = emalloc(sizeof(char*) * arrayCount);

    
    while (1) {
        while (*end != '\n' && *end) { end++; };
        lineLength = end - start;
        
        currentLine = psmalloc(cPointers, lineLength + 1);
        //currentLine = emalloc(lineLength + 1);
        
        currentLine = strncpy(currentLine, start, lineLength);
        currentLine[lineLength] = 0;

        array[lineIndex] = currentLine;
        lineIndex++;
        if (!(*end)) { break; }
        end++;
        start = end;
    }
    
    for (; lineIndex < arrayCount; lineIndex++) {
        array[lineIndex] = "";
    }
    
    *arrayOutput = array;
    *countOutput = arrayCount;
    return SUGAR_BUILD_RESULT_PASS;
}

char *arrayOfLinesToString(char **array, size_t count) {
    size_t totalLength = 0;
    
    char *result = 0;
    for (size_t i = 0; i < count; i++) {
        totalLength += (strlen(array[i]) + 1); // plus a '\n' character
    }
    
    result = psmalloc(cPointers, (totalLength+1)*sizeof(char));
    //result = emalloc((totalLength + 1) * sizeof(char));
    
    result[0] = 0;      //Need to initialize a value before using strcat;
    for (size_t i = 0; i < count; i++) {
        if(i==0){
            if(sizeof(array[i])-5>0){
                char substr[strlen(array[i])-4];
                strncpy(substr, array[i]+5, strlen(array[i])-5);
                substr[strlen(array[i])-5] = '\0';
                strcat(result, substr);
                if (i < count -1) {
                    strcat(result, "\n");
                }
            }
        }else{
            strcat(result, array[i]);
            if (i < count -1) {
                strcat(result, "\n");
            }
        }
    }
    
  
    result[totalLength] = 0;
    
    return result;
}

//This function is purely for debugging purposes
void print_out(Conditions condition){
    for(int i = 0; i< condition.conditionCount; i++){
        MatchCondition c = condition.conditions[i];
        php_printf("Logical Operator: %s, Key: %s, Operator: %s, Value: %s<br>", c.logicalOperator, c.key, c.operator, c.value);
    }
}


char *processFile(char *buffer, SUGAR_BUILD_FLAVOR mainBuildFlavor) {
    // get number of lines in buffer
    //Initialize the condition stack
    cStack.size = 0;
    for(int i = 0; i < 10; i++){
        cStack.classifiers[i].conditionCount = 0;
    }

    SUGAR_BUILD_RESULT result = SUGAR_BUILD_RESULT_PASS;
    
    char **arrayOfLines = 0;
    size_t lineCount = 0;
    
    // Don't think there are more than 10 conditions
    MatchCondition matchedConditions[10] = {0};
    size_t matchedConditionCount;
    SUGAR_BUILD_MARK currentBuildMark = SUGAR_BUILD_MARK_NONE;
    SUGAR_BUILD_FLAVOR currentBuildFlavor = mainBuildFlavor;
    
    
    if (SUGAR_BUILD_RESULT_FAIL == stringToArrayOfLines(buffer, &arrayOfLines, &lineCount)) {
        return NULL;
    }
    
    for (size_t i = 0; i < lineCount; i++) {
        const char *line = arrayOfLines[i];
        SUGAR_BUILD_MARK buildMark;
        SUGAR_BUILD_FLAVOR buildFlavor;
        result = getSugarBuildMark(line, &buildMark, matchedConditions, ARRAY_SIZE(matchedConditions), &matchedConditionCount);
        if (SUGAR_BUILD_RESULT_PASS != result) {
            return NULL;
        }
        
        if (SUGAR_BUILD_MARK_NONE == buildMark) {
            if(cStack.size > 0){
                if(!matchesConditions(cStack.classifiers[cStack.size-1].conditions, cStack.classifiers[cStack.size-1].conditionCount, currentBuildFlavor)){
                    arrayOfLines[i] = commentOutLine(line);
                }
            }
        } else {
            currentBuildMark = buildMark;
        }
    }
    
    
    /*php_printf("<br>----DEBUGGING----<br>");
    for(int i = 0; i<lineCount; i++){
        php_printf("%s<br>", arrayOfLines[i]);
    }*/

    /*for(size_t i=0; i<lineCount; i++){
        PointerStack_append(cPointers, arrayOfLines[i]);
        efree(arrayOfLines[i]);
    }
    efree(arrayOfLines);*/

    char* to_return = arrayOfLinesToString(arrayOfLines, lineCount);
    
    return to_return;
}

char *commentOutLine(const char *line) {
    size_t length = (strlen(line) + 4);
    char* newLine = psmalloc(cPointers,length * sizeof(char));
    //char *newLine = emalloc(length * sizeof(char));
    
    snprintf(newLine, length, "// %s", line);
    
    return newLine;
}

SUGAR_BUILD_RESULT getSugarBuildMark(const char *line, SUGAR_BUILD_MARK *buildMark, MatchCondition *matchedConditions, size_t maxConditionCount, size_t *conditionCount) {
    regex_t regex;
    int success = SUGAR_BUILD_MARK_NONE;
    SUGAR_BUILD_RESULT result = SUGAR_BUILD_RESULT_PASS;
    const int maxMatches = 3;
    regmatch_t matches[maxMatches];
    char *optionString = 0;
    
    if (!buildMark) {
        return SUGAR_BUILD_RESULT_FAIL;
    }
    
    *buildMark = SUGAR_BUILD_MARK_NONE;
    
    // success = regcomp(&regex, "/\\/\\/\\s*(BEGIN|END|FILE|ELSE)\\s*SUGARCRM\\s*(.*) ONLY", REG_ICASE | REG_EXTENDED);
    //success = regcomp(&regex, "\\/\\/[[:space:]]*(BEGIN|END|FILE|ELSE)[[:space:]+]SUGARCRM[[:space:]+](.*)[[:space:]+]ONLY", REG_ICASE | REG_EXTENDED);
    success = regcomp(&regex, "\\/\\/[[:space:]]*(BEGIN|END)[[:space:]+]SUGARCRM[[:space:]+](.*)[[:space:]+]ONLY", REG_ICASE | REG_EXTENDED);
    if (success) {
        php_printf("=== Failed!! ===\n");
        return SUGAR_BUILD_RESULT_FAIL;
    }
    
    success = regexec(&regex, line, maxMatches, matches, 0);
    if (success) {
        *buildMark = SUGAR_BUILD_MARK_NONE;
    } else {
        if (matches[1].rm_so == -1 || matches[1].rm_eo == -1) {
            // this should not happen
            *buildMark = SUGAR_BUILD_MARK_NONE;
        } else {
            
            if (0 == strncmp(SUGAR_BUILD_MARK_BEGIN_STR, line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so)) {
                *buildMark = SUGAR_BUILD_MARK_BEGIN;
            } else if (0 == strncmp(SUGAR_BUILD_MARK_END_STR, line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so)) {
                *buildMark = SUGAR_BUILD_MARK_END;
            } else if (0 == strncmp(SUGAR_BUILD_MARK_FILE_STR, line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so)) {
                *buildMark = SUGAR_BUILD_MARK_FILE;
            }
            /*            } else if (0 == strncmp(SUGAR_BUILD_MARK_ELSE_STR, line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so)) {
                            *buildMark = SUGAR_BUILD_MARK_ELSE;
                        }*/
            else {
                result = SUGAR_BUILD_RESULT_FAIL;
            }
            
            if (matches[2].rm_so == -1 || matches[2].rm_eo == -1 || matches[2].rm_eo < matches[2].rm_so) {
                // this should not happen
                *buildMark = SUGAR_BUILD_MARK_NONE;
            } else {
                
                size_t length = matches[2].rm_eo - matches[2].rm_so;
                optionString = psmalloc(cPointers, (length + 1) * sizeof(char));
                //optionString = emalloc((length + 1) * sizeof(char));
                
                strncpy(optionString, line + matches[2].rm_so,length);
                
                parseSugarBuildOptions(optionString, matchedConditions, maxConditionCount, conditionCount, buildMark);
                
                //efree(optionString);
            }
            
        }
    }
    
    regfree(&regex);
    return result;
}

void safeStringCopy(char *dest, size_t destSize, const char *source, size_t numOfBytesToCopy) {
    memset(&dest[0], 0, sizeof(dest));
    if (numOfBytesToCopy < destSize) {
        strncpy(dest, source, numOfBytesToCopy);
    }
}

SUGAR_BUILD_RESULT parseSugarBuildOptions(const char *options, MatchCondition matchedConditions[], size_t maxConditionCount, size_t *conditionCount, SUGAR_BUILD_MARK *buildMark) {
    
    regex_t regex;
    int success = SUGAR_BUILD_MARK_NONE;
    int result = 0;
    const int maxMatches = 5;
    regmatch_t matches[maxMatches] = {0};
    const char *currentOptions = options; //pointer to the current processing option in the loop
    
    if (!conditionCount) {
        return SUGAR_BUILD_RESULT_FAIL;
    }
    
    *conditionCount = 0;
    
    success = regcomp(&regex, "[[:space:]]*(&&|\\|\\|)?[[:space:]]*(flav|dep|lic)(=|!=)(een|ent|pro|dev|ult|corp|os|od|com|sub|int)", REG_ICASE | REG_EXTENDED);
    if (success) {
        php_printf("=== Failed!! ===<br>");
        return SUGAR_BUILD_RESULT_FAIL;
    }
    
    // regex doesn't support global search, so we need to repeat it as many times as we can find more matches
    size_t indexToEndOfLastMatch = 0;
    size_t matchedConditionCount = 0;
    
    while (matches[0].rm_so != -1 && matchedConditionCount < maxConditionCount) {
        currentOptions += indexToEndOfLastMatch;
        success = regexec(&regex, currentOptions, maxMatches, matches, 0);
        indexToEndOfLastMatch = matches[0].rm_eo;
        
        if (success) {
            if(*buildMark!=SUGAR_BUILD_MARK_END){
                cStack.size++;
            }else if(*buildMark==SUGAR_BUILD_MARK_END){
                if(cStack.size!=0){
                    cStack.classifiers[cStack.size-1].conditionCount=0;
                    cStack.size--;
                }
            }
            result = SUGAR_BUILD_RESULT_FAIL;
            break;
        } else {
            
            if (matches[1].rm_so != -1) {
                safeStringCopy(matchedConditions[matchedConditionCount].logicalOperator, MAX_CONDITION_PROPERTY_LEN, currentOptions + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
                //php_printf("%lu: Logical operator: %s<br>", matchedConditionCount, matchedConditions[matchedConditionCount].logicalOperator);
            }
            
            if (matches[2].rm_so != -1) {
                safeStringCopy(matchedConditions[matchedConditionCount].key, MAX_CONDITION_PROPERTY_LEN, currentOptions + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
                //php_printf("%lu: Key             : %s<br>", matchedConditionCount, matchedConditions[matchedConditionCount].key);
            }
            
            if (matches[3].rm_so != -1) {
                safeStringCopy(matchedConditions[matchedConditionCount].operator, MAX_CONDITION_PROPERTY_LEN, currentOptions + matches[3].rm_so, matches[3].rm_eo - matches[3].rm_so);
                //php_printf("%lu: Operator        : %s<br>", matchedConditionCount, matchedConditions[matchedConditionCount].operator);
            }
            
            if (matches[4].rm_so != -1) {
                safeStringCopy(matchedConditions[matchedConditionCount].value, MAX_CONDITION_PROPERTY_LEN, currentOptions + matches[4].rm_so, matches[4].rm_eo - matches[4].rm_so);
                if(*buildMark == SUGAR_BUILD_MARK_BEGIN){
                    cStack.classifiers[cStack.size].conditions[cStack.classifiers[cStack.size].conditionCount] = matchedConditions[matchedConditionCount];
                    cStack.classifiers[cStack.size].conditionCount++;
                }
                
                //php_printf("%lu: Operator        : %s<br>", matchedConditionCount, matchedConditions[matchedConditionCount].value);
                //php_printf("Size: %i<br>", cStack.size);

            }
            matchedConditionCount++;
        }
        
        
    }
    
    
    
    *conditionCount = matchedConditionCount;
    regfree(&regex);
    return SUGAR_BUILD_RESULT_PASS;
    
}

SUGAR_BUILD_FLAVOR getFlavor(const char *flavor) {
    if (0 == strcmp(flavor, "pro")) {
        return SUGAR_BUILD_FLAVOR_PROFESSIONAL;
    } else if (0 == strcmp(flavor, "ent")) {
        return SUGAR_BUILD_FLAVOR_ENTERPRISE;
    } else if (0 == strcmp(flavor, "ult")) {
        return SUGAR_BUILD_FLAVOR_ULTIMATE;
    } else if (0 == strcmp(flavor, "com")) {
        return SUGAR_BUILD_FLAVOR_COMMUNITY;
    } else if (0 == strcmp(flavor, "corp")) {
        return SUGAR_BUILD_FLAVOR_CORPORATE;
    } else if (0 == strcmp(flavor, "dev")) {
        return SUGAR_BUILD_FLAVOR_DEV;
    } else if (0 == strcmp(flavor, "int")) {
        return SUGAR_BUILD_FLAVOR_INTERNAL;
    }
    else {
        return SUGAR_BUILD_FLAVOR_NONE;
    }
}

int isEqualCondition(MatchCondition condition) {
    return 0 == strcmp("=", condition.operator); // otherwise assume it's "!="
}

SUGAR_BUILD_LOGICAL_OPERATOR logicalOperator(const char *condition) {
    if (condition == 0 || 0 == strcmp("", condition)) {
        return SUGAR_BUILD_LOGICAL_OPERATOR_NONE;
    } else if (0 == strcmp("&&", condition)) {
        return SUGAR_BUILD_LOGICAL_OPERATOR_AND;
    } else if (0 == strcmp("||", condition)) {
        return SUGAR_BUILD_LOGICAL_OPERATOR_OR;
    } else {
        return SUGAR_BUILD_LOGICAL_OPERATOR_UNDEFINED;
    }
}

int matchesCondition(MatchCondition condition, SUGAR_BUILD_FLAVOR currentBuildFlavor) {
    SUGAR_BUILD_FLAVOR conditionFlavor;
    int to_return = 1;
    if (0 == strcmp("flav", condition.key)) {
        conditionFlavor = getFlavor(condition.value);
        

        if (isEqualCondition(condition)) {
            to_return = currentBuildFlavor >= conditionFlavor;
        } else {
            to_return = currentBuildFlavor != conditionFlavor;
        }
    }
       return to_return;
}

int matchesConditions(MatchCondition conditions[], size_t conditionsCount, SUGAR_BUILD_FLAVOR currentBuildFlavor) {
    int result = 0;
    int currentResult = 0;
    for (size_t i = 0; i < conditionsCount; i++) {
        currentResult = matchesCondition(conditions[i], currentBuildFlavor);
        if (i == 0) {
            result = currentResult;
        } else {
            if (SUGAR_BUILD_LOGICAL_OPERATOR_AND == logicalOperator(conditions[i].logicalOperator)) {
                result = result && currentResult;
            } else if (SUGAR_BUILD_LOGICAL_OPERATOR_OR == logicalOperator(conditions[i].logicalOperator)) {
                result = result || currentResult;
            }
        }
    }

    return result;
}

//This is the function that gets called from Nero to start off the parsing.
char* testFunction(const char* filePath, const char* buildFlavor, PointerStack* pStack){
    
    cPointers = pStack;
    
    char *buffer;
    size_t size;
    copyFileContentToBuffer(filePath, &buffer, &size);
    
    char* to_return = processFile(buffer, getFlavor(buildFlavor));
    //efree(buffer);
    //php_printf("Size: %i<br>",cPointers->size);
    return to_return;
    
}
