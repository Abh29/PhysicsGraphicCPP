#ifndef FTGRAPHICS_FT_ATTACHMENT_H
#define FTGRAPHICS_FT_ATTACHMENT_H

#include "ft_headers.h"
#include "ft_device.h"

namespace ft {

	class Device;

class Attachment {

public:

	using pointer = std::shared_ptr<Attachment>;

	Attachment();
	~Attachment();

private:

	Device::pointer 				_ftDevice;
	VkAttachmentDescription			_attachmentDescription;
	VkAttachmentReference			_attachmentReference;

};

}


#endif //FTGRAPHICS_FT_ATTACHMENT_H
