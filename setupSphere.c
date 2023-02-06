/*  -------------------------------
	Working command to build this:
	-------------------------------

	gcc `pkg-config --cflags gtk+-3.0` -o setupSphere setupSphere.c Sphere.c vec.c `pkg-config --libs gtk+-3.0` -lglut -lGL -lglfw3 -ldl -lm -lGL -lGLU -lX11 -pthread
	./setupSphere

    For debugging **only**, use this:
    gcc `pkg-config --cflags gtk+-3.0`  -o setupSphere -g setupSphere.c Sphere.c vec.c `pkg-config --libs gtk+-3.0` -lglut -lGL -lglfw3 -ldl -lm -lGL -lGLU -lX11 -pthread
    gdb setupSphere

	This is a dummy application representing the layout of both the plasmas application and the heart application (the basics).
	Demo is basically just a menu at the top, a GLArea in the middle, and a menu at the bottom. Both menus just have some labels
	to illustrate where the real projects will actually display real values and/or buttons that actually have useful information
	or actually perform some real action.
*/

#include <gtk/gtk.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "Sphere.h"
#include <cglm/cglm.h>

void init_buffer_objects(GError *error);
void init_shaders(GError *error);

static void on_realize (GtkGLArea *area);
static gboolean render(GtkGLArea *area, GdkGLContext *context);

float cameraDistance = 4.0f;
unsigned int shaderProgram;
int screenWidth, screenHeight;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

unsigned int attribVertexPosition;
mat4 matrixModelView;
mat4 matrixProjection;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

double xValue, yValue, zValue;

unsigned int VAO, vboId1, iboId1;
// unsigned int VAO2, vboId2, iboId2;
struct Sphere *sphere1; //(1.0f, 8, 8, false);
//struct Sphere *sphere2;

float cameraAngleX;
float cameraAngleY;
GtkWidget *gl_area;

// Vertex shader
/*
    We aren't utilizing textures, and are just going to draw an orange sphere.
    So, we only need access to the model, view, and projection matrices.
*/
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n\n"
    "void main()\n"
    "{\n"
    " gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\0";

// Fragment shader
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

static void print_hello(GtkWidget *widget, gpointer data) {
	g_print("Hello World\n"); // print to a terminal if the application was started from one.
}

static gboolean scale_changedX (GtkRange *range, gpointer data)
{
    xValue = gtk_range_get_value (range);
    g_print("Some information...%f\n", xValue);
    gtk_gl_area_queue_render(gl_area);

    return TRUE;
}

static gboolean scale_changedY (GtkRange *range, gpointer data)
{
    yValue = gtk_range_get_value (range);
    g_print("Some information...%f\n", yValue);
    gtk_gl_area_queue_render(gl_area);

    return TRUE;
}

static gboolean scale_changedZ (GtkRange *range, gpointer data)
{
    zValue = gtk_range_get_value (range);
    g_print("Some information...%f\n", zValue);
    gtk_gl_area_queue_render(gl_area);

    return TRUE;
}

/*
	So basically this is going to be the method where we are putting the rendering stuff, where we redraw things.
*/
static gboolean render(GtkGLArea *area, GdkGLContext *context)
{
    glClearColor(0.2, 0.4, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO);
    // Bind these buffers so we use the data in them.
    glBindBuffer(GL_ARRAY_BUFFER, vboId1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId1);

    // For the vertex attribute, we need to set up how to read the data.
    // The sphere class provides this function to get the stride.
    int stride = getInterleavedStride(sphere1);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);

    // Set camera view.
    mat4 view = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    glm_translate(view, (vec3){0.0f, 0.0f, -cameraDistance});

    // Set model position.
    mat4 model = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    glm_rotate(model, (float)glfwGetTime() * 50.0f * 3.14159f/180.0f, (vec3){0.5f, 1.0f, 0.0f});
    glm_translate(model, (vec3){xValue/10.0f, yValue/10.0f, zValue/10.0f});

    // Pass model position to the shader.
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);

    // Pass camera position to the shader.
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);

    // from toPerspective() function
    const float N = 0.1f;
    const float F = 100.0f;
    const float DEG2RAD = 3.141592f / 180;
    const float FOV_Y = 40.0f * DEG2RAD;
    glViewport(0, 0, (GLsizei)SCR_WIDTH, (GLsizei)SCR_HEIGHT);
    float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    // perspective takes in (fovy, aspect, near, far)
    mat4 projection;
    glm_perspective(FOV_Y, aspectRatio, N, F, projection);
    // glm::mat4 projection = glm::perspective(FOV_Y, aspectRatio, N, F);
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);

    // Draw our sphere.
    glDrawElements(GL_TRIANGLES,            // primitive type
                getIndexCount(sphere1), // # of indices
                GL_UNSIGNED_INT,         // data type
                (void*)0);               // ptr to indices

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

	return TRUE;
}

/*
	This function sets up the window layout and adds all the controls.
*/
static void activate(GtkApplication *app, gpointer user_data)
{
	printf("hello activation....\n");
	// GtkWidget is the base class that all widgets in GTK+ derive from. Manages widget lifestyle, states, and style.
	GtkWidget *window = gtk_application_window_new(app);
	GtkWidget *grid = gtk_grid_new();
    GtkWidget *button = gtk_button_new_with_mnemonic ("_Exit");
	gl_area = gtk_gl_area_new();
    gtk_widget_set_hexpand(gl_area, TRUE);
    gtk_widget_set_vexpand(gl_area, TRUE);

	GtkWidget *labelXPos = gtk_label_new("X position:");
	GtkWidget *labelYPos = gtk_label_new("Y position:");
	GtkWidget *labelZPos = gtk_label_new("Z position:");

    GtkWidget *xScale = gtk_scale_new_with_range (
        GTK_ORIENTATION_HORIZONTAL,
        -10,      //gdouble min
        10,     //gdouble max
        0.5);     //gdouble step
    gtk_widget_set_hexpand (xScale, TRUE);
    GtkWidget *yScale = gtk_scale_new_with_range (
        GTK_ORIENTATION_HORIZONTAL,
        -10,      //gdouble min
        10,     //gdouble max
        0.5);     //gdouble step
    gtk_widget_set_hexpand (yScale, TRUE);
    GtkWidget *zScale = gtk_scale_new_with_range (
        GTK_ORIENTATION_HORIZONTAL,
        -10,      //gdouble min
        10,     //gdouble max
        0.5);     //gdouble step
    gtk_widget_set_hexpand (zScale, TRUE);
    g_signal_connect(G_OBJECT(xScale), "value-changed",
        G_CALLBACK(scale_changedX), NULL);
    g_signal_connect(G_OBJECT(yScale), "value-changed",
        G_CALLBACK(scale_changedY), NULL);
    g_signal_connect(G_OBJECT(zScale), "value-changed",
        G_CALLBACK(scale_changedZ), NULL);

    /*gtk_grid_set_column_homogeneous(grid, 0);
    gtk_grid_set_row_homogeneous(grid, 0);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 50);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 10);*/

    // gtk_gl_area_set_auto_render(gl_area, 1);

	/* -------- Customizing a few things: setting title, changing size of window, entering the window on the screen --------*/

	// doing this: GTK_WINDOW(window) casts the window (which is a pointer to a GtkWidget object) to a GtkWindow - GTK_WINDOW is a macro.

	gtk_window_set_title(GTK_WINDOW(window), "GTK Sphere Tutorial");	// specify window with GTK_WINDOW(window), and pass title to display
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);	// specify window with GTK_WINDOW(window), and pass width, height
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // Tried grid layout. It's trash.
	//gtk_grid_attach(GTK_GRID(grid), button,         0, 0, 1, 1);
	int rowSpanOfGLArea = 50;  // Going to have to play around with this...GTK is a little unfriendly in terms of sizing.
	gtk_grid_attach(GTK_GRID(grid), gl_area,        0, 0, 2, 1);
	gtk_grid_attach(GTK_GRID(grid), labelXPos,      0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), xScale,         1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), labelYPos,      0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), yScale,         1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), labelZPos,      0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), zScale,         1, 3, 1, 1);



	// Some demo button functionality.
	g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);  /* print_hello is the event handler, NULL because print_hello
										doesn't take any data.*/
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
	// the swapped version of g_signal,_connect allows the callback function to take a parameter passed in as data.

	// Connect to the render signal and connect to the realize signal, which are the render functions and initialization functions, respectively.
	g_signal_connect(gl_area, "render", G_CALLBACK (render), NULL);
	g_signal_connect(gl_area, "realize", G_CALLBACK(on_realize), NULL);

    /*GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_box_pack_start(GTK_BOX(vbox),label,0,0,0);
    gtk_box_pack_start(GTK_BOX(vbox),button,0,0,0);
    gtk_box_pack_start(GTK_BOX(vbox),gl_area,0,0,0);
    gtk_box_pack_start(GTK_BOX(vbox),labelXPos,0,0,0);
    gtk_box_pack_start(GTK_BOX(vbox),label3,0,0,0);

    gtk_container_add(GTK_CONTAINER(window),vbox); */



	gtk_container_add(GTK_CONTAINER(window), grid); /* this will actually add the button to the window (technically the button_box,
	//							 but the button_box contains the button */
	gtk_widget_show_all(window);
}

// Used to initialize OpenGL state, e.g. buffer objects or shaders.
static void on_realize (GtkGLArea *area)
{
	sphere1 = createSphere(1.0f, 8, 8);
    // sphere2 = createSphere(0.75f, 12, 12);
	// We need to make the context current if we want to call GL API
	gtk_gl_area_make_current (area);

	// If there were errors during the initialization or when trying to make the context current, this
	// function will return a GError for you to catch
	// If error is UnNULL, meaning it isn't not set (double negative), it is set, return and see what error is.
	if (gtk_gl_area_get_error (area) != NULL) {
		return;
	}

	// You can also use gtk_gl_area_set_error() in order to show eventual initialization errors on the
	// GtkGLArea widget itself
	GError *error = NULL;
	init_buffer_objects (&error);
	if (error != NULL)
	{
	    gtk_gl_area_set_error (area, error);
	    g_error_free (error);
	    return;
	}

	init_shaders (&error);
	if (error != NULL)
    {
      gtk_gl_area_set_error (area, error);
      g_error_free (error);
      return;
    }
	printf("OnRealize function ended\n");
}

/*
	This function has one purpose which is to buffer the triangle data--it does all the VAO and VBO stuff
	which to be fair, I'm not completely sure how it works in detail.
*/
void init_buffer_objects(GError *errorObj) {
	// ----- copied from setup.c
	// set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    /*float vertices[] = {
        // positions
        // colors
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f // top
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Tell OpenGL how the data is formatted for the vertex attribute
    // 0 - index of attribute, we are modifying vertex attribute which we set to index 0
    // 3 - size, so when we read, how many elements to read at a time.
    // GL_FLOAT - just specifies we are reading floats.
    // GL_FALSE - read them as fixed point values (whatever that means)
    // 6 * sizeof(float) = 24 - byte offset, that's how many bytes as a group to read at a time.
    // (void*)0 - pointer for element to start at, we are starting at 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Tell OpenGL how the data is formatted for the color attribute
    // Last attribute is different, because we are starting at 4th float, or index 3, which is byte 12.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    //glBindVertexArray(0);

    // bind the VAO (it was already bound, but just to demonstrate): seeing as we only have a single VAO we can
    // just bind it beforehand before rendering the respective triangle; this is another approach.
    glBindVertexArray(VAO);*/


    // from our sphereDemo
    glGenVertexArrays(1, &VAO);
    //glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &vboId1);
    //glGenBuffers(1, &vboId2);
    glGenBuffers(1, &iboId1);
    //glGenBuffers(1, &iboId2);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vboId1);
    //printf("To my suspect spot.\n");
    glBufferData(GL_ARRAY_BUFFER, getInterleavedVertexSize(sphere1), getInterleavedVertices(sphere1), GL_STATIC_DRAW);
    //printf("Past my suspect spot.\n");
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId1);
    //printf("To my second suspect spot\n");
    float * myindices = getIndices(sphere1);
    //printf("myindices[0]=%d\n", myindices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, getIndexSize(sphere1), getIndices(sphere1), GL_STATIC_DRAW);
    //printf("Past my second suspect spot\n");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    int stride = getInterleavedStride(sphere1);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // *-----------Now for the second one.---------------------------------------------------------------
    /*glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, vboId2);
    glBufferData(GL_ARRAY_BUFFER, getInterleavedVertexSize(sphere2), getInterleavedVertices(sphere2), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId2);
    //float * myindices2 = getIndices(sphere2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, getIndexSize(sphere2), getIndices(sphere2), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    int stride2 = getInterleavedStride(sphere2);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride2, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);*/


    //printf("Successfully initialized VAO, VBO, IBO, etc. End of initVBO().\n");
	g_print("init_buffer_objects finished\n");
}

/*
	This function has one purpose, to compile the shaders for the application. In this demo, it just glCompileShader
	the shaders for the colorful triangle.
*/
void init_shaders(GError *errorObj)
{
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        printf("Check the vertex fragment GLSL code.\n\nGoodbye!\n\n");
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
        printf("Check the fragment shader GLSL code.\n\nGoodbye!\n\n");
    }
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
        printf("The fragment and vertex shaders individually compiled, but the shader program did not compile.\n\nGoodbye!\n\n");
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    //printf("Successfully compiled shader program, end of initShader().\n");
    g_print("init_shaders() finished\n");
}

/*
	Keep this clean - this is just where we create an application and start it up. We do connect the activate signal,
	but that's about as fancy as we need to get here. It's not that you can't, it's just much cleaner if you do the
	other stuff in its own dedicated spot.
*/

int main(int argc, char *argv[])
{
	GtkApplication *app = gtk_application_new("edu.tarleton.pmg.complex-plasmas", G_APPLICATION_FLAGS_NONE);	// create a new application (just a container to hold everything)
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL); // This will cause the activate function we created to be called
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app); // Tidy up and free the memory when we are through.
	return 0;
}
