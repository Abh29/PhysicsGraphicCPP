#ifndef FTGRAPHICS_FT_DEFINES_H
#define FTGRAPHICS_FT_DEFINES_H

#include "ft_headers.h"

namespace ft {

    #define NDEBUG

	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	constexpr int POINT_LIGHT_MAX_COUNT = 5;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] bool isComplete() const;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct PointLightObject {
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 color;
		alignas(16) glm::vec3 attenuation; // (constant, linear, quadratic)
		float intensity;
		float radius;
		float angle; // (0.0 to 180.0)
		float exponent;
	};

	struct UniformBufferObject {
        alignas(16) glm::vec3 	        lightColor;
        alignas(16) glm::vec3 	        lightDirection;
        alignas(4)  float		        ambient;
		alignas(16) glm::mat4 			view;
		alignas(16) glm::mat4 			proj;
        alignas(16) glm::vec3           eyePosition;
		alignas(4) uint32_t 			pLCount;
		alignas(16) PointLightObject	lights[POINT_LIGHT_MAX_COUNT] = {};
	};

	struct PushConstantObject {
        alignas(16) glm::mat4 			model;
        uint32_t                        id;
	};


	enum class KeyActions {
		KEY_PRESS,
		KEY_RELEASE,
		KEY_REPEAT,
	};

	enum class KeyboardKeys {
		KEY_UNKNOWN,
		KEY_SPACE,
		KEY_COMMA,
		KEY_MINUS,
		KEY_PERIOD,
		KEY_SLASH,
		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_SEMICOLON,
		KEY_EQUAL,
		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,
		KEY_LEFT_BRACKET,
		KEY_BACKSLASH,
		KEY_RIGHT_BRACKET,
		KEY_GRAVE_ACCENT,
		KEY_WORLD_1,
		KEY_WORLD_2,
		KEY_ESCAPE,
		KEY_ENTER,
		KEY_TAB,
		KEY_BACKSPACE,
		KEY_INSERT,
		KEY_DELETE,
		KEY_RIGHT,
		KEY_LEFT,
		KEY_DOWN,
		KEY_UP,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_HOME,
		KEY_END,
		KEY_CAPS_LOCK,
		KEY_SCROLL_LOCK,
		KEY_NUM_LOCK,
		KEY_PRINT_SCREEN,
		KEY_PAUSE,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_F13,
		KEY_F14,
		KEY_F15,
		KEY_F16,
		KEY_F17,
		KEY_F18,
		KEY_F19,
		KEY_F20,
		KEY_F21,
		KEY_F22,
		KEY_F23,
		KEY_F24,
		KEY_F25,
		KEY_KP_0,
		KEY_KP_1,
		KEY_KP_2,
		KEY_KP_3,
		KEY_KP_4,
		KEY_KP_5,
		KEY_KP_6,
		KEY_KP_7,
		KEY_KP_8,
		KEY_KP_9,
		KEY_KP_DECIMAL,
		KEY_KP_DIVIDE,
		KEY_KP_MULTIPLY,
		KEY_KP_SUBTRACT,
		KEY_KP_ADD,
		KEY_KP_ENTER,
		KEY_KP_EQUAL,
		KEY_LEFT_SHIFT,
		KEY_LEFT_CONTROL,
		KEY_LEFT_ALT,
		KEY_LEFT_SUPER,
		KEY_RIGHT_SHIFT,
		KEY_RIGHT_CONTROL,
		KEY_RIGHT_ALT,
		KEY_RIGHT_SUPER,
		KEY_MENU,
	};

	enum class MouseKeys {
		MOUSE_BUTTON_1,
		MOUSE_BUTTON_2,
		MOUSE_BUTTON_3,
		MOUSE_BUTTON_4,
		MOUSE_BUTTON_5,
		MOUSE_BUTTON_6,
		MOUSE_BUTTON_7,
		MOUSE_BUTTON_8,
		MOUSE_BUTTON_LAST,
		MOUSE_BUTTON_LEFT,
		MOUSE_BUTTON_RIGHT,
		MOUSE_BUTTON_MIDDLE,
	};

	constexpr uint32_t MODEL_HIDDEN_BIT = 1u;
	constexpr uint32_t MODEL_SELECTABLE_BIT = 1u << 1;
    constexpr uint32_t MODEL_SELECTED_BIT = 1u << 2;
    constexpr uint32_t MODEL_HAS_COLOR_TEXTURE_BIT = 1u << 3;
    constexpr uint32_t MODEL_HAS_NORMAL_TEXTURE_BIT = 1u << 4;
    constexpr uint32_t MODEL_HAS_INSTANCES_BIT = 1u << 5;
    constexpr uint32_t MODEL_HAS_INDICES_BIT = 1u << 6;
    constexpr uint32_t MODEL_SIMPLE_BIT = 1u << 7;
    constexpr uint32_t MODEL_IS_EMPTY_BIT = 1u << 8;
    constexpr uint32_t MODEL_TEXTURED_BIT = 1u << 10;
    constexpr uint32_t MODEL_TRIANGLE_BIT = 1u << 11;
    constexpr uint32_t MODEL_LINE_BIT = 1u << 12;
    constexpr uint32_t MODEL_POINT_BIT = 1u << 13;


}


#endif //FTGRAPHICS_FT_DEFINES_H
