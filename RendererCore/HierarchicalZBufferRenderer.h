#pragma once

#include "BaseRenderer.h"
#include "DepthMipmap.h"
namespace Poorenderer {

	class HierarchicalZBufferRenderer :public BaseRenderer {

	public:
		void SetAttachments(size_t width, size_t height) override;
		void Rasterization() override;

	private:
		DepthMipmap depthMipmap;
	};

}