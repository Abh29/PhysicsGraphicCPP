#ifndef FTGRAPHICS_FT_SWAPCHAIN_H
#define FTGRAPHICS_FT_SWAPCHAIN_H

#include "ft_headers.h"
#include "ft_physicalDevice.h"
#include "ft_device.h"
#include "ft_surface.h"


namespace ft {

	class Device;
	class Surface;
	struct SwapChainSupportDetails;

	class SwapChain {

	public:
		SwapChain(std::shared_ptr<PhysicalDevice> &physicalDevice,
				  std::shared_ptr<Device> &device,
				  std::shared_ptr<Surface> &surface,
				  uint32_t width, uint32_t height);

		~SwapChain();


		[[nodiscard]] VkSwapchainKHR getVKSwapChain() const;
		[[nodiscard]] VkFormat	getVKSwapChainImageFormat() const;
		[[nodiscard]] VkExtent2D	getVKSwapChainExtent() const;
		[[nodiscard]] std::vector<VkImage> getVKSwapChainImages() const;
		[[nodiscard]] std::vector<VkImageView>	getVKSwapChainImageViews() const;


	private:
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilitiesKhr);
		void createImageViews();

		std::shared_ptr<PhysicalDevice>		_ftPhysicalDevice;
		std::shared_ptr<Device>				_ftDevice;
		std::shared_ptr<Surface>			_ftSurface;
		VkSwapchainKHR 						_swapChain;
		VkFormat							_swapChainImageFormat;
		VkExtent2D 							_swapChainExtent;
		std::vector<VkImage>				_swapChainImages;
		std::vector<VkImageView>			_swapChainImageViews;
		uint32_t 							_width;
		uint32_t 							_height;

	};


}

#endif //FTGRAPHICS_FT_SWAPCHAIN_H
