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
		using pointer = std::shared_ptr<Model>;
		Model(Device::pointer device, CommandPool::pointer commandPool,
			  std::string filePath, uint32_t bufferCount);
		~Model() = default;

		static uint32_t ID();
		void bind(CommandBuffer::pointer commandBuffer, uint32_t index);
		void draw(CommandBuffer::pointer commandBuffer);

		uint32_t addCopy(const InstanceData &copyData = {});
		[[nodiscard]] InstanceData* getInstanceData(uint32_t id);
		bool setInstanceData(InstanceData &copyData, uint32_t id);
		[[nodiscard]] bool findID(uint32_t id) const;
		[[nodiscard]] std::array<InstanceData, MAX_COPY_COUNT>& getCopies();
		[[nodiscard]] bool isSelected(uint32_t id) const;
		uint32_t getFirstId() const;
		std::array<uint32_t, MAX_COPY_COUNT>& getIds();
		bool select(uint32_t id);
		bool unselect(uint32_t id);
		void selectAll();
		void unselectAll();

	private:
		void loadModel();
		void writePerInstanceData(uint32_t index);
		void createVertexBuffer();
		void createIndexBuffer();

		Device::pointer								_ftDevice;
		CommandPool::pointer						_ftCommandPool;
		std::array<uint32_t, MAX_COPY_COUNT>		_ids;
		std::array<InstanceData, MAX_COPY_COUNT>	_copies;
		uint32_t 									_copiesCount;
		Buffer::pointer 							_ftVertexBuffer;
		Buffer::pointer 							_ftIndexBuffer;
		std::vector<Buffer::pointer> 				_ftInstanceBuffers;
		std::vector<Vertex>							_vertices;
		std::vector<uint32_t>						_indices;
		std::string 								_modelPath;
		bool 										_hasIndices;
		std::array<bool, MAX_COPY_COUNT>			_selected;
	};

}

#endif //FTGRAPHICS_FT_MODEL_H
