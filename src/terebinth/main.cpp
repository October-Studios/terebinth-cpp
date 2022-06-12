#include "error_handler.h"
#include "terebinth_program.h"

#include <iostream>

std::vector<std::string> cmd_line_args;

struct Flags {
  std::string my_path;
  std::vector<std::string> in_files;
  bool debug = false;
  bool help = false;
  bool version = false;
  bool run_interpreted = true;
  std::string cpp_out_file = "";
  std::string bin_out_file = "";
  bool run_compiled = false;
  bool flag_error = false;
};

Flags GetFlags(int argc, char **argv);

int main(int argc, char **argv) {
  Flags flags = GetFlags(argc, argv);

  if (flags.flag_error) {
    std::cerr << "Try 'terebinth -h' for help" << std::endl;
    return 0;
  }
  if (flags.help) {
    std::cout << "terebinth v" << VERSION_MAJOR << "." << VERSION_MINOR << "."
              << VERSION_PATCH << std::endl;
    std::cout << "usage: terebinth [options] [source] [options]" << std::endl;
    std::cout << "options: " << std::endl;
    std::cout << "-v, -version      display the version of terebinth"
              << std::endl;
    std::cout
        << "-d, -debug        display debugging info before running the program"
        << std::endl;
    std::cout << "-r, -run          run the program with the interpreter"
              << std::endl;
    std::cout << "                      active by default if no transpiling "
                 "commands are present"
              << std::endl;
    std::cout << "                      currently, anything after -r is ignored"
              << std::endl;
    std::cout << "-cpp [file]       transpile to C++ and save the output in "
                 "the given file"
              << std::endl;
    std::cout
        << "-bin [file]       transpile, compile with GCC and save the binary"
        << std::endl;
    std::cout << "-e, -execute      transpile, compile and execute the binary"
              << std::endl;
    std::cout << "                      any combination of -cpp, -bin, and -e "
                 "can be used"
              << std::endl;
    std::cout << "                      like -r, anything after -e is ignored"
              << std::endl;
    std::cout << "-h, -help         display this help and quit" << std::endl;
    std::cout << "\n\n";

    return 0;
  }
  if (flags.version) {
    std::cout << "terebinth version: v" << VERSION_MAJOR << "." << VERSION_MINOR
              << "." << VERSION_PATCH << std::endl;
    return 0;
  }

  TerebinthProgram program;

  if (flags.in_files.empty()) {
    std::cout << "No source files specified" << std::endl;
    std::cout << "Try 'terebinth -h' for help" << std::endl;
    return 0;
  } else if (flags.in_files.size() > 1) {
    std::cout << "Multiple source files specified, terebinth does not "
                 "currently support this"
              << std::endl;
    std::cout << "Try 'terebinth -h' for help" << std::endl;
    return 0;
  }

  program.ResolveProgram(flags.in_files[0], flags.debug);

  if (flags.run_interpreted) {
    if (error.GetIfErrorLogged()) {
      if (flags.debug) {
        std::cout
            << std::endl
            << ">>>>>>   execution aborted due to previous error     <<<<<<"
            << std::endl;
      } else {
        std::cout << "program not executed due to errors" << std::endl;
      }
    } else {
      if (flags.debug) {
        std::cout << std::endl
                  << "running program..." << std::endl
                  << std::endl;
      }
      program.Execute();
    }
  }

  if (!flags.cpp_out_file.empty() || !flags.bin_out_file.empty() ||
      flags.run_compiled) {
    std::string cpp_code = program.GetCpp();

    if (error.GetIfErrorLogged()) {
      if (flags.debug) {
        std::cout << std::endl
                  << ">>>>>>   transpiling failed    <<<<<<" << std::endl;
      } else {
        std::cout << "transpiling failed" << std::endl;
      }
    } else {
      std::string cpp_file_name = flags.cpp_out_file;

      if (cpp_file_name.empty()) {
        cpp_file_name = "terebinth_transpile.cpp";
      }

      if (flags.debug) {
        std::cout << std::end
                  << std::PutStringInBox(cpp_code, "C++ code", true, false, -1)
                  << std::endl;
      }

      WriteFile(cpp_file_name, cpp_code, flags.debug);

      if (!flags.bin_out_file.empty() || flags.run_compiled) {
        std::string bin_file_name = flags.bin_out_file;

        if (bin_file_name.empty()) {
          bin_file_name = "terebinth_compiled";
        }

        std::string cmd;
        cmd =
            "g++ -std=c++11 '" + cpp_file_name + "' -o '" + bin_file_name + "'";

        if (flags.debug) {
          std::cout << "running '" + cmd + "'" << std::endl;
        }

        RunCmd(cmd, true);

        if (flags.run_compiled) {
          if (flags.debug) {
            std::cout << std::endl;
          }
        }
      }
    }
  }
}

Flags GetFlags(int argc, char **argv) {
  bool after = false;
  Flags flags;

  for (auto i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (!after) {
      if (arg.size() > 1 && arg[0] == '-') {
        std::string flag = arg.substr(1, string::npos);

        if (flag == "d" || flag == "debug") {
          flags.debug = true;
        } else if (flag == "v" || flag == "version") {
          flags.version = true;
        } else if (flag == "h" || flag == "help") {
          flags.help = true;
        } else if (flag == "r" || flag == "run") {
          flags.run_compiled = false;
          flags.run_interpreted = true;
          after = true;
        } else if (flag == "cpp") {
          if (i + 1 >= argc) {
            std::cout << "output file must follow '-cpp' flag";
            flags.flag_error = true;
          }

          flags.run_interpreted = false;
          flags.cpp_out_file = std::string(argv[i + 1]);

          i++;
        } else if (flag == "bin") {
          if (i + 1 >= argc) {
            std::cout << "output file must follow '-bin' flag";
            flags.flag_error = true;
          }

          flags.run_interpreted = false;
          flags.bin_out_file = std::string(argv[i + 1]);

          i++;
        } else if (flag == "e" || flag == "execute") {
          flags.run_compiled = true;
          flags.run_interpreted = false;
          after = true;
        } else {
          std::cout << "unknown flag '" + flag + "'" << std::endl;
          flags.flag_error = true;
        }
      } else {
        flags.in_files.push_back(arg);
        cmd_line_args.push_back(arg);
      }
    } else {
      cmd_line_args.push_back(arg);
    }
  }

  return flags;
}