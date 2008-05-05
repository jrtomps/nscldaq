#include <sys/types.h>
#include <regex.h>
#include <string>

using namespace std;

// True if pattern regexp has at least one match for str.

bool 
grep(string str, string pattern)
{
  regex_t CompiledPattern;
  regmatch_t MatchInfo;	// Only want first match.

  regcomp(&CompiledPattern, pattern.c_str(), 0);

  int match = regexec(&CompiledPattern, str.c_str(),
		    1, &MatchInfo, 0);
  regfree(&CompiledPattern);
  return (match == 0);
}
