#include <string>

std::string get_file_basename(const std::string filename) {
    return filename.substr(filename.find_last_of("/") + 1);
}

std::string get_file_basename_no_ext(const std::string filename) {
    const auto basename = get_file_basename(filename);
    return basename.substr(0, basename.find_last_of("."));
}

std::string get_folder(const std::string filename) {
    return filename.substr(0, filename.find_last_of("/"));
}