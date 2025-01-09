#pragma once

#include "BaseRenderer.h"
#include "DepthMipmap.h"
#include "BVH.h"
namespace Poorenderer {

	class HierarchicalZBufferRenderer :public BaseRenderer {

		friend class BVHTree;
	public:
		void SetAttachments(size_t width, size_t height) override;
		void Rasterization() override;

		// 获取对应片元的质心
		glm::vec3 GetPrimitiveCentroid(size_t primitiveId) const;
	private:
		DepthMipmap depthMipmap;
		bool bUseBVH = true;
		std::shared_ptr<BVHTree> bvhTree;
		void TransverseBVHNode(BVHNode* Node);
		
	};

}