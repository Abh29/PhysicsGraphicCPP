#include <string>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif

#include "ftGraphics.h"
#include "ft_particle.h"

int main(int argc, char **argv) {

  std::string path;
  if (argc > 1)
    path = std::string(argv[1]);
  else {
    std::cout << "no input file was provided\n";
    std::cout << "defaulting to misk/ft_scene.json" << std::endl;
    path = std::string("misk/ft_scene.json");
  }

  try {
    ft::Application firstApp{};
    firstApp.setScenePath(path);
    firstApp.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
