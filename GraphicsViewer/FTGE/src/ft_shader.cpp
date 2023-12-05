#include "../include.h"

ft::Shader::Shader(ft::Device::pointer &device, const std::string& shaderPath, ShaderType shaderType) :
_shaderPath(shaderPath),
_shaderType(shaderType),
_device(device)
{
	auto shaderCode = readFile(_shaderPath);
	createShaderModule(shaderCode);
}

ft::Shader::~Shader() {
	vkDestroyShaderModule(_device->getVKDevice(), _shaderModule, nullptr);
}

VkShaderModule ft::Shader::getShaderModule() const {return _shaderModule;}
ft::Shader::ShaderType ft::Shader::getShaderType() const {return _shaderType;}

std::vector<char> ft::Shader::readFile(const std::string &filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file " + filename);
	}
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void ft::Shader::createShaderModule(const std::vector<char> &code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
	// the alignment requirements are satisfied by std::allocator in std::vector
	if (vkCreateShaderModule(_device->getVKDevice(), &createInfo, nullptr, &_shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a shader module!");
	}
}

