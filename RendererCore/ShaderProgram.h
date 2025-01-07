#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "logger.h"

namespace Poorenderer {

	struct ShaderAttributes {
		glm::vec3 Position;
		glm::vec3 Normal;
	};
	struct ShaderUniforms
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat3 inverseTransposeModel;
		glm::vec3 light;
	};

	// Rasterization interpolation Properties
	struct ShaderVaryings
	{
		glm::vec3 Normal;

	};
	struct ShaderBuiltin {
		// Vertex Shader Output
		glm::vec4 Position = glm::vec4{ 0.f };
		glm::vec3 worldNormal;
		
		// Fragment Shader Input
		glm::vec4 FragCoord;

		// Fragment Shader Output
		glm::vec4 FragColor;
	};

	class ShaderProgram
	{
	public:
		// Vertex Shader
		void BindVertexAttribute(const ShaderAttributes &attribute)
		{
			in = attribute;
		}
		void VertexShaderMain()
		{
			gl.Position = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(in.Position, 1.0);
			gl.worldNormal = uniforms.inverseTransposeModel * in.Normal;
		}

		// Fragment Shader
		void BinderShaderVarying(const ShaderVaryings& varyings) 
		{
			v = varyings;
		}
		void FragmentShaderMain()
		{
			float intensity = std::max(0.0f, glm::dot(glm::normalize(v.Normal), glm::normalize(uniforms.light)));
			gl.FragColor = glm::vec4(intensity, intensity, intensity, 1.0);
		}
		void BindUniforms(const ShaderUniforms& uniformbuffer)
		{
			uniforms = uniformbuffer;
		}
		inline ShaderBuiltin GetShaderBuiltin() const {
			return gl;
		}

	private:
		ShaderAttributes in;
		ShaderUniforms uniforms;
		ShaderBuiltin gl;
		ShaderVaryings v;

	};

}