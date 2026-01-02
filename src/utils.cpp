#include "utils.hpp"

// Anticlockwise version
std::vector<unsigned int> generate_grid_indices_acw(const unsigned int size) {
    std::vector<unsigned int> r;
    for (unsigned int i = 0; i <= size - 2; i++) {
        for (unsigned int j = 0; j <= size - 2; j++) {
            r.push_back(i * size + j);
            r.push_back(i * size + j + size);
            r.push_back(i * size + j + 1);
            r.push_back(i * size + j + size);
            r.push_back(i * size + j + size + 1);
            r.push_back(i * size + j + 1);
        }
    }
    return r;
}

// Clockwise version
std::vector<unsigned int> generate_grid_indices_cw(const unsigned int size) {
    std::vector<unsigned int> r;
    for (unsigned int i = 0; i <= size - 2; i++) {
        for (unsigned int j = 0; j <= size - 2; j++) {
            r.push_back(i * size + j);
            r.push_back(i * size + j + 1);
            r.push_back(i * size + j + size);
            r.push_back(i * size + j + size + 1);
            r.push_back(i * size + j + size);
            r.push_back(i * size + j + 1);
        }
    }
    return r;
}

GLuint LoadShadersFromFile(const char *vertex_file_path, const char *fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else
	{
		printf("Vertex shader not found %s.\n", vertex_file_path);
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}
	else
	{
		printf("Fragment shader not found %s.\n", fragment_file_path);
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling vertex shader : %s\n", vertex_file_path);
	char const *VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	if (!Result) {
		printf("Error compiling vertex shader : %s\n", vertex_file_path);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
			printf("%s\n", &VertexShaderErrorMessage[0]);
		}
		return 0;
	}

	// Compile Fragment Shader
	printf("Compiling fragment shader : %s\n", fragment_file_path);
	char const *FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	if (!Result) {
		printf("Error compiling fragment shader : %s\n", fragment_file_path);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			printf("%s\n", &FragmentShaderErrorMessage[0]);
		}
		return 0;
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	if (!Result) {
		printf("Error linking program\n");
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("%s\n", &ProgramErrorMessage[0]);
		}
		return 0;
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLuint LoadShadersFromFile(const char *vertex_file_path, const char *fragment_file_path, const char *geometry_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

    // Read the Vertex Shader code from file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open())
    {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else
    {
        printf("Vertex shader not found %s.\n", vertex_file_path);
        return 0;
    }

    // Read the Fragment Shader code from file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open())
    {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }
    else
    {
        printf("Fragment shader not found %s.\n", fragment_file_path);
        return 0;
    }

    // Read the Geometry Shader code from file (optional)
    std::string GeometryShaderCode;
	std::ifstream GeometryShaderStream(geometry_file_path, std::ios::in);
	if (GeometryShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << GeometryShaderStream.rdbuf();
		GeometryShaderCode = sstr.str();
		GeometryShaderStream.close();
	}
	else
	{
		printf("Geometry shader not found %s.\n", geometry_file_path);
		return 0;
	}
    

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling vertex shader : %s\n", vertex_file_path);
    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    if (!Result) {
        printf("Error compiling vertex shader : %s\n", vertex_file_path);
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0) {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            printf("%s\n", &VertexShaderErrorMessage[0]);
        }
        return 0;
    }

    // Compile Fragment Shader
    printf("Compiling fragment shader : %s\n", fragment_file_path);
    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    if (!Result) {
        printf("Error compiling fragment shader : %s\n", fragment_file_path);
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0)
        {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            printf("%s\n", &FragmentShaderErrorMessage[0]);
        }
        return 0;
    }

    // Compile Geometry Shader (if provided)
 
	printf("Compiling geometry shader : %s\n", geometry_file_path);
	char const *GeometrySourcePointer = GeometryShaderCode.c_str();
	glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer, NULL);
	glCompileShader(GeometryShaderID);

	glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
	if (!Result) {
		printf("Error compiling geometry shader : %s\n", geometry_file_path);
		glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> GeometryShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
			printf("%s\n", &GeometryShaderErrorMessage[0]);
		}
		return 0;
	}


    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glAttachShader(ProgramID, GeometryShaderID);

    glLinkProgram(ProgramID);

    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if (!Result) {
        printf("Error linking program\n");
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0)
        {
            std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }
        return 0;
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDetachShader(ProgramID, GeometryShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    glDeleteShader(GeometryShaderID);

    return ProgramID;
}
