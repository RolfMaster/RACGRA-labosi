#pragma once
#include "sb7.h"
#include <vmath.h>
#include <cstdlib>
#include <cmath>

#define MANY_CUBES
#define MAX_PARTICLE_NUM 1000
#define MIN_LIFESPAN 0.5f

//void glClearBufferfv(GLenum buffer, GLint drawBuffer, const GLfloat * value);
struct Particle {
	vmath::vec3 pos;
	vmath::vec3 vel;
	float timeAlive;
	float lifespan;
};

class my_application : public sb7::application
{
	void init()
	{
		static const char title[] = "cestice lab2";

		sb7::application::init();

		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		for (Particle &part : particles) {
			part.lifespan = getRandNum() * MIN_LIFESPAN;
		}

		static const char * vs_source[] =
		{
			"#version 450 core                                                  \n"
			"                                                                   \n"
			"in vec4 position;                                                  \n"
			"                                                                   \n"
			"out VS_OUT                                                         \n"
			"{                                                                  \n"
			"    vec4 color;                                                    \n"
			"} vs_out;                                                          \n"
			"                                                                   \n"
			"uniform mat4 mv_matrix;                                            \n"
			"uniform mat4 proj_matrix;                                          \n"
			"                                                                   \n"
			"void main(void)                                                    \n"
			"{                                                                  \n"
			"    gl_Position = proj_matrix * mv_matrix * position;              \n"
			"    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);      \n"
			"}                                                                  \n"
		};

		static const char * fs_source[] =
		{
			"#version 450 core                                                  \n"
			"                                                                   \n"
			"out vec4 color;                                                    \n"
			"                                                                   \n"
			"in VS_OUT                                                          \n"
			"{                                                                  \n"
			"    vec4 color;                                                    \n"
			"} fs_in;                                                           \n"
			"                                                                   \n"
			"void main(void)                                                    \n"
			"{                                                                  \n"
			"    color = fs_in.color;                                           \n"
			"}                                                                  \n"
		};

		program = glCreateProgram();
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, fs_source, NULL);
		glCompileShader(fs);

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, vs_source, NULL);
		glCompileShader(vs);

		glAttachShader(program, vs);
		glAttachShader(program, fs);

		glLinkProgram(program);

		mv_location = glGetUniformLocation(program, "mv_matrix");
		proj_location = glGetUniformLocation(program, "proj_matrix");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		static const GLfloat vertex_positions[] =
		{
			-0.25f,  0.25f, -0.25f,
			-0.25f, -0.25f, -0.25f,
			 0.25f, -0.25f, -0.25f,
			 
			 0.25f, -0.25f, -0.25f,
			 0.25f,  0.25f, -0.25f,
			-0.25f,  0.25f, -0.25f,
		};

		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertex_positions),
			vertex_positions,
			GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	float getRandNum() {
		return 2 * ((float)rand() / (float)RAND_MAX) - 1.0f;
	}

	void initializeParticle(Particle * particle) {
		particle->pos = sourceCoords;
		particle->timeAlive = 0;
		//particle->lifespan = 5.0f;
		particle->lifespan = MIN_LIFESPAN + 2.0 * (float)rand() / (float)RAND_MAX;
		//particle->vel = { 1.0f, 0.0f, 0.0f };
		particle->vel = { 1.0f, getRandNum()/5.0f, getRandNum()/5.0f };
		particle->vel *= 1.0f;
	}

	void updateParticles(double currentTime) {
		double dt = currentTime - previousTime; //time delta
		previousTime = currentTime;

		sourceCoords[1] = 0.2f * sinf(currentTime*3.0f);
		sourceCoords[0] = 0.2f * cosf(currentTime*3.0f);

		for (Particle &part : particles) {
			part.timeAlive += dt;
			if (part.timeAlive > part.lifespan) {
				initializeParticle(&part);
			}
			else {
				part.pos += part.vel * dt;
				part.vel += (float)dt * (vmath::normalize(attractorCoords - part.pos) 
							* 5.0f / std::pow(vmath::length(attractorCoords - part.pos), 2.0f));
			}
		}
	}

	virtual void render(double currentTime)
	{
		updateParticles(currentTime);

		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, green);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(program);

		glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);



		for (Particle part : particles)
		{
			
			vmath::mat4 mv_matrix = vmath::translate(part.pos) *
				vmath::scale(0.015f);
			
			glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
		glDeleteBuffers(1, &buffer);
	}

	void onResize(int w, int h)
	{
		sb7::application::onResize(w, h);

		aspect = (float)w / (float)h;
		proj_matrix = vmath::perspective(50.0f, aspect, 0.1f, 1000.0f);
	}

private:
	GLuint          program;
	GLuint          vao;
	GLuint          buffer;
	GLint           mv_location;
	GLint           proj_location;

	float           aspect;
	vmath::mat4     proj_matrix;

	Particle particles[MAX_PARTICLE_NUM];
	vmath::vec3 attractorCoords = { 0.5f, 0.7f, -2.0f };
	vmath::vec3 sourceCoords = { 0.0f, 0.0f, -3.0f };

	double previousTime = 0;
};

DECLARE_MAIN(my_application);