#ifndef APPLICATION
#define APPLICATION

struct GLFWwindow;

class Application
{
public:

    Application(int width, int height);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

	void Run();

private:

    void InitializeWindow();
    void InitializeVulkan();

    int m_Width;
    int m_Height;
    GLFWwindow* m_Window;
};

#endif