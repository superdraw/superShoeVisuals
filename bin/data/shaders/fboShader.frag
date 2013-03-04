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

float rand2(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float proportionTo(float n1,float n2,float percent){
	// p = between 0 and 1
	return ((n2-n1)*percent)+n1;
}

void main(){
    vec2 st = gl_TexCoord[0].st;
    vec4 colorBase = texture2DRect(tex0, st);
    vec4 col = colorBase;
    vec2 timeVal = vec2(timeValX,timeValY);
    // pull a couple more coordinates:
   // if(col.r>0.5 && col.b >0.5 && col.g>0.5){
        // we're most likely white,

//        col.r += rand2(gl_FragCoord.xy*timeValX)*2.-1.;
//        col.g += rand2(gl_FragCoord.xy*timeValX)*2.-1.;
//        col.b += rand2(gl_FragCoord.xy*timeValX)*2.-1.;
        
        if(distortAmount8!=0){
            for (int i =0; i<5;i++) {
                float p = i*.1;
                vec2 pt = vec2(st.x-p*100.*distortAmount8,st.y-p*100.*distortAmount8);
                vec4 newCol = texture2DRect(tex0,pt);
                if(newCol.r>0.5 && newCol.b >0.5 && newCol.g>0.5){
                    col+=newCol*.1;
                }
                
            }
        }
//
    //}
//    vec4 colorBase = gl_frag
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
   // col.a = rand2(gl_FragCoord.xy);
    
    if(distortAmount5>0.){

//        col.a  = proportionTo(col.a,sin(distortAmount5*gl_FragCoord.y)*(1-distortAmount5),distortAmount5 );

    }
    if(distortAmount9>0.){
//        col.a  = proportionTo(col.a,rand2(vec2(timeValX*10.,timeValY)*gl_FragCoord.xy)*2.-1.,distortAmount9 );
    }
    
//    col.r += rand2(gl_FragCoord.xy*timeValX)*2.-1.;
//    col.g += rand2(gl_FragCoord.xy*timeValX)*2.-1.;
//    col.b += rand2(gl_FragCoord.xy*timeValX)*2.-1.;
//    col.a += rand2(gl_FragCoord.xy*timeValX)*2.-1.;
//    col.a += sin(timeValX);
    gl_FragColor =col;
	
}