#include "AppBase.h"

static const int windowWidth = 1280;
static const int windowHeight = 720;
static const char* const appTitle = "Vulkan Practice";


int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// 使用しない変数の警告を回避
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	// glfwの設定
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, 0);
	auto window = glfwCreateWindow(windowWidth, windowHeight, appTitle, nullptr, nullptr);

	// Vulkanの初期化
	AppBase app;
	app.Initialize(window, appTitle);

	// メインループ
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