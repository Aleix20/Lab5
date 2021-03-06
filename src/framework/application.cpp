#include "application.h"
#include "utils.h"
#include "image.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"
#include "material.h"

Camera* camera = NULL;
Mesh* mesh = NULL;
Matrix44 model_matrix;

Shader* shader_actual = NULL;
Shader* shader = NULL;
Shader* shader2 = NULL;
Shader* shader3 = NULL;
Shader* shader4 = NULL;
Texture* texture = NULL;
Texture* texture2 = NULL;
Light* light = NULL;

Vector3 ambient_light(0.6, 0.6, 0.6);
Material* materials[3];
bool multipleModels = false;
int nmodels = 0;

Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	
	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(Vector3(0,20,20),Vector3(0,10,0),Vector3(0,1,0));
	camera->setPerspective(60,window_width / window_height,0.1,10000);

	//then we load a mesh
	mesh = new Mesh();
	mesh->loadOBJ("../res/meshes/lee.obj");

	//load the texture
	texture = new Texture();
	texture2 = new Texture();
	if(!texture->load("../res/textures/lee_color_specular.tga")|| !texture2->load("../res/textures/lee_normal.tga"))
	{
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}

	//we load a shader
	shader = Shader::Get("../res/shaders/texture.vs","../res/shaders/texture.fs");
	shader2= Shader::Get("../res/shaders/phong.vs", "../res/shaders/phong.fs");
	shader3 = Shader::Get("../res/shaders/phongW.vs", "../res/shaders/phongW.fs");
	shader4 = Shader::Get("../res/shaders/phongWN.vs", "../res/shaders/phongWN.fs");
	shader_actual = shader;
	//load whatever you need here
	light = new Light();
	materials[0] = new Material();
	materials[1] = new Material(Vector3{ 1.0, 0.5, 0.31 }, Vector3{ 1.0, 0.5, 0.31 }, Vector3{0.5,0.5,0.5}, 30.0);
	materials[2] = new Material(Vector3{ 0.2, 1.0, 0.31 }, Vector3{ 0.2, 1.0, 0.31 }, Vector3{ 0.5,0.5,0.5 }, 30.0);
}

//render one frame
void Application::render(void)
{
	if (!multipleModels) { //Si no estem en el cas de renderitzar m?ltiples models, el n?mero de models ser? 1
		nmodels = 1;
	}
	else {
		nmodels = 3;
	}
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable( GL_DEPTH_TEST );

	//Get the viewprojection
	camera->aspect = window_width / window_height;
	Matrix44 viewprojection = camera->getViewProjectionMatrix();
	
	//enable the shader
	shader_actual->enable();

	//Enviem valors uniformes necessaris al shader actual
	shader_actual->setUniform3("camera_position", camera->eye);
	shader_actual->setUniform3("light_ambient", ambient_light);
	shader_actual->setUniform3("light_position", light->position);
	shader_actual->setUniform3("light_diffuse", light->diffuse_color);
	shader_actual->setUniform3("light_specular", light->specular_color);
	shader_actual->setTexture("color_texture", texture, 0 ); //set texture in slot 0
	shader_actual->setTexture("texture_normal", texture2, 1);

	//Funci? per renderitzar models
	renderModels(viewprojection, nmodels);
	

	//disable shader
	shader_actual->disable();

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}
void Application::renderModels(Matrix44& viewprojection, int nmodels)
{
	//enviem tantes model_matrix com models, despla?ades minimament en l'eix x i les dividim entre dos per fer un a cada costat
	for (int i = 0; i <= nmodels/2; i++) {
		//Enviem la matrix model per cada mesh
		Matrix44 model_matrix;
		model_matrix.setIdentity();
		
		//desplacem els models
		model_matrix.translate(i * -17, 0, -10*i); 
		
		shader_actual->setMatrix44("model", model_matrix); //upload the transform matrix to the shader
		shader_actual->setMatrix44("viewprojection", viewprojection);//upload viewprojection info to the shader
		
		//Deixem el material per defecte
		if (i==0) {
			shader_actual->setUniform3("material_ambient", materials[0]->ambient);
			shader_actual->setUniform3("material_diffuse", materials[0]->diffuse);
			shader_actual->setUniform3("material_specular", materials[0]->specular);
			shader_actual->setFloat("material_shininess", materials[0]->shininess);
		}
		//Enviem un material diferent
		else {
			
			shader_actual->setUniform3("material_ambient", materials[2]->ambient);
			shader_actual->setUniform3("material_diffuse", materials[2]->diffuse);
			shader_actual->setUniform3("material_specular", materials[2]->specular);
			shader_actual->setFloat("material_shininess", materials[2]->shininess);
		}
		mesh->render(GL_TRIANGLES);
	}
	for (int i = 0; i <= nmodels / 2; i++) {
		Matrix44 model_matrix;
		model_matrix.setIdentity();

		model_matrix.translate(i * 17, 0, -10*i); //example of translation

		shader_actual->setMatrix44("model", model_matrix); //upload the transform matrix to the shader
		shader_actual->setMatrix44("viewprojection", viewprojection);//upload viewprojection info to the shader
		if (i == 0) {
			shader_actual->setUniform3("material_ambient", materials[0]->ambient);
			shader_actual->setUniform3("material_diffuse", materials[0]->diffuse);
			shader_actual->setUniform3("material_specular", materials[0]->specular);
			shader_actual->setFloat("material_shininess", materials[0]->shininess);
		}
		else {
		
			shader_actual->setUniform3("material_ambient", materials[1]->ambient);
			shader_actual->setUniform3("material_diffuse", materials[1]->diffuse);
			shader_actual->setUniform3("material_specular", materials[1]->specular);
			shader_actual->setFloat("material_shininess", materials[1]->shininess);
		}
		mesh->render(GL_TRIANGLES);
	}
}
//called after render
void Application::update(double seconds_elapsed)
{
	if (keystate[SDL_SCANCODE_SPACE])
	{
		model_matrix.rotateLocal(seconds_elapsed,Vector3(0,1,0));
	}

	if (keystate[SDL_SCANCODE_RIGHT])
		camera->eye = camera->eye + Vector3(1, 0, 0) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_LEFT])
		camera->eye = camera->eye + Vector3(-1, 0, 0) * seconds_elapsed * 10.0;
	if (keystate[SDL_SCANCODE_UP])
		camera->eye = camera->eye + Vector3(0, 1, 0) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_DOWN])
		camera->eye = camera->eye + Vector3(0, -1, 0) * seconds_elapsed * 10.0;
}

//keyboard press event 
void Application::onKeyPressed( SDL_KeyboardEvent event )
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode){
		case SDL_SCANCODE_R: Shader::ReloadAll(); break;
        case SDL_SCANCODE_ESCAPE: exit(0); break; //ESC key, kill the app
		case SDL_SCANCODE_1:
			shader_actual = shader;
			break;

		case SDL_SCANCODE_2:
			shader_actual = shader2;
			break;

		case SDL_SCANCODE_3:
			shader_actual = shader3;
			break;

		case SDL_SCANCODE_4:
			shader_actual = shader4;
			break;
		case SDL_SCANCODE_M :
			if (!multipleModels) {
				multipleModels = true;
			}
			else {
				multipleModels = false;
			}
			break;
	}

}

//mouse button event
void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{

	}
}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{

	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}
