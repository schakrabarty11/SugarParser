
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

SUGAR_BUILD_FLAVOR lastFlavor = SUGAR_BUILD_FLAVOR_NONE;



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
    
    buffer = malloc(size + 1);
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
    array = malloc(sizeof(char*) * arrayCount);
    
    while (1) {
        while (*end != '\n' && *end) { end++; };
        lineLength = end - start;
        
        currentLine = malloc(lineLength + 1);
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
    for (size_t i = 1; i < count; i++) {
        totalLength += (strlen(array[i]) + 1); // plus a '\n' character
    }
    
    result = malloc((totalLength + 1) * sizeof(char));
    result[0] = 0;
    strcat(result, " ");
    for (size_t i = 1; i < count; i++) {
        strcat(result, array[i]);
        if (i < count -1) {
            strcat(result, "\n");
        }
    }
    
  
    result[totalLength] = 0;
    
    return result;
}

char *processFile(char *buffer, SUGAR_BUILD_FLAVOR mainBuildFlavor) {
    // get number of lines in buffer

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
            
            if(currentBuildMark == SUGAR_BUILD_MARK_BEGIN){
                if(lastFlavor > mainBuildFlavor){
                    arrayOfLines[i] = commentOutLine(line);
                }
            }else if (currentBuildMark != SUGAR_BUILD_MARK_NONE) {
                //arrayOfLines[i] = commentOutLine(line);
            }
            
            
        } else {
            currentBuildMark = buildMark;
            if (SUGAR_BUILD_MARK_FILE == buildMark) {
                currentBuildMark = SUGAR_BUILD_MARK_FILE;
            }
        }
    }
    /*php_printf("<br>|||||<br>");
    for(int i = 0; i<lineCount; i++){
        php_printf("%s<br>", arrayOfLines[i]);
    }
    
    php_printf("<br>|||||<br>");
    php_printf("%s<br>", to_return1);*/
    char* to_return = arrayOfLinesToString(arrayOfLines, lineCount);
    return to_return;
}

char *commentOutLine(const char *line) {
    size_t length = (strlen(line) + 4);
    char *newLine = malloc(length * sizeof(char));
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
    success = regcomp(&regex, "\\/\\/[[:space:]*](BEGIN|END|FILE|ELSE)[[:space:]+]SUGARCRM[[:space:]+](.*)[[:space:]+]ONLY", REG_ICASE | REG_EXTENDED);
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
            //            } else if (0 == strncmp(SUGAR_BUILD_MARK_ELSE_STR, line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so)) {
            //                *buildMark = SUGAR_BUILD_MARK_ELSE;
            //            }
            else {
                result = SUGAR_BUILD_RESULT_FAIL;
            }
            
            if (matches[2].rm_so == -1 || matches[2].rm_eo == -1 || matches[2].rm_eo < matches[2].rm_so) {
                // this should not happen
                *buildMark = SUGAR_BUILD_MARK_NONE;
            } else {
                
                size_t length = matches[2].rm_eo - matches[2].rm_so;
                optionString = malloc((length + 1) * sizeof(char));
                strncpy(optionString, line + matches[2].rm_so,length);
                
                parseSugarBuildOptions(optionString, matchedConditions, maxConditionCount, conditionCount);
                
                free(optionString);
            }
            
        }
    }
    
    regfree(&regex);
    return result;
}

void safeStringCopy(char *dest, size_t destSize, const char *source, size_t numOfBytesToCopy) {
    if (numOfBytesToCopy < destSize) {
        strncpy(dest, source, numOfBytesToCopy);
    }
}

SUGAR_BUILD_RESULT parseSugarBuildOptions(const char *options, MatchCondition matchedConditions[], size_t maxConditionCount, size_t *conditionCount) {
    
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
    
    success = regcomp(&regex, "[[:space:]]*(&&|\\|\\|)?[[:space:]]*(flav|dep)(=|!=)(een|ent|pro|dev|ult|corp|os|od)", REG_ICASE | REG_EXTENDED);
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
            result = SUGAR_BUILD_RESULT_FAIL;
            break;
        } else {
            
            if (matches[1].rm_so != -1) {
                safeStringCopy(matchedConditions[matchedConditionCount].logicalOperator, MAX_CONDITION_PROPERTY_LEN, currentOptions + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
                /*php_printf("%lu: Logical operator: %s<br>", matchedConditionCount, matchedConditions[matchedConditionCount].logicalOperator);*/
            }
            
            if (matches[2].rm_so != -1) {
                safeStringCopy(matchedConditions[matchedConditionCount].key, MAX_CONDITION_PROPERTY_LEN, currentOptions + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
                /*php_printf("%lu: Key             : %s<br>", matchedConditionCount, matchedConditions[matchedConditionCount].key);*/
            }
            
            if (matches[3].rm_so != -1) {
                safeStringCopy(matchedConditions[matchedConditionCount].operator, MAX_CONDITION_PROPERTY_LEN, currentOptions + matches[3].rm_so, matches[3].rm_eo - matches[3].rm_so);
                /*php_printf("%lu: Operator        : %s<br>", matchedConditionCount, matchedConditions[matchedConditionCount].operator);*/
            }
            
            if (matches[4].rm_so != -1) {
                safeStringCopy(matchedConditions[matchedConditionCount].value, MAX_CONDITION_PROPERTY_LEN, currentOptions + matches[4].rm_so, matches[4].rm_eo - matches[4].rm_so);
                /*php_printf("%lu: Operator        : %s<br>", matchedConditionCount, matchedConditions[matchedConditionCount].value);*/
                const char* flavor = matchedConditions[matchedConditionCount].value;
                if (0 == strcmp(flavor, "pro")) {
                    lastFlavor =  SUGAR_BUILD_FLAVOR_PROFESSIONAL;
                } else if (0 == strcmp(flavor, "ent")) {
                    lastFlavor =  SUGAR_BUILD_FLAVOR_ENTERPRISE;
                } else if (0 == strcmp(flavor, "ult")) {
                    lastFlavor =  SUGAR_BUILD_FLAVOR_ULTIMATE;
                } else {
                    lastFlavor =  SUGAR_BUILD_FLAVOR_NONE;
                }
               
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
    } else {
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

    if (0 == strcmp("flav", condition.key)) {
        conditionFlavor = buildFlavor(condition.value);
        
        if (isEqualCondition(condition)) {
            return currentBuildFlavor >= conditionFlavor;
        } else {
            return currentBuildFlavor != conditionFlavor;
        }
    }
    
    return 1;
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

char* testFunction(const char* filePath, const char* buildFlavor){
    char *buffer;
    size_t size;
    copyFileContentToBuffer(filePath, &buffer, &size);
    return processFile(buffer, getFlavor(buildFlavor));
    
}
