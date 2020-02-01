#pragma once
#include <sb7.h>
#include <vmath.h>

#include <object.h>
#include <shader.h>
#include <sb7ktx.h>


class my_application : public sb7::application
{
	void init()
	{
		static const char title[] = "lab3 spheremapping";

		sb7::application::init();

		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		object.load("media/objects/torus_nrms_tc.sbm");

		spheremaps[0] = sb7::ktx::file::load("media/textures/envmaps/spheremap1.ktx");
		spheremaps[1] = sb7::ktx::file::load("media/textures/envmaps/spheremap2.ktx");
		spheremaps[2] = sb7::ktx::file::load("media/textures/envmaps/spheremap3.ktx");

		current_spheremap = spheremaps[0];



		glActiveTexture(GL_TEXTURE0);
		spheremap3d = sb7::ktx::file::load("media/textures/envmaps/mountains3d.ktx");

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		glActiveTexture(GL_TEXTURE1);
		glossmap = sb7::ktx::file::load("media/textures/pattern1.ktx");




		load_shaders();

		current_program = program;

		GLint spheremaplocation = glGetUniformLocation(program, "spheremap");
		glUseProgram(program); 
		glUniform1i(spheremaplocation, 0);

		GLint spheremap3dlocation = glGetUniformLocation(program3d, "spheremap3d");
		GLint glossmaplocation = glGetUniformLocation(program3d, "glossmap");
		glUseProgram(program3d); 
		glUniform1i(glossmaplocation, 0);
		glUniform1i(spheremap3dlocation, 1);
		


		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	void load_shaders()
	{
		if (program)
			glDeleteProgram(program);

		GLuint vs, fs;

		vs = sb7::shader::load("media/shaders/my_application/spheremap.vs.glsl", GL_VERTEX_SHADER);
		fs = sb7::shader::load("media/shaders/my_application/spheremap.fs.glsl", GL_FRAGMENT_SHADER);

		program = glCreateProgram();
		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);

		glDeleteShader(vs);
		glDeleteShader(fs);

		mv_location = glGetUniformLocation(program, "mv_matrix");
		proj_location = glGetUniformLocation(program, "proj_matrix");




		if (program3d)
			glDeleteProgram(program3d);
		vs = sb7::shader::load("media/shaders/my_application/glossy.vs.glsl", GL_VERTEX_SHADER);
		fs = sb7::shader::load("media/shaders/my_application/glossy.fs.glsl", GL_FRAGMENT_SHADER);

		program3d = glCreateProgram();
		glAttachShader(program3d, vs);
		glAttachShader(program3d, fs);
		glLinkProgram(program3d);
		glDeleteShader(vs);
		glDeleteShader(fs);
	}
	

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, &one);

		if (current_program == program) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, current_spheremap);
		}
		else if (current_program == program3d) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, glossmap);

			glActiveTexture(GL_TEXTURE0+1);
			glBindTexture(GL_TEXTURE_3D, spheremap3d);
		}

		glUseProgram(current_program);

		

		vmath::mat4 proj_matrix = vmath::perspective(60.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f) *
			vmath::rotate((float)currentTime*10.0f, 1.0f, 0.0f, 0.0f) *
			vmath::rotate((float)currentTime*10.0f, 0.0f, 1.0f, 0.0f) *
			vmath::rotate((float)currentTime*8.0f, 0.0f, 0.0f, 1.0f) *
			vmath::translate(0.0f, -0.0f, 0.0f);


		glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);

		object.render();
	}

	virtual void shutdown()
	{
		glDeleteProgram(program);
		glDeleteBuffers(3, spheremaps);
	}

	virtual void onKey(int key, int action)
	{
		if (action)
		{
			switch (key)
			{
			case 'R': load_shaders();
				break;
			case 'E':
				spheremap_index = (spheremap_index + 1) % 3;
				current_spheremap = spheremaps[spheremap_index];
				break;
			case 'D': 
				if (current_program == program)
					current_program = program3d;
				else
					current_program = program;
				break;
			}
		}
	}

private:
	GLuint spheremaps[3];
	GLuint spheremap3d;
	GLuint glossmap;

	GLuint current_spheremap;

	sb7::object     object;

	float           aspect;
	vmath::mat4     proj_matrix;

	GLuint program;
	GLuint program3d;
	GLuint current_program;

	GLint mv_location;
	GLint proj_location;

	int spheremap_index = 0;
};

DECLARE_MAIN(my_application);