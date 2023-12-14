#ifndef FTGRAPHICS_FT_MODEL_H
#define FTGRAPHICS_FT_MODEL_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_buffer.h"
#include "ft_command.h"
#include "ft_vertex.h"

#define MAX_COPY_COUNT 100

namespace ft {

	class Model {
	public:
		Model(Device::pointer device, CommandPool::pointer commandPool,
			  std::string filePath, Buffer::pointer instanceBuffer);
		~Model() = default;

		void bind(CommandBuffer::pointer commandBuffer);
		void draw(CommandBuffer::pointer commandBuffer);
		void addCopy(const InstanceData &copyData = {});
		[[nodiscard]] InstanceData* getInstanceData(uint32_t id);
		bool setInstanceData(InstanceData &copyData, uint32_t id);
		[[nodiscard]] bool findID(uint32_t id) const;
		[[nodiscard]] std::array<InstanceData, MAX_COPY_COUNT>& getCopies();
		static uint32_t ID();

	private:
		void loadModel();
		void writePerInstanceData();
		void createVertexBuffer();
		void createIndexBuffer();

		Device::pointer								_ftDevice;
		CommandPool::pointer						_ftCommandPool;
		std::array<uint32_t, MAX_COPY_COUNT>		_ids;
		std::array<InstanceData, MAX_COPY_COUNT>	_copies;
		uint32_t 									_copiesCount;
		Buffer::pointer 							_ftVertexBuffer;
		Buffer::pointer 							_ftIndexBuffer;
		Buffer::pointer 							_ftInstanceBuffer;
		std::vector<Vertex>							_vertices;
		std::vector<uint32_t>						_indices;
		std::string 								_modelPath;
		bool 										_hasIndices;
	};

}

#endif //FTGRAPHICS_FT_MODEL_H
