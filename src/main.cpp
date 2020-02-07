// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
#include <iostream>
#include <fstream>
#include "math.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif


#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>

// Timer
#include <chrono>

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject COLOR_VBO;

using namespace std;
Eigen::MatrixXf V(2,3);
Eigen::MatrixXf VC(2,3);
Eigen::MatrixXf selectedColor(3,3);
Eigen::MatrixXf animStart(3,3);
Eigen::MatrixXf animStartColor(3,3);
Eigen::MatrixXf animEnd(3,3);

Eigen::MatrixXf transMat(3,3);

Eigen::MatrixXf newtr(2,3);

Eigen::Matrix4f view(4,4);
Eigen::MatrixXf redcolor(1,3);

Eigen::MatrixXf colors(9,3);


int tricount=0;
double arr[6];
int closest=-1;


int counter = -1;
int animcounter=0;
bool insertMode=false, translationMode=false, deleteMode=false, colMode=false, keyMove=false;
int tri_num=-1;
double origx=0, origy=0;
int tri_rotate=-1;
int ver_color=-1;
int tri_keyframe=-1;
int tri_animate=-1;
float stepsx0, stepsx1, stepsx2, stepsy0, stepsy1, stepsy2, stepsz0, stepsz1, stepsz2;
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void color_triangle(GLFWwindow* window, int button, int action, int mods)
{
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);

  // Get the size of the window
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  // Convert screen position to world coordinates
  double xworld = ((xpos/double(width))*2)-1;
  double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw


  float prev=100.0;
  for(int i=3;i<V.cols();i++)
  {
      float tri_x=V(0,i);
      float tri_y=V(1,i);
      float distance=sqrt(abs((tri_x - xworld)*(tri_x - xworld) + (tri_y - yworld)*(tri_y - yworld)));
      if(distance<prev)
      {
          closest=i;
          prev=distance;
      }
  }
  std::cout<<"closest vert "<<closest<<std::endl;
}



void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        V.col(0) << xworld, yworld, 1;

    // Upload the change to the GPU
    VBO.update(V);
}



void drawline(GLFWwindow* window)
{
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  // Get the size of the window
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  double xworld = ((xpos/double(width))*2)-1;
  double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    newtr.resize(V.rows(),V.cols()+1);
    for(int i=0;i<V.cols();i++)
        newtr.col(i) = V.col(i);
    newtr.col(newtr.cols() - 1) << xworld, yworld, 1.0;
    VBO.update(newtr);
}

void triupdate(GLFWwindow* window, Eigen::MatrixXf temp)
{
  tricount++;
  if(tricount==3)
  {
      tricount=0;
      COLOR_VBO.update(temp);
  }
}

void create_triangle(GLFWwindow* window, int button, int action, int mods)
{
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  // Get the size of the window
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  double xworld = ((xpos/double(width))*2)-1;
  double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

      Eigen::MatrixXf temp(V.rows(),V.cols());
      temp=V;
      int vrows=V.rows();
      int vcols=V.cols();
      V.resize(V.rows(),vcols+1);
      for(int i=0;i<vcols;i++)
          V.col(i) = temp.col(i);
      V.col(vcols) << xworld, yworld, 1.0;
      temp=VC;
      vrows=VC.rows();
      vcols=VC.cols();
      VC.resize(VC.rows(),vcols+1);
      for(int i=0;i<vcols;i++)
          VC.col(i) = temp.col(i);
      VC.col(vcols)<<1,0,0;
      triupdate(window, VC);
      VBO.update(V);
  }
}

bool bary(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y)
{
    float temp = ((y2 -y3)*(x1 - x3) + (x3 -x2) * (y1 -y3));
    float alpha = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3))/temp;
    float beta = ((y3 - y1)* (x - x3) + (x1 - x3)*(y - y3))/temp;
    float gamma = 1 - alpha - beta;

    if (0 <= alpha && alpha <= 1 && 0 <= beta && beta <= 1 && 0 <= gamma && gamma <= 1)
      return true;
    else
      return false;

}

bool ontriangle(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y)
{
    float A = abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2.0);
    float A1 = abs((x*(y2-y3) + x2*(y3-y)+ x3*(y-y2))/2.0);
    float A2 = abs((x1*(y-y3) + x*(y3-y1)+ x3*(y1-y))/2.0);
    float A3  = abs((x1*(y2-y) + x2*(y-y1)+ x*(y1-y2))/2.0);
    if(A == A1 + A2 + A3)
      return true;
    else
      return false;
}

int inside_triangle(float x,float y)
{
    for(int i=3;i<V.cols();i+=3)
    {
        if(bary(V(0,i),V(1,i),V(0,i+1),V(1,i+1),V(0,i+2),V(1,i+2),x,y) ||
           ontriangle(V(0,i),V(1,i),V(0,i+1),V(1,i+1),V(0,i+2),V(1,i+2),x,y))
        {
            arr[0]=V(0,i);
            arr[1]=V(1,i);
            arr[2]=V(0,i+1);
            arr[3]=V(1,i+1);
            arr[4]=V(0,i+2);
            arr[5]=V(1,i+2);
            return i;
        }
    }
    return -1;
}

void makeblue(int bluetri)
{
  VC(0,bluetri)=0;
  VC(1,bluetri)=0;
  VC(2,bluetri)=1;
  VC(0,bluetri+1)=0;
  VC(1,bluetri+1)=0;
  VC(2,bluetri+1)=1;
  VC(0,bluetri+2)=0;
  VC(1,bluetri+2)=0;
  VC(2,bluetri+2)=1;
}

void zoom(float dim)
{
  view << view(0,0)*dim,0,0,0,
          0,view(1,1)*dim,0,0,
          0,0,view(2,2),0,
          0,0,0,view(3,3);
}

void pan_a()
{
  view << view(0,0),0,0,view(0,3)-0.1,
          0,view(1,1),0,0,
          0,0,view(2,2),0,
          0,0,0,view(3,3);
}

void pan_d()
{
  view << view(0,0),0,0,view(0,3)+0.1,
          0,view(1,1),0,0,
          0,0,view(2,2),0,
          0,0,0,view(3,3);
}

void pan_w()
{
  view << view(0,0),0,0,0,
          0,view(1,1),0,view(1,3)+0.1,
          0,0,view(2,2),0,
          0,0,0,view(3,3);
}

void pan_s()
{
  view << view(0,0),0,0,0,
          0,view(1,1),0,view(1,3)-0.1,
          0,0,view(2,2),0,
          0,0,0,view(3,3);
}

void move_triangle(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw
    int tempx=inside_triangle(xworld,yworld);
    if (inside_triangle(xworld,yworld) > 0) {
        int start = inside_triangle(xworld, yworld);
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            translationMode=true;
            tri_rotate=start;
            tri_num = start;
            origx = xworld;
            origy = yworld;

            for(int xyz=0;xyz<3;xyz++)
            {
                selectedColor(0,xyz)=VC(0,start);
                selectedColor(1,xyz)=VC(1,start);
                selectedColor(2,xyz)=VC(2,start);
            }

            makeblue(start);
            COLOR_VBO.update(VC);

        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            translationMode=true;
            origx = 0;
            origy = 0;
            tri_num = -1;

            for(int xyz=0;xyz<3;xyz++)
            {
                VC(0,start+xyz)=selectedColor(0,xyz);
                VC(1,start+xyz)=selectedColor(1,xyz);
                VC(2,start+xyz)=selectedColor(2,xyz);
            }
            COLOR_VBO.update(VC);
        }

    } else
    {
        tricount = -1;
    }
}


Eigen::MatrixXf deleteMatrix(Eigen::MatrixXf delmat, int start)
{
  int delcols = delmat.cols();
  Eigen::MatrixXf mat2(delmat.rows(), delcols - 3);
  for (int i = 0; i < start; i++) {
      mat2.col(i) = delmat.col(i);
  }
  for (int i = start; (i < delcols) && (i+3 <V.cols()) ; i++) {
      mat2.col(i) = delmat.col(i + 3);
  }
  return mat2;
}


void delete_triangle(GLFWwindow* window, int button, int action, int mods) {

  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  // Get the size of the window
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  double xworld = ((xpos/double(width))*2)-1;
  double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

  int tempx=inside_triangle(xworld,yworld);
  if (inside_triangle(xworld,yworld) > 0) {
      int start = inside_triangle(xworld, yworld);
      std::cout<<"Triangle number "<<start/3<<" is deleted."<<std::endl;
      if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
          int x = V.cols();
          int xrows = V.rows();
          Eigen::MatrixXf postDeletion(xrows,x-3);
          postDeletion = deleteMatrix(V,start);
          V.resize(V.rows(), x - 3);
          V = postDeletion;
          VBO.update(V);
          postDeletion = deleteMatrix(VC,start);
          x = VC.cols();
          VC.resize(VC.rows(), x - 3);
          VC = postDeletion;
          COLOR_VBO.update(VC);
      }
    }


}
void rotate_point(float cx,float cy,float angle,float &px,float &py)
{
    float s = sin(angle);
    float c = cos(angle);
    px -= cx;
    py -= cy;
    float xnew = px * c - py * s;
    float ynew = px * s + py * c;
    px = xnew + cx;
    py = ynew + cy;


}
void rotate_positive(GLFWwindow* window){
    if(tri_rotate>=0) {
        float centerX = (V(0, tri_rotate) + V(0, tri_rotate + 1) + V(0, tri_rotate + 2)) / 3;
        float centerY = (V(1, tri_rotate) + V(1, tri_rotate + 1) + V(1, tri_rotate + 2)) / 3;
        float s = sin(0.174533);
        float c = cos(0.174533);
        rotate_point(centerX,centerY,0.174533,V(0, tri_rotate),V(1, tri_rotate));
        rotate_point(centerX,centerY,0.174533,V(0, tri_rotate+1),V(1, tri_rotate+1));
        rotate_point(centerX,centerY,0.174533,V(0, tri_rotate+2),V(1, tri_rotate+2));
        VBO.update(V);

    }

}

void rotate_negative(GLFWwindow* window){
    if(tri_rotate>=0) {
        float centerX = (V(0, tri_rotate) + V(0, tri_rotate + 1) + V(0, tri_rotate + 2)) / 3;
        float centerY = (V(1, tri_rotate) + V(1, tri_rotate + 1) + V(1, tri_rotate + 2)) / 3;
        rotate_point(centerX,centerY,-0.174533,V(0, tri_rotate),V(1, tri_rotate));
        rotate_point(centerX,centerY,-0.174533,V(0, tri_rotate+1),V(1, tri_rotate+1));
        rotate_point(centerX,centerY,-0.174533,V(0, tri_rotate+2),V(1, tri_rotate+2));
        VBO.update(V);

    }

}


void increase_size(GLFWwindow* window)
{
    if(tri_rotate>=0) {
        float centerX = (V(0, tri_rotate) + V(0, tri_rotate + 1) + V(0, tri_rotate + 2)) / 3;
        float centerY = (V(1, tri_rotate) + V(1, tri_rotate + 1) + V(1, tri_rotate + 2)) / 3;
        V(0, tri_rotate) = centerX + (V(0, tri_rotate)-centerX)*1.25;
        V(0, tri_rotate+1) = centerX + (V(0, tri_rotate+1)-centerX)*1.25;
        V(0, tri_rotate+2) = centerX + (V(0, tri_rotate+2)-centerX)*1.25;
        V(1, tri_rotate) = centerY + (V(1, tri_rotate)-centerY)*1.25;
        V(1, tri_rotate+1) = centerY + (V(1, tri_rotate+1)-centerY)*1.25;
        V(1, tri_rotate+2) = centerY + (V(1, tri_rotate+2)-centerY)*1.25;
        VBO.update(V);

    }

}

void decrease_size(GLFWwindow* window)
{
  if(tri_rotate>=0) {
      float centerX = (V(0, tri_rotate) + V(0, tri_rotate + 1) + V(0, tri_rotate + 2)) / 3;
      float centerY = (V(1, tri_rotate) + V(1, tri_rotate + 1) + V(1, tri_rotate + 2)) / 3;
      V(0, tri_rotate) = centerX + (V(0, tri_rotate)-centerX)*0.75;
      V(0, tri_rotate+1) = centerX + (V(0, tri_rotate+1)-centerX)*0.75;
      V(0, tri_rotate+2) = centerX + (V(0, tri_rotate+2)-centerX)*0.75;
      V(1, tri_rotate) = centerY + (V(1, tri_rotate)-centerY)*0.75;
      V(1, tri_rotate+1) = centerY + (V(1, tri_rotate+1)-centerY)*0.75;
      V(1, tri_rotate+2) = centerY + (V(1, tri_rotate+2)-centerY)*0.75;
      VBO.update(V);

  }

}

void savesvg(GLFWwindow* window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ofstream svgfile;
    insertMode=false;
    svgfile.open("kedar.svg");
    if (svgfile.is_open())
  {

    svgfile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    svgfile << "<svg width=\""<<width<<"\" height=\""<<height<<"\" version=\"1.200000\" xmlns=\"http://www.w3.org/2000/svg\""
            " xmlns:xlink=\"http://www.w3.org/1999/xlink\"  >\n";

    for(int i=3;i<V.cols();i=i+3)
    {
        Eigen::Vector4f p1(V(0,i),V(1,i),0,1);
        Eigen::Vector4f p2(V(0,i+1),V(1,i+1),0,1);
        Eigen::Vector4f p3(V(0,i+2),V(1,i+2),0,1);

        float ax,ay,bx,by,cx,cy;
        ax=((( p1[0] + 1 ) / 2.0) *width );
        ay=((( 1 - p1[1] ) / 2.0) *height );
        bx=((( p2[0] + 1 ) / 2.0) *width );
        by=((( 1 - p2[1] ) / 2.0) *height ) ;
        cx=((( p3[0] + 1 ) / 2.0) *width );
        cy=((( 1 - p3[1] ) / 2.0) *height );


        svgfile << " <defs>\n"
                "      <linearGradient id=\"fadeA-1"<<i<<"\" gradientUnits=\"objectBoundingBox\" x1=\"0.5\" y1=\"0\" x2=\"1\" y2=\"1\">\n"
                "        <stop offset=\"0%\" stop-color=\"rgb("<<roundf((float)VC(0,i)*255)<<","<<roundf((float)VC(1,i)*255)<<","<<roundf((float)VC(2,i)*255)<<")\"/>\n"
                "        <stop offset=\"100%\" stop-color=\"rgb("<<roundf((float)VC(0,i+2)*255)<<","<<roundf((float)VC(1,i+2)*255)<<","<<roundf((float)VC(2,i+2)*255)<<")\"/>\n"
                "      </linearGradient>\n"
                "      <linearGradient id=\"fadeB-1"<<i<<"\" gradientUnits=\"objectBoundingBox\" x1=\"0\" y1=\"1\" x2=\"0.75\" y2=\"0.5\">\n"
                "        <stop offset=\"0%\" stop-color=\"rgb("<<roundf((float)VC(0,i+1)*255)<<","<<roundf((float)VC(1,i+1)*255)<<","<<roundf((float)VC(2,i+1)*255)<<")\"/>\n"
                "        <stop offset=\"100%\" stop-color=\"rgb("<<roundf((float)VC(0,i+1)*255)<<","<<roundf((float)VC(1,i+1)*255)<<","<<roundf((float)VC(2,i+1)*255)<<")\" stop-opacity=\"0\" />\n"
                "      </linearGradient>\n"
                "      </defs>\n"
                "      <path id=\"pathA-1"<<i<<"\" d=\"M "<<ax<<","<<ay<<" L "<<bx<<","<<by<<" "<<cx<<","<<cy<<" Z\" fill=\"url(#fadeA-1"<<i<<")\"/>\n"
                "      <path id=\"pathB-1"<<i<<"\" d=\"M "<<ax<<","<<ay<<" L "<<bx<<","<<by<<" "<<cx<<","<<cy<<" Z\" fill=\"url(#fadeB-1"<<i<<")\"/>\n";
    }


    svgfile <<"</svg>\n";
    svgfile.close();
    cout<<"Done writing";
  }
  else
  std::cout<<"couldnt open"<<std::endl;

}
void selectTriangleKeyframe(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw
    int tempx=inside_triangle(xworld,yworld);
  /*  if (inside_triangle(xworld,yworld) > 0) {
        int start = inside_triangle(xworld, yworld);
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            tri_keyframe=start;
            std::cout<<"triangle number "<<(tri_keyframe/3)<<" selected. Now move triangle and press "<<std::endl;
          }
        }*/

    if (inside_triangle(xworld,yworld) > 0) {
        int start = inside_triangle(xworld, yworld);
      //  std::cout<<"start "<<start<<std::endl;
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            keyMove=true;
            tri_animate = tri_keyframe =start;
            origx = xworld;
            origy = yworld;
            std::cout<<"Triangle selected"<<std::endl;

            for(int xyz=0;xyz<3;xyz++)
            {
                selectedColor(0,xyz)=VC(0,start);
                selectedColor(1,xyz)=VC(1,start);
                selectedColor(2,xyz)=VC(2,start);
                animStart(0,xyz)=V(0,start+xyz);
                animStart(1,xyz)=V(1,start+xyz);
                animStart(2,xyz)=V(2,start+xyz);
                animStartColor(0,xyz)=V(0,start+xyz);
                animStartColor(1,xyz)=V(1,start+xyz);
                animStartColor(2,xyz)=V(2,start+xyz);
            }
          //  std::cout<<"triangle orig position x "<<V(0,start)<<" "<<V(0,start)<<" "<<V(0,start)<<std::endl;
          //  std::cout<<"triangle orig position y "<<V(1,start)<<" "<<V(1,start)<<" "<<V(1,start)<<std::endl;
            makeblue(start);
            COLOR_VBO.update(VC);

        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            keyMove=true;
            origx = 0;
            origy = 0;
            tri_animate = -1;

            for(int xyz=0;xyz<3;xyz++)
            {
                VC(0,start+xyz)=selectedColor(0,xyz);
                VC(1,start+xyz)=selectedColor(1,xyz);
                VC(2,start+xyz)=selectedColor(2,xyz);
                animEnd(0,xyz)=V(0,start+xyz);
                animEnd(1,xyz)=V(1,start+xyz);
                animEnd(2,xyz)=V(2,start+xyz);
            }

            COLOR_VBO.update(VC);
            //std::cout<<"triangle original position "<<animStart(0,0)<<" "<<animStart(0,1)<<" "<<animStart(0,2)<<std::endl;
            //std::cout<<"triangle final position x "<<animEnd(0,0)<<" "<<animEnd(0,1)<<" "<<animEnd(0,2)<<std::endl;
          //  std::cout<<"triangle final position y "<<animEnd(1,0)<<" "<<animEnd(1,1)<<" "<<animEnd(1,2)<<std::endl;
            //std::cout<<"triangle final position "<<animEnd(2,0)<<" "<<animEnd(2,1)<<" "<<animEnd(2,2)<<std::endl;

              //VC(0,start)=animStartColor(0,0);
              //VC(1,start)=animStartColor(1,0);
              //VC(2,start)=animStartColor(2,0);
              V(0,start)=animStart(0,0);
              V(1,start)=animStart(1,0);
              V(2,start)=animStart(2,0);
              //VC(0,start+1)=animStartColor(0,1);
              //VC(1,start+1)=animStartColor(1,1);
              //VC(2,start+1)=animStartColor(2,1);
              V(0,start+1)=animStart(0,1);
              V(1,start+1)=animStart(1,1);
              V(2,start+1)=animStart(2,1);
              //VC(0,start+2)=animStartColor(0,2);
              //VC(1,start+2)=animStartColor(1,2);
              //VC(2,start+2)=animStartColor(2,2);
              V(0,start+2)=animStart(0,2);
              V(1,start+2)=animStart(1,2);
              V(2,start+2)=animStart(2,2);


            //std::cout<<"triangle back position x "<<animStart(0,0)<<" "<<animStart(0,1)<<" "<<animStart(0,2)<<std::endl;
            //std::cout<<"triangle back position y "<<animStart(1,0)<<" "<<animStart(1,1)<<" "<<animStart(1,2)<<std::endl;
            VBO.update(V);
            COLOR_VBO.update(VC);
            stepsx0=(animStart(0,0)-animEnd(0,0))/15;
            stepsy0=(animStart(1,0)-animEnd(1,0))/15;
            stepsz0=(animStart(2,0)-animEnd(2,0))/15;
            stepsx1=(animStart(0,1)-animEnd(0,1))/15;
             stepsy1=(animStart(1,1)-animEnd(1,1))/15;
             stepsz1=(animStart(2,1)-animEnd(2,1))/15;
             stepsx2=(animStart(0,2)-animEnd(0,2))/15;
             stepsy2=(animStart(1,2)-animEnd(1,2))/15;
             stepsz2=(animStart(2,2)-animEnd(2,2))/15;
        }

    } else
    {
        animcounter = -1;
    }
}
void colornew(int closest, int keyp)
{

  keyp=keyp-1;
    VC(0,closest)=colors(keyp,0);
    VC(1,closest)=colors(keyp,1);
    VC(2,closest)=colors(keyp,2);
    //std::cout<<"color "<<std::endl<< 0<<" " <<keyp<<" "<<colors(keyp,0)<<std::endl<<"1 "<<keyp<<" "<<colors(keyp,1)<<std::endl<<"2 "<<keyp<<" "<<colors(keyp,2)<<std::endl;


  COLOR_VBO.update(VC);
  //colMode=false;
//  std::cout<<"color mode ended"<<std::endl;
}

void keyframeadv()
{

  //std::cout<<"one step"<<std::endl;
  float distance2=sqrt(abs((V(0,tri_keyframe) - animEnd(0,0))*(V(0,tri_keyframe) - animEnd(0,0)) + (V(1,tri_keyframe) - animEnd(1,0))*(V(1,tri_keyframe) - animEnd(1,0))));

if(distance2<0.1)
return;
  V(0,tri_keyframe)-=stepsx0;
  V(1,tri_keyframe)-=stepsy0;
  V(2,tri_keyframe)-=stepsz0;
  V(0,tri_keyframe+1)-=stepsx1;
  V(1,tri_keyframe+1)-=stepsy1;
  V(2,tri_keyframe+1)-=stepsz1;
  V(0,tri_keyframe+2)-=stepsx2;
  V(1,tri_keyframe+2)-=stepsy2;
  V(2,tri_keyframe+2)-=stepsz2;
  VBO.update(V);




}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    switch (key)
    {
        case GLFW_KEY_I:
          if(action == GLFW_PRESS)
          {
          std::cout<<"Insertion Mode"<<std::endl;
          insertMode=true;
          tricount=0;
          }
          glfwSetMouseButtonCallback(window, create_triangle);
          break;

      case GLFW_KEY_O:
          if(action == GLFW_PRESS)
          {
          std::cout<<"Translation Mode"<<std::endl;
          translationMode=true;
          }
          glfwSetMouseButtonCallback(window, move_triangle);
          break;

      case GLFW_KEY_P:
          if(action == GLFW_PRESS)
          {
          std::cout<<"Deletion Mode"<<std::endl;
          deleteMode=true;
          glfwSetMouseButtonCallback(window, delete_triangle);
          }
          break;
      case GLFW_KEY_B:
          if(action == GLFW_PRESS)
          {
          std::cout<<"Saving SVg"<<std::endl;
          //deleteMode=true;
          savesvg(window);
          }
          break;
      case GLFW_KEY_MINUS:
          if(action == GLFW_PRESS)
          {
              std::cout<<"Zoom Out"<<std::endl;
              zoom(0.8);
          }
          break;
      case GLFW_KEY_EQUAL:
          if(action == GLFW_PRESS)
          {
              std::cout<<"Zoom In"<<std::endl;
              zoom(1.2);
          }
          break;

          case GLFW_KEY_A:
              if(action == GLFW_PRESS)
              {
                  std::cout<<"Pan A"<<std::endl;
                  pan_a();
              }
              break;
      case GLFW_KEY_D:
          if(action == GLFW_PRESS)
          {
              std::cout<<"Pan D"<<std::endl;
              pan_d();
          }
          break;
      case GLFW_KEY_W:
          if(action == GLFW_PRESS)
          {
              std::cout<<"Pan W"<<std::endl;
              pan_w();
          }
          break;
      case GLFW_KEY_S:
          if(action == GLFW_PRESS)
          {
              std::cout<<"Pan S"<<std::endl;
              pan_s();
          }
          break;
      case GLFW_KEY_M:
          if(action == GLFW_PRESS)
          {
              insertMode=false;
              translationMode=false;
              deleteMode=false;
              colMode=false;
              tri_keyframe=-1;
              std::cout<<"Keyframe mode entered, select triangle and press GLFW_KEY_LEFT_BRACKET."<<std::endl;
              glfwSetMouseButtonCallback(window, selectTriangleKeyframe);

          }
          break;
      case GLFW_KEY_LEFT_BRACKET:
          if(action == GLFW_PRESS)
              {
                  insertMode=false;
                  translationMode=false;
                  deleteMode=false;
                  colMode=false;
                  if(tri_keyframe==-1)
                  {
                    std::cout<<"Keyframe triangle not selected!! Select first using GLFW_KEY_M and move to a final destination."<<std::endl;
                  }
                  else
                  {
                    std::cout<<"Animation Started press Right bracket to advance."<<std::endl;

                    keyframeadv();
                    counter++;
                  }


              }
          break;
      case GLFW_KEY_RIGHT_BRACKET:
          if(action == GLFW_PRESS)
              {
                  insertMode=false;
                  translationMode=false;
                  deleteMode=false;
                  colMode=false;
                  if(tri_keyframe==-1)
                  {
                    std::cout<<"Keyframe triangle not selected!! Select first using GLFW_KEY_M and move to a final destination."<<std::endl;
                  }
                  else
                  {
                    //std::cout<<"Animation Started press Right bracket to advance."<<std::endl;
                    if(counter<20)
                    keyframeadv();
                  }


              }
          break;
      case  GLFW_KEY_1:
          if(action == GLFW_PRESS && colMode==true)
          {
              colornew(closest,1);
          }
          break;
        case GLFW_KEY_2:
        if(action == GLFW_PRESS && colMode==true)
        {
            colornew(closest,2);
        }
        break;
        case  GLFW_KEY_3:
        if(action == GLFW_PRESS && colMode==true)
        {
            colornew(closest,3);

        }
        break;
        case  GLFW_KEY_4:
        if(action == GLFW_PRESS && colMode==true)
        {
            colornew(closest,4);
        }
        break;
        case  GLFW_KEY_5:
        if(action == GLFW_PRESS && colMode==true)
        {
            colornew(closest,5);
        }
        break;
        case  GLFW_KEY_6:
        if(action == GLFW_PRESS && colMode==true)
        {
            colornew(closest,6);
        }
        break;
        case  GLFW_KEY_7:
        if(action == GLFW_PRESS && colMode==true)
        {
            colornew(closest,7);
        }
        break;
        case  GLFW_KEY_8:
        if(action == GLFW_PRESS && colMode==true)
        {
            colornew(closest,8);
        }
        break;
        case  GLFW_KEY_9:
        if(action == GLFW_PRESS && colMode==true)
        {
            colornew(closest,9);
        }
        break;
        case GLFW_KEY_H:
            if(action == GLFW_PRESS)
            {
            if(translationMode)
            rotate_negative(window);
            else
            std::cout<<"Cannot rotate, translation mode not active!"<<std::endl;
            }
            break;
        case GLFW_KEY_J:
            if(action == GLFW_PRESS)
            {
            if(translationMode)
            rotate_positive(window);
            else
            std::cout<<"Cannot rotate, translation mode not active!"<<std::endl;
            }
            break;
        case GLFW_KEY_K:
            if(action == GLFW_PRESS)
            {
            if(translationMode)
            {
                std::cout<<"Scaling primitive"<<std::endl;
                increase_size(window);
            }
            else
            std::cout<<"Cannot scale up, translation mode not active!"<<std::endl;
            }
            break;
        case GLFW_KEY_L:
            if(action == GLFW_PRESS)
            {
            if(translationMode)
            {
              std::cout<<"Scaling primitive"<<std::endl;
              decrease_size(window);
            }
            else
            std::cout<<"Cannot scale up, translation mode not active!"<<std::endl;
            }
            break;
        case GLFW_KEY_C:
            if(action == GLFW_PRESS)
            {
              if(colMode==false)
            {
              std::cout<<"Color Mode enabled"<<std::endl;
            colMode=true;
          }
          else
          {
            std::cout<<"Color Mode disabled"<<std::endl;
          colMode=false;
        }

            glfwSetMouseButtonCallback(window, color_triangle);
          }
            break;
        default:
            break;
    }

    // Upload the change to the GPU
    VBO.update(V);
}
void TranslateTriangle(GLFWwindow* window)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    double xworldn = xworld-origx;
    double yworldn = yworld-origy;
    V(0,tri_num)=arr[0]+xworldn;
    V(1,tri_num)=arr[1]+yworldn;
    V(0,tri_num+1)=arr[2]+xworldn;
    V(1,tri_num+1)=arr[3]+yworldn;
    V(0,tri_num+2)=arr[4]+xworldn;
    V(1,tri_num+2)=arr[5]+yworldn;
    VBO.update(V);

}

void KeyTranslateTriangle(GLFWwindow* window)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    double xworldn = xworld-origx;
    double yworldn = yworld-origy;
    V(0,tri_animate)=arr[0]+xworldn;
    V(1,tri_animate)=arr[1]+yworldn;
    V(0,tri_animate+1)=arr[2]+xworldn;
    V(1,tri_animate+1)=arr[3]+yworldn;
    V(0,tri_animate+2)=arr[4]+xworldn;
    V(1,tri_animate+2)=arr[5]+yworldn;
    VBO.update(V);

}
int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
      glewExperimental = true;
      GLenum err = glewInit();
      if(GLEW_OK != err)
      {
        /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      }
      glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
      fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();


    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar* vertex_shader =
            "#version 150 core\n"
                    "in vec2 position;"
                    "in vec3 color;"
                    "out vec3 f_color;"
                    "uniform mat4 view;"
                    "void main()"
                    "{"
                    "    gl_Position = view * vec4(position, 0.0, 1.0);"
                    "    f_color = color;"
                    "}";
    const GLchar* fragment_shader =
            "#version 150 core\n"
                    "in vec3 f_color;"
                    "out vec4 outColor;"
                    "uniform vec3 triangleColor;"
                    "void main()"
                    "{"
                    "    outColor = vec4(f_color, 1.0);"
                    "}";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();

    VBO.init();
    V.resize(3,3);
//    cout<<"Values after resize are "<<V<<endl;
    V  << 0,0,0,0,0,0,1,1,1;
    redcolor << 1,0,0;

    VBO.update(V);

    COLOR_VBO.init();
    VC.resize(3,3);
    VC<<
       1,  0, 0,
            1,  0, 0,
            1,  0, 0;

    COLOR_VBO.update(VC);
    view << 1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1;

    selectedColor <<0,0,0,
                    0,0,0,
                    0,0,0;

    colors<<1.0,0.0,0.0,
            0.0,1.0,0.0,
            0.0,0.0,1.0,
            1.0,1.0,0.0,
            0.0,1.0,1.0,
            1.0,0.0,1.0,
            0.5,0.0,0.5,
            0.0,0.5,0.5,
            1.0,1.0,1.0;
    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    program.bindVertexAttribArray("position",VBO);
    program.bindVertexAttribArray("color",COLOR_VBO);

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Bind your VAO (not necessary if you have only one)
        //VAO.bind();

        // Bind your program
        program.bind();

        // Set the uniform value depending on the time difference
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        glUniform3f(program.uniform("triangleColor"), (float)(sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle
        glDrawArrays(GL_TRIANGLES, 0, V.cols());

        if(tricount==1 && insertMode==true) {

            drawline(window);
            glDrawArrays(GL_LINE_LOOP, newtr.cols()-2, 2);
        }
        else if(tricount==2 && insertMode==true) {

            drawline(window);
            glDrawArrays(GL_LINE_LOOP, newtr.cols()-3, 3);
        }
        else if(translationMode==true && tri_num>0)
        {
            TranslateTriangle(window);
            int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if(state == GLFW_RELEASE)
            {
                translationMode=true;
                //keyMove=true;
                origx=0;
                origy=0;
                tri_num=-1;
                arr[0]=0;
                arr[1]=0;
                arr[2]=0;
                arr[3]=0;
                arr[4]=0;
                arr[5]=0;
                transMat <<0,0,0,0,0,0,0,0,0;
            }
        }
        else if(keyMove==true && tri_animate>0)
        {
          KeyTranslateTriangle(window);
          int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
          if(state == GLFW_RELEASE)
          {
              //translationMode=true;
              keyMove=true;
              origx=0;
              origy=0;
              tri_num=-1;
              arr[0]=0;
              arr[1]=0;
              arr[2]=0;
              arr[3]=0;
              arr[4]=0;
              arr[5]=0;
              transMat <<0,0,0,0,0,0,0,0,0;
          }
        }

        glUniformMatrix4fv(program.uniform("view"),1,GL_FALSE,view.data());

        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
