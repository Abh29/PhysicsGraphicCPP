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

class JsonParser : public ft::Parser {
public:
  using pointer = std::shared_ptr<JsonParser>;

  JsonParser(const Device::pointer &, const TexturePool::pointer &,
             const ThreadPool::pointer &, const OneTextureRdrSys::pointer &,
             const TwoTextureRdrSys::pointer &, const SkyBoxRdrSys::pointer &);

  ~JsonParser() = default;

  void parseSceneFile(const ft::Scene::pointer &scene,
                      const std::string &filePath,
                      float aspect = 0.75f) override;
  void saveSceneToFile(const ft::Scene::pointer &scene,
                       const std::string &filePath) override;

private:
  void loadCamera(const Scene::pointer &scene, nlohmann::json &data,
                  float aspect);
  void loadSkyBox(const Scene::pointer &scene, nlohmann::json &data);
  void loadModels(const Scene::pointer &scene, nlohmann::json &data);
  void loadLights(const Scene::pointer &scene, nlohmann::json &data);

  OneTextureRdrSys::pointer _ftTexturedRdrSys;
  TwoTextureRdrSys::pointer _ft2TexturedRdrSys;
  SkyBoxRdrSys::pointer _ftSkyBoxRdrSys;
};

} // namespace ft

#endif // FT_JSON_PARSER_H
