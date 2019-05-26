#include "AppBase.h"

static const int windowWidth = 1280;
static const int windowHeight = 720;
static const char* const appTitle = "Vulkan Practice";


int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// �g�p���Ȃ��ϐ��̌x�������
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	// glfw�̐ݒ�
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, 0);
	auto window = glfwCreateWindow(windowWidth, windowHeight, appTitle, nullptr, nullptr);

	// Vulkan�̏�����
	AppBase app;
	app.Initialize(window, appTitle);

	// ���C�����[�v
	while (glfwWindowShouldClose(window) == GLFW_FALSE)
	{
		// �}�E�X����Ȃǂ̃C�x���g�����o���L�^����
		glfwPollEvents();

		// �`�悷��
		app.Render();
	}

	// �I������
	app.Terminate();
	glfwTerminate();
	return 0;
}