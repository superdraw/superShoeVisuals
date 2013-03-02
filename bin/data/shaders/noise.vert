#version 120

uniform float timeValX = 0.0;
uniform float timeValY = 0.0;
uniform float distortAmount1 = 1.0;
uniform float distortAmount2 = 1.0;
uniform float distortAmount3 = 1.0;
uniform float distortAmount4 = 1.0;
uniform float distortAmount5 = 1.0;
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

	//generate some noise values based on vertex position and the time value which comes in from our OF app
	float noiseAmntX = noise( vec2(-timeValX + pos.x / 1000.0f, 100.0f), 20.0 );
	float noiseAmntY = noise( vec2(timeValY + pos.y / 1000.0f, pos.x / 2000.0f), 20.0 );

	//generate noise for our blue pixel value
	float noiseB = noise( vec2(timeValY * 0.25, pos.y / 2000.0f), 20.0 );

	//lets also figure out the distance between the mouse and the vertex and apply a repelling force away from the mouse
	vec2 d = pos.xy - mouse;
    
	float dist =  sqrt(d.x*d.x + d.y*d.y);
    float maxDist = 1200;
    float masterAreaModifier = 1;
	if( dist < maxDist && dist > 0  ){
		
        
		//lets get the distance into 0-1 ranges
		float pct = dist / maxDist;
		
		//this turns our linear 0-1 value into a curved 0-1 value
        //pct *= pct;

		//flip it so the closer we are the greater the repulsion
        masterAreaModifier = pct;
		pct = 1.0 - pct;
		
		//normalize our repulsion vector
		d /= dist;
		float p = dist/maxDist;
		//apply the repulsion to our position
		pos.x += d.x * p * 1800.0f*distortAmount3*masterAreaModifier ;
		pos.y += d.y * p * 1800.0f*distortAmount3*masterAreaModifier ;
	}
    vec4 col = gl_Color;

	//modify our position with the smooth noise
	pos.x += noiseAmntX * 20.0 *distortAmount1*masterAreaModifier;
    
    
    pos.x+=rand2(pos2D);
	pos.y += noiseAmntY * 10.0 *distortAmount1*masterAreaModifier;
	
    float aadd2 = 0.;
    if(distortAmount2!=0){
        float roundAmt = 300.*distortAmount2;
        pos.x = floor(pos.x/roundAmt)*roundAmt;
        pos.y = floor(pos.y/roundAmt)*roundAmt;
      //  aadd2 = rand2(vec2(pos.x*distortAmount2, pos.y*distortAmount2))
    }
    
    if(distortAmount4!=0){
//        float roundAmt = 100.0*distortAmount2;
//        pos.x+=distortAmount4*100.;
        if(mod(floor(pos.x),4)==0){
            pos.y +=rand2(pos2D)*distortAmount4*5000*masterAreaModifier;
            pos.y -=rand2(pos2Dinv)*distortAmount4*5000*masterAreaModifier;
            col.a-=distortAmount4*2;
            //col.a = 0;
        }else{
//            pos.y -=rand2(pos2Dinv)*distortAmount4*5000*masterAreaModifier;
//            col.a-=distortAmount4*2;
//            col.a = 1-distortAmount4;
        }

//        pos.x = floor(pos.x/roundAmt)*roundAmt;
//        pos.y = floor(pos.y/roundAmt)*roundAmt;
        //  aadd2 = rand2(vec2(pos.x*distortAmount2, pos.y*distortAmount2))
    }
    if(distortAmount5!=0){
        // modulate color based on distance:
        //pos.x+=sin(masterAreaModifier*distortAmount5*20)*distortAmount5*100;
        //pos.y+=cos(masterAreaModifier*distortAmount5*20)*distortAmount5*100;
        float a = sin(timeValX+ masterAreaModifier*distortAmount5*100)*distortAmount5;
        float r =cos(timeValX+ masterAreaModifier*distortAmount5*100)*distortAmount5;
        float g =cos(timeValX+ masterAreaModifier*distortAmount5*40)*distortAmount5;
                float b =sin(timeValX+ masterAreaModifier*distortAmount5*80)*distortAmount5;
        col.a = proportionTo(col.a,a,distortAmount5);
            col.r =proportionTo(col.r,r,distortAmount5);
                    col.g =proportionTo(col.g,g,distortAmount5);
                    col.b =proportionTo(col.b,b,distortAmount5);
    }
    
	//finally set the pos to be that actual position rendered
	gl_Position = pos;

    // distortion 5:
    
	//modify our color
	
//	col.b += noiseB;
    col.a += noiseB*distortAmount1*masterAreaModifier;
    col.a+=distortAmount2*masterAreaModifier;
	
	gl_FrontColor =  col;
}
 