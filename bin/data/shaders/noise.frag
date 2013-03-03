#version 120

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

float proportionTo(float n1,float n2,float percent){
	// p = between 0 and 1
	return ((n2-n1)*percent)+n1;
}

void main(){
	//this is the fragment shader
	//this is where the pixel level drawing happens
	//gl_FragCoord gives us the x and y of the current pixel its drawing
	
	//we grab the x and y and store them in an int
	float xVal = gl_FragCoord.x;
	float yVal = gl_FragCoord.y;
	
	//we use the mod function to only draw pixels if they are every 2 in x or every 4 in y
//	if( mod(xVal, 2.0) == 0.5 && mod(yVal, 4.0) == 0.5 ){
//		
//    }else{
//		gl_FragColor.a = 0.0;
//	}
    vec4 col = gl_Color;
    if(distortAmount5>0.){
//        col+= col* cos(sin(timeValX))*distortAmount5;
        col.a  = proportionTo(col.a,sin(distortAmount5*gl_FragCoord.y)*(1-distortAmount5),distortAmount5 );
//        col.a = (1-distortAmount5) + sin(distortAmount5*gl_FragCoord.y)*(1-distortAmount5);
    }
    if(distortAmount9>0.){
        col.a  = proportionTo(col.a,sin(distortAmount9*gl_FragCoord.y*timeValX*10*gl_FragCoord.x),distortAmount9 );
    }
    gl_FragColor =col;
	
}