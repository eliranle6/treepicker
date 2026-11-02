#ifndef PICKER_MARKUP_H__
#define PICKER_MARKUP_H__

#include <string>
#include <vector>

#include "picker/picker_options.h"

namespace picker {

// The markup of a caller's text (a status, a preview's line), one for all of them: plain text, until
// MARK, a label color's name and MARK again, after which it's in that color, until the next such
// mark (an empty color's: plain again). A mark without its color's end: the rest is text.
constexpr char MARK = '\x01';

// The parts of a text: each with its text, and its color ("": plain). Empty ones are dropped.
std::vector<Label> parse_segments(const std::string& text);

// The labels of a status (an update with an empty ID): its parts, in their colors (yellow, for plain
// text or what isn't a label color)
std::vector<Label> parse_status(const std::string& status);

// A preview's line in the column: all its text, highlighted across the column in the color it starts
// with ("" for none)
Label parse_preview_line(const std::string& line);

} // namespace picker

#endif
