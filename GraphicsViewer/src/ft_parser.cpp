#include "../includes/ft_parser.h"
#include <cstdint>
#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <nlohmann/json_fwd.hpp>
#include <ostream>
#include <stdexcept>
#include <tinygltf/json.hpp>

ft::Parser::Parser(const Device::pointer &device,
                   const TexturePool::pointer &mPool,
                   const ThreadPool::pointer &tPool)
    : _ftDevice(device), _ftMaterialPool(mPool), _ftThreadPool(tPool) {}
