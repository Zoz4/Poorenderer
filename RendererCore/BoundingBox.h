#pragma once

#include<glm/vec3.hpp>
#include<glm/mat4x4.hpp>
#include<limits>

namespace Poorenderer {
	class BoundingBox {
	public:
		BoundingBox() = default;
		// a, b是BoundingBox的边界点
		BoundingBox(const glm::vec3& a, const glm::vec3& b) : min(a), max(b) {};
		// 传入三角形的顶点，获取三角形的包围盒
		BoundingBox(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
			float minX = std::min(std::min(v0.x, v1.x), v2.x);
			float minY = std::min(std::min(v0.y, v1.y), v2.y);
			float minZ = std::min(std::min(v0.z, v1.z), v2.z);
			float maxX = std::max(std::max(v0.x, v1.x), v2.x);
			float maxY = std::max(std::max(v0.y, v1.y), v2.y);
			float maxZ = std::max(std::max(v0.z, v1.z), v2.z);

			min = glm::vec3{ minX, minY, minZ };
			max = glm::vec3{ maxX, maxY, maxZ };
		}
		BoundingBox(const BoundingBox& rhs) {
			min = rhs.min;
			max = rhs.max;
		}

		// 返回包围盒最长的轴
		size_t GetMaxExent() const{
			glm::vec3 distance = max - min;
			if (distance.x > distance.y && distance.x > distance.z) return 0;
			else if (distance.y > distance.z) return 1;
			else return 2;
		}

	public:
		glm::vec3 min{std::numeric_limits<float>::max(),std::numeric_limits<float>::max() ,std::numeric_limits<float>::max()};
		glm::vec3 max{ std::numeric_limits<float>::min(),std::numeric_limits<float>::min() ,std::numeric_limits<float>::min()};
	};

	// 合并两个包围盒
	BoundingBox Union(const BoundingBox& b0, const BoundingBox& b1);
}