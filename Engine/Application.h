#pragma once

#include <vector>

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

class Module;
class ModuleWindow;

class Application
{
public:

	ModuleWindow* window;
	

private:

	std::vector<Module*> list_modules;
	

public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

private:

	void AddModule(Module* module);
};