/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// spissmandelbrot.cpp : Defines the entry point for the console application.
// nakedsoftware.org, spi@oifii.org, stephane.poirier@oifii.org

#include "stdafx.h"

/*
int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}
*/



#include <windows.h>
#include  <scrnsave.h>
#include  <GL/gl.h>
#include <GL/glu.h>
//#include "glut.h" // include GLUT library header
#include <math.h>
#include <ctime>
//#include <iostream>
//#include <fstream>
#include <stdio.h>
#include <vector>
#include "resource.h"

//std::ofstream myofstream;
FILE* pFILE = NULL;

BYTE global_alpha=200;

class glfloatpair
{
public:
	GLfloat x;
	GLfloat y;

	glfloatpair(GLfloat xx, GLfloat yy)
	{
		x = xx;
		y = yy;
	}
	glfloatpair(double xx, double yy)
	{
		x = xx;
		y = yy;
	}
};

std::vector<glfloatpair*> global_pairvector;
std::vector<glfloatpair*>::iterator it;

//get rid of these warnings:
//truncation from const double to float
//conversion from double to float
#pragma warning(disable: 4305 4244) 
                        

//Define a Windows timer

#define TIMER 1



//These forward declarations are just for readability,
//so the big three functions can come first 

void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void GetConfig();               
void WriteConfig(HWND hDlg);
void SetupAnimation(int Width, int Height);
void CleanupAnimation();
void OnTimer(HDC hDC);


int Width, Height; //globals for size of screen



int windowID;

GLfloat minX = -2.2f, maxX = 0.8f, minY = -1.5f, maxY = 1.5; // complex plane boundaries
GLfloat stepX = (maxX - minX)/(GLfloat)Width;
GLfloat stepY = (maxY - minY)/(GLfloat)Height;

GLfloat xCenter = (minX + maxX)/2.0;
GLfloat yCenter = (minY + maxY)/2.0;
GLfloat xWidth = abs(maxX - minX);
GLfloat yHeight = abs(maxY - minY);

//GLfloat[] black = {0.0f, 0.0f, 0.0f}; // black color
GLfloat black[] = {0.0f, 0.0f, 0.0f}; // black color
const int paletteSize = 128;
GLfloat palette[paletteSize][3];

const GLfloat radius = 5.0f;
bool fullScreen=false;


void mandelbrotzoom()
{
	/*
	GLfloat xCenter = (minX + maxX)/2.0;
	GLfloat yCenter = (minY + maxY)/2.0;
	*/
	//pick a mandelbrot edge point for center and keep window width constant
	int random_integer;
	int lowest=0, highest=global_pairvector.size()-1;
	int range=(highest-lowest)+1;
	random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));
	//myofstream << random_integer << " out of " << highest << "\n"; //this line had not effect
	//if(pFILE) fprintf(pFILE, "picked %d out of %d\n", random_integer, highest);
	GLfloat xCenterNew = (global_pairvector[random_integer])->x;
	GLfloat yCenterNew = (global_pairvector[random_integer])->y; 
	GLfloat xWidthNew = xWidth/16.0; //constant
	GLfloat yHeightNew = yHeight/16.0; //constant
	minX = xCenterNew - xWidthNew/2.0;
	maxX = xCenterNew + xWidthNew/2.0;
	minY = yCenterNew - yHeightNew/2.0;
	maxY = yCenterNew + yHeightNew/2.0;

	//same as before
	stepX = (maxX - minX)/(GLfloat)Width;
	stepY = (maxY - minY)/(GLfloat)Height;

}

//****************************************
GLfloat* calculateColor(GLfloat u, GLfloat v)
{
	GLfloat re = u;
	GLfloat im = v;
	GLfloat tempRe=0.0;
	for(int i=0; i < paletteSize; i++)
	{
		tempRe = re*re - im*im + u;
		im = re * im * 2 + v;
		re = tempRe;
		if( (re*re + im*im) > radius )
		{
			//spi, begin
			//to display edges only
			//if(i<32) return black;
			//spi, end
			return palette[i];
		}
	}
	return black;
}

//****************************************
void calculateEdges(GLfloat u, GLfloat v)
{
	GLfloat re = u;
	GLfloat im = v;
	GLfloat tempRe=0.0;
	for(int i=0; i < paletteSize; i++)
	{
		tempRe = re*re - im*im + u;
		im = re * im * 2 + v;
		re = tempRe;
		if( (re*re + im*im) > radius )
		{
			//to display edges only
			if(i<32) 
			{
				/*
				myofstream << "i<32\n";
				*/
				return; //black;
			} 
			//add pair to vector
			global_pairvector.push_back(new glfloatpair(u,v));
			/*
			myofstream << u << ", ";
			myofstream << v << "\n";
			*/
			if(pFILE) fprintf(pFILE, "u,v=%f,%f\n",u,v);
			return; // palette[i];
		}
	}
	return;// black;
}

/*
//****************************************
void repaint() 
{// function called to repaint the window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen buffer
	glBegin(GL_POINTS); // start drawing in single pixel mode
	for(GLfloat y = maxY; y >= minY; y -= stepY)
	{
		for(GLfloat x = minX; x <= maxX; x += stepX)
		{
			glColor3fv(calculateColor(x,y)); // set color
			glVertex3f(x, y, 0.0f); // put pixel on screen (buffer) - [ 1 ]
		}
	}
	glEnd(); // end drawing
	glutSwapBuffers(); // swap the buffers - [ 2 ]
}
*/

/*
//****************************************
void reshape (int w, int h)
{ // function called when window size is changed
	stepX = (maxX-minX)/(GLfloat)w; // calculate new value of step along X axis
	stepY = (maxY-minY)/(GLfloat)h; // calculate new value of step along Y axis
	glViewport (0, 0, (GLsizei)w, (GLsizei)h); // set new dimension of viewable screen
	glutPostRedisplay(); // repaint the window
}
*/

//****************************************
void createPalette()
{
	
	for(int i=0; i < 32; i++)
	{
		palette[i][0] = (8*i)/(GLfloat)255;
		palette[i][1] = (128-4*i)/(GLfloat)255;
		palette[i][2] = (255-8*i)/(GLfloat)255;
	}
	for(int i=0; i < 32; i++)
	{
		palette[32+i][0] = (GLfloat)1;
		palette[32+i][1] = (8*i)/(GLfloat)255;
		palette[32+i][2] = (GLfloat)0;
	}
	for(int i=0; i < 32; i++)
	{
		palette[64+i][0] = (128-4*i)/(GLfloat)255;
		palette[64+i][1] = (GLfloat)1;
		palette[64+i][2] = (8*i)/(GLfloat)255;
	}
	for(int i=0; i < 32; i++)
	{
		palette[96+i][0] = (GLfloat)0;
		palette[96+i][1] = (255-8*i)/(GLfloat)255;
		palette[96+i][2] = (8*i)/(GLfloat)255; 
	}
	
	/*
	//red tone palette
	for(int i=0; i < paletteSize; i++)
	{
		palette[i][0] = i/(GLfloat)paletteSize;
		palette[i][1] = 0/(GLfloat)paletteSize;
		palette[i][2] = 0/(GLfloat)paletteSize;
	}
	*/
	/*
	//blue tone palette
	for(int i=0; i < paletteSize; i++)
	{
		palette[i][0] = 0/(GLfloat)paletteSize;
		palette[i][1] = 0/(GLfloat)paletteSize;
		palette[i][2] = i/(GLfloat)paletteSize;
	}
	*/
	/*
	//green tone palette
	for(int i=0; i < paletteSize; i++)
	{
		palette[i][0] = 0/(GLfloat)paletteSize;
		palette[i][1] = i/(GLfloat)paletteSize;
		palette[i][2] = 0/(GLfloat)paletteSize;
	}
	*/
	/*
	//gray tone palette
	for(int i=0; i < paletteSize; i++)
	{
		palette[i][0] = i/(GLfloat)paletteSize;
		palette[i][1] = i/(GLfloat)paletteSize;
		palette[i][2] = i/(GLfloat)paletteSize;
	}
	*/
	/*
	//black and white
	for(int i=0; i < 32; i++)
	{
		palette[i][0] = 255/(GLfloat)255;
		palette[i][1] = 255/(GLfloat)255;
		palette[i][2] = 255/(GLfloat)255;
	}
	for(int i=0; i < 32; i++)
	{
		palette[32+i][0] = (GLfloat)0;
		palette[32+i][1] = (GLfloat)0;
		palette[32+i][2] = (GLfloat)0;
	}
	for(int i=0; i < 32; i++)
	{
		palette[64+i][0] = 255/(GLfloat)255;
		palette[64+i][1] = 255/(GLfloat)255;
		palette[64+i][2] = 255/(GLfloat)255;
	}
	for(int i=0; i < 32; i++)
	{
		palette[96+i][0] = (GLfloat)0;
		palette[96+i][1] = (GLfloat)0;
		palette[96+i][2] = (GLfloat)0; 
	}
	*/
	/*
	//black and yellow
	for(int i=0; i < 32; i++)
	{
		palette[i][0] = 255/(GLfloat)255;
		palette[i][1] = 204/(GLfloat)255;
		palette[i][2] = 0/(GLfloat)255;
	}
	for(int i=0; i < 32; i++)
	{
		palette[32+i][0] = (GLfloat)0;
		palette[32+i][1] = (GLfloat)0;
		palette[32+i][2] = (GLfloat)0;
	}
	for(int i=0; i < 32; i++)
	{
		palette[64+i][0] = 255/(GLfloat)255;
		palette[64+i][1] = 204/(GLfloat)255;
		palette[64+i][2] = 0/(GLfloat)255;
	}
	for(int i=0; i < 32; i++)
	{
		palette[96+i][0] = (GLfloat)0;
		palette[96+i][1] = (GLfloat)0;
		palette[96+i][2] = (GLfloat)0; 
	}
	*/
}




//////////////////////////////////////////////////
////   INFRASTRUCTURE -- THE THREE FUNCTIONS   ///
//////////////////////////////////////////////////


// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{
  static HDC hDC;
  static HGLRC hRC;
  static RECT rect;

	//debug
	//myofstream.open("debug.txt", std::ios::out);
	//myofstream << "test line\n";
    pFILE = fopen("debug.txt", "w");
	//fprintf(pFILE, "test1\n");

  //////////////////////////
	//initialize random number
	//////////////////////////
	srand((unsigned)time(0));

  switch ( message ) 
  {

  case WM_CREATE: 
		//spi, avril 2015, begin
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);
		//SetLayeredWindowAttributes(h, 0, 200, LWA_ALPHA);
		//spi, avril 2015, end

		// get window dimensions
		GetClientRect( hWnd, &rect );
		Width = rect.right;         
		Height = rect.bottom;
    
        //get configuration from registry
        GetConfig();

        // setup OpenGL, then animation
		InitGL( hWnd, hDC, hRC );
		SetupAnimation(Width, Height);

		//store mandelbrot edges
		for(GLfloat y = maxY; y >= minY; y -= stepY)
		{
			for(GLfloat x = minX; x <= maxX; x += stepX)
			{
				calculateEdges(x,y); 
			}
		}
		/*
		//these lines have not effect?????
		myofstream << "test line2\n";
		myofstream.flush();
		*/

		//set timer to tick every 10 ms
		//SetTimer( hWnd, TIMER, 10, NULL );
		//SetTimer( hWnd, TIMER, 10000, NULL );
		//SetTimer( hWnd, TIMER, 5000, NULL );
		SetTimer( hWnd, TIMER, 2500, NULL );
    return 0;
 
  case WM_DESTROY:
		KillTimer( hWnd, TIMER );
        CleanupAnimation();
		CloseGL( hWnd, hDC, hRC );
		for(it=global_pairvector.begin(); it!=global_pairvector.end(); it++)
		{
			if(*it) delete *it;
		}
		//myofstream.close();
		if(pFILE) fclose(pFILE);

    return 0;

  case WM_TIMER:
    OnTimer(hDC);       //animate!      
    return 0;                           

  }

  return DefScreenSaverProc(hWnd, message, wParam, lParam );

}

bool bTumble = true;


BOOL WINAPI
ScreenSaverConfigureDialog(HWND hDlg, UINT message, 
                           WPARAM wParam, LPARAM lParam)
{

  //InitCommonControls();  
  //would need this for slider bars or other common controls

  HWND aCheck;

  switch ( message ) 
  {

        case WM_INITDIALOG:
                LoadString(hMainInstance, IDS_DESCRIPTION, szAppName, 40);

                GetConfig();
				/*
                aCheck = GetDlgItem( hDlg, IDC_TUMBLE );
                SendMessage( aCheck, BM_SETCHECK, 
                bTumble ? BST_CHECKED : BST_UNCHECKED, 0 );
				*/

    return TRUE;

  case WM_COMMAND:
    switch( LOWORD( wParam ) ) 
                { 
			/*
            case IDC_TUMBLE:
                        bTumble = (IsDlgButtonChecked( hDlg, IDC_TUMBLE ) == BST_CHECKED);
                        return TRUE;
			*/
                //cases for other controls would go here

            case IDOK:
                        WriteConfig(hDlg);      //get info from controls
                        EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
                        return TRUE; 

                case IDCANCEL: 
                        EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
                        return TRUE;   
                }

  }     //end command switch

  return FALSE; 
}



// needed for SCRNSAVE.LIB
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
  return TRUE;
}


/////////////////////////////////////////////////
////   INFRASTRUCTURE ENDS, SPECIFICS BEGIN   ///
////                                          ///
////    In a more complex scr, I'd put all    ///
////     the following into other files.      ///
/////////////////////////////////////////////////


// Initialize OpenGL
static void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC)
{
  
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory( &pfd, sizeof pfd );
  pfd.nSize = sizeof pfd;
  pfd.nVersion = 1;
  //pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //blaine's
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  
  hDC = GetDC( hWnd );
  
  int i = ChoosePixelFormat( hDC, &pfd );  
  SetPixelFormat( hDC, i, &pfd );

  hRC = wglCreateContext( hDC );
  wglMakeCurrent( hDC, hRC );

}

// Shut down OpenGL
static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );

  ReleaseDC( hWnd, hDC );
}


void SetupAnimation(int Width, int Height)
{
		//spi, begin
		createPalette();
		stepX = (maxX-minX)/(GLfloat)Width; // calculate new value of step along X axis
		stepY = (maxY-minY)/(GLfloat)Height; // calculate new value of step along Y axis
		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		//spi, end
        //window resizing stuff
        glViewport(0, 0, (GLsizei) Width, (GLsizei) Height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
		//spi, begin
        //glOrtho(-300, 300, -240, 240, 25, 75); //original
        //glOrtho(-300, 300, -8, 8, 25, 75); //spi, last
		glOrtho(minX, maxX, minY, maxY, ((GLfloat)-1), (GLfloat)1); //spi
        //glOrtho(-150, 150, -120, 120, 25, 75); //spi
		//spi, end
        
		/*
		glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();
        gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); //original
                //camera xyz, the xyz to look at, and the up vector (+y is up)
        */

        //background
        glClearColor(0.0, 0.0, 0.0, 0.0); //0.0s is black


		/*
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
        glShadeModel(GL_SMOOTH); 
		*/

        //no need to initialize any objects
        //but this is where I'd do it

		/*
        glColor3f(0.1, 1.0, 0.3); //green
		*/

}

static GLfloat spin=0;   //a global to keep track of the square's spinning


void OnTimer(HDC hDC) //increment and display
{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* //no rotation
        spin = spin + 1; 
		*/
        glPushMatrix();
        /*
		glRotatef(spin, 0.0, 0.0, 1.0);
		*/
        glPushMatrix();
		

		/*
        glTranslatef(150, 0, 0); //original
		*/

		/* //no rotation
        if(bTumble)
                glRotatef(spin * -3.0, 0.0, 0.0, 1.0); 
        else
                glRotatef(spin * -1.0, 0.0, 0.0, 1.0);  
		*/

		//spi, begin
		/*
        //draw the square (rotated to be a diamond)

        float xvals[] = {-30.0, 0.0, 30.0, 0.0};
        float yvals[] = {0.0, -30.0, 0.0, 30.0};

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
                for (int i=0; i < 4; i++)
                        glVertex2f(xvals[i], yvals[i]);
        glEnd();
		*/
		mandelbrotzoom();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
		glOrtho(minX, maxX, minY, maxY, ((GLfloat)-1), (GLfloat)1);
 
		glBegin(GL_POINTS); // start drawing in single pixel mode
		for(GLfloat y = maxY; y >= minY; y -= stepY)
		{
			for(GLfloat x = minX; x <= maxX; x += stepX)
			{
				glColor3fv(calculateColor(x,y)); // set color
				glVertex3f(x, y, 0.0f); // put pixel on screen (buffer) - [ 1 ]
			}
		}
		glEnd(); // end drawing
		//spi, end

        glPopMatrix();

        glFlush();
        SwapBuffers(hDC);
        glPopMatrix();
}

void CleanupAnimation()
{
        //didn't create any objects, so no need to clean them up
}



/////////   REGISTRY ACCESS FUNCTIONS     ///////////

void GetConfig()
{

        HKEY key;
        //DWORD lpdw;

        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                //"Software\\GreenSquare", //lpctstr
                L"Software\\Oifii\\Screensaver", //lpctstr
                0,                      //reserved
                KEY_QUERY_VALUE,
                &key) == ERROR_SUCCESS) 
        {
                DWORD dsize = sizeof(bTumble);
                DWORD dwtype =  0;

                //RegQueryValueEx(key,"Tumble", NULL, &dwtype, (BYTE*)&bTumble, &dsize);
                RegQueryValueEx(key,L"Tumble", NULL, &dwtype, (BYTE*)&bTumble, &dsize);


                //Finished with key
                RegCloseKey(key);
        }
        else //key isn't there yet--set defaults
        {
                bTumble = true;
        }
        
}

void WriteConfig(HWND hDlg)
{

        HKEY key;
        DWORD lpdw;

        if (RegCreateKeyEx( HKEY_CURRENT_USER,
                //"Software\\GreenSquare", //lpctstr
                L"Software\\Oifii\Screensaver", //lpctstr
                0,                      //reserved
                L"",                     //ptr to null-term string specifying the object type of this key
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                NULL,
                &key,
                &lpdw) == ERROR_SUCCESS)
                
        {
                //RegSetValueEx(key,"Tumble", 0, REG_DWORD, 
                RegSetValueEx(key,L"Tumble", 0, REG_DWORD, 
                        (BYTE*)&bTumble, sizeof(bTumble));

                //Finished with keys
                RegCloseKey(key);
        }

}