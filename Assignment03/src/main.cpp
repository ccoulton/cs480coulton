#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <chrono>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
using namespace std;

//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex //https://github.com/ccoulton/cs480coulton.git
{
    GLfloat position[3];
    GLfloat color[3];
};

//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;
GLuint vbo_geometry2;// VBO handle for our geometry
float isRotate = 0.0;
int orbit = 1;
//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader
GLint loc_mvpmat2;
//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 model2;
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye position of the camera
glm::mat4 projection;//eye->clip lens of the camera
glm::mat4 mvp;//premultiplied modelviewprojection
glm::mat4 mvp2;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state, int x, int y);
void Rotation_menu(int id);
void top_menu(int id);

//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

const char *shaderloader(char *input);

float Rotate(float radians);

//--Main
int main(int argc, char **argv)
{
    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Assignment03- Moons");
	
    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
	glutMouseFunc(mouse);//Called if there is mouse input
	//glutCreateMenu(Menu);
	int sub_menu = glutCreateMenu(Rotation_menu);
	glutAddMenuEntry("Start CCW Rotation", 1);
	glutAddMenuEntry("Start CLW Rotation", 2);
	glutAddMenuEntry("Stop Rotation", 3);
	glutCreateMenu(top_menu);
	glutAddSubMenu("Rotation options", sub_menu);
	glutAddMenuEntry("Quit", 2);
	//sub_menu = glut
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    cleanUp();
    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    mvp = projection * view * model;
	mvp2= projection * view * model2;
	
    //enable the shader program
    glUseProgram(program);
	
    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
	
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));
	//draw first obj
    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count
	
	//upload second model to shader
	glUniformMatrix4fv(loc_mvpmat2, 1, GL_FALSE, glm::value_ptr(mvp2));
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry2);
	glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	
    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    //total time
    static float angle = 0.0;
    static float turn  = 0.0;
    float dt = getDT();// if you have anything moving, use dt.
    angle += dt * M_PI/2; //move through 90 degrees a second
    turn  += dt * M_PI*isRotate;
    //Rotate(3.14);
    
    model = glm::translate(glm::mat4(1.0f), glm::vec3(4.0 * sin(angle)*orbit, 0.0, 4.0 * cos(angle)*orbit));
    model = glm::rotate(model,turn , glm::vec3(0.0,1.0,0.0));
    model2= glm::translate(model, glm::vec3(3.0* sin(angle*2), 0.0, 3.0 * cos(angle*2)));
    model2= glm::rotate(model2, turn/2, glm::vec3(0.0,1.0,0.0));
    // Update the state of the scene
    glutPostRedisplay();//call the display callback
}


void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    if((key == 27)||(key == 'q')||(key == 'Q'))//ESC
        exit(0);
    else if ((key == 'm')||(key == 'M')) //menu
    	Rotation_menu(1);
    else if ((key == 'a')||(key == 'A'))
    	Rotation_menu(2);
    else if ((key == 'z')||(key == 'Z'))
    	Rotation_menu(3);
    /*else if (key == 43){//increase speed
    	cout<<"+ being hit"<<endl;
    	cout<<isRotate<<endl;
    	if	(isRotate >= 0.0)	//if already spinning CCW increase rate
    		isRotate += 2.0;	//or if not spinning start CCW
    	else//(isRotate < 0.0)	//if already spinning CLW increase rate
    		isRotate -= 2.0;
    	}
    else if (key == 45){
    	if 	(isRotate < 0.0)	//if spinning CLW decrease speed
    		isRotate += 2.0;	
    	else//(isRotate > 0.0) //if spinning CCW decrease speed
    		isRotate -= 2.0;	//or if not spinning start CLW
    	}//decrease speed*/ //BLARG doesn't allow me to grab it + and -
}

void mouse(int button, int state, int x, int y){
	//Mouse handler
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{ //what does left button do?
		if (orbit<0)
			orbit = 1;
		else
			orbit = -1;
		}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		{ //what does right button do?
		sleep(0);
		}
	}

void Rotation_menu(int id){
	switch(id)
		{
		case 1: //ccw rotate
			isRotate = 45.0;
			break;
		case 2: //clw rotate
			isRotate =-45.0;
			break;
		case 3: //stop rotate
			isRotate = 0.0;
			break;
		}
	glutPostRedisplay();
	}
	
void top_menu(int id){
	switch (id)
		{
		case 1:
			Rotation_menu(id);
			break;
		case 2:
			exit(0);
			break;
		}
	glutPostRedisplay();
	}
	
			
bool initialize()
{
    // Initialize basic geometry and shaders for this example

    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    Vertex geometry[] = { {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},

                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}}
                        };
    Vertex geometry2[] ={ {{-.5, -.5, -.5}, {0.0, 0.0, 0.0}},
                          {{-.5, -.5, .5}, {0.0, 0.0, .5}},
                          {{-.5, .5, .5}, {0.0, .5, .5}},

                          {{.5, .5, -.5}, {.5, .5, 0.0}},
                          {{-.5, -.5, -.5}, {0.0, 0.0, 0.0}},
                          {{-.5, .5, -.5}, {0.0, .5, 0.0}},
                          
                          {{.5, -.5, .5}, {.5, 0.0, .5}},
                          {{-.5, -.5, -.5}, {0.0, 0.0, 0.0}},
                          {{.5, -.5, -.5}, {.5, 0.0, 0.0}},
                          
                          {{.5, .5, -.5}, {.5, .5, 0.0}},
                          {{.5, -.5, -.5}, {.5, 0.0, 0.0}},
                          {{-.5, -.5, -.5}, {0.0, 0.0, 0.0}},

                          {{-.5, -.5, -.5}, {0.0, 0.0, 0.0}},
                          {{-.5, .5, .5}, {0.0, .5, .5}},
                          {{-.5, .5, -.5}, {0.0, .5, 0.0}},

                          {{.5, -.5, .5}, {.5, 0.0, .5}},
                          {{-.5, -.5, .5}, {0.0, 0.0, .5}},
                          {{-.5, -.5, -.5}, {0.0, 0.0, 0.0}},

                          {{-.5, .5, .5}, {0.0, .5, .5}},
                          {{-.5, -.5, .5}, {0.0, 0.0, .5}},
                          {{.5, -.5, .5}, {.5, 0.0, .5}},
                          
                          {{.5, .5, .5}, {.5, .5, .5}},
                          {{.5, -.5, -.5}, {.5, 0.0, 0.0}},
                          {{.5, .5, -.5}, {.5, .5, 0.0}},

                          {{.5, -.5, -.5}, {.5, 0.0, 0.0}},
                          {{.5, .5, .5}, {.5, .5, .5}},
                          {{.5, -.5, .5}, {.5, 0.0, .5}},

                          {{.5, .5, .5}, {.5, .5, .5}},
                          {{.5, .5, -.5}, {.5, .5, 0.0}},
                          {{-.5, .5, -.5}, {0.0, .5, 0.0}},

                          {{.5, .5, .5}, {.5, .5, .5}},
                          {{-.5, .5, -.5}, {0.0, .5, 0.0}},
                          {{-.5, .5, .5}, {0.0, .5, .5}},

                          {{.5, .5, .5}, {.5, .5, .5}},
                          {{-.5, .5, .5}, {0.0, .5, .5}},
                          {{.5, -.5, .5}, {.5, 0.0, .5}}
                        };
    // Create a Vertex Buffer object to store this vertex info on the GPU*/
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);
	glGenBuffers(1, &vbo_geometry2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(geometry2), geometry2, GL_STATIC_DRAW);
    //--Geometry done*/

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    //TODO: make shader into a class so you can init and make stored shaders
    // Note the added uniform!
    char infile [12] = "shadervs.cg";
    const char *vs = shaderloader(infile);
	infile[6] = 'f';
    const char *fs = shaderloader(infile);

    //compile the shaders
    GLint shader_status;

    // Vertex shader first
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    }

    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    } //sed to store and transform them. 
	printf("shaders compiled\n");
    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program, /*sed to store and transform them.*/  const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }
    loc_mvpmat2= glGetUniformLocation(program, const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat2== -1)
    	{
    	std::cerr<< "[F] MVP2 NOT FOUND"<<std::endl;
    	return false;
    	}
    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
    glDeleteBuffers(1, &vbo_geometry2);
}

//returns the time delta
float getDT()
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}
//Loads shader definations in from files
const char *shaderloader(char *input){
	
	FILE* infile = fopen(input, "rb");   //Open file
  	if(fseek(infile, 0, SEEK_END) == -1) return NULL;
  	long size = ftell(infile);			//get file size
  	if(fseek(infile, 0, SEEK_SET) == -1) return NULL;
  	char *shader = (char*) malloc( (size_t) size +1  ); 
 
  	fread(shader, 1, (size_t)size, infile); //read from file into shader
  	if(ferror(infile)) {
    	free(shader);
    	return NULL;
  		}
 
  	fclose(infile);
  	shader[size] = '\0';
  	return shader;
	/*ifstream infile;
	infile.open(filename);
	char *shader = (char*) malloc((size_t) infile.gcout());
	if (!infile.is_open())
		cout<<"Error Opening file"<<endl;
	else{
		cout<<"File opened"<<endl;
		while(infile.good()){
			infile.read(shader, infile.gcout());
			}
		}
	shader[strlen(shader)] = '\0';
	const char *output = shader;
	infile.close();
	return(output);*/
	}
//total rotate = rotx*yawy*pitchz
float Rotate(float radians){
/* centerpos.x - pointx = calcx
   centerpos.y - pointy = calcy
   transx = calcx*cos(radians)-calcy*sin(radians)
   transy = calcy*cos(radians)+calcy*sin(radians)
   newx = centerpos.x + transx
   newy = centerpos.y + transy
   already part of the GLM libs! yay!*/
   return(0.0);
   }

