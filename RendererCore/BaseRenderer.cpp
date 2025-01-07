#include "BaseRenderer.h"
#include "Model.h"
#include "logger.h"
#include "BoundingBox.h"

#include <array>
#include <limits>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace Poorenderer {
	void BaseRenderer::SetAttachments(size_t width, size_t height)
	{
		// COLOR FORMAT: RGB
		colorAttachment.resize(width * height * 3, 127);
		depthBuffer.resize(width * height, std::numeric_limits<float>::max());
	}
	void BaseRenderer::SetViewport(int x, int y, int width, int height)
	{
		viewport.x = (float)x;
		viewport.y = (float)y;
		viewport.width = (float)width;
		viewport.height = (float)height;

		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		viewport.absMinDepth = std::min(viewport.minDepth, viewport.maxDepth);
		viewport.absMaxDepth = std::max(viewport.minDepth, viewport.maxDepth);

		viewport.innerO.x = viewport.x + viewport.width / 2.f;
		viewport.innerO.y = viewport.y + viewport.height / 2.f;
		viewport.innerO.z = viewport.minDepth;
		viewport.innerO.w = 0.f;

		viewport.innerP.x = viewport.width / 2.f;    // divide by 2 in advance
		viewport.innerP.y = viewport.height / 2.f;   // divide by 2 in advance
		viewport.innerP.z = viewport.maxDepth - viewport.minDepth;
		viewport.innerP.w = 1.f;
	}
	void BaseRenderer::SetVertexArrayObject(const Model& model)
	{
		vertices = model.vertices;
		normals = model.normals;
		indices = model.indices;
	}
	void BaseRenderer::SetShaderResources(const ShaderUniforms& resource)
	{
		shaderProgram.BindUniforms(resource);
	}
	void BaseRenderer::VertexShaderStage()
	{
		clipPositions.resize(vertices.size());
		worldNormals.resize(vertices.size());

		for (size_t i = 0; i < vertices.size(); ++i) {
			ShaderAttributes shaderAttributes{};
			shaderAttributes.Position = vertices[i];
			shaderAttributes.Normal = normals[i];

			shaderProgram.BindVertexAttribute(shaderAttributes);
			shaderProgram.VertexShaderMain();

			clipPositions[i] = shaderProgram.GetShaderBuiltin().Position;
			worldNormals[i] = shaderProgram.GetShaderBuiltin().worldNormal;
			
			LOGD("In: {} Clip: {}, worldNormal: {}", 
				glm::to_string(vertices[i]).c_str(), 
				glm::to_string(clipPositions[i]).c_str(),
				glm::to_string(worldNormals[i]).c_str()
			);
		}
	}
	void BaseRenderer::PrimitiveAssembly()
	{
		// Case: Primitive is Triangle
		primitives.resize(indices.size() / 3);

		for (size_t i = 0; i < primitives.size(); ++i) {
			primitives[i].indices[0] = indices[i * 3];
			primitives[i].indices[1] = indices[i * 3 + 1];
			primitives[i].indices[2] = indices[i * 3 + 2];
			primitives[i].discard = false;
		}
	}
	void BaseRenderer::PerspectiveDivide()
	{
		screenPositions.resize(clipPositions.size());
		for (size_t i = 0; i < clipPositions.size(); ++i)
		{
			screenPositions[i] = clipPositions[i];
			float invW = 1.f / screenPositions[i].w;
			screenPositions[i] *= invW;
			screenPositions[i].w = invW;

			LOGD("PerspectiveDivide: {}",
				glm::to_string(screenPositions[i]).c_str()
			);
		}


	}
	void BaseRenderer::ViewportTransform()
	{
		/*
			 y
			 |
			 |
			o|______x
		*/
		for (size_t i = 0; i < screenPositions.size(); ++i)
		{
			screenPositions[i] *= viewport.innerP;
			screenPositions[i] += viewport.innerO;
			LOGD("ViewportTransform: {}",
				glm::to_string(screenPositions[i]).c_str()
			);
		}
	}
	void BaseRenderer::Rasterization()
	{
		for (auto& triangle : primitives) {
			
			// Case: Polygons Mode == GL_FILL (Only Triangle)
			if (triangle.discard) {
				continue;
			}
			BoundingBox bound = CalculateTriangleBoundingBox(triangle, viewport.width, viewport.height);
			LOGD("Xmin:{} Xmax:{}", bound.min.x, bound.max.x);
			LOGD("Ymin:{} Ymax:{}", bound.min.y, bound.max.y);

			for (int x = bound.min.x; x < bound.max.x; ++x) {
				for (int y = bound.min.y; y < bound.max.y; ++y) {
					
					// (x, y): fragment coordinate
					// (x+0.5, y+0.5): sample point
					glm::vec4 barycentric;
					if (CalculateBarycentric(x + 0.5, y + 0.5, triangle, barycentric)) {

						size_t idx = x + y * viewport.width;

						// z: Depth after Perspective Divide
						// w: Inverse Camera Space Depth: 1/{z_camera}
						std::array<glm::vec3, 3> screenZW = {
							glm::vec3(screenPositions[triangle.indices[0]].zw, 0.0f),
							glm::vec3(screenPositions[triangle.indices[1]].zw, 0.0f),
							glm::vec3(screenPositions[triangle.indices[2]].zw, 0.0f)
						};
						// fragZW.xy value is the interpolate result
						glm::vec3 fragZW;
						Interpolate(screenZW, fragZW, barycentric);
						
						// Barycentric Correction
						glm::vec4 invW(
							screenPositions[triangle.indices[0]].w,
							screenPositions[triangle.indices[1]].w,
							screenPositions[triangle.indices[2]].w, 1.0f);
						barycentric *= 1.f / fragZW[1] * invW;

						// Do Early-Z Here

						// float z = 1.f / fragZW[1];
						std::array<glm::vec3, 3> propertiesIn = {
							worldNormals[triangle.indices[0]],
							worldNormals[triangle.indices[1]],
							worldNormals[triangle.indices[2]]
						};
						ShaderVaryings varyings{};
						Interpolate(propertiesIn, varyings.Normal, barycentric);
						// FragmentShaderStage
						shaderProgram.BinderShaderVarying(varyings);
						shaderProgram.FragmentShaderMain();

						glm::vec4 fragColor = shaderProgram.GetShaderBuiltin().FragColor;

						// Z-Test
						if (fragZW[0] < depthBuffer[idx]) {
							depthBuffer[idx] = fragZW[0];
							colorAttachment[idx * 3] = static_cast<uint8_t>(255*fragColor.x);
							colorAttachment[idx * 3 + 1] = static_cast <uint8_t>(255*fragColor.y);
							colorAttachment[idx * 3 + 2] = static_cast <uint8_t>(255*fragColor.z);
						}
					}
				}
			}
		}
		stbi_flip_vertically_on_write(true);
		stbi_write_png(RESOURCES_DIR"output.png", viewport.width, viewport.height, 3, colorAttachment.data(), viewport.width*3);
	}

	void BaseRenderer::FragmentShaderStage()
	{

	}

	BoundingBox BaseRenderer::CalculateTriangleBoundingBox(const Primitive& triangle, float width, float height)
	{
		
		std::array<glm::vec4, 3> verts = {
			screenPositions[triangle.indices[0]],
			screenPositions[triangle.indices[1]],
			screenPositions[triangle.indices[2]],
		};

		float minX = std::min(std::min(verts[0].x, verts[1].x), verts[2].x);
		float minY = std::min(std::min(verts[0].y, verts[1].y), verts[2].y);
		float maxX = std::max(std::max(verts[0].x, verts[1].x), verts[2].x);
		float maxY = std::max(std::max(verts[0].y, verts[1].y), verts[2].y);

		minX = std::max(minX - 0.5f, 0.f);
		minY = std::max(minY - 0.5f, 0.f);
		maxX = std::min(maxX + 0.5f, width - 1.f);
		maxY = std::min(maxY + 0.5f, height - 1.f);

		glm::vec3 min (minX, minY, 0.f);
		glm::vec3 max (maxX, maxY, 0.f);
		return { min, max };
	}

	bool BaseRenderer::CalculateBarycentric(float x, float y, const Primitive& triangle, glm::vec4& barycentric)
	{
		std::array<glm::vec4, 3> verts = {
			screenPositions[triangle.indices[0]],
			screenPositions[triangle.indices[1]],
			screenPositions[triangle.indices[2]],
		};
		std::array<glm::vec4, 4> vertsFlat = {
			glm::vec4(verts[2].x, verts[1].x, verts[0].x, 0.f),
			glm::vec4(verts[2].y, verts[1].y, verts[0].y, 0.f),
			glm::vec4(verts[0].z, verts[1].z, verts[2].z, 0.f),
			glm::vec4(verts[0].w, verts[1].w, verts[2].w, 0.f),
		};

		glm::vec3 u = glm::cross(
			glm::vec3(vertsFlat[0]) - glm::vec3(verts[0].xx, x),
			glm::vec3(vertsFlat[1]) - glm::vec3(verts[0].yy, y)
		);

		if (std::abs(u.z) < FLT_EPSILON) {
			return false;
		}
		u /= u.z;
		barycentric = glm::vec4(1.f - (u.x + u.y), u.y, u.x, 0.f );

		if (barycentric.x < 0 || barycentric.y < 0 || barycentric.z < 0) {
			return false;
		}
		return true;
	}

	void BaseRenderer::Interpolate(const std::array<glm::vec3, 3> propertiesIn, glm::vec3& propertyOut, const glm::vec4& barycentric)
	{
		propertyOut = glm::vec3(propertiesIn[0] * barycentric.x + propertiesIn[1] * barycentric.y + propertiesIn[2] * barycentric.z);
	}

}