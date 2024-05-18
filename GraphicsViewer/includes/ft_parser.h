#ifndef FT_JSON_PARSER_H
#define FT_JSON_PARSER_H

#include "ft_device.h"
#include "ft_rendering_systems.h"
#include "ft_scene.h"
#include "ft_texture.h"
#include "ft_threads.h"
#include <memory>
#include <nlohmann/json.hpp>

namespace ft {

class Parser {
public:
  using pointer = std::shared_ptr<Parser>;

  Parser(const Device::pointer &, const TexturePool::pointer &,
         const ThreadPool::pointer &);
  virtual ~Parser() = default;

  virtual void parseSceneFile(const ft::Scene::pointer &scene,
                              const std::string &filePath,
                              float aspect = 0.75) = 0;
  virtual void saveSceneToFile(const ft::Scene::pointer &scene,
                               const std::string &filePath) = 0;

protected:
  Device::pointer _ftDevice;
  TexturePool::pointer _ftMaterialPool;
  ThreadPool::pointer _ftThreadPool;
};

} // namespace ft

#endif // FT_JSON_PARSER_H
