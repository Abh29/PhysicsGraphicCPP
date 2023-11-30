#ifndef FTGRAPHICS_FT_APP_H
#define FTGRAPHICS_FT_APP_H


#include "ft_headers.h"
#include "ft_window.h"
#include "ft_instance.h"

namespace ft {

	class Application {
	public:
		static constexpr uint32_t W_WIDTH = 800;
		static constexpr uint32_t W_HEIGHT = 600;

		Application();
		~Application();

		void run();

	private:
		void initApplication();


		Window								_ftWindow;
		Instance*							_ftInstance;
		std::vector<const char*> 			_validationLayers;
		std::vector<const char*> 			_deviceExtensions;
	};

}

#endif //FTGRAPHICS_FT_APP_H
