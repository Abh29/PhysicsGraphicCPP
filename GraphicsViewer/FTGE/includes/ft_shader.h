#ifndef FTGRAPHICS_FT_SHADER_H
#define FTGRAPHICS_FT_SHADER_H

#include "ft_headers.h"
#include "ft_device.h"

namespace ft {

	class Device;

	class Shader {

	public:
		using ShaderType = enum class _shaderType {
			FT_VERTEX_SHADER,
			FT_FRAGMENT_SHADER,
			FT_COMPUTE_SHADER
		};

		using pointer = std::shared_ptr<Shader>;

		Shader(Device::pointer &device, const std::string& shaderPath, ShaderType shaderType);
		~Shader();

		Shader(const Shader& other) = delete;
		Shader operator=(const Shader& other) = delete;


		VkShaderModule getVKShaderModule() const;
		ShaderType getShaderType() const;

	private:

		std::vector<char>	readFile(const std::string& filename);
		void createShaderModule(const std::vector<char>& code);


		std::string					_shaderPath;
		VkShaderModule 				_shaderModule;
		const ShaderType 			_shaderType;
		Device::pointer 			_ftDevice;

	};

}


#endif //FTGRAPHICS_FT_SHADER_H
