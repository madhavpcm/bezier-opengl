
attribute highp vec2 coord2d;
attribute lowp vec4 colAttr;
varying lowp vec4 col;
uniform highp mat4 matrix;
void main() {
   col = vec4(coord2d.x,coord2d.y, 1, 1) ;

   gl_Position = matrix * vec4(coord2d,0,1);
};
