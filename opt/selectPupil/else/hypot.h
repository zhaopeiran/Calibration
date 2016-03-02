#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>



static float robust_pytagoras_after_MOLAR_MORRIS(float a, float b){

	a=abs(a);
	b=abs(b);

	float p=a>b?a:b;
	float q=a<b?a:b;

	float r,s;
	
	while(q>0){
		r= (q/p)*(q/p);
		s=r/(4+r);
		p=p+(1*s*p);
		q=s*q;
	}

	return p;

}

static float hypot(float a,float b)
{
	a=abs(a);
	b=abs(b);
    float t=a<b?a:b;
	float x=a>b?a:b;

    t = t/x;
    return x*sqrt(1+(t*t));
}



//#define __HI( x ) ( ((int*)(&x))[0] )
//#define __LO( x ) ( ((int*)(&x))[1] )
#define __HI( x ) ( ((int*)(&x))[1] )
#define __LO( x ) ( ((int*)(&x))[0] )

double __ieee754_hypot(double x, double y){
	double a=x,b=y,t1,t2,y1,y2,w;
	int j,k,ha,hb;

	ha = __HI(x)&0x7fffffff;	/* high word of  x */
	hb = __HI(y)&0x7fffffff;	/* high word of  y */
	if(hb > ha) {a=y;b=x;j=ha; ha=hb;hb=j;} else {a=x;b=y;}
	__HI(a) = ha;	/* a <- |a| */
	__HI(b) = hb;	/* b <- |b| */
	if((ha-hb)>0x3c00000) {return a+b;} /* x/y > 2**60 */
	k=0;
	if(ha > 0x5f300000) {	/* a>2**500 */
	   if(ha >= 0x7ff00000) {	/* Inf or NaN */
	       w = a+b;			/* for sNaN */
	       if(((ha&0xfffff)|__LO(a))==0) w = a;
	       if(((hb^0x7ff00000)|__LO(b))==0) w = b;
	       return w;
	   }
	   /* scale a and b by 2**-600 */
	   ha -= 0x25800000; hb -= 0x25800000;	k += 600;
	   __HI(a) = ha;
	   __HI(b) = hb;
	}
	if(hb < 0x20b00000) {	/* b < 2**-500 */
	    if(hb <= 0x000fffff) {	/* subnormal b or 0 */	
		if((hb|(__LO(b)))==0) return a;
		t1=0;
		__HI(t1) = 0x7fd00000;	/* t1=2^1022 */
		b *= t1;
		a *= t1;
		k -= 1022;
	    } else {		/* scale a and b by 2^600 */
	        ha += 0x25800000; 	/* a *= 2^600 */
		hb += 0x25800000;	/* b *= 2^600 */
		k -= 600;
	   	__HI(a) = ha;
	   	__HI(b) = hb;
	    }
	}
    /* medium size a and b */
	w = a-b;
	if (w>b) {
	    t1 = 0;
	    __HI(t1) = ha;
	    t2 = a-t1;
	    w  = sqrt(t1*t1-(b*(-b)-t2*(a+t1)));
	} else {
	    a  = a+a;
	    y1 = 0;
	    __HI(y1) = hb;
	    y2 = b - y1;
	    t1 = 0;
	    __HI(t1) = ha+0x00100000;
	    t2 = a - t1;
	    w  = sqrt(t1*y1-(w*(-w)-(t1*y2+t2*b)));
	}
	if(k!=0) {
	    t1 = 1.0;
	    __HI(t1) += (k<<20);
	    return t1*w;
	} else return w;
}


