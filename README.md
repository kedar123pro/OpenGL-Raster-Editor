# OpenGL Raster Editor
2D editor using OpenGL, GLFW and Eigen for vector operations

Dependencies: OpenGL, GLFW, Eigen, 

How to compile?
---------------
1. Clone this repository 

2. Run in the `/build` directory: 
`cmake ..`

3. Next, run: `make build`


How to use?
-----------
To draw triangles: Press 'i' to enter insertion mode, then draw a triangle with 3 consecutive mouse clicks on the viewport.

To transform triangles: Press 'o' to enter translation mode, then select a triangle and move it in the viewport.

To delete triangles: Press 'p' to enter deletion mode and click on any triangle to delete it

To handle view transforms: Press '+' or '-' to zoom in or out, respectively. Use 'w','a','s','d' to pan around the viewport.
