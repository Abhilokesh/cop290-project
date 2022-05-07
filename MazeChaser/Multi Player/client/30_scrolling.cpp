/*This source code copyrighted by Lazy Foo' Productions (2004-2022)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, vectors, and strings
#include <enet/enet.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <sstream>

//The dimensions of the level
const int LEVEL_WIDTH = 12800;
const int LEVEL_HEIGHT = 6400;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;



//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );
		
		#if defined(SDL_TTF_MAJOR_VERSION)
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//The dot that will move around on the screen
class Dot
{
    public:
		//The dimensions of the dot
		static const int DOT_WIDTH = 20;
		static const int DOT_HEIGHT = 20;

		//Maximum axis velocity of the dot
		static const int DOT_VEL = 15;

		//Initializes the variables
		Dot();

		//Takes key presses and adjusts the dot's velocity
		void handleEvent( SDL_Event& e );

		//Moves the dot
		void move( SDL_Rect wall[] );

		//Shows the dot on the screen relative to the camera
		void render( int camX, int camY );

		//Position accessors
		int getPosX();
		int getPosY();
		int getVelX();
		int getVelY();

    private:
		//The X and Y offsets of the dot
		int mPosX, mPosY;

		//The velocity of the dot
		int mVelX, mVelY;

		//Dot's collision box
		SDL_Rect mCollider;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Box collision detector
bool checkCollision( SDL_Rect a, SDL_Rect& b );

bool testf;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Globally used font
TTF_Font *gFont = NULL;

ENetAddress address;
ENetHost* client;
ENetEvent event;
ENetPeer* peer;

//Scene textures
LTexture gDotTexture;
LTexture gBGTexture;
LTexture gCTexture;
LTexture gGMTexture;
LTexture gTimeTextTexture;
LTexture gPromptTextTexture;

//The music that will be played
Mix_Music *gMusic = NULL;

//The sound effects that will be used
Mix_Chunk *gScratch = NULL;
Mix_Chunk *gHigh = NULL;
Mix_Chunk *gMedium = NULL;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#if defined(SDL_TTF_MAJOR_VERSION)
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}

	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

Dot::Dot()
{
    //Initialize the offsets
    mPosX = 10496;
    mPosY = 32;

    //Set collision box dimension
	mCollider.w = DOT_WIDTH;
	mCollider.h = DOT_HEIGHT;

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;
}

void Dot::handleEvent( SDL_Event& e )
{
    //If a key was pressed
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_1: mVelY -= DOT_VEL; break;
            case SDLK_2: mVelY += DOT_VEL; break;
            case SDLK_3: mVelX -= DOT_VEL; break;
            case SDLK_4: mVelX += DOT_VEL; break;
        }
    }
    //If a key was released
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_1: mVelY += DOT_VEL; break;
            case SDLK_2: mVelY -= DOT_VEL; break;
            case SDLK_3: mVelX += DOT_VEL; break;
            case SDLK_4: mVelX -= DOT_VEL; break;
        }
    }
}

void Dot::move( SDL_Rect wall[] )
{
    //Move the dot left or right
    mPosX += mVelX;
	mCollider.x = mPosX;
	//Move the dot up or down
    mPosY += mVelY;
    mCollider.y = mPosY;
	bool c = false;
	// int i=0;
	// while(i<18){
	// 	c = c || checkCollision( mCollider, wall[i] );
	// 	i++;
	// }
	for (int i = 0; i < 99; i++)
	{
		c = c || checkCollision( mCollider, wall[i] );
	}
	
    //If the dot collided or went too far to the left or right
    if( ( mPosX < 0 ) || ( mPosX + DOT_WIDTH > LEVEL_WIDTH ) || c )
    {
        //Move back
        mPosX -= mVelX;
		mCollider.x = mPosX;
    }

    //If the dot collided or went too far up or down
    if( ( mPosY < 0 ) || ( mPosY + DOT_HEIGHT > LEVEL_HEIGHT ) || c )
    {
        //Move back
        mPosY -= mVelY;
		mCollider.y = mPosY;
    }
}

void Dot::render( int camX, int camY )
{
    //Show the dot relative to the camera
	gDotTexture.render( mPosX - camX, mPosY - camY );
}

int Dot::getPosX()
{
	return mPosX;
}

int Dot::getPosY()
{
	return mPosY;
}

int Dot::getVelX()
{
	return mVelX;
}

int Dot::getVelY()
{
	return mVelY;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}

				 //Initialize SDL_ttf
				if( TTF_Init() == -1 )
				{
					printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
					success = false;
				}

				 //Initialize SDL_mixer
				if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
				{
					printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load music
	gMusic = Mix_LoadMUS( "beat.wav" );
	if( gMusic == NULL )
	{
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}
	
	//Load sound effects
	gScratch = Mix_LoadWAV( "pauseState.wav" );
	if( gScratch == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	gHigh = Mix_LoadWAV( "bPress.wav" );
	if( gHigh == NULL )
	{
		printf( "Failed to load high sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	gMedium = Mix_LoadWAV( "high.wav" );
	if( gMedium == NULL )
	{
		printf( "Failed to load high sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	//Load dot texture
	if( !gDotTexture.loadFromFile( "dot.bmp" ) )
	{
		printf( "Failed to load dot texture!\n" );
		success = false;
	}

	//Load background texture
	if( !gBGTexture.loadFromFile( "map.png" ) )
	{
		printf( "Failed to load background texture!\n" );
		success = false;
	}

	//Load congrats
	if( !gCTexture.loadFromFile( "congratulations.png" ) )
	{
		printf( "Failed to load game over texture!\n" );
		success = false;
	}

	//Load game over
	if( !gGMTexture.loadFromFile( "GameOver.png" ) )
	{
		printf( "Failed to load game over texture!\n" );
		success = false;
	}

	//Open the font
	gFont = TTF_OpenFont( "lazy.ttf", 28 );
	if( gFont == NULL )
	{
		printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
		success = false;
	}
	else
	{
		//Set text color as black
		SDL_Color textColor = { 255, 255, 255, 255 };
		
		//Load prompt texture
		if( !gPromptTextTexture.loadFromRenderedText( "Press Enter to Restart the Game.", textColor ) )
		{
			printf( "Unable to render prompt texture!\n" );
			success = false;
		}
	}

	return success;
}

void close()
{
	//Free loaded images
	gDotTexture.free();
	gBGTexture.free();
	gCTexture.free();
	gGMTexture.free();
	gTimeTextTexture.free();
	gPromptTextTexture.free();

	//Free the sound effects
	Mix_FreeChunk( gScratch );
	gScratch = NULL;
	gHigh = NULL;
	gMedium = NULL;
	
	//Free the music
	Mix_FreeMusic( gMusic );
	gMusic = NULL;

	//Free global font
	TTF_CloseFont( gFont );
	gFont = NULL;

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision( SDL_Rect a, SDL_Rect& b )
{
    //The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //Calculate the sides of rect A
    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    //Calculate the sides of rect B
    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    //If any of the sides from A are outside of B
    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    //If none of the sides from A are outside B
    return true;
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{	
		//client = { 0 };
        client = enet_host_create(NULL /* create a client host */,
            1 /* only allow 1 outgoing connection */,
            2 /* allow up 2 channels to be used, 0 and 1 */,
            0 /* assume any amount of incoming bandwidth */,
            0 /* assume any amount of outgoing bandwidth */);
        if (client == NULL) {
            fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
            exit(EXIT_FAILURE);
        }
        address = {0};
        enet_address_set_host(&address, "127.0.0.1");
        
        //address.host = ENET_HOST_ANY; /* Bind the server to the default localhost.     */
        address.port = 8123; /* Bind the server to port 7777. */
        peer = enet_host_connect(client, &address, 2, 0);
        if (peer == NULL) {
            fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
            exit(EXIT_FAILURE);
        }
        testf=true;
         /* Wait up to 5 seconds for the connection attempt to succeed. */
        if (enet_host_service(client, &event, 5000) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT) {
            puts("Connection to some.server.net:1234 succeeded.");
        } else {
            /* Either the 5 seconds are up or a disconnect event was */
            /* received. Reset the peer in the event the 5 seconds   */
            /* had run out without any significant event.            */
            enet_peer_reset(peer);
            puts("Connection to some.server.net:1234 failed.");
        }
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//The dot that will be moving around on the screen
			Dot dot;

			//Set text color as black
			SDL_Color textColor = { 255, 255, 255, 255 };

			//Current time start time
			Uint32 startTime = 0;

			//In memory text stream
			std::stringstream timeText;

			//Set the wall
			SDL_Rect wall2;
			wall2.x = 992;
			wall2.y = 64;
			wall2.w = 608;
			wall2.h = 3648;

			SDL_Rect wall1;
			wall1.x = 1248;
			wall1.y = 3712;
			wall1.w = 352;
			wall1.h = 512;

			SDL_Rect wall3;
			wall3.x = 832;
			wall3.y = 3712;
			wall3.w = 288;
			wall3.h = 512;

			SDL_Rect wall4;
			wall4.x = 416;
			wall4.y = 3456;
			wall4.w = 704;
			wall4.h = 640;

			SDL_Rect wall5;
			wall5.x = 416;
			wall5.y = 4096;
			wall5.w = 288;
			wall5.h = 256;

			SDL_Rect wall6;
			wall6.x = 64;
			wall6.y = 4256;
			wall6.w = 640;
			wall6.h = 96;

			SDL_Rect wall7;
			wall7.x = 64;
			wall7.y = 4352;
			wall7.w = 768;
			wall7.h = 672;

			SDL_Rect wall8;
			wall8.x = 960;
			wall8.y = 4352;
			wall8.w = 640;
			wall8.h = 672;

			SDL_Rect wall9;
			wall9.x = 64;
			wall9.y = 5152;
			wall9.w = 576;
			wall9.h = 1184;

			SDL_Rect wall10;
			wall10.x = 640;
			wall10.y = 5408;
			wall10.w = 128;
			wall10.h = 928;

			SDL_Rect wall11;
			wall11.x = 768;
			wall11.y = 5152;
			wall11.w = 512;
			wall11.h = 1184;

			SDL_Rect wall12;
			wall12.x = 1280;
			wall12.y = 5152;
			wall12.w = 320;
			wall12.h = 224;

			SDL_Rect wall13;
			wall13.x = 1280;
			wall13.y = 5504;
			wall13.w = 320;
			wall13.h = 832;

			SDL_Rect wall14;
			wall14.x = 1728;
			wall14.y = 64;
			wall14.w = 1600;
			wall14.h = 448;

			SDL_Rect wall15;
			wall15.x = 2688;
			wall15.y = 512;
			wall15.w = 640;
			wall15.h = 224;

			SDL_Rect wall16;
			wall16.x = 3328;
			wall16.y = 64;
			wall16.w = 2560;
			wall16.h = 416;

			SDL_Rect wall17;
			wall17.x = 3456;
			wall17.y = 608;
			wall17.w = 128;
			wall17.h = 1436;

			SDL_Rect wall18;
			wall18.x = 3584;
			wall18.y = 736;
			wall18.w = 128;
			wall18.h = 1312;

			SDL_Rect wall19;
			wall19.x = 3712;
			wall19.y = 480;
			wall19.w = 2176;
			wall19.h = 1568;

			SDL_Rect wall20;
			wall20.x = 5888;
			wall20.y = 1664;
			wall20.w = 512;
			wall20.h = 384;

			SDL_Rect wall21;
			wall21.x = 1728;
			wall21.y = 640;
			wall21.w = 832;
			wall21.h = 1408;

			SDL_Rect wall22;
			wall22.x = 2688;
			wall22.y = 864;
			wall22.w = 640;
			wall22.h = 1184;

			SDL_Rect wall23;
			wall23.x = 1728;
			wall23.y = 2176;
			wall23.w = 832;
			wall23.h = 640;

			SDL_Rect wall24;
			wall24.x = 2688;
			wall24.y = 2176;
			wall24.w = 640;
			wall24.h = 640;

			SDL_Rect wall25;
			wall25.x = 1728;
			wall25.y = 2944;
			wall25.w = 320;
			wall25.h = 1536;

			SDL_Rect wall26;
			wall26.x = 2048;
			wall26.y = 3008;
			wall26.w = 128;
			wall26.h = 1472;

			SDL_Rect wall27;
			wall27.x = 2176;
			wall27.y = 2944;
			wall27.w = 512;
			wall27.h = 1536;

			SDL_Rect wall28;
			wall28.x = 2688;
			wall28.y = 3072;
			wall28.w = 128;
			wall28.h = 1408;

			SDL_Rect wall29;
			wall29.x = 2816;
			wall29.y = 2816;
			wall29.w = 512;
			wall29.h = 1184;

			SDL_Rect wall30;
			wall30.x = 2816;
			wall30.y = 4000;
			wall30.w = 256;
			wall30.h = 480;

			SDL_Rect wall31;
			wall31.x = 1728;
			wall31.y = 4608;
			wall31.w = 1088;
			wall31.h = 416;

			SDL_Rect wall32;
			wall32.x = 1728;
			wall32.y = 5152;
			wall32.w = 768;
			wall32.h = 1056;

			SDL_Rect wall33;
			wall33.x = 2496;
			wall33.y = 5152;
			wall33.w = 448;
			wall33.h = 1184;

			SDL_Rect wall34;
			wall34.x = 2944;
			wall34.y = 4608;
			wall34.w = 256;
			wall34.h = 1728;

			SDL_Rect wall35;
			wall35.x = 3200;
			wall35.y = 4128;
			wall35.w = 9536;
			wall35.h = 2208;

			SDL_Rect wall36;
			wall36.x = 1600;
			wall36.y = 6336;
			wall36.w = 768;
			wall36.h = 16;

			SDL_Rect wall37;
			wall37.x = 3456;
			wall37.y = 2176;
			wall37.w = 3328;
			wall37.h = 1824;

			SDL_Rect wall38;
			wall38.x = 6784;
			wall38.y = 2431;
			wall38.w = 256;
			wall38.h = 1568;

			SDL_Rect wall39;
			wall39.x = 7040;
			wall39.y = 2944;
			wall39.w = 128;
			wall39.h = 1056;

			SDL_Rect wall40;
			wall40.x = 6784;
			wall40.y = 1792;
			wall40.w = 128;
			wall40.h = 256;

			SDL_Rect wall41;
			wall41.x = 6912;
			wall41.y = 1792;
			wall41.w = 256;
			wall41.h = 512;

			SDL_Rect wall42;
			wall42.x = 7168;
			wall42.y = 1792;
			wall42.w = 768;
			wall42.h = 2208;

			SDL_Rect wall43;
			wall43.x = 7936;
			wall43.y = 2304;
			wall43.w = 128;
			wall43.h = 1696;

			SDL_Rect wall44;
			wall44.x = 8064;
			wall44.y = 2240;
			wall44.w = 128;
			wall44.h = 1760;

			SDL_Rect wall45;
			wall45.x = 8064;
			wall45.y = 1792;
			wall45.w = 128;
			wall45.h = 320;

			SDL_Rect wall46;
			wall46.x = 8192;
			wall46.y = 1792;
			wall46.w = 480;
			wall46.h = 2208;

			SDL_Rect wall47;
			wall47.x = 8672;
			wall47.y = 1280;
			wall47.w = 1760;
			wall47.h = 2720;

			SDL_Rect wall48;
			wall48.x = 10432;
			wall48.y = 1280;
			wall48.w = 608;
			wall48.h = 1792;

			SDL_Rect wall49;
			wall49.x = 6016;
			wall49.y = 256;
			wall49.w = 512;
			wall49.h = 1280;

			SDL_Rect wall50;
			wall50.x = 6528;
			wall50.y = 256;
			wall50.w = 128;
			wall50.h = 1792;

			SDL_Rect wall51;
			wall51.x = 6656;
			wall51.y = 256;
			wall51.w = 1888;
			wall51.h = 896;

			SDL_Rect wall52;
			wall52.x = 8544;
			wall52.y = 256;
			wall52.w = 960;
			wall52.h = 256;

			SDL_Rect wall53;
			wall53.x = 5888;
			wall53.y = 64;
			wall53.w = 3744;
			wall53.h = 64;

			SDL_Rect wall54;
			wall54.x = 6784;
			wall54.y = 1280;
			wall54.w = 832;
			wall54.h = 384;

			SDL_Rect wall55;
			wall55.x = 7744;
			wall55.y = 1280;
			wall55.w = 800;
			wall55.h = 384;

			SDL_Rect wall56;
			wall56.x = 9632;
			wall56.y = 64;
			wall56.w = 320;
			wall56.h = 448;

			SDL_Rect wall57;
			wall57.x = 8672;
			wall57.y = 640;
			wall57.w = 1408;
			wall57.h = 512;

			SDL_Rect wall58;
			wall58.x = 10080;
			wall58.y = 192;
			wall58.w = 512;
			wall58.h = 960;

			SDL_Rect wall59;
			wall59.x = 10592;
			wall59.y = 64;
			wall59.w = 448;
			wall59.h = 1088;

			SDL_Rect wall60;
			wall60.x = 11040;
			wall60.y = 64;
			wall60.w = 128;
			wall60.h = 320;

			SDL_Rect wall61;
			wall61.x = 9952;
			wall61.y = 48;
			wall61.w = 512;
			wall61.h = 16;

			SDL_Rect wall62;
			wall62.x = 11168;
			wall62.y = 64;
			wall62.w = 352;
			wall62.h = 1088;

			SDL_Rect wall63;
			wall63.x = 11520;
			wall63.y = 64;
			wall63.w = 640;
			wall63.h = 320;

			SDL_Rect wall64;
			wall64.x = 11648;
			wall64.y = 512;
			wall64.w = 512;
			wall64.h = 640;

			SDL_Rect wall65;
			wall65.x = 12288;
			wall65.y = 64;
			wall65.w = 448;
			wall65.h = 1088;

			SDL_Rect wall66;
			wall66.x = 11168;
			wall66.y = 1280;
			wall66.w = 352;
			wall66.h = 832;

			SDL_Rect wall67;
			wall67.x = 11648;
			wall67.y = 1280;
			wall67.w = 512;
			wall67.h = 832;

			SDL_Rect wall68;
			wall68.x = 11168;
			wall68.y = 2240;
			wall68.w = 992;
			wall68.h = 320;

			SDL_Rect wall69;
			wall69.x = 12288;
			wall69.y = 1280;
			wall69.w = 448;
			wall69.h = 2720;

			SDL_Rect wall70;
			wall70.x = 10560;
			wall70.y = 3200;
			wall70.w = 608;
			wall70.h = 800;

			SDL_Rect wall71;
			wall71.x = 11168;
			wall71.y = 2688;
			wall71.w = 992;
			wall71.h = 1312;

			SDL_Rect wall72;
			wall72.x = 416;
			wall72.y = 64;
			wall72.w = 576;
			wall72.h = 3424;

			SDL_Rect wall73;
			wall73.x = 64;
			wall73.y = 64;
			wall73.w = 352;
			wall73.h = 4192;

			SDL_Rect wall74;
			wall74.x = 704;
			wall74.y = 4320;
			wall74.w = 32;
			wall74.h = 32;

			SDL_Rect wall75;
			wall75.x = 1600;
			wall75.y = 6272;
			wall75.w = 32;
			wall75.h = 32;

			SDL_Rect wall76;
			wall76.x = 2656;
			wall76.y = 512;
			wall76.w = 32;
			wall76.h = 32;

			SDL_Rect wall77;
			wall77.x = 3328;
			wall77.y = 480;
			wall77.w = 32;
			wall77.h = 32;

			SDL_Rect wall78;
			wall78.x = 3680;
			wall78.y = 480;
			wall78.w = 32;
			wall78.h = 32;

			SDL_Rect wall79;
			wall79.x = 5888;
			wall79.y = 1632;
			wall79.w = 32;
			wall79.h = 32;

			SDL_Rect wall80;
			wall80.x = 2784;
			wall80.y = 2816;
			wall80.w = 32;
			wall80.h = 32;

			SDL_Rect wall81;
			wall81.x = 3072;
			wall81.y = 4000;
			wall81.w = 32;
			wall81.h = 32;

			SDL_Rect wall82;
			wall82.x = 2464;
			wall82.y = 6208;
			wall82.w = 32;
			wall82.h = 32;

			SDL_Rect wall83;
			wall83.x = 2912;
			wall83.y = 5120;
			wall83.w = 32;
			wall83.h = 32;

			SDL_Rect wall84;
			wall84.x = 3168;
			wall84.y = 4576;
			wall84.w = 32;
			wall84.h = 32;

			SDL_Rect wall85;
			wall85.x = 6784;
			wall85.y = 2400;
			wall85.w = 32;
			wall85.h = 32;

			SDL_Rect wall86;
			wall86.x = 6880;
			wall86.y = 2048;
			wall86.w = 32;
			wall86.h = 32;

			SDL_Rect wall87;
			wall87.x = 7136;
			wall87.y = 2304;
			wall87.w = 32;
			wall87.h = 32;

			SDL_Rect wall88;
			wall88.x = 8640;
			wall88.y = 1760;
			wall88.w = 32;
			wall88.h = 32;

			SDL_Rect wall89;
			wall89.x = 10432;
			wall89.y = 3072;
			wall89.w = 32;
			wall89.h = 32;

			SDL_Rect wall90;
			wall90.x = 6496;
			wall90.y = 1536;
			wall90.w = 32;
			wall90.h = 32;

			SDL_Rect wall91;
			wall91.x = 6656;
			wall91.y = 1152;
			wall91.w = 32;
			wall91.h = 32;

			SDL_Rect wall92;
			wall92.x = 8544;
			wall92.y = 512;
			wall92.w = 32;
			wall92.h = 32;

			SDL_Rect wall93;
			wall93.x = 9600;
			wall93.y = 128;
			wall93.w = 32;
			wall93.h = 32;

			SDL_Rect wall94;
			wall94.x = 5888;
			wall94.y = 128;
			wall94.w = 32;
			wall94.h = 32;

			SDL_Rect wall95;
			wall95.x = 9952;
			wall95.y = 64;
			wall95.w = 32;
			wall95.h = 32;

			SDL_Rect wall96;
			wall96.x = 10048;
			wall96.y = 608;
			wall96.w = 32;
			wall96.h = 32;

			SDL_Rect wall97;
			wall97.x = 10560;
			wall97.y = 160;
			wall97.w = 32;
			wall97.h = 32;

			SDL_Rect wall98;
			wall98.x = 11520;
			wall98.y = 384;
			wall98.w = 32;
			wall98.h = 32;

			SDL_Rect wall99;
			wall99.x = 11136;
			wall99.y = 3168;
			wall99.w = 32;
			wall99.h = 32;


			SDL_Rect wall[] = {wall1, wall2, wall3, wall4, wall5, wall6, wall7, wall8, wall9, wall10, wall11, wall12, wall13, wall14, wall15, wall16, wall17, wall18, wall19, wall20, wall21, wall22, wall23, wall24, wall25, wall26, wall27, wall28, wall29, wall30, wall31, wall32, wall33, wall34, wall35, wall36, wall37, wall38, wall39, wall40, wall41, wall42, wall43, wall44, wall45, wall46, wall47, wall48, wall49, wall50, 
				wall51, wall52, wall53, wall54, wall55, wall56, wall57, wall58, wall59, wall60, wall61, wall62, wall63, wall64, wall65, wall66, wall67, wall68, wall69, wall70, wall71, wall72, wall73, wall74, wall75, wall76, wall77, wall78, wall79, wall80, wall81, wall82, wall83, wall84, wall85, wall86, wall87, wall88, wall89, wall90, wall91, wall92, wall93, wall94, wall95, wall96, wall97, wall98, wall99};

			//The camera area
			SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

			int e1 = 0;
			int score = 300;
			int energy = 0;

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}
					// //Reset start time on return keypress
					// else if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN )
					// {
					// 	startTime = SDL_GetTicks();
					// }
					else{
						Mix_PlayMusic( gMusic, -1 );
					}

					//Handle input for the dot
					dot.handleEvent( e );
				}

				//Move the dot
				dot.move( wall );

				//Center the camera over the dot
				camera.x = ( dot.getPosX() + Dot::DOT_WIDTH / 2 ) - SCREEN_WIDTH / 2;
				camera.y = ( dot.getPosY() + Dot::DOT_HEIGHT / 2 ) - SCREEN_HEIGHT / 2;

				//Keep the camera in bounds
				if( camera.x < 0 )
				{ 
					camera.x = 0;
				}
				if( camera.y < 0 )
				{
					camera.y = 0;
				}
				if( camera.x > LEVEL_WIDTH - camera.w )
				{
					camera.x = LEVEL_WIDTH - camera.w;
				}
				if( camera.y > LEVEL_HEIGHT - camera.h )
				{
					camera.y = LEVEL_HEIGHT - camera.h;
				}

				//Set text to be rendered
				int dX = dot.getPosX();
				int dY = dot.getPosY();
				int vX = dot.getVelX();
				int vY = dot.getVelY();
				if(dX > 7840 && dY > 1152 && dX < 7872 && dY < 1216 && (vX>5 || vX<-5)){
					e1+=20;
					score-=50;
					Mix_PlayChannel( -1, gHigh, 0 );
				}
				if(dX > 7104 && dY > 2848 && dX < 7168 && dY < 2912 && (vY>5 || vY<-5)){
					e1+=20;
					score-=30;
					Mix_PlayChannel( -1, gHigh, 0 );
				}
				if(dX > 11040 && dY > 576 && dX < 11104 && dY < 608 && (vY>5 || vY<-5)){
					e1+=20;
					score-=30;
					Mix_PlayChannel( -1, gHigh, 0 );
				}
				if(dX > 10528 && dY > 3072 && dX < 10560 && dY < 3136 && (vX>5 || vX<-5)){
					e1+=20;
					score-=30;
					Mix_PlayChannel( -1, gHigh, 0 );
				}
				if(dX > 3392 && dY > 2720 && dX < 3456 && dY < 2752 && (vY>5 || vY<-5)){
					e1+=20;
					score-=30;
					Mix_PlayChannel( -1, gHigh, 0 );
				}
				if(dX > 1664 && dY > 512 && dX < 1728 && dY < 544 && (vY>5 || vY<-5)){
					e1+=20;
					score-=30;
					Mix_PlayChannel( -1, gHigh, 0 );
				}
				if(dX > 1120 && dY > 3776 && dX < 1184 && dY < 3808 && (vY>5 || vY<-5)){
					e1+=20;
					score-=30;
					Mix_PlayChannel( -1, gHigh, 0 );
				}
				if(dX > 1344 && dY > 5440 && dX < 1376 && dY < 5504 && (vX>5 || vX<-5)){
					e1+=20;
					score-=30;
					Mix_PlayChannel( -1, gHigh, 0 );
				}
				if(dX > 8096 && dY > 2176 && dX < 8128 && dY < 2240 && (vX>5 || vX<-5)){
					score+=50;
					Mix_PlayChannel( -1, gMedium, 0 );
				}
				if(dX > 6464 && dY > 1664 && dX < 6528 && dY < 1696 && (vY>5 || vY<-5)){
					score+=50;
					Mix_PlayChannel( -1, gMedium, 0 );
				}
				if(dX > 12160 && dY > 1536 && dX < 12224 && dY < 1568 && (vY>5 || vY<-5)){
					score+=50;
					Mix_PlayChannel( -1, gMedium, 0 );
				}
				if(dX > 3456 && dY > 4000 && dX < 3488 && dY < 4064 && (vX>5 || vX<-5)){
					score+=50;
					Mix_PlayChannel( -1, gMedium, 0 );
				}
				if(dX > 640 && dY > 5312 && dX < 704 && dY < 5344 && (vY>5 || vY<-5)){
					score+=50;
					Mix_PlayChannel( -1, gMedium, 0 );
				}
				if(dX > 2688 && dY > 2976 && dX < 2752 && dY < 3008 && (vY>5 || vY<-5)){
					score+=50;
					Mix_PlayChannel( -1, gMedium, 0 );
				}
				if(dX > 3584 && dY > 640 && dX < 3648 && dY < 672 && (vY>5 || vY<-5)){
					score+=50;
					Mix_PlayChannel( -1, gMedium, 0 );
				}
				if(dX > 9024 && dY > 1152 && dX < 9344 && dY < 1216){}
				else if(dX > 11104 && dY > 288 && dX < 11168 && dY < 608){}
				else if(dX > 12480 && dY > 1152 && dX < 12800 && dY < 1216){}
				else if(dX > 11424 && dY > 2176 && dX < 11744 && dY < 2240){}
				else if(dX > 12512 && dY > 4000 && dX < 12832 && dY < 4064){}
				else if(dX > 6368 && dY > 4064 && dX < 6688 && dY < 4128){}
				else if(dX > 5952 && dY > 704 && dX < 6016 && dY < 1024){}
				else if(dX > 7936 && dY > 2080 && dX < 8000 && dY < 2400){}
				else if(dX > 2976 && dY > 2112 && dX < 3296 && dY < 2176){}
				else if(dX > 768 && dY > 4000 && dX < 832 && dY < 4320){}
				else{
					energy = 300 - 0.01*(SDL_GetTicks() - startTime) + e1;
				}
				timeText.str( "" );
				timeText << "Current score : " << score ;
				timeText << " | Energy left : " << energy ;
				
				//Render text
				if( !gTimeTextTexture.loadFromRenderedText( timeText.str().c_str(), textColor ) )
				{
					printf( "Unable to render time texture!\n" );
				}

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				//Render background
				gBGTexture.render( 0, 0, &camera );

				//Render objects
				dot.render( camera.x, camera.y );

				//Render textures
				gPromptTextTexture.render( ( SCREEN_WIDTH - gPromptTextTexture.getWidth() ) / 2, 0 );
				gTimeTextTexture.render( ( SCREEN_WIDTH - gTimeTextTexture.getWidth() ) / 2, 32 );

				//Render congrats
				if(dX>3328 && dY>2400 && dX<3360 && dY<2528 && score>0 && energy>0){
					gCTexture.render( 320, 32);
					Mix_PlayChannel( -1, gScratch, 0 );
					if(e.type == SDL_KEYUP){
						SDL_Delay(1000);
						quit = true;
					}
				}

				//Render game over
				if( score<0 || energy<=0 ){
					gGMTexture.render( 160, 64);
					Mix_PlayChannel( -1, gScratch, 0 );
					if(e.type == SDL_KEYUP){
						SDL_Delay(5000);
						quit = true;
					}
					
				}

				//Update screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}