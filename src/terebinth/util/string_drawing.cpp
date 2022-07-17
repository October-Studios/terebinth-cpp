#include "util/string_drawing.h"
#include "util/string_array.h"

namespace str {

std::string GetBoxedString(const std::string& in, std::string box_name, bool show_line_nums,
    bool always_width_max, int max_width) {
  std::vector<std::string> lines;
  int width;

  if (max_width < 0) {
    max_width = GetTermWidth() - 4;
  }

  str::SplitByLine(lines, TabsToSpaces(in));

  int start_line_num = 1;

  while (!lines.empty() && lines[0].empty()) {
		lines.erase(lines.begin());
		start_line_num++;
	}
	
	while (!lines.empty() && lines.back().empty()) {
		lines.pop_back();
	}
	
	int name_width = GetWidth(box_name);
	
	if (show_line_nums) {
		int num_chars = GetWidth(std::to_string(lines.size() + start_line_num - 1));
		
		for (int i = 0; i < (int)lines.size(); i++) {
			lines[i] = Pad(std::to_string(i + start_line_num), num_chars + 5, ALIGNMENT_RIGHT, " ", "", "    ") + lines[i];
		}
	}
	
	if (always_width_max) {
		width = max_width;
	}	else {
		width = std::min(std::max(str::GetMaxWidth(lines), name_width + 4 - 2), max_width);
	}
	
	str::PadWidths(lines, width);
	
	for (int i = 0; i < (int)lines.size(); i++) {
		lines[i] = " ┃ " + lines[i] + " ┃ ";
	}
	
  std::string bottom;
  std::string empty_ln;
	
	for (int i = 0; i < width; i++) {
		bottom += "━";
		empty_ln += " ";
	}
	
	if (box_name.empty()) {
		lines.insert(lines.begin(), " ┃ " + bottom + " ┃ ");
		lines.insert(lines.begin(), " ┏━" + empty_ln + "━┓ ");
	}	else {
    std::string underline;
		for (int i = 0; i < name_width; i++) {
			underline += "─";
		}
		
		lines.insert(lines.begin(), " ┃" + Pad(underline,	width + 2, ALIGNMENT_CENTER, " ", "╰─", "─╯") + "┃ ");
		lines.insert(lines.begin(), " ┏" + Pad(box_name,	width + 2, ALIGNMENT_CENTER, "━", "┥ ", " ┝") + "┓ ");
		lines.insert(lines.begin(), "  " + Pad(underline,	width + 2, ALIGNMENT_CENTER, " ", "╭─", "─╮") + "  ");
	}
	
	
	lines.push_back(" ┃ " + empty_ln + " ┃ ");
	lines.push_back(" ┗━" + bottom + "━┛ ");

	return Join(lines);
}

void PutArrayInTreeNodeBox(std::vector<std::string>& data) {
	int width = data.empty() ? 0 : GetWidth(data[0]);
	
	for (int i = 0; i < (int)data.size(); i++) {
		data[i] = "│ " + data[i] + " │";
	}
	
	data.push_back("╰─" + Pad("", width, ALIGNMENT_LEFT, "─") + "─╯");
	data.insert(data.begin(), "╭─" + Pad("┴", width, ALIGNMENT_CENTER, "─") + "─╮");
}

std::string PutStringInTreeNodeBox(const std::string& in) {
  std::vector<std::string> ary;
	SplitByLine(ary, in);
	PutArrayInTreeNodeBox(ary);
	return Join(ary);
}

std::string MakeList(std::vector<std::string>& data) {
  std::vector<std::string> ary;
	
	for (int i = 0; i < int(data.size()); i++) {
    std::vector<std::string> elem_ary;
		str::SplitByLine(elem_ary, data[i]);
		int x_pos = str::GetGlyphPosOf(elem_ary[0], "┴");
		if (x_pos >= 0)	{
			ary.push_back("┠─" + str::Pad("", x_pos, str::ALIGNMENT_LEFT, "─") + "╮");
		}
		for (auto i : elem_ary) {
			ary.push_back("┃ "+i);
		}
	}
	
	PadWidths(ary);
	
	for (int i = 0; i < int(ary.size()); i++) {
		ary[i] = ary[i] + " ┃";
	}
	
	int width = str::GetMaxWidth(ary);
	
	ary.insert(ary.begin(), "┏━┴" + PadString("", width - 4, str::ALIGNMENT_LEFT, "━") + "┓");
	ary.push_back("┗" + PadString("", width - 2, str::ALIGNMENT_LEFT, "━") + "┛");
	
	return str::Join(ary);
}

std::string MakeRootUpBinaryTree(const std::string& root, const std::string& left_branch,
    const std::string& right_branch, const std::string& left_leaf, const std::string& right_leaf) {
  std::vector<std::string> left_ary;
	SplitByLine(left_ary, left_leaf);
	
  std::vector<std::string> right_ary;
	SplitByLine(right_ary, right_leaf);
	
	while (left_ary.size() < right_ary.size())
		left_ary.push_back("");
	while (right_ary.size() < left_ary.size())
		right_ary.push_back("");
	
  std::vector<std::string> root_ary;
	SplitByLine(root_ary, root);
	if (root_ary.empty())
		root_ary.push_back("");
	PadWidths(root_ary);
	
	int root_width = GetMaxWidth(root_ary);
	PadWidths(left_ary);
	PadWidths(right_ary);
	
	int left_pad_width = 0;
	
	if (!left_leaf.empty()) {
		int end_x_pos = GetGlyphPosOf(left_ary[0], "┴");
		int left_branch_width = GetWidth(left_branch);
		if (end_x_pos>0) {
			int left_margin = 0;
			if (left_margin > 0)	{
				for (int i = 0; i < (int)left_ary.size(); i++) {
					left_ary[i] = Pad("", left_margin) + left_ary[i];
				}
				end_x_pos += left_margin;
			}
			int middle_y_pos = root_ary.size() / 2;
			root_ary[middle_y_pos] = "┤" + Sub(root_ary[middle_y_pos], 1, -1);
			left_pad_width = GetMaxWidth(left_ary) - root_width / 2;
			if (end_x_pos + 1 > left_pad_width)
				left_pad_width = end_x_pos + 1;
			left_pad_width = std::max(left_pad_width, (int)ceil(left_branch_width/2.0)+end_x_pos);
			for (int i = 0; i < (int)root_ary.size(); i++) {
				if (i < middle_y_pos)
					root_ary[i] = PadString("", left_pad_width) + root_ary[i];
				else if (i == middle_y_pos)
					root_ary[i] = PadString("", end_x_pos) + "╭" + PadString("", left_pad_width - end_x_pos - 1, ALIGNMENT_LEFT, "─") + root_ary[i];
				else if (i == middle_y_pos + 1 && !left_branch.empty()) {
					root_ary[i] = PadString("", end_x_pos - floor(left_branch_width / 2.0)) + left_branch + PadString("", left_pad_width - end_x_pos - ceil(left_branch_width / 2.0)) + root_ary[i];
				}	else
					root_ary[i] = PadString("", end_x_pos) + "│" + PadString("", left_pad_width - end_x_pos - 1) + root_ary[i];
			}
		}
	}
	
	// right connection line
	if (!right_leaf.empty()) {
		int end_x_pos = GetGlyphPosOf(right_ary[0], "┴");
		int right_branch_width = GetWidth(right_branch);
		if (end_x_pos > 0) {
			int middle_y_pos = root_ary.size() / 2;
			root_ary[middle_y_pos] = Sub(root_ary[middle_y_pos], 0, GetWidth(root_ary[middle_y_pos]) - 1) + "├";
			int inset = root_width / 2;
			inset = std::min(end_x_pos - (int)floor(right_branch_width / 2.0), inset);
			for (int i = 0; i < (int)root_ary.size(); i++) {
				if (i < middle_y_pos)
					root_ary[i] = root_ary[i] + PadString("", GetMaxWidth(right_ary) - inset);
				else if (i == middle_y_pos)
					root_ary[i] = root_ary[i] + PadString("", end_x_pos - inset, ALIGNMENT_LEFT, "─") + "╮";
				else if (i == middle_y_pos + 1 && !right_branch.empty())
					root_ary[i] = root_ary[i] + Pad("", end_x_pos-inset - floor(right_branch_width / 2.0)) + right_branch;
				else
					root_ary[i] = root_ary[i] + PadString("", end_x_pos - inset) + "│";
			}
			
			for (int i = 0; i < (int)left_ary.size(); i++) {
				left_ary[i] = left_ary[i] + Pad("", root_width + left_pad_width - GetWidth(left_ary[i]) - inset);
			}
		}
	}
	
	for (int i = 0; i < (int)left_ary.size(); i++) {
		root_ary.push_back(left_ary[i] + right_ary[i]);
	}
	
	PadWidths(root_ary);
	
	return Join(root_ary);
}

}  // namespace str
