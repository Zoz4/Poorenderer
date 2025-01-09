#pragma once

#include <vector>

namespace Poorenderer {


	class BoundingBox;
	class DepthMipmap {
	public:

		DepthMipmap():width(0), height(0), levelNums(0), depthBuffer() {}
		DepthMipmap(size_t width, size_t _height);

		// 更新位于(x, y)像素的深度值， 并维护DepthMipmap
		void Update(size_t x, size_t y, float depth);

		// 判断当前覆盖的屏幕区域是否被遮挡
		bool IsOccluded(const BoundingBox& bound, float minDepth) const;

		// 获取Mipmap层级level中坐标为(x, y)的数据在缓冲区中的索引
		size_t GetIndex(size_t x, size_t y, size_t level) const ;

		float GetDepth(size_t x, size_t y) const;

	private:
		size_t width, height, levelNums;
		std::vector<std::vector<float>> depthBuffer;
	};
}