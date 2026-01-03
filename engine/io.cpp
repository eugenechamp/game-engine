#include "io.hpp"
#include <fstream>

auto io::read_file(const std::filesystem::path &path, openmode mode) -> std::string {
  std::ifstream file;
	file.open(path, mode);
	std::string source;

	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		auto size = file.tellg();
		file.seekg(0, std::ios::beg);
		source.resize(static_cast<size_t>(size), ' ');
		
		file.read(&source[0], size);
		
		return std::move(source);
	}

	return "";
}