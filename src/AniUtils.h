/*
 *  AniUtils.h
 *  OGLlearnPart2
 *
 *  Created by josh ott on 3/12/10.
 *  Copyright 2010 Interval Studios Inc. All rights reserved.
 *
 */
#pragma once



#ifndef AniUtils_HEAD // protect against double imports
#define AniUtils_HEAD


// extern "C" is required to use this file in a c++ or objective c++ file. 
// See http://www.parashift.com/c++-faq-lite/mixing-c-and-cpp.html#faq-32.3
#ifdef __cplusplus
extern "C" {
#endif
//
//#include <math.h>
//#include "SFMT.h"
//#include "ofMain.h"

    
static inline float square(float val){
    return val * val;
}

//static inline float randomFloat(){
//	//return arc4random()%10000/(float)10000.0;
//	return ofRandom();
//}
//static inline float randomRange(float n1, float n2){
//	//float r = (arc4random()%10000)/(float)10000;
//	float r = genrand_real1();
//	return (r*(n2-n1))+n1;
//}
//// this doesn't look like it does what is implied by it's name, please document.  also: modulous operations are expensive....
//static inline int randomRangeInt(int n1, int n2){
//    return (rand() % (n2 - n1)) + n1;
//}    
    
static inline float easeTo(float targ,float goal,float divisor,float tol){
//	return targ+(goal-targ)/divisor;
	if (fabs(goal-targ)>tol) {
		//if(divisor==0)return 0;
		float amt = (goal-targ)/divisor;
		// should just return targ if round(amt)==0
		return targ+amt;
	} else {
		return goal;
	}
}
static inline float easeToInc(float targ,float goal,float divisor,float tol){
	float dif = (goal-targ);

	if (fabs(dif)>tol) {
//		if(divisor==0)return 0;
        return dif/divisor;
		//float amt = dif/divisor;
		// should just return targ if round(amt)==0
		
	} else {
		return dif;
	}
}
static inline float incrementTo(float targ, float goal, float inc){
	float ret = targ;
	if(goal-targ<0){
		inc*=-1;
		// down we go
		ret+=inc;
		if(ret<goal)ret=goal;
	}else if (goal-targ>0){
		ret+=inc;
		if(ret>goal)ret = goal;
	}
	return ret;
}
static inline float incrementToInc(float targ, float goal, float inc){
    float ret = 0;
    if(goal-targ<0){
        ret =-inc;
        // down we go
        //ret+=inc;
        if(targ+ret<goal)ret=goal-targ;
    }else if (goal-targ>0){
        ret=inc;
        if(ret+targ>goal)ret = goal-targ;
    }
    return ret;
}
static inline float proportionTo(float n1,float n2,float percent){
	// p = between 0 and 1
	return ((n2-n1)*percent)+n1;
}
static inline float proportionToCurve(float v1,float v2,float p,float *cur,int arrayLength){
    if(p<0||p>1) return proportionTo(v1,v2,p);
    // cur = array (of any length) specifying proportional values
    float idx = p*(arrayLength-1);
    int lowIndex =  floorf(idx);
    int highIndex = ceilf(idx);
    float idxm = idx;
    //	if(lowIndex != 0) idxm = idx%lowIndex;
    if(lowIndex!=0) idxm = fmodf(idx,lowIndex);
    float actualP = proportionTo(cur[lowIndex],cur[highIndex],idxm);
    return proportionTo(v1,v2,actualP);
}
static inline float distance( float x1,float y1,float x2,float y2) {
	float dx = x2-x1;
	float dy = y2-y1;
	return sqrt((dx*dx) + (dy*dy));
}
static inline float distance3D( float x1,float y1,float z1,float x2,float y2,float z2) {
    float dx = x2-x1;
    float dy = y2-y1;
    float dz = z2-z1;
    return sqrt((dx*dx) + (dy*dy) + (dz*dz));
}
static inline float distanceSquared( float x1,float y1,float x2,float y2) {
	float dx = x2-x1;
	float dy = y2-y1;
	return (dx*dx) + (dy*dy);
//	return ((x2-x1)*(x2-x1)) + ((y2-y1)*(y2-y1));
}
static inline float distanceSquared3D( float x1,float y1,float z1,float x2,float y2,float z2) {
    float dx = x2-x1;
    float dy = y2-y1;
    float dz = z2-z1;
    return (dx*dx) + (dy*dy) + (dz*dz);
}

static inline float constrain(float num,float low,float high){
	if(num<low){
		return low;
	}
	if(num>high){
		return high;
	}
	return num;
}
// this is currently very expensive:  optmize!  there's a cheaper way to do the same thing!ah I'm 
//static inline float numBounce(float n,float min,float max){
//	float range = max-min;
//	float modn = fmodf(n,range);
//	if(fmodf(floor(n/range), 2)!=1){
//	    return modn+min;
//	}else{
//	    return (range-modn)+min;
//	}
//}
static inline float contrastize(float num){
    if (num<.5) {
        return num*num;
    }else{
        return 1-num*num;
    }
}
    
//    #define TWO_PI M_PI*2
static inline float getRadianAngleDifference(float a1, float a2){
    float a = a2 - a1;
    a += (a>M_PI) ? -(M_PI*2) : (a<-M_PI) ? (M_PI*2) : 0;
    return a;
}
    
    
#ifdef __cplusplus
	}
#endif

#endif