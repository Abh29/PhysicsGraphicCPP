#ifndef FTGRAPHICS_FT_SCENE_H
#define FTGRAPHICS_FT_SCENE_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_model.h"
#include "ft_device.h"
#include "ft_command.h"
#include "ft_buffer.h"
#include "ft_swapChain.h"
#include "ft_camera.h"
#include "ft_pipeline.h"
#include "ft_texture.h"
#include "ft_rendering_systems.h"
#include "ft_tools.h"

namespace ft {


	class Scene {

	public:
		using pointer = std::shared_ptr<Scene>;

		Scene(Device::pointer device, std::vector<Buffer::pointer> ubos);
		~Scene() = default;

		void drawSimpleObjs(CommandBuffer::pointer &commandBuffer, const GraphicsPipeline::pointer& pipeline, uint32_t index);
        void drawTexturedObjs(CommandBuffer::pointer, GraphicsPipeline::pointer, TexturedRdrSys::pointer, uint32_t index);
        void drawPickableObjs(const CommandBuffer::pointer &commandBuffer, const GraphicsPipeline::pointer& pipeline, uint32_t index);
		uint32_t addModelFromObj(std::string objectPath, ft::InstanceData data);
        uint32_t addModelsFromGltf(std::string, DescriptorPool::pointer, DescriptorSetLayout::pointer);
		uint32_t addObjectCopyToTheScene(uint32_t id, InstanceData data);
        void addTextureToObject(uint32_t id, Texture::pointer texture);
		void addPointLightToTheScene(PointLightObject& pl);
		[[nodiscard]] Camera::pointer getCamera() const;
		void setCamera(Camera::pointer camera);
		void setGeneralLight(glm::vec3 color, glm::vec3 direction, float ambient);
		void updateCameraUBO();
		PointLightObject* getLights();
		PushConstantObject& getGeneralLighting();
		[[nodiscard]] std::vector<Model::pointer> getModels() const;
        void setMaterialPool(TexturePool::pointer pool);
        bool select(uint32_t id);
        void unselectAll();


	private:

		Device::pointer												_ftDevice;
		std::vector<Model::pointer>									_models;
		std::vector<Buffer::pointer>								_ftUniformBuffers;
		PushConstantObject											_generalLighting;
		Camera::pointer												_camera;
		UniformBufferObject											_ubo;
        TexturePool::pointer                                        _ftTexturePool;
        std::map<uint32_t, std::vector<Model::pointer>>             _materialToModel;
	};


    class GLTFScene {
    public:

        struct Texture_ {
            uint32_t imageIndex;
        };

        struct Material_ {
            glm::vec4   colorFactor = glm::vec4(1.0f);
            uint32_t    colorTextureIndex;
            uint32_t    normalTextureIndex;
            std::string alphaModel = "OPAQUE";
            float alphaCutOff;
            bool doubleSided = false;
            VkDescriptorSet descriptorSet;
            VkPipeline pipeline;
        };

        struct Primitive_ {
            uint32_t firstIndex;
            uint32_t indexCount;
            uint32_t materialIndex;
        };

        struct Mesh_ {
            std::vector<Primitive_> primitives;
        };

        struct Node_ {
            Node_* parent;
            std::vector<Node_*> children;
            Mesh_ mesh;
            glm::mat4 matrix;
            std::string name;
            bool visible = true;
            ~Node_() {
                for (auto& child: children)
                    delete child;
            }
        };

        GLTFScene(Device::pointer device, std::vector<Buffer::pointer> ubos):
        _ftDevice(std::move(device)), _ftUniformBuffers(std::move(ubos)) {
            // point lights
            _ubo.pLCount = 0;
            // general lighting
            _generalLighting.lightColor = {1.0f, 1.0f, 1.0f};
            _generalLighting.lightDirection = {10.0f, 10.0f, 10.0f};
            _generalLighting.ambient = 0.2f;
        }

        ~GLTFScene() = default;

        void loadGLTFFile(std::string filename, DescriptorPool::pointer pool, DescriptorSetLayout::pointer layout);
        void setMaterialPool(TexturePool::pointer pool);
        void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node_* parent);
        void createVertexBuffer();
        void createIndexBuffer();
        void drawNode(Node_* node, const CommandBuffer::pointer& commandBuffer, GraphicsPipeline::pointer, TexturedRdrSys::pointer, uint32_t frameIndex);
        void draw(const CommandBuffer::pointer& commandBuffer, GraphicsPipeline::pointer, TexturedRdrSys::pointer, uint32_t frameIndex);

    private:


        Device::pointer     								        _ftDevice;
        Buffer::pointer 							                _ftVertexBuffer;
        Buffer::pointer 					                		_ftIndexBuffer;
        std::vector<Buffer::pointer>                 				_ftInstanceBuffers;
        std::vector<Vertex>	        						        _vertices;
        std::vector<uint32_t>						                _indices;
        std::string 								                _modelPath;
        bool 										                _hasIndices;
        Texture::pointer                                            _ftMaterial = nullptr;
        std::vector<Model::pointer>									_models;
        std::vector<Buffer::pointer>								_ftUniformBuffers;
        PushConstantObject											_generalLighting;
        Camera::pointer												_camera;
        UniformBufferObject											_ubo;
        TexturePool::pointer                                        _ftTexturePool;
        std::map<uint32_t, std::vector<Model::pointer>>             _textureToModel;
        std::array<InstanceData, MAX_COPY_COUNT>	                _copies;

        std::vector<Material_> _materials;
        std::vector<Texture_> _textures;
        std::vector<Texture::pointer>  _images;
        std::vector<Node_*> _nodes;

    };

}



#endif //FTGRAPHICS_FT_SCENE_H
