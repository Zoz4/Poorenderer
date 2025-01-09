#include "BVH.h"
#include "Logger.h"
#include "HierarchicalZBufferRenderer.h"
#include <algorithm>

namespace Poorenderer {
	BVHTree::BVHTree(const HierarchicalZBufferRenderer& renderer)
	{
		LOGI("Building BVH Tree");
		std::vector<size_t> triangles;
		for (size_t i = 0; i < renderer.primitives.size(); ++i) {
			if (!renderer.primitives[i].discard) {
				triangles.push_back(i);
			}
		}
		root = BuildBVHNode(0, triangles.size(), triangles, renderer);
		LOGI("Build Finished!");
	}

	std::shared_ptr<BVHNode> BVHTree::BuildBVHNode(size_t start, size_t end, std::vector<size_t>& triangles, const HierarchicalZBufferRenderer& renderer)
	{
		std::shared_ptr<BVHNode> node = std::make_shared<BVHNode>();
		BoundingBox bounds;
		for (size_t i = start; i < end; ++i) {
			glm::vec3 v0 = renderer.screenPositions[renderer.primitives[triangles[i]].indices[0]];
			glm::vec3 v1 = renderer.screenPositions[renderer.primitives[triangles[i]].indices[1]];
			glm::vec3 v2 = renderer.screenPositions[renderer.primitives[triangles[i]].indices[2]];
			bounds = Union(bounds, BoundingBox(v0, v1, v2));
		}
		node->bounds = bounds;

		// 三角形数量少于叶子节点的数量要求
		int numTriangles = end - start;
		if (numTriangles <= MAX_TRIANGLES_PER_NODE) {
			node->isLeaf = true;
			for (size_t i = start; i < end; ++i) {
				node->triangles.push_back(triangles[i]);
			}
			return node;
		}

		// 选择最长的轴进行划分
		size_t axis = bounds.GetMaxExent();
		float mid = (bounds.min[axis] + bounds.max[axis]) * 0.5f;

		int midIdx = start + (end - start) / 2;
		std::sort(triangles.begin() + start, triangles.begin() + end,
			[axis, &renderer](const size_t& lhs, const size_t& rhs) {
				return renderer.GetPrimitiveCentroid(lhs)[axis] < renderer.GetPrimitiveCentroid(rhs)[axis];
			});

		node->left = BuildBVHNode(start, midIdx, triangles, renderer);
		node->right = BuildBVHNode(midIdx, end, triangles, renderer);

		return node;
	}
}