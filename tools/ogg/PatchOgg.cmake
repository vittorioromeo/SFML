file(READ "${OGG_DIR}/CMakeLists.txt" OGG_CMAKELISTS_CONTENTS)
string(REPLACE "VERSION 2.8.12" "VERSION 3.24" OGG_CMAKELISTS_CONTENTS "${OGG_CMAKELISTS_CONTENTS}")
string(REPLACE "\n\nadd_library(ogg" "\nset(CMAKE_DEBUG_POSTFIX d)\nadd_library(ogg" OGG_CMAKELISTS_CONTENTS "${OGG_CMAKELISTS_CONTENTS}")
file(WRITE "${OGG_DIR}/CMakeLists.txt" "${OGG_CMAKELISTS_CONTENTS}")
