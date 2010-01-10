#include <strings.h>
#include <SDL.h>
#include <dlfcn.h>

#include <GLES/egl.h>
#include <GLES/gl.h>

#include "../client/client.h"
#include "../renderer/tr_local.h"
#include "../sys/sys_local.h"

static SDL_Surface *screen = NULL;
static const SDL_VideoInfo *videoInfo = NULL;

#if 1
void qglArrayElement(GLint i)
{
}

void qglCallList(GLuint list)
{
}

void qglDrawBuffer(GLenum mode)
{
}

void qglLockArrays(GLint i, GLsizei size)
{
}

void qglUnlockArrays(void)
{
}
#endif

void GLimp_LogComment(char *comment)
{
	//ri.Printf(PRINT_ALL, "GLimp_LogComment: %s\n", comment);
}

void GLimp_SetGamma(unsigned char red[256], unsigned char green[256],
		    unsigned char blue[256])
{
	// todo
}

qboolean GLimp_SpawnRenderThread(void (*function) (void))
{
	// todo?
	return qfalse;
}

void GLimp_FrontEndSleep(void)
{
	// todo
}

void *GLimp_RendererSleep(void)
{
	// todo
	return NULL;
}

void GLimp_RenderThreadWrapper(void *data)
{
	// todo
}

void GLimp_WakeRenderer(void *data)
{
	// todo
}

static void qglMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
	qglMultiTexCoord4f(target,s,t,1,1);
}

static qboolean GLimp_HaveExtension(const char *ext)
{
	const char *ptr = Q_stristr( glConfig.extensions_string, ext );
	if (ptr == NULL)
		return qfalse;
	ptr += strlen(ext);
	return ((*ptr == ' ') || (*ptr == '\0'));  // verify it's complete string.
}

/*
===============
GLimp_InitExtensions
===============
*/
static void GLimp_InitExtensions( void )
{
	if ( !r_allowExtensions->integer )
	{
		ri.Printf( PRINT_ALL, "* IGNORING OPENGL EXTENSIONS *\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	glConfig.textureCompression = TC_NONE;

	// GL_EXT_texture_compression_s3tc
	if ( GLimp_HaveExtension( "GL_ARB_texture_compression" ) &&
	     GLimp_HaveExtension( "GL_EXT_texture_compression_s3tc" ) )
	{
		if ( r_ext_compressed_textures->value )
		{
			glConfig.textureCompression = TC_S3TC_ARB;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_compression_s3tc\n" );
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_compression_s3tc\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_compression_s3tc not found\n" );
	}

	// GL_S3_s3tc ... legacy extension before GL_EXT_texture_compression_s3tc.
	if (glConfig.textureCompression == TC_NONE)
	{
		if ( GLimp_HaveExtension( "GL_S3_s3tc" ) )
		{
			if ( r_ext_compressed_textures->value )
			{
				glConfig.textureCompression = TC_S3TC;
				ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
			}
			else
			{
				ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
		}
	}


	// GL_EXT_texture_env_add
	glConfig.textureEnvAddAvailable = qtrue; //qfalse;
#if 0
	if ( GLimp_HaveExtension( "EXT_texture_env_add" ) )
	{
		if ( r_ext_texture_env_add->integer )
		{
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		}
		else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}
#endif

	// GL_ARB_multitexture
	/*
	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	*/
	//if ( GLimp_HaveExtension( "GL_ARB_multitexture" ) )
	{
		if ( r_ext_multitexture->value )
		{
			qglMultiTexCoord2fARB = qglMultiTexCoord2f;
			qglActiveTextureARB = qglActiveTexture;
			qglClientActiveTextureARB = qglClientActiveTexture;

			if ( qglActiveTextureARB )
			{
				GLint glint = 0;
				qglGetIntegerv( GL_MAX_TEXTURE_UNITS, &glint );
				glConfig.numTextureUnits = (int) glint;
				if ( glConfig.numTextureUnits > 1 )
				{
					ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				}
				else
				{
					qglMultiTexCoord2fARB = NULL;
					qglActiveTextureARB = NULL;
					qglClientActiveTextureARB = NULL;
					ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	}
#if 0
	else
	{
		ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}
#endif

#if 0
	// GL_EXT_compiled_vertex_array
	if ( GLimp_HaveExtension( "GL_EXT_compiled_vertex_array" ) )
	{
		if ( r_ext_compiled_vertex_array->value )
		{
			ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			qglLockArraysEXT = ( void ( APIENTRY * )( GLint, GLint ) ) SDL_GL_GetProcAddress( "glLockArraysEXT" );
			qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) ) SDL_GL_GetProcAddress( "glUnlockArraysEXT" );
			if (!qglLockArraysEXT || !qglUnlockArraysEXT)
			{
				ri.Error (ERR_FATAL, "bad getprocaddress");
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	}
	else
#endif
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}

	textureFilterAnisotropic = qfalse;
#if 0
	if ( GLimp_HaveExtension( "GL_EXT_texture_filter_anisotropic" ) )
	{
		if ( r_ext_texture_filter_anisotropic->integer ) {
			qglGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, (GLint *)&maxAnisotropy );
			if ( maxAnisotropy <= 0 ) {
				ri.Printf( PRINT_ALL, "...GL_EXT_texture_filter_anisotropic not properly supported!\n" );
				maxAnisotropy = 0;
			}
			else
			{
				ri.Printf( PRINT_ALL, "...using GL_EXT_texture_filter_anisotropic (max: %i)\n", maxAnisotropy );
				textureFilterAnisotropic = qtrue;
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_filter_anisotropic\n" );
		}
	}
	else
#endif
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_filter_anisotropic not found\n" );
	}
}

/*
===============
GLimp_EndFrame

Responsible for doing a swapbuffers
===============
*/
void GLimp_EndFrame( void )
{
	// don't flip if drawing to front buffer
	if ( Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
	{
		SDL_GL_SwapBuffers();
	}
}

/*
===============
GLimp_Shutdown
===============
*/
void GLimp_Shutdown( void )
{
	IN_Shutdown();
	
	SDL_QuitSubSystem( SDL_INIT_VIDEO );
	//nanoGL_Destroy();
	screen = NULL;

	Com_Memset( &glConfig, 0, sizeof( glConfig ) );
	Com_Memset( &glState, 0, sizeof( glState ) );
}

/*
===============
GLimp_Init

This routine is responsible for initializing the OS specific portions
of OpenGL
===============
*/
void GLimp_Init( void )
{
	const char*   glstring;
	int sdlcolorbits;
	int colorbits, depthbits, stencilbits;
	int samples;
	SDL_Surface *vidscreen = NULL;
	Uint32 flags = SDL_OPENGLES;
	
	ri.Printf(PRINT_ALL, "Initializing OpenGL subsystem\n");
	
	bzero(&glConfig, sizeof(glConfig));
	
	/*
	if(nanoGL_Init()) {
		ri.Printf( PRINT_ALL, "nanoGL_Init( ) FAILED (%s)\n", dlerror());
		assert(0);
	}
	*/
	
	// sys_unix has this as NOP: useless?
	Sys_GLimpInit( );
	
	if (!SDL_WasInit( SDL_INIT_VIDEO )) {
		if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
			ri.Printf( PRINT_ALL, "SDL_Init( SDL_INIT_VIDEO ) FAILED (%s)\n",
					SDL_GetError());
			assert(0);
		}
	}

	// specify we want OpenGL|ES 1.10
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	if (r_fullscreen->integer)
	{
		flags |= SDL_FULLSCREEN;
		glConfig.isFullscreen = qtrue;
	}
	else
		glConfig.isFullscreen = qfalse;

	glConfig.vidWidth = 320;
	glConfig.vidHeight = 480;
	glConfig.windowAspect = 480.0 / 320.0;

	colorbits = r_colorbits->value;
	if ((!colorbits) || (colorbits >= 32))
		colorbits = 24;

	if (!r_depthbits->value)
		depthbits = 24;
	else
		depthbits = r_depthbits->value;
	stencilbits = r_stencilbits->value;
	//samples = r_ext_multisample->value;


	sdlcolorbits = 4;
	if (colorbits == 24)
		sdlcolorbits = 8;

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, sdlcolorbits );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, sdlcolorbits );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, sdlcolorbits );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depthbits );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, stencilbits );

	samples = 0;
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, samples ? 1 : 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, samples );

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	if( SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, r_swapInterval->integer ) < 0 )
		ri.Printf( PRINT_ALL, "r_swapInterval requires libSDL >= 1.2.10\n" );

	//SDL_WM_SetCaption(CLIENT_WINDOW_TITLE, CLIENT_WINDOW_MIN_TITLE);
	//SDL_ShowCursor(0);

	if (!(vidscreen = SDL_SetVideoMode(glConfig.vidWidth, glConfig.vidHeight, colorbits, flags)))
	{
		ri.Printf( PRINT_ALL, "SDL_SetVideoMode failed: %s\n", SDL_GetError( ) );
		assert(0);
	}

	ri.Printf( PRINT_ALL, "Using %d/%d/%d Color bits, %d depth, %d stencil display.\n",
		sdlcolorbits, sdlcolorbits, sdlcolorbits, depthbits, stencilbits);

	glConfig.colorBits = colorbits;
	glConfig.depthBits = depthbits;
	glConfig.stencilBits = stencilbits;
	glConfig.textureCompression = TC_NONE;
	
	if (!vidscreen)
	{
		ri.Printf( PRINT_ALL, "Couldn't get a visual\n" );
		assert(0);
	}

	screen = vidscreen;

	glstring = (char *) qglGetString (GL_RENDERER);
	ri.Printf( PRINT_ALL, "GL_RENDERER: %s\n", glstring );

	IN_Init( );
	
	glConfig.deviceSupportsGamma = SDL_SetGamma( 1.0f, 1.0f, 1.0f ) >= 0;
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;

	Q_strncpyz(glConfig.vendor_string,
		   (const char *)qglGetString(GL_VENDOR),
		   sizeof(glConfig.vendor_string));
	Q_strncpyz(glConfig.renderer_string,
		   (const char *)qglGetString(GL_RENDERER),
		   sizeof(glConfig.renderer_string));
	Q_strncpyz(glConfig.version_string,
		   (const char *)qglGetString(GL_VERSION),
		   sizeof(glConfig.version_string));
	Q_strncpyz(glConfig.extensions_string,
		   (const char *)qglGetString(GL_EXTENSIONS),
		   sizeof(glConfig.extensions_string));
		
	GLimp_InitExtensions();
	
	ri.Printf(PRINT_ALL, "------------------\n");
}
