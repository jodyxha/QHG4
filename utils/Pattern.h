/*============================================================================
| Pattern
| 
|  Represents a pattern including wild cards with a match() function to 
|  match it to a string.
|
|  Used by FileList and ArrayShare
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __PATTERN_H__
#define __PATTERN_H__

#include <vector>
#include <map>
#include <string>
#include "types.h"

typedef std::vector<VEC_CHARS>   CHAR_LIST_LIST;
typedef std::vector<stringvec>   STRING_LIST_LIST;

const bool PAT_INCLUDE = true;
const bool PAT_EXCLUDE = false;

class Pattern {
public:
    Pattern();
    ~Pattern();
    void clear();
    void addPattern(const char *pFilePattern);

    bool  matchPattern(int iPat, const char *pString, stringvec &vMatches);
    uint numPatterns() { return (uint)m_vvPatternParts.size();};

protected:
    void  preparePattern(char *pPattern);
    bool  subMatch(unsigned int iPat, unsigned int iPart, const char *pString, stringvec &vMatches);
    STRING_LIST_LIST  m_vvPatternParts;
    CHAR_LIST_LIST    m_vvPatternModes;
};


#endif

