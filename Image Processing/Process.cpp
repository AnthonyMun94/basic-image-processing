/*
===============================================================================================
Name    : Mun Hao Ran
Date    : 7-9-2016
Purpose : Generate image with the following effects when the user press:
1 - Flip image horizontally
2 - Flip image vertically
3 - Increase brightness by 5%
4 - Decrease brightness by 5%
5 - Enhance contrast using contrast stretch technique
6 - Retain 30% central of image and blur the remaining 70%.
===============================================================================================
*/

#include <SDL.h>

// The following statement is to hide the console window
#pragma comment (linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

SDL_Window *window = NULL;
SDL_Renderer *render = NULL;

// Surface for images
SDL_Surface *originalImage = NULL;
SDL_Surface *resultImage = NULL;
SDL_Surface* tempImage = NULL;

SDL_Texture *originalTexture = NULL;
SDL_Texture *resultTexture = NULL;

int window_w = 600, window_h = 400;
char *bmpFile = "House.bmp";

bool flipped_horizontal = false;
bool flipped_vertical = false;

int bright_count = 0;

// Constant variables
const double brightness = 0.05;
const double PI = 3.142;

// Flip direction enum
enum flipDirection
{
	HORIZONTAL, VERTICAL
};

// Brightness option enum
enum brightnessOption
{
	INCREASE, DECREASE
};

// Truncate RGB value to ensure it is within 0 to 255
int truncateRGB(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	if (value > 255)
	{
		value = 255;
	}

	return value;
}

// Convert to YUV value
void convertToYUV(Uint8 r, Uint8 g, Uint8 b, double &y, double &u, double &v)
{
	y = r * 0.299000 + g * 0.587000 + b * 0.114000;
	u = r * -0.168736 + g * -0.331264 + b * 0.500000;
	v = r * 0.500000 + g * -0.418688 + b * -0.081312;
}

// Convert to RGB value
void convertToRGB(double y, double u, double v, Uint8 &r, Uint8 &g, Uint8 &b)
{
	r = truncateRGB(y + 1.407500 * v);
	g = truncateRGB(y - 0.345500 * u - 0.716900 * v);
	b = truncateRGB(y + 1.779000 * u);
}

// Read a pixel value from surface
Uint32 getPixel32(SDL_Surface *surface, int x, int y)
{
	// Convert pixels to 32 bit
	Uint32 *pix = (Uint32 *)surface->pixels;

	// Return pixels
	return pix[(y * surface->w) + x];
}

// Set a pixel to surface
void setPixel32(SDL_Surface *surface, int x, int y, Uint32 pix)
{
	// Convert pixels to 32 bit
	Uint32 *pixels = (Uint32 *)surface->pixels;

	// Set pixels
	pixels[(y * surface->w) + x] = pix;
}

// Flip image horizontally/vertically
void flipImage(SDL_Surface *surface, flipDirection flip_direction, bool isFlipped = false)
{
	SDL_Surface *temp = surface;
	int width = temp->w;
	int height = temp->h;
	int w, h;

	// Lock surface when accessing pixel
	if (SDL_MUSTLOCK(temp))
	{
		SDL_LockSurface(temp);
	}

	if (flip_direction == HORIZONTAL) // Flip horizontally
	{
		Uint32 left_pixel, right_pixel;

		// Swap left pixel and right pixel
		for (w = 0; w < width / 2; w++)
		{
			for (h = 0; h < height; h++)
			{
				left_pixel = getPixel32(temp, w, h); // Get left pixel
				right_pixel = getPixel32(temp, width - w - 1, h); // Get right pixel
				setPixel32(temp, w, h, right_pixel);
				setPixel32(temp, width - w - 1, h, left_pixel);
			}
		}

		if (!isFlipped)
		{
			flipped_horizontal = !flipped_horizontal;
		}
	}
	if (flip_direction == VERTICAL) // Flip vertically
	{
		Uint32 top_pixel, bottom_pixel;

		// Swap top pixel and bottom pixel
		for (w = 0; w < width; w++)
		{
			for (h = 0; h < height / 2; h++)
			{
				top_pixel = getPixel32(temp, w, h); // Get top pixel
				bottom_pixel = getPixel32(temp, w, height - h - 1); // Get bottom pixel
				setPixel32(temp, w, h, bottom_pixel);
				setPixel32(temp, w, height - h - 1, top_pixel);
			}
		}

		if (!isFlipped)
		{
			flipped_vertical = !flipped_vertical;
		}
	}

	// Unlock surface when completed
	if (SDL_MUSTLOCK(temp))
	{
		SDL_UnlockSurface(temp);
	}

	resultImage = temp;
}

// Increase/decrease brightness by 5%
void adjustBrightness(SDL_Surface *source, SDL_Surface *destination, brightnessOption brightness_option)
{
	SDL_Surface *temp = destination;
	SDL_BlitSurface(source, NULL, temp, NULL);

	int width = temp->w;
	int height = temp->h;
	int w, h;
	Uint32 cur_pixel;

	if (flipped_horizontal) // Flip temporary image horizontally
	{
		flipImage(temp, HORIZONTAL, true);
	}
	if (flipped_vertical) // Flip temporary image vertically
	{
		flipImage(temp, VERTICAL, true);
	}

	// Lock surface when accessing pixel
	if (SDL_MUSTLOCK(temp))
	{
		SDL_LockSurface(temp);
	}

	Uint8 r, g, b;
	double y, u, v;

	if (brightness_option == INCREASE) // Increase brightness
	{
		bright_count++;
	}
	if (brightness_option == DECREASE) // Decrease brightness
	{
		bright_count--;
	}

	for (w = 0; w < width; w++)
	{
		for (h = 0; h < height; h++)
		{
			cur_pixel = getPixel32(temp, w, h);

			// Get RGB value from image
			SDL_GetRGB(cur_pixel, temp->format, &r, &g, &b);

			convertToYUV(r, g, b, y, u, v);

			// Increase/decrease brightness
			y = y * (1 + (brightness * bright_count));

			convertToRGB(y, u, v, r, g, b);

			// Map converted RGB value to image
			cur_pixel = SDL_MapRGB(temp->format, r, g, b);

			setPixel32(temp, w, h, cur_pixel);
		}
	}

	// Unlock surface when completed
	if (SDL_MUSTLOCK(temp))
	{
		SDL_UnlockSurface(temp);
	}

	resultImage = temp;
}

// Enhance contrast using contrast stretch technique
void contrastStretch(SDL_Surface *surface)
{
	SDL_Surface *temp = surface;
	int width = temp->w;
	int height = temp->h;
	int w, h;
	Uint32 cur_pixel;

	// Lock surface when accessing pixel
	if (SDL_MUSTLOCK(temp))
	{
		SDL_LockSurface(temp);
	}

	Uint8 r, g, b;
	double y, u, v;
	double minimum = 255;
	double maximum = 0;

	for (w = 0; w < width; w++)
	{
		for (h = 0; h < height; h++)
		{
			cur_pixel = getPixel32(temp, w, h);

			// Get RGB value from image
			SDL_GetRGB(cur_pixel, temp->format, &r, &g, &b);

			// Convert to YUV value
			convertToYUV(r, g, b, y, u, v);

			// Find maximum and minimum of luminance
			if (y > maximum)
			{
				maximum = (Uint8)y;
			}
			if (y < minimum)
			{
				minimum = (Uint8)y;
			}

			double delta = maximum - minimum;

			// Increase contrast of image
			y = (y - minimum) * (255 / delta);

			// Convert to RGB value
			convertToRGB(y, u, v, r, g, b);

			// Map converted RGB value to image
			cur_pixel = SDL_MapRGB(temp->format, r, g, b);

			setPixel32(temp, w, h, cur_pixel);
		}
	}

	// Unlock surface when completed
	if (SDL_MUSTLOCK(temp))
	{
		SDL_UnlockSurface(temp);
	}

	resultImage = temp;
}

// Retain 30% central of image and blur the remaining 70%
void blurFilter(SDL_Surface *surface)
{
	// 5x5 matrix low-pass filter
	const double filter[5][5] = { { 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 } };

	SDL_Surface *temp = surface;
	int width = temp->w;
	int height = temp->h;
	int w, h;
	Uint32 cur_pixel;

	// Lock surface when accessing pixel
	if (SDL_MUSTLOCK(temp))
	{
		SDL_LockSurface(temp);
	}

	Uint8 r, g, b;
	double avgR, avgG, avgB;

	// Calculate circle radius
	double radius = sqrt((width * height) * 0.3 / PI);

	for (w = 2; w < (width - 2); w++)
	{
		for (h = 2; h < height - 2; h++)
		{
			// Calculate temporary width and height
			double temp_w = w - (width / 2);
			double temp_h = h - (height / 2);

			// Calculate temporary radius
			double temp_r = sqrt((temp_w * temp_w) + (temp_h * temp_h));

			// Skip central pixels based on radius
			if (temp_r <= radius)
			{
				continue;
			}

			avgR = 0;
			avgG = 0;
			avgB = 0;

			for (int x = -2; x <= 2; x++)
			{
				for (int y = -2; y <= 2; y++)
				{
					cur_pixel = getPixel32(temp, w + x, h + y);

					// Get RGB value from image
					SDL_GetRGB(cur_pixel, temp->format, &r, &g, &b);

					// Applying low-pass filter on each RGB value
					avgR = avgR + ((filter[2 + x][2 + y] * r) / 25);
					avgG = avgG + ((filter[2 + x][2 + y] * g) / 25);
					avgB = avgB + ((filter[2 + x][2 + y] * b) / 25);

					r = truncateRGB(avgR);
					g = truncateRGB(avgG);
					b = truncateRGB(avgB);
				}
			}

			cur_pixel = SDL_MapRGB(temp->format, r, g, b);
			setPixel32(temp, w, h, cur_pixel);
		}
	}

	// Unlock surface when completed
	if (SDL_MUSTLOCK(temp))
	{
		SDL_UnlockSurface(temp);
	}
}

// Load BMP image into a surface for editing
SDL_Surface* loadImageToSurface(char *file)
{
	// Load image to surface
	SDL_Surface *loadedImage = SDL_LoadBMP(file);

	// Convert image to match screen format
	loadedImage = SDL_ConvertSurfaceFormat(loadedImage, SDL_PIXELFORMAT_ARGB8888, 0);

	return loadedImage;
}

// Map surface to texture on rendering device
SDL_Texture* surfaceToTexture(SDL_Surface *surf, SDL_Renderer *ren)
{
	// Create texture to hold surface
	SDL_Texture *texture = NULL;

	texture = SDL_CreateTextureFromSurface(ren, surf);

	return texture;
}

// Draw SDL_Texture to SDL_Renderer at position x, y
void applyTexture(int x, int y, SDL_Texture *source, SDL_Renderer *destination)
{
	// Temporary rectangle to hold texture
	SDL_Rect rect;

	// Setup position of rectangle
	rect.x = x;
	rect.y = y;
	rect.w = window_w;
	rect.h = window_h;

	// Copy to destination renderer
	SDL_RenderCopy(destination, source, NULL, &rect);
}

int main(int argc, char* args[])
{
	// Initialize all SDL subsystems
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return 1;
	}

	// Create display window  at the center of screen
	window = SDL_CreateWindow("Assignment 1 - Task 2", 10, 30, window_w, window_h, SDL_WINDOW_SHOWN);

	// Create renderer for drawing
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// Set drawing color to light blue.
	SDL_SetRenderDrawColor(render, 180, 180, 250, 255);

	// Reset window size base on image size
	SDL_SetWindowSize(window, window_w * 2, window_h);

	// Load images
	originalImage = loadImageToSurface(bmpFile);
	resultImage = loadImageToSurface(bmpFile);
	tempImage = loadImageToSurface(bmpFile);

	originalTexture = surfaceToTexture(originalImage, render);

	// Clear renderer
	SDL_RenderClear(render);

	// Apply texture to renderer
	applyTexture(0, 0, originalTexture, render);
	applyTexture(window_w, 0, originalTexture, render);

	// Event handler
	SDL_Event e;

	bool quit = false;

	// While application is running
	while (!quit)
	{
		SDL_WaitEvent(&e);

		if (e.type == SDL_QUIT)
		{
			quit = true;
			break;
		}
		else if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.sym)
			{
				// Flip image horizontally
			case SDLK_1:
				flipImage(resultImage, HORIZONTAL);
				break;

				// Flip image vertically
			case SDLK_2:
				flipImage(resultImage, VERTICAL);
				break;

				// Increase brightness
			case SDLK_3:
				adjustBrightness(originalImage, tempImage, INCREASE);
				break;

				// Decrease brightness
			case SDLK_4:
				adjustBrightness(originalImage, tempImage, DECREASE);
				break;

				// Enhance contrast
			case SDLK_5:
				contrastStretch(resultImage);
				break;

				// Blur image
			case SDLK_6:
				blurFilter(resultImage);
				break;

				// Quit
			case SDLK_q:
				quit = true;
				break;

			default: break;
			}

			resultTexture = surfaceToTexture(resultImage, render);

			// Clear renderer
			SDL_RenderClear(render);

			// Apply texture to renderer
			applyTexture(0, 0, originalTexture, render);
			applyTexture(window_w, 0, resultTexture, render);
		}

		// Display present render
		SDL_RenderPresent(render);
	}

	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}