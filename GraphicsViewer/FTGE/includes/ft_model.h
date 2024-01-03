#ifndef FTGRAPHICS_FT_MODEL_H
#define FTGRAPHICS_FT_MODEL_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_buffer.h"
#include "ft_command.h"
#include "ft_vertex.h"
#include "ft_material.h"

#define MAX_COPY_COUNT 100

namespace ft {

	class Model {
	public:
		using pointer = std::shared_ptr<Model>;
		Model(Device::pointer device, std::string filePath,
			  uint32_t bufferCount);
		~Model() = default;

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
        void setMaterial(Material::pointer material);
        void unsetMaterial();
        bool hasMaterial();

	private:
		void loadModel();
		void writePerInstanceData(uint32_t index);
		void createVertexBuffer();
		void createIndexBuffer();

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
        Material::pointer                           _ftMaterial = nullptr;

	};

}

#endif //FTGRAPHICS_FT_MODEL_H
