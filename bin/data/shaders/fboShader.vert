#version 120

uniform sampler2DRect tex0;

uniform float distortAmount1 = 1.0;
uniform float distortAmount2 = 1.0;
uniform float distortAmount3 = 1.0;
uniform float distortAmount4 = 1.0;
uniform float distortAmount5 = 1.0;
uniform float distortAmount6 = 1.0;
uniform float distortAmount7 = 1.0;
uniform float distortAmount8 = 1.0;
uniform float distortAmount9 = 1.0;

uniform float timeValX = 0.0;
uniform float timeValY = 0.0;


uniform vec2 mouse;

//generate a random value from four points
vec4 rand(vec2 A,vec2 B,vec2 C,vec2 D){ 

	vec2 s=vec2(12.9898,78.233); 
	vec4 tmp=vec4(dot(A,s),dot(B,s),dot(C,s),dot(D,s)); 

	return fract(sin(tmp) * 43758.5453)* 2.0 - 1.0; 
} 
float proportionTo(float n1,float n2,float percent){
	// p = between 0 and 1
	return ((n2-n1)*percent)+n1;
}
//this is similar to a perlin noise function
float noise(vec2 coord,float d){ 

	vec2 C[4]; 

	float d1 = 1.0/d;

	C[0]=floor(coord*d)*d1; 

	C[1]=C[0]+vec2(d1,0.0); 

	C[2]=C[0]+vec2(d1,d1); 

	C[3]=C[0]+vec2(0.0,d1);


	vec2 p=fract(coord*d); 

	vec2 q=1.0-p; 

	vec4 w=vec4(q.x*q.y,p.x*q.y,p.x*p.y,q.x*p.y); 

	return dot(vec4(rand(C[0],C[1],C[2],C[3])),w); 
} 
float rand2(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(){

	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	//get our current vertex position so we can modify it
	vec4 pos = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
    vec2 pos2D = pos.xy;
    vec2 pos2Dinv =pos.yx;
	
	//lets also figure out the distance between the mouse and the vertex and apply a repelling force away from the mouse
	vec2 d = pos.xy - mouse;
    
	float dist =  sqrt(d.x*d.x + d.y*d.y);
    float maxDist = 1200;
    float masterAreaModifier = 1;
    vec4 col = gl_Color;
    
	
	gl_FrontColor =  col;
    
    gl_Position = pos;
}
 