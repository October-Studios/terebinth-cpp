#include <unordered_map>
#include <vector>


/**
 * @brief
 *
 */
class CharacterClassifier {
public:
  enum Type {
    WHITESPACE,
    LINE_BREAK,
    NEWLINE,
    LETTER,
    DIGIT,
    OPERATOR,
    STRING_QUOTE,
    SINGLE_LINE_COMMENT,
    MULTI_LINE_COMMENT_START,
    MULTI_LINE_COMMENT_END,
    UNKNOWN
  };
};