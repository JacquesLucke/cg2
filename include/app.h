#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class App {
 public:
  App(GLFWwindow* window);
  
  virtual ~App() = 0;
  
  virtual void setup() {}
  virtual void teardown() {}
  virtual void update() {}
  
 protected:
  GLFWwindow* window;
};

static unsigned int compileShader(unsigned int type, const std::string& source);
static unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader);

class TestApp : public App {
 public:
  TestApp(GLFWwindow* window);
  
  void setup() final override;
  void teardown() final override;
  void update() final override;

 private:
  unsigned int buffer;
};
