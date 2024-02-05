#ifndef FTGRAPHICS_FT_MODEL_H
#define FTGRAPHICS_FT_MODEL_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_buffer.h"
#include "ft_command.h"
#include "ft_vertex.h"
#include "ft_texture.h"

#define MAX_COPY_COUNT 100

namespace ft {

	class Model {
	public:
		using pointer = std::shared_ptr<Model>;
		Model(Device::pointer device, std::string filePath,
			  uint32_t bufferCount);

        Model(Device::pointer device, const tinygltf::Model &gltfInput,
              const tinygltf::Node &node, uint32_t bufferCount);

		~Model();

		static uint32_t ID();
		static glm::vec3 uint32ToVec3(uint32_t value);
		static uint32_t  vec3ToUint32(glm::vec3 &v);
		void bind(CommandBuffer::pointer commandBuffer, uint32_t index);
		void draw(CommandBuffer::pointer commandBuffer);

		uint32_t addCopy(const InstanceData &copyData = {});
		[[nodiscard]] InstanceData* getInstanceData(uint32_t id);
		bool setInstanceData(InstanceData &copyData, uint32_t id);
		[[nodiscard]] bool findID(uint32_t id) const;
		[[nodiscard]] std::array<InstanceData, MAX_COPY_COUNT>& getCopies();
		[[nodiscard]] bool isSelected(uint32_t id) const;
		[[nodiscard]] uint32_t getFirstId() const;
		std::array<uint32_t, MAX_COPY_COUNT>& getIds();
		bool select(uint32_t id);
		bool unselect(uint32_t id);
		void selectAll();
		void unselectAll();
		void overrideFlags(uint32_t id, uint32_t flags);
		void setFlags(uint32_t id, uint32_t flags);
		void unsetFlags(uint32_t id, uint32_t flags);
        void setMaterial(uint32_t materialID, uint32_t nodeID = 0);
        void unsetMaterial();
        bool hasMaterial();

        void remapMaterials(std::map<uint32_t, uint32_t> idToMaterial, TexturePool::pointer pool);
        void remapTextures(std::map<uint32_t, uint32_t> idToTexture, TexturePool::pointer pool);

	private:
		void loadModel();
		void writePerInstanceData(uint32_t index);
		void createVertexBuffer();
		void createIndexBuffer();
        void loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &gltfInput, Node* parent);
        void drawNode(CommandBuffer::pointer commandBuffer, Node* node);

		Device::pointer								_ftDevice;
		std::array<uint32_t, MAX_COPY_COUNT>		_ids;
		std::array<InstanceData, MAX_COPY_COUNT>	_copies;
		std::array<uint32_t, MAX_COPY_COUNT>		_flags;
		uint32_t 									_copiesCount;
		Buffer::pointer 							_ftVertexBuffer;
		Buffer::pointer 							_ftIndexBuffer;
		std::vector<Buffer::pointer> 				_ftInstanceBuffers;
		std::vector<Vertex>							_vertices;
		std::vector<uint32_t>						_indices;
		std::string 								_modelPath;
		bool 										_hasIndices;
        Node*                                       _node = nullptr;
        std::map<uint32_t, Node*>                   _allNodes;

	};
/*
//
//    class GLTFModel {
//        public:
//
//        struct Material_ {
//            glm::vec4   colorFactor = glm::vec4(1.0f);
//            uint32_t    colorTextureIndex;
//            uint32_t    normalTextureIndex;
//            std::string alphaModel = "OPAQUE";
//            float alphaCutOff;
//            bool doubleSided = false;
//            VkDescriptorSet descriptorSet;
//            VkPipeline pipeline;
//        };
//
//        struct Primitive_ {
//            uint32_t firstIndex;
//            uint32_t indexCount;
//            uint32_t materialIndex;
//        };
//
//        struct Mesh_ {
//            std::vector<Primitive_> mesh;
//        };
//
//        struct Node_ {
//            Node_* parent;
//            std::vector<Node_*> children;
//            Mesh_ mesh;
//            glm::mat4 matrix;
//            std::string name;
//            bool visible = true;
//            ~Node_() {
//                for (auto& child: children)
//                    delete child;
//            }
//        };
//
//
//        void loadGLTFFile(std::string filename) {
//            tinygltf::Model gltfInput;
//            tinygltf::TinyGLTF gltfContext;
//            std::string error, warning;
//
//            if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, filename))
//                throw std::runtime_error("Could not open GLTF file!\n");
//
//
//
//
//
//
//        }
//
//        private:
//
//
//
//
//
//        Device::pointer								_ftDevice;
//        std::array<uint32_t, MAX_COPY_COUNT>		_ids;
//        std::array<InstanceData, MAX_COPY_COUNT>	_copies;
//        std::array<uint32_t, MAX_COPY_COUNT>		_flags;
//        uint32_t 									_copiesCount;
//        Buffer::pointer 							_ftVertexBuffer;
//        Buffer::pointer 							_ftIndexBuffer;
//        std::vector<Buffer::pointer> 				_ftInstanceBuffers;
//        std::vector<Vertex>							_vertices;
//        std::vector<uint32_t>						_indices;
//        std::string 								_modelPath;
//        bool 										_hasIndices;
//        Texture::pointer                           _ftMaterial = nullptr;
//    };
*/
}

#endif //FTGRAPHICS_FT_MODEL_H
