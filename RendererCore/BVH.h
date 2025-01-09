#pragma once

#include <vector>
#include <memory>
#include "BoundingBox.h"

namespace Poorenderer {

	class HierarchicalZBufferRenderer;

	struct BVHNode
	{
		BoundingBox bounds;
		std::shared_ptr<BVHNode> left;
		std::shared_ptr<BVHNode> right;
		std::vector<size_t> triangles;// 存储三角形在`renderer.primitives`中的索引
		bool isLeaf;

	};
	class BVHTree
	{
		friend class HierarchicalZBufferRenderer;
	public:
		BVHTree(const HierarchicalZBufferRenderer& renderer);
		//BVHTree(const BVHTree& rhs);

	private:
		// `triangles` 是未被剔除的三角形数组
		// `start`, `end`为上述数组的下标范围[start, end)
		std::shared_ptr<BVHNode> BuildBVHNode(size_t start, size_t end, std::vector<size_t>& triangles, const HierarchicalZBufferRenderer& renderer);

	private:
		std::shared_ptr<BVHNode> root;
	
		const size_t MAX_TRIANGLES_PER_NODE = 1000;

	};
	
}