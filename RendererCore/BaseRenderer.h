#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "ShaderProgram.h"


namespace Poorenderer {
	
	class Model;
	class BoundingBox;
	struct Viewport {
		float x;
		float y;
		float width;
		float height;
		float minDepth;
		float maxDepth;
		// ref: https://registry.khronos.org/vulkan/specs/1.0/html/chap24.html#vertexpostproc-viewport
		glm::vec4 innerO;
		glm::vec4 innerP;

		float absMinDepth;
		float absMaxDepth;
	};
	struct Primitive {
		// Only Primitive Type: Triangle

		bool discard = false;
		bool frontFacing = true;
		size_t indices[3] = {0, 0, 0};

	};
	class BaseRenderer
	{
	public:

		/*
		* SetAttachments

		 ---General Graphics Pipeline---:
		* BeginRenderPass
		* SetViewPort		    [ ✓ ]
		* SetVertexArrayObject  [ ✓ ]
		* SetShaderProgram      [  ]
		* setShaderResources    [ ✓ ]
		* SetPipelineStates		[  ]
		* Draw 
			* VertexShaderStage [ ✓ ]
			* PrimitiveAssembly [ ✓ ]
			* Clipping			[  ]: [TODO]: Build BVH and do clipping by Frustum 
			* PerspectiveDivide [ ✓ ]
			* ViewportTransform [ ✓ ]
			* FaceCulling		[  ]
			* Rasterization
				* FragmentShaderStage 
		* EndRenderPass
		*/


		void SetAttachments(size_t width, size_t height);

		void SetViewport(int x, int y, int width, int height);
		void SetVertexArrayObject(const Model& model);
		// Only Implement the Uniform Resource
		void SetShaderResources(const ShaderUniforms &resource);
		void VertexShaderStage();

		void PrimitiveAssembly();
		void Clipping();
		void PerspectiveDivide();
		void ViewportTransform();

		void FaceCulling();

		virtual void Rasterization();

		// void RasterizationTriangle();
		// void FragmentShaderStage();

		inline void* GetColorAttachmentData() { return colorAttachment.data(); }

	protected:
		BoundingBox CalculateTriangleBoundingBox(const Primitive& triangle, float width, float height);
		// Calculate Barycentric, return true if Point is inside traingle
		// now barycentric is not correct (screen space triange's barycentric)
		bool CalculateBarycentric(float x, float y, const Primitive& triangle, glm::vec4& barycentric);
		// Interpolate Properties by Barycentric (Get ShaderVaryings)
		void Interpolate(const std::array<glm::vec3,3> propertiesIn, glm::vec3 &propertyOut, const glm::vec4& barycentric);


		Viewport viewport;

		// Vertex Array Object
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<uint32_t> indices;

		// Shader Program
		ShaderProgram shaderProgram;

		// After VertexShaderStage:
		std::vector<glm::vec4> clipPositions; 	// Clip Space Vertex Positions 
		std::vector<glm::vec3> worldNormals;	// World Space Normals

		// Primitives
		std::vector<Primitive> primitives;

		// Screen Space Vertex Positions
		std::vector<glm::vec4> screenPositions;


		// Framebuffer
		std::vector<uint8_t> colorAttachment;
		std::vector<float> depthBuffer;
		
		
	};


}