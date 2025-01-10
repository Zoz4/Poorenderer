#include <iostream>
#include <memory>
#include "RendererCore/Logger.h"
#include "RendererCore/Model.h"
#include "RendererCore/BaseRenderer.h"
#include "RendererCore/ScanLineRenderer.h"
#include "RendererCore/HierarchicalZBufferRenderer.h"

#include <glm/mat3x3.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/gl.h>
#include <GLFW/glfw3.h>



int main(void)
{
	//spdlog::set_pattern(LOGGER_FORMAT);
	spdlog::set_level(spdlog::level::level_enum(SPDLOG_ACTIVE_LEVEL));

	LOGI("Start...");

	//Poorenderer::Model model(RESOURCES_DIR "triangle.obj");
	Poorenderer::Model model(RESOURCES_DIR "bunny.obj");
	//Poorenderer::Model model(RESOURCES_DIR "homer.obj"); 
	//Poorenderer::Model model(RESOURCES_DIR "happy_buddha.obj");
	//Poorenderer::Model model(RESOURCES_DIR "xyzrgb_dragon.obj");

    std::shared_ptr<Poorenderer::BaseRenderer> render = std::make_shared<Poorenderer::HierarchicalZBufferRenderer>();
	render->SetOutputFileName("output.png");

	const int WIDTH = 800;
	const int HEIGHT = 600;

	render->SetAttachments(WIDTH, HEIGHT);

	render->SetViewport(0, 0, WIDTH, HEIGHT);

	render->SetVertexArrayObject(model);

	// [TODO]: Camera
	glm::vec3 eye(0.0f, 0.0f, 3.0f);
	glm::vec3 center(0.0f, 0.0f, -1.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	constexpr float fov = glm::radians(45.0f);
	constexpr float aspect = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
	constexpr float near = 0.1f;
	constexpr float far = 100.0f;

	Poorenderer::ShaderUniforms uniform{};
	//uniform.model = glm::mat4(1.0); // triangle.obj
	uniform.model = glm::scale(glm::translate(glm::rotate(glm::mat4(1.0), glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f)), glm::vec3(0.35, -1, 0)), glm::vec3(10, 10, 10)); // bunny.obj
	//uniform.model = glm::scale(glm::translate(glm::rotate(glm::mat4(1.0), glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f)), glm::vec3(-0.5, -0.5, 1.1)), glm::vec3(1, 1, 1)); // homer.obj
	//uniform.model = glm::scale(glm::translate(glm::rotate(glm::mat4(1.0), glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f)), glm::vec3(0, -1.5, 0)), glm::vec3(10, 10, 10)); // happy_buddha.obj
	//uniform.model = glm::scale(glm::translate(glm::rotate(glm::mat4(1.0), glm::radians(230.f), glm::vec3(0.f, 1.f, 0.f)), glm::vec3(-0.5, 0, 0)), glm::vec3(0.012, 0.012, 0.012)); // xyzrgb_dragon.obj
	uniform.view = glm::lookAt(eye, center, up);
	uniform.projection = glm::perspective(fov, aspect, near, far);
	uniform.inverseTransposeModel = glm::mat3(glm::transpose(glm::inverse(uniform.model)));
	uniform.light = glm::normalize(glm::vec3(1.0, 1.0, 1.0));

	render->SetShaderResources(uniform);

	// Draw 
	{
		render->VertexShaderStage();
		render->PrimitiveAssembly();
		render->Clipping();
		render->PerspectiveDivide();
		render->ViewportTransform();
		LOGI("First Rasterization Start....");
		render->Rasterization();
		LOGI("First Rasterization End....\n");
		LOGI("Second Rasterization Start....");
		render->Rasterization();
		LOGI("Second Rasterization End....");
		LOGI("Third Rasterization Start....");
		render->Rasterization();
		LOGI("Third Rasterization End....");
	}

	LOGI("End...");

	// Visualization
	{
		if (!glfwInit()) {
			LOGE("Failed to initialize GLFW");
			return -1;
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Poorenderer", NULL, NULL);
		if (window == NULL)
		{
			LOGE("Failed to create GLFW window");
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(window);

		gladLoadGL(glfwGetProcAddress);
		glViewport(0, 0, WIDTH, HEIGHT);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		while (!glfwWindowShouldClose(window)) {

			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, render->GetColorAttachmentData());
			glfwPollEvents();
			glfwSwapBuffers(window);
		}
		glfwTerminate();
	}
	return 0;
}