
#include "../external/Angel/inlcude/Angel/Angel.h"   //  If Angel.h exists
#include "../external/Angel/inlcude/Angel/mat.h"   //
#include "../external/Angel/inlcude/Angel/vec.h"   //

namespace Angel {

// Create a NULL-terminated string by reading the provided file
//static
char* readShaderSource(const char* shaderFile)
{
	FILE* fp;
	#ifdef _WIN32
		fopen_s(&fp,shaderFile, "r");
	#else
		fp = fopen(shaderFile, "r");
	#endif

    char* buf;
    long size;
    if(fp == NULL) return(NULL);
    fseek(fp, 0L, SEEK_END); 
	size = ftell(fp);
    fseek(fp, 0L, SEEK_SET); 
	buf = (char*) malloc((size + 1) * sizeof(char));
    fread(buf, 1, size, fp);
	buf[size] = '\0';  /* null termination */
    fclose(fp);

	printf("%s\n", buf);

    return buf;


}


// Create a GLSL program object from vertex and fragment shader files
GLuint
InitShader(const char* vShaderFile, const char* fShaderFile)
{
   
	struct Shader {
	const char*  filename;
	GLenum       type;
	GLchar*      source;
    }  shaders[2] = {
	{ vShaderFile, GL_VERTEX_SHADER, NULL },
	{ fShaderFile, GL_FRAGMENT_SHADER, NULL }
    };

    GLuint program = glCreateProgram();
    
    for ( int i = 0; i < 2; ++i ) {
	Shader& s = shaders[i];
	s.source = readShaderSource( s.filename );

	std::cout << "Printed Shader Read Status" << std::endl;
		
	if ( shaders[i].source == NULL ) {
	    std::cerr << "Failed to read " << s.filename << std::endl;
		exit( EXIT_FAILURE );
		}

	GLuint shader = glCreateShader( s.type );
	glShaderSource( shader, 1, (const GLchar**) &s.source, NULL );
	glCompileShader( shader );

	GLint  compiled;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
	if ( !compiled ) {
	    std::cerr << s.filename << " failed to compile:" << std::endl;
		GLint  logSize;
	    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
	    char* logMsg = new char[logSize];
	    glGetShaderInfoLog( shader, logSize, NULL, logMsg );
	    std::cerr << logMsg << std::endl;
	    delete [] logMsg;
		int q; std::cin >> q;
		exit( EXIT_FAILURE );
	}

	delete [] s.source;

	glAttachShader( program, shader );
    }

    /* link  and error check */
    glLinkProgram(program);

    GLint  linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if ( !linked ) {
	std::cerr << "Shader program failed to link" << std::endl;
	GLint  logSize;
	glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logSize);
	char* logMsg = new char[logSize];
	glGetProgramInfoLog( program, logSize, NULL, logMsg );
	std::cerr << logMsg << std::endl;
	delete [] logMsg;

	exit( EXIT_FAILURE );
    }

    /* use program object */
    glUseProgram(program);

    return program;

	
}

}  // Close namespace Angel block
