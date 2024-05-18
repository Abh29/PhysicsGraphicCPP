#ifndef INCLUDE_INCLUDES_FT_SCENEOBJECT_H_
#define INCLUDE_INCLUDES_FT_SCENEOBJECT_H_

#include "ft_component.h"
#include "ft_headers.h"
#include "ft_model.h"

namespace ft {

class Scene;

class SceneObject {
public:
  using pointer = std::shared_ptr<SceneObject>;
  using raw_ptr = SceneObject *;

  friend class Scene;

  explicit SceneObject(const ft::Model::pointer &model) : _model(model) {}

  inline void update(float duration) {
    for (auto &c : _components)
      c->update(duration);
  };

  inline ft::Model::pointer getModel() const { return _model; };
  inline ft::Model::pointer &getModel() { return _model; }

  template <typename T> T *getComponent() {
    for (auto &c : _components) {
      // auto& t = *c;
      if (typeid(*c) == typeid(T))
        return dynamic_cast<T *>(c.get());
    }

    return nullptr;
  };

  template <typename T, typename... Args>
  std::shared_ptr<T> addComponent(Args &&...args) {
    auto c = std::make_shared<T>(std::forward<Args>(args)...);
    _components.push_back(c);
    return c;
  };

  std::vector<Component::pointer> &getAllComponents() { return _components; }

protected:
  ft::Model::pointer _model;
  std::vector<Component::pointer> _components;
};

}; // namespace ft

#endif // INCLUDE_INCLUDES_FT_SCENEOBJECT_H_
