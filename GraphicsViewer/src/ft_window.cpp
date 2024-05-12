#include "../includes/ft_window.h"
#include <GLFW/glfw3.h>

ft::Window::Window(uint32_t w, uint32_t h, std::string name,
                   GLFWframebuffersizefun resizeCallback,
                   ft::EventListener::pointer eventListener)
    : _width(w), _height(h), _name(name), _resizeCallback(resizeCallback),
      _ftEventListener(eventListener) {
  initKeys();
  initWindow();
}

ft::Window::~Window() {
  glfwDestroyWindow(_window);
  glfwTerminate();
}

void ft::Window::initWindow() {
  if (glfwInit() != GLFW_TRUE)
    throw std::runtime_error("GLFW could not initialize a window!");

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  //	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // remove this later

  _window = glfwCreateWindow(_width, _height, _name.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(_window, this);
  glfwSetFramebufferSizeCallback(_window, _resizeCallback);
  glfwSetKeyCallback(_window, keyCallback);
  glfwSetMouseButtonCallback(_window, cursorClickCallback);
  glfwSetScrollCallback(_window, scrollCallback);
  glfwSetWindowSizeCallback(_window, resizeCallback);
  // glfwSetCursorPosCallback(_window, cursorPositionCallback);
}

bool ft::Window::shouldClose() { return glfwWindowShouldClose(_window); }

void ft::Window::close() { glfwSetWindowShouldClose(_window, true); }

void ft::Window::pollEvents() { glfwPollEvents(); }

std::vector<const char *> ft::Window::getRequiredExtensions() const {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  return extensions;
}

VkSurfaceKHR ft::Window::createVKSurface(const Instance &instance) {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(instance.getVKInstance(), _window, nullptr,
                              &surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create a window surface!");
  }
  return surface;
}

std::pair<uint32_t, uint32_t> ft::Window::queryCurrentWidthHeight() {
  glfwGetFramebufferSize(_window, reinterpret_cast<int *>(&_width),
                         reinterpret_cast<int *>(&_height));
  return {_width, _height};
}

void ft::Window::waitEvents() { glfwWaitEvents(); }

void ft::Window::keyCallback(GLFWwindow *window, int key, int scancode,
                             int action, int mods) {
  auto *ftw = reinterpret_cast<ft::Window *>(glfwGetWindowUserPointer(window));
  if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window, true);
  } else {
    ft::KeyboardEvent kev(key, scancode, action, mods);
    ftw->getEventListener()->fireInstante(kev);
  }
}

void ft::Window::cursorClickCallback(GLFWwindow *window, int button, int action,
                                     int mods) {
  auto *ftw = reinterpret_cast<ft::Window *>(glfwGetWindowUserPointer(window));
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  double &oldx = ftw->_cursorInfo.x;
  double &oldy = ftw->_cursorInfo.y;
  auto &then = ftw->_cursorInfo.clickTp;
  auto now = std::chrono::steady_clock::now();

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      ftw->_cursorInfo.lPress = true;
      oldx = x;
      oldy = y;
      then = now;
    } else if (action == GLFW_RELEASE) {
      ftw->_cursorInfo.lPress = false;
      const std::chrono::duration<double> diff = now - then;
      if (diff.count() < 0.3) {
        ft::CursorEvent cev(button, action, mods, x, y);
        ftw->getEventListener()->fireInstante(cev);
      } else {
        ft::CursorDragReleaseEvent cdrev(button, x, y);
        ftw->getEventListener()->fireInstante(cdrev);
      }
    }
  }
}

void ft::Window::scrollCallback(GLFWwindow *window, double xoffset,
                                double yoffset) {
  auto *ftw = reinterpret_cast<ft::Window *>(glfwGetWindowUserPointer(window));
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  ft::ScrollEvent sev(xoffset, yoffset, x, y);
  ftw->getEventListener()->fireInstante(sev);
}

void ft::Window::resizeCallback(GLFWwindow *window, int width, int height) {
  auto *ftw = reinterpret_cast<ft::Window *>(glfwGetWindowUserPointer(window));
  ft::ScreenResizeEvent srev(width, height);
  ftw->getEventListener()->fireInstante(srev);
}

void ft::Window::cursorPositionCallback(GLFWwindow *window, double xpos,
                                        double ypos) {
  auto *ftw = reinterpret_cast<ft::Window *>(glfwGetWindowUserPointer(window));
  auto &info = ftw->_cursorInfo;

  if (!info.lPress)
    return;

  ft::CursorDragEvent cdev(GLFW_MOUSE_BUTTON_LEFT, xpos, ypos, info.x, info.y);
  ftw->getEventListener()->fireInstante(cdev);
}

ft::EventListener::pointer &ft::Window::getEventListener() {
  return _ftEventListener;
}

int ft::Window::KEY(KeyboardKeys key) const { return _keys.find(key)->second; }
int ft::Window::MOUSE(MouseKeys key) const {
  return _mouseKeys.find(key)->second;
}
int ft::Window::ACTION(KeyActions action) const {
  return _keyActions.find(action)->second;
}

void ft::Window::initKeys() {
  _keys.insert(std::make_pair(KeyboardKeys::KEY_UNKNOWN, GLFW_KEY_UNKNOWN));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_SPACE, GLFW_KEY_SPACE));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_COMMA, GLFW_KEY_COMMA));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_MINUS, GLFW_KEY_MINUS));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_PERIOD, GLFW_KEY_PERIOD));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_SLASH, GLFW_KEY_SLASH));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_0, GLFW_KEY_0));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_1, GLFW_KEY_1));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_2, GLFW_KEY_2));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_3, GLFW_KEY_3));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_4, GLFW_KEY_4));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_5, GLFW_KEY_5));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_6, GLFW_KEY_6));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_7, GLFW_KEY_7));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_8, GLFW_KEY_8));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_9, GLFW_KEY_9));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_SEMICOLON, GLFW_KEY_SEMICOLON));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_EQUAL, GLFW_KEY_EQUAL));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_A, GLFW_KEY_A));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_B, GLFW_KEY_B));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_C, GLFW_KEY_C));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_D, GLFW_KEY_D));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_E, GLFW_KEY_E));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F, GLFW_KEY_F));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_G, GLFW_KEY_G));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_H, GLFW_KEY_H));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_I, GLFW_KEY_I));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_J, GLFW_KEY_J));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_K, GLFW_KEY_K));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_L, GLFW_KEY_L));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_M, GLFW_KEY_M));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_N, GLFW_KEY_N));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_O, GLFW_KEY_O));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_P, GLFW_KEY_P));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_Q, GLFW_KEY_Q));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_R, GLFW_KEY_R));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_S, GLFW_KEY_S));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_T, GLFW_KEY_T));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_U, GLFW_KEY_U));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_V, GLFW_KEY_V));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_W, GLFW_KEY_W));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_X, GLFW_KEY_X));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_Y, GLFW_KEY_Y));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_Z, GLFW_KEY_Z));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_LEFT_BRACKET, GLFW_KEY_LEFT_BRACKET));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_BACKSLASH, GLFW_KEY_BACKSLASH));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_RIGHT_BRACKET, GLFW_KEY_RIGHT_BRACKET));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_GRAVE_ACCENT, GLFW_KEY_GRAVE_ACCENT));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_WORLD_1, GLFW_KEY_WORLD_1));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_WORLD_2, GLFW_KEY_WORLD_2));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_ESCAPE, GLFW_KEY_ESCAPE));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_ENTER, GLFW_KEY_ENTER));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_TAB, GLFW_KEY_TAB));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_BACKSPACE, GLFW_KEY_BACKSPACE));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_INSERT, GLFW_KEY_INSERT));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_DELETE, GLFW_KEY_DELETE));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_RIGHT, GLFW_KEY_RIGHT));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_LEFT, GLFW_KEY_LEFT));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_DOWN, GLFW_KEY_DOWN));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_UP, GLFW_KEY_UP));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_PAGE_UP, GLFW_KEY_PAGE_UP));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_PAGE_DOWN, GLFW_KEY_PAGE_DOWN));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_HOME, GLFW_KEY_HOME));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_END, GLFW_KEY_END));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_CAPS_LOCK, GLFW_KEY_CAPS_LOCK));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_SCROLL_LOCK, GLFW_KEY_SCROLL_LOCK));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_NUM_LOCK, GLFW_KEY_NUM_LOCK));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_PRINT_SCREEN, GLFW_KEY_PRINT_SCREEN));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_PAUSE, GLFW_KEY_PAUSE));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F1, GLFW_KEY_F1));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F2, GLFW_KEY_F2));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F3, GLFW_KEY_F3));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F4, GLFW_KEY_F4));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F5, GLFW_KEY_F5));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F6, GLFW_KEY_F6));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F7, GLFW_KEY_F7));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F8, GLFW_KEY_F8));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F9, GLFW_KEY_F9));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F10, GLFW_KEY_F10));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F11, GLFW_KEY_F11));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F12, GLFW_KEY_F12));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F13, GLFW_KEY_F13));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F14, GLFW_KEY_F14));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F15, GLFW_KEY_F15));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F16, GLFW_KEY_F16));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F17, GLFW_KEY_F17));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F18, GLFW_KEY_F18));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F19, GLFW_KEY_F19));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F20, GLFW_KEY_F20));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F21, GLFW_KEY_F21));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F22, GLFW_KEY_F22));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F23, GLFW_KEY_F23));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F24, GLFW_KEY_F24));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_F25, GLFW_KEY_F25));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_0, GLFW_KEY_KP_0));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_1, GLFW_KEY_KP_1));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_2, GLFW_KEY_KP_2));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_3, GLFW_KEY_KP_3));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_4, GLFW_KEY_KP_4));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_5, GLFW_KEY_KP_5));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_6, GLFW_KEY_KP_6));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_7, GLFW_KEY_KP_7));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_8, GLFW_KEY_KP_8));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_9, GLFW_KEY_KP_9));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_KP_DECIMAL, GLFW_KEY_KP_DECIMAL));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_DIVIDE, GLFW_KEY_KP_DIVIDE));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_KP_MULTIPLY, GLFW_KEY_KP_MULTIPLY));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_KP_SUBTRACT, GLFW_KEY_KP_SUBTRACT));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_ADD, GLFW_KEY_KP_ADD));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_ENTER, GLFW_KEY_KP_ENTER));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_KP_EQUAL, GLFW_KEY_KP_EQUAL));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_LEFT_SHIFT, GLFW_KEY_LEFT_SHIFT));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_LEFT_CONTROL, GLFW_KEY_LEFT_CONTROL));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_LEFT_ALT, GLFW_KEY_LEFT_ALT));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_LEFT_SUPER, GLFW_KEY_LEFT_SUPER));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_RIGHT_SHIFT, GLFW_KEY_RIGHT_SHIFT));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_RIGHT_CONTROL, GLFW_KEY_RIGHT_CONTROL));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_RIGHT_ALT, GLFW_KEY_RIGHT_ALT));
  _keys.insert(
      std::make_pair(KeyboardKeys::KEY_RIGHT_SUPER, GLFW_KEY_RIGHT_SUPER));
  _keys.insert(std::make_pair(KeyboardKeys::KEY_MENU, GLFW_KEY_MENU));

  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_1, GLFW_MOUSE_BUTTON_1));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_2, GLFW_MOUSE_BUTTON_2));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_3, GLFW_MOUSE_BUTTON_3));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_4, GLFW_MOUSE_BUTTON_4));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_5, GLFW_MOUSE_BUTTON_5));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_6, GLFW_MOUSE_BUTTON_6));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_7, GLFW_MOUSE_BUTTON_7));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_8, GLFW_MOUSE_BUTTON_8));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_LAST, GLFW_MOUSE_BUTTON_LAST));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_LEFT));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_RIGHT));
  _mouseKeys.insert(
      std::make_pair(MouseKeys::MOUSE_BUTTON_MIDDLE, GLFW_MOUSE_BUTTON_MIDDLE));

  _keyActions.insert(std::make_pair(KeyActions::KEY_PRESS, GLFW_PRESS));
  _keyActions.insert(std::make_pair(KeyActions::KEY_RELEASE, GLFW_RELEASE));
  _keyActions.insert(std::make_pair(KeyActions::KEY_REPEAT, GLFW_REPEAT));
}

void *ft::Window::getRawWindowPointer() const { return _window; }
