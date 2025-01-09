#include "HierarchicalZBufferRenderer.h"
#include "BoundingBox.h"

#include <stb_image_write.h>

#include <array>

namespace Poorenderer {
	void HierarchicalZBufferRenderer::SetAttachments(size_t width, size_t height)
	{
		colorAttachment.resize(width * height * 3, 127);
		//Create Depth Mipmap
		depthMipmap = DepthMipmap(width, height);
	}
	void HierarchicalZBufferRenderer::Rasterization()
	{
		for (auto& triangle : primitives) {
			if (triangle.discard) {
				continue;
			}
			const glm::vec4& v0 = screenPositions[triangle.indices[0]];
			const glm::vec4& v1 = screenPositions[triangle.indices[1]];
			const glm::vec4& v2 = screenPositions[triangle.indices[2]];

			BoundingBox bound = CalculateTriangleBoundingBox(triangle, viewport.width, viewport.height);
			float minDepth = std::min(std::min(v0.z, v1.z), v2.z);

			// Note: Depth mipmap stores the max depth of a screen area
			// If is the area of bound in depth mipmap has depth value less than `minDepth`,
			// it means that the area will occlude the bound.
			// if( occluded ) continue;
			if (depthMipmap.IsOccluded(bound, minDepth)) {
				LOGD("Occlusion Happened!");
				continue;
			}

			// Below Almostly Same with BaseRenderer
			for (int x = bound.min.x; x <= bound.max.x; ++x) {
				for (int y = bound.min.y; y <= bound.max.y; ++y) {

					// (x, y): fragment coordinate
					// (x+0.5, y+0.5): sample point
					glm::vec4 barycentric;
					if (CalculateBarycentric(x + 0.5, y + 0.5, triangle, barycentric)) {

						size_t idx = x + y * viewport.width;

						// z: Depth after Perspective Divide
						// w: Inverse Camera Space Deph: 1/{z_camera}
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
						if (fragZW[0] < depthMipmap.GetDepth(x, y)) {

							//depthBuffer[idx] = fragZW[0];
							//Update Depth Mipmap
							depthMipmap.Update(x, y, fragZW[0]);
							colorAttachment[idx * 3] = static_cast<uint8_t>(255 * fragColor.x);
							colorAttachment[idx * 3 + 1] = static_cast <uint8_t>(255 * fragColor.y);
							colorAttachment[idx * 3 + 2] = static_cast <uint8_t>(255 * fragColor.z);
						}
					}
				}
			}
		}

		stbi_flip_vertically_on_write(true);
		stbi_write_png((RESOURCES_DIR + FileName).c_str(), viewport.width, viewport.height, 3, colorAttachment.data(), viewport.width * 3);
	}
}