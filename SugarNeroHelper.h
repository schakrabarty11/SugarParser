//
//  SugarNeroHelper.h
//  SugarNeroC
//
//  Created by Yuk Lai Suen on 4/27/15.
//  Copyright (c) 2015 Yuk Lai. All rights reserved.
//

#ifndef SugarNeroC_SugarNeroHelper_h
#define SugarNeroC_SugarNeroHelper_h

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

void testGetSugarBuildMark();
void testParseSugarBuildOptions();
void testLineCountInString();
void testStringToArrayOfLines();
void testArrayOfLinesToString();
void testCommentOutLine();
void testMatchesCondition();
void testMatchesConditions();

static const char *SUGAR_BUILD_MARK_BEGIN_STR = "BEGIN";
static const char *SUGAR_BUILD_MARK_END_STR = "END";
static const char *SUGAR_BUILD_MARK_FILE_STR = "FILE";
// static const char *SUGAR_BUILD_MARK_ELSE_STR = "ELSE";
typedef enum {
    SUGAR_BUILD_FLAVOR_NONE,
    SUGAR_BUILD_FLAVOR_PROFESSIONAL,
    SUGAR_BUILD_FLAVOR_ENTERPRISE,
    SUGAR_BUILD_FLAVOR_ULTIMATE,
    SUGAR_BUILD_FLAVOR_COMMUNITY,
    SUGAR_BUILD_FLAVOR_DEV,
    SUGAR_BUILD_FLAVOR_CORPORATE,
    SUGAR_BUILD_FLAVOR_INTERNAL,


} SUGAR_BUILD_FLAVOR;

typedef enum {
    SUGAR_BUILD_MARK_NONE,
    SUGAR_BUILD_MARK_BEGIN,
    SUGAR_BUILD_MARK_END,
    SUGAR_BUILD_MARK_FILE,
    //    SUGAR_BUILD_MARK_ELSE // this case does not seem to exist in the code
} SUGAR_BUILD_MARK;

typedef enum {
    SUGAR_BUILD_LOGICAL_OPERATOR_NONE,
    SUGAR_BUILD_LOGICAL_OPERATOR_OR,
    SUGAR_BUILD_LOGICAL_OPERATOR_AND,
    SUGAR_BUILD_LOGICAL_OPERATOR_UNDEFINED
} SUGAR_BUILD_LOGICAL_OPERATOR;

typedef enum {
    SUGAR_BUILD_RESULT_PASS,
    SUGAR_BUILD_RESULT_FAIL
    
} SUGAR_BUILD_RESULT;

#ifndef MAX_CONDITION_PROPERTY_LEN
#define MAX_CONDITION_PROPERTY_LEN 10
#endif


typedef struct {
    char logicalOperator[MAX_CONDITION_PROPERTY_LEN];   // && or || or nothing
    char key[MAX_CONDITION_PROPERTY_LEN];               // flav or dep
    char operator[MAX_CONDITION_PROPERTY_LEN];          // = or !=
    char value[MAX_CONDITION_PROPERTY_LEN];             // value for flav or dep
} MatchCondition;

typedef struct{
    MatchCondition conditions[10]; //There shouldn't be more than 10 concurrent classifiers.
    size_t conditionCount;
}Conditions;

typedef struct{
    Conditions classifiers[10]; //There shouldn't be more than 10 nested classifiers.
    size_t size;
}Stack;


SUGAR_BUILD_RESULT copyFileContentToBuffer(const char *filePath, char **outputBuffer, size_t *outputSize);
char *processFile(char *buffer, SUGAR_BUILD_FLAVOR buildFlavor);
SUGAR_BUILD_RESULT getSugarBuildMark(const char *line, SUGAR_BUILD_MARK *buildMark, MatchCondition *matchedConditions, size_t maxConditionCount, size_t *conditionCount);
SUGAR_BUILD_RESULT parseSugarBuildOptions(const char *options, MatchCondition matchedConditions[], size_t maxConditionCount, size_t *conditionCount, SUGAR_BUILD_MARK *buildMark);
SUGAR_BUILD_RESULT stringToArrayOfLines(const char *buffer, char ***arrayOutput, size_t *countOutput);
size_t lineCountInString(const char *buffer);
char *arrayOfLinesToString(char **array, size_t count);
char *commentOutLine(const char *line);
int matchesCondition(MatchCondition condition, SUGAR_BUILD_FLAVOR currentBuildFlavor);
int matchesConditions(MatchCondition conditions[], size_t conditionsCount, SUGAR_BUILD_FLAVOR currentBuildFlavor);


#endif

