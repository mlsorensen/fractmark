/*
Written by Marcus Sorensen
Copyright (C) 2007-2011
learnitwithme.com

This code has been successfully compiled and run on Ubuntu Linux and 
Windows 7 x64. It makes use of the GD library, freely available at libgd.org. 
The source for this program (FractMark) is distributed under the Clarified 
Artistic License. This is unsupported software, but please make an effort
to report any bugs to me at <mlsorensen@mlsorensen.com>

Versions:
0.2 - Release Candidate 1

0.5 - Trying new color algorithm that scales with iterations.  Fixed bug in 
      percent printing where different threads could potentially think the total
      height was different.
*/

#include <gd.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/timeb.h>

char version[] = "FractMark 0.2 (02/07/2011) by Marcus Sorensen";

gdImagePtr im;
struct timeb starttm; //keep track of timing on things
struct timeb endtm; //keep track of timing on things
int width = 7680; //image width
int height = 2160; //image height, ypos changes, this won't
int ypos = 2160; //current line
int r = 22; //red base 23
int g = 193; //green base 161
int b = 229; //blue base 173
long double xmin = -.431; //negative x axis
long double xmax = -.357; //positive x axis
long double ymin = -.613; //negative y axis
long double ymax; //calculate high y axis according to aspect ratio;
long double xsize;//increment unit for real number line (how big a pixel is);
long double ysize;//increment unit for imaginary;
int limit = 20000; //number of times to iterate before giving up and painting background color
int (*pcolor)[1]; //pointer to array of allocated colors
char *outfile[255] = {""};//output file name
long long unsigned (*iter)[1];//pointer to array of thread iterations
int numthreads = 8;//number of threads to use

extern char *optarg;

pthread_mutex_t height_mutex     = PTHREAD_MUTEX_INITIALIZER;

//prototypes 
void *colorpixel(void *ptr);
void colorallocate();
void newcolorallocate(int r,int g,int b);
void parseargs(int argc, char *argv[]);
void usage();

int main(int argc, char *argv[]) {
	
	/*process arguments*/
	
	parseargs(argc,argv);

	int colors[limit];
	pcolor = &colors;
	long long unsigned iterations[numthreads];
	iter = &iterations;
	long long unsigned iter_total = 0;
	
	float starttime;
	float endtime;
	
	FILE *out;
	im = gdImageCreateTrueColor(width,height);//gd image
	ymax = ymin + (xmax - xmin) * ((long double)height / width);
	xsize = (xmax - xmin) / (width - 1);
	ysize = (ymax - ymin) / (height - 1);
	int i;
	
	pthread_t threads[numthreads];//create an array of threads
	
	//let's allocate some colors
	
	colorallocate(r,g,b);

	//tell us what we're creating
	printf("creating a %d by %d image\nthreads set to %d\ncoordinates are (X,x,y) = (%f,%f,%f)\n", width, height, numthreads,(float)xmax,(float)xmin,(float)ymin);

	//get start time
	ftime(&starttm);
	long long int startsec = starttm.time;
	double startms = starttm.millitm;

	//put our threads to work
	for(i = 0; i < numthreads; i++) {
		iterations[i] = 0;
		printf("Spawning worker %d\n",i);
		pthread_create(&threads[i], NULL, colorpixel, (void *)(long)i);
	}

	//wait for the threads to complete
	for(i = 0; i < numthreads; i++) {
		pthread_join( threads[i], NULL);
	}

	//get end time
	ftime(&endtm);
	long long int endsec = endtm.time;
	double endms = endtm.millitm;
	double totaltime = (double)(endsec-startsec)+((endms-startms)/1000);

	//add up iterations
	for(i = 0; i < numthreads; i++) {
		iter_total += iterations[i];
	}
	
	printf("100%%\ndone in %8.3f seconds, %llu iterations (%10.2lf per second)\n\nFractMark score: %4.2lf\n\n",totaltime,iter_total,(double)iter_total/totaltime,(((double)iter_total/totaltime)/(double)100000000)*3.14);

	//write the resulting image
	if(strlen(*outfile) > 0) {
		printf("saving image to %s\n",*outfile);
		out = fopen(*outfile,"wb");
		if(strstr(*outfile,".jpg\0") != NULL) {
			gdImageJpeg(im, out,85);
		}
		else {
			gdImagePng(im, out);
		}
		fclose(out);
	}	

	//clean up
	gdImageDestroy(im);
}

void *colorpixel(void *ptr) {
	long num = (long)ptr;
	int percent = ceil((float)height/10);
	
	while(ypos > 0) {		

		pthread_mutex_lock(&height_mutex);
		int pixy = ypos;
		ypos--;
		pthread_mutex_unlock(&height_mutex);
		long double y = ymax - ((long double)ypos * ysize);
		int j;

		for(j = 0; j < width; j++) {
			long double x = xmin + ((long double)j * xsize);
			int pixx = j;
			long double zx = x;
			long double zy = y;
			int k;
			for(k = 0; k < limit; k++) {
				long double zx2 = zx * zx;
				long double zy2 = zy * zy;
				if(zx2 + zy2 > 4.0) {
					gdImageSetPixel(im, pixx, pixy, *pcolor[k]);
					*iter[num] += k;
					break;
				}
				zy = 2.0 * zx * zy + y;
				zx = zx2 - zy2 + x;
			}
			*iter[num] += k;
		}
		
		if(pixy % percent == 0) {
			int progress = ((float)(height-pixy)/height) *100;
			printf("%d%%..",progress);
			fflush(stdout);
		}
	}
}
/*
void colorallocate(int r, int g, int b) {
	int tempr,tempb,tempg,i,num,lowarr[768][3];
	
	tempr=r;
	tempg=g;
	tempb=b;
	num=0;
	
	*pcolor[0] = gdImageColorAllocate(im,0,0,0);//background is black
	
	while(tempr>0 || tempg>0 || tempb>0) {
	  	for(i=0;i<3;i++){
	  		
	  		if(i==0 && tempr == 0){		
	  				continue;
	  		}
	  		if(i==1 && tempg == 0){		
	  				continue;
	  		}
	  		if(i==2 && tempb == 0){		
	  				continue;
	  		}
	  		
	  		if(i==0){		
	  				tempr--;
	  		}
	  		if(i==1){
	  				tempg--;
	  		}
	  		if(i==2){
	  				tempb--;
	  		}
	  		num++;
	  		lowarr[num][0] = tempr;
	  		lowarr[num][1] = tempg;
	  		lowarr[num][2] = tempb;
	  		
	  	}
	}
	
	for(i=0;i<num;i++){
		*pcolor[i+1] = gdImageColorAllocate(im,lowarr[num-i][0],lowarr[num-i][1],lowarr[num-i][2]);	
	}
	
	tempr = r;
	tempg = g;
	tempb = b;
	
	while(tempr < 255 || tempg < 255 || tempb < 255 || num < limit) {
		for(i=0;i<3;i++){
	  		if(i==0){
	  				tempr++;
	  		}
	  		if(i==1){
	  				tempg++;
	  		}
	  		if(i==2){
	  				tempb++;
	  		}
	  		if(tempr > 255)
	  				tempr = 255;
	  		if(tempg > 255)
	  				tempg = 255;
	  		if(tempb > 255)
	  				tempb = 255;
	  		
	  		*pcolor[num]=gdImageColorAllocate(im,tempr,tempg,tempb);
	  		num++;
		}
	  		
	}
}*/

void colorallocate(int r, int g, int b) {
	double rstep,gstep,bstep,rinc,ginc,binc,sf;
	int i;
	sf = limit * .02;
	
	rstep = r / sf;
	gstep = g / sf;
	bstep = b / sf;
	rinc = 0;
	ginc = 0;
	binc = 0;	
	
	//gradient up to chosen color at 67% of colors
	for(i=0;i<sf;i++){
		*pcolor[i]=gdImageColorAllocate(im,(int)rinc,(int)ginc,(int)binc);
		rinc += rstep;
		ginc += gstep;
		binc += bstep;
	}
	
	//rest of 33% colors go to white
	
	sf = limit * .02;
	
	rstep = (255 - rinc) / sf;
	gstep = (255 - ginc) / sf;
	bstep = (255 - binc) / sf;
	
	int whitelimit = (limit-(limit * .96));
	for(i;i<whitelimit;i++){
		*pcolor[i]=gdImageColorAllocate(im,(int)rinc,(int)ginc,(int)binc);
		rinc += rstep;
		ginc += gstep;
		binc += bstep;
	}

	
	for(i;i<limit;i++){
		*pcolor[i]=gdImageColorAllocate(im,255,255,255);	
	}
	
	
}

void parseargs(int argc, char *argv[]) {
	int c;
	
	while ((c = getopt (argc, argv, "ho:t:l:W:H:c:x:X:y:r:g:b:")) != -1){
		switch(c) {
			case 'o':
				if(strlen(optarg) > (sizeof(outfile)/sizeof(*outfile))){
					fprintf(stderr,"\nfile name too long, size should be <= %lu chars\n\n",sizeof(outfile)/sizeof(*outfile));
					exit(1);
				}
				*outfile = optarg;
				if(strstr(*outfile,".jpg\0") == NULL && strstr(*outfile,".png\0") == NULL){
					*outfile = strcat(*outfile,".png");
				}
				break;
			case 't':
				numthreads = atoi(optarg);
				break;
			case 'l':
				limit = atoi(optarg);
				break;
			case 'W':
				width = atoi(optarg);
				break;
			case 'H':
				height = atoi(optarg);
				ypos = height;
				break;
			case 'x':
				xmin = (long double)strtod(optarg,NULL);
				break;
			case 'X':
				xmax = (long double)strtod(optarg,NULL);
				break;
			case 'y':
				ymin = (long double)strtod(optarg,NULL);
				break;
			case 'r':
				r = atoi(optarg);
				break;
			case 'g':
				g = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 'h':
				usage();
				break;
			
		}
	}
}

void usage(){
	fprintf(stderr,"\n%s\n\n",version);
	fprintf(stderr,"Usage: fractmark [OPTION]...\n");
	fprintf(stderr,"With no options, program runs iterations, prints out results, and ends\n\n");
	fprintf(stderr,"Defaults used are:\n");
	fprintf(stderr,"  save image:                        NONE\n");
	fprintf(stderr,"  number of threads:                 %d\n",numthreads);
	fprintf(stderr,"  limit of iterations per pixel:     %d\n",limit);
	fprintf(stderr,"  width in pixels:                   %d\n",width);
	fprintf(stderr,"  height in pixels:                  %d\n",height);
	fprintf(stderr,"  coordinates (Max X, Min X, Min Y): %f, %f, %f\n",(double)xmax,(double)xmin,(double)ymin);
	fprintf(stderr,"  color bias (r, g, b):              %d, %d, %d\n",r,g,b);
	fprintf(stderr,"\n");
	fprintf(stderr,"OPTIONS:\n");
	fprintf(stderr,"  -o FILENAME   Save image as FILENAME(.png default, or specify .png or .jpg in name)\n");
	fprintf(stderr,"  -t INTEGER    Number of worker threads to spawn\n");
	fprintf(stderr,"  -l INTEGER    Limit of iterations at which to give up and color pixel black\n");
	fprintf(stderr,"  -W INTEGER    Width of field in pixels\n");
	fprintf(stderr,"  -H INTEGER    Height of field in pixels\n");
	fprintf(stderr,"  -X FLOAT      Decimal number, maximum X coordinate of field\n");
	fprintf(stderr,"  -x FLOAT      Decimal number, minimum X coordinate of field\n");
	fprintf(stderr,"  -y FLOAT      Decimal number, minimum Y coordinate of field.\n"); 
        fprintf(stderr,"                  Note: Max Y is auto-calcualted based on aspect ratio W x H\n");
        fprintf(stderr,"  -r INTEGER    Red bias of image (0-255)\n");
        fprintf(stderr,"  -g INTEGER    Green bias of image (0-255)\n");
        fprintf(stderr,"  -b INTEGER    Blue bias of image (0-255)\n");
        fprintf(stderr,"  -h            Print this help dialog and exit\n");
	exit(1);
}


/*
                    The Clarified Artistic License

				Preamble

The intent of this document is to state the conditions under which a
Package may be copied, such that the Copyright Holder maintains some
semblance of artistic control over the development of the package,
while giving the users of the package the right to use and distribute
the Package in a more-or-less customary fashion, plus the right to make
reasonable modifications.

Definitions:

	"Package" refers to the collection of files distributed by the
	Copyright Holder, and derivatives of that collection of files
	created through textual modification.

	"Standard Version" refers to such a Package if it has not been
	modified, or has been modified in accordance with the wishes
	of the Copyright Holder as specified below.

	"Copyright Holder" is whoever is named in the copyright or
	copyrights for the package.

	"You" is you, if you're thinking about copying or distributing
	this Package.

	"Distribution fee" is a fee you charge for providing a copy
        of this Package to another party.

	"Freely Available" means that no fee is charged for the right to
        use the item, though there may be fees involved in handling the
        item.  It also means that recipients of the item may redistribute
        it under the same conditions they received it.

1. You may make and give away verbatim copies of the source form of the
Standard Version of this Package without restriction, provided that you
duplicate all of the original copyright notices and associated disclaimers.

2. You may apply bug fixes, portability fixes and other modifications
derived from the Public Domain, or those made Freely Available, or from
the Copyright Holder.  A Package modified in such a way shall still be
considered the Standard Version.

3. You may otherwise modify your copy of this Package in any way, provided
that you insert a prominent notice in each changed file stating how and
when you changed that file, and provided that you do at least ONE of the
following:

    a) place your modifications in the Public Domain or otherwise make them
    Freely Available, such as by posting said modifications to Usenet or an
    equivalent medium, or placing the modifications on a major network
    archive site allowing unrestricted access to them, or by allowing the
    Copyright Holder to include your modifications in the Standard Version
    of the Package.

    b) use the modified Package only within your corporation or organization.

    c) rename any non-standard executables so the names do not conflict
    with standard executables, which must also be provided, and provide
    a separate manual page for each non-standard executable that clearly
    documents how it differs from the Standard Version.

    d) make other distribution arrangements with the Copyright Holder.

    e) permit and encourge anyone who receives a copy of the modified Package
       permission to make your modifications Freely Available
       in some specific way.


4. You may distribute the programs of this Package in object code or
executable form, provided that you do at least ONE of the following:

    a) distribute a Standard Version of the executables and library files,
    together with instructions (in the manual page or equivalent) on where
    to get the Standard Version.

    b) accompany the distribution with the machine-readable source of
    the Package with your modifications.

    c) give non-standard executables non-standard names, and clearly
    document the differences in manual pages (or equivalent), together
    with instructions on where to get the Standard Version.

    d) make other distribution arrangements with the Copyright Holder.

    e) offer the machine-readable source of the Package, with your
       modifications, by mail order.

5. You may charge a distribution fee for any distribution of this Package.
If you offer support for this Package, you may charge any fee you choose
for that support.  You may not charge a license fee for the right to use
this Package itself.  You may distribute this Package in aggregate with
other (possibly commercial and possibly nonfree) programs as part of a
larger (possibly commercial and possibly nonfree) software distribution,
and charge license fees for other parts of that software distribution,
provided that you do not advertise this Package as a product of your own.
If the Package includes an interpreter, You may embed this Package's
interpreter within an executable of yours (by linking); this shall be
construed as a mere form of aggregation, provided that the complete
Standard Version of the interpreter is so embedded.

6. The scripts and library files supplied as input to or produced as
output from the programs of this Package do not automatically fall
under the copyright of this Package, but belong to whoever generated
them, and may be sold commercially, and may be aggregated with this
Package.  If such scripts or library files are aggregated with this
Package via the so-called "undump" or "unexec" methods of producing a
binary executable image, then distribution of such an image shall
neither be construed as a distribution of this Package nor shall it
fall under the restrictions of Paragraphs 3 and 4, provided that you do
not represent such an executable image as a Standard Version of this
Package.

7. C subroutines (or comparably compiled subroutines in other
languages) supplied by you and linked into this Package in order to
emulate subroutines and variables of the language defined by this
Package shall not be considered part of this Package, but are the
equivalent of input as in Paragraph 6, provided these subroutines do
not change the language in any way that would cause it to fail the
regression tests for the language.

8. Aggregation of the Standard Version of the Package with a commercial
distribution is always permitted provided that the use of this Package
is embedded; that is, when no overt attempt is made to make this Package's
interfaces visible to the end user of the commercial distribution.
Such use shall not be construed as a distribution of this Package.

9. The name of the Copyright Holder may not be used to endorse or promote
products derived from this software without specific prior written permission.

10. THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

				The End
*/
