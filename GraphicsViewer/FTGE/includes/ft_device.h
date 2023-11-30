#ifndef FTGRAPHICS_FT_DEVICE_H
#define FTGRAPHICS_FT_DEVICE_H

#include "ft_headers.h"

namespace ft {

	class Device {

	public:
		Device();
		~Device();


		VkDevice  getVKDevice() const;


	private:
		VkDevice 					_device;

	};

}


#endif //FTGRAPHICS_FT_DEVICE_H
