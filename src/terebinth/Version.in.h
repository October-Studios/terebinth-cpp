// terebinth's version format is vMAJOR.MINOR.PATCH

#define TEREBINTH_VERSION_MAJOR "@TEREBINTH_VERSION_MAJOR@";
#define TEREBINTH_VERSION_MINOR "@TEREBINTH_VERSION_MINOR@";
#define TEREBINTH_VERSION_PATCH "@TEREBINTH_VERSION_PATCH@";

class Version {
 public:
  static int Major() {
    return std::stoi(TEREBINTH_VERSION_MAJOR);
  }
  static int Minor() {
    return std::stoi(TEREBINTH_VERSION_MINOR);
  }
  static int Patch() {
    return std::stoi(TEREBINTH_VERSION_PATCH);
  }
};