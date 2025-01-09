#include "BoundingBox.h"

namespace Poorenderer{
	BoundingBox Poorenderer::Union(const BoundingBox& b0, const BoundingBox& b1)
	{
		BoundingBox result;
		result.min = glm::vec3(std::min(b0.min.x, b1.min.x), std::min(b0.min.y, b1.min.y), std::min(b0.min.z, b1.min.z));
		result.max = glm::vec3(std::max(b0.max.x, b1.max.x), std::max(b0.max.y, b1.max.y), std::max(b0.max.z, b1.max.z));
		return result;
	}
}