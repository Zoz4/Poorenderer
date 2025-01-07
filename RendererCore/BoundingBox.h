#pragma once

#include<glm/vec3.hpp>
#include<glm/mat4x4.hpp>


namespace Poorenderer {
	class BoundingBox {
	public:
		BoundingBox() = default;
		BoundingBox(const glm::vec3& a, const glm::vec3& b) : min(a), max(b) {};
	public:
		glm::vec3 min{ 0.f, 0.f, 0.f };
		glm::vec3 max{ 0.f, 0.f, 0.f };
	};
}