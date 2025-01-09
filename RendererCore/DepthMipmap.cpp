#include "DepthMipmap.h"
#include "BoundingBox.h"

#include <cmath>
#include <limits>

namespace Poorenderer {

	DepthMipmap::DepthMipmap(size_t w, size_t h) :width(w), height(h)
	{
		levelNums = std::floor(std::log2(std::min(width, height)));
		depthBuffer.resize(levelNums);
		for (size_t level = 0; level < levelNums; ++level) {
			size_t levelWidth = (width >> level) + 1;
			size_t levelHeitht = (height >> level) + 1;
			depthBuffer[level].resize(levelWidth * levelHeitht, std::numeric_limits<float>::max());
		}
	}

	void DepthMipmap::Update(size_t x, size_t y, float depth)
	{
		size_t idx = y * width + x;

		depthBuffer[0][idx] = depth;

		for (size_t level = 1; level < levelNums; ++level) {

			size_t curX = x >> level;
			size_t curY = y >> level;
			size_t curWidth = width >> level;

			size_t curIdx = curWidth * curY + curX;

			float& curDepth = depthBuffer[level][curIdx];

			// 当前level中的深度（上层level的最大深度）小于要设置的值，无需继续往下更新
			if (curDepth <= depth) {
				break;
			}

			curDepth = depth;
			float maxDepth = depth;
			for (size_t dy = 0; dy < 2; ++dy) {
				for (size_t dx = 0; dx < 2; ++dx) {
					size_t prevX = (curX << 1) + dx;
					size_t prevY = (curY << 1) + dy;
					if (prevX < width && prevY < height) {
						size_t prevWidth = curWidth << 1;
						size_t prevIdx = prevY * prevWidth + prevX;
						maxDepth = std::max(maxDepth, depthBuffer[level - 1][prevIdx]);
					}

				}
			}
			if (maxDepth >= curDepth) {
				break;
			}
			curDepth = maxDepth;
		}

	}

	bool DepthMipmap::IsOccluded(const BoundingBox& bound, float minDepth) const
	{
		size_t xMin = bound.min.x, xMax = bound.max.x;
		size_t yMin = bound.min.y, yMax = bound.max.y;

		size_t level = 0;
		for (; level < levelNums-1; ++level) {

			if ((xMax >> level) == (xMin >> level) && (yMax >> level) == (yMin >> level))
			{
				break;
			}
		}
		return minDepth > depthBuffer[level][GetIndex(xMin, yMin, level)];
	}

	size_t DepthMipmap::GetIndex(size_t x, size_t y, size_t level) const
	{
		size_t curWidth = width >> level;
		size_t curX = x >> level;
		size_t curY = y >> level;
		return curY*curWidth + curX;
	}

	float DepthMipmap::GetDepth(size_t x, size_t y) const
	{
		return depthBuffer[0][GetIndex(x, y, 0)];
	}
}