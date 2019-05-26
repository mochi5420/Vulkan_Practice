#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "AppBase.h"

const int windowWidth = 1280;
const int windowHeight = 720;
const char* appTitle = "VulkanPractice";


int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// 使用しない変数の警告を回避
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// glfwの設定
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, 0);
	auto window = glfwCreateWindow(windowWidth, windowHeight, appTitle, nullptr, nullptr);

	// Vulkanの初期化
	AppBase app;
	app.Initialize(window, appTitle);

	while (glfwWindowShouldClose(window) == GLFW_FALSE)
	{
		// マウス操作などのイベントを取り出し記録する
		glfwPollEvents();

		// 描画する
		app.Render();
	}

	// 終了処理
	app.Terminate();
	glfwTerminate();
	return 0;
}