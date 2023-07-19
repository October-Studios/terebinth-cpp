module;
export module Util.StringDrawing;

import Util.StringUtils;

export {

namespace str {
std::string GetBoxedString(const std::string &in, std::string box_name = "",
                           bool show_line_nums = false,
                           bool always_width_max = false, int max_width = -1);

void PutArrayInTreeNodeBox(std::vector<std::string> &data);

std::string PutStringInTreeNodeBox(const std::string &in);

std::string MakeList(std::vector<std::string> &data);

std::string MakeRootUpBinaryTree(const std::string &root,
                                 const std::string &left_branch,
                                 const std::string &right_branch,
                                 const std::string &left_leaf,
                                 const std::string &right_leaf);

}  // namespace str

}  // export
