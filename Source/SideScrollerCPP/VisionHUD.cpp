/*
* VisionHUD: Heads-Up Display class for OpenCV projects with UnrealEngine4
*
* Adapted from https://answers.unrealengine.com/questions/193827/how-to-get-texture-pixels-using-utexturerendertarg.html
*
* Copyright (C) 2017 Simon D. Levy
*
* MIT License
*/

#include "VisionHUD.h"

// Use whatever machine-vision algorithm you like
#include "EdgeDetection.h"

AVisionHUD::AVisionHUD()
{
	// Get Minimap render target from blueprint
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> MiniMapTexObj(RENDER_TARGET_NAME);
	MiniMapTextureRenderTarget = MiniMapTexObj.Object;

	// Create Texture2D to store MiniMapTex content
	UTexture2D* MiniMapTexture = UTexture2D::CreateTransient(MiniMapTextureRenderTarget->SizeX, MiniMapTextureRenderTarget->SizeY, RENDER_TARGET_PIXEL_FORMAT);

#if WITH_EDITORONLY_DATA
	MiniMapTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
	MiniMapTexture->SRGB = MiniMapTextureRenderTarget->SRGB;

	MiniMapRenderTarget = MiniMapTextureRenderTarget->GameThread_GetRenderTargetResource();

	// Allocate memory for BGR image bytes
	_rows = MiniMapTextureRenderTarget->SizeY;
	_cols = MiniMapTextureRenderTarget->SizeX;
	_bgrbytes = new uint8_t[_rows*_cols*3];

	// Specify a machine-vision algorithm
	_algorithm = new EdgeDetection(this, LEFTX, TOPY);
}

AVisionHUD::~AVisionHUD()
{
	delete _bgrbytes;
	delete _algorithm;
}

void AVisionHUD::DrawHUD()
{
	Super::DrawHUD();	

	// Draw the image to the HUD
	DrawTextureSimple(MiniMapTextureRenderTarget, LEFTX, TOPY, 1.0f, true);

	// Read the pixels from the RenderTarget and store them in a FColor array
	MiniMapRenderTarget->ReadPixels(MiniMapSurfData);

	// Convert the FColor array to a BRG byte array
	for (int x = 0; x < _cols; ++x) {

		for (int y = 0; y < _rows; ++y) {

			int k = x + y * _cols;

			FColor PixelColor = MiniMapSurfData[k];

			_bgrbytes[k*3]   = PixelColor.B;
			_bgrbytes[k*3+1] = PixelColor.G;
			_bgrbytes[k*3+2] = PixelColor.R;
		}
	}

    // Convert BGR bytes to OpenCV Mat
    cv::Mat bgrimg(_rows, _cols, CV_8UC3, _bgrbytes);

	// Run your vision algorithm on the OpenCV Mat
    _algorithm->perform(bgrimg);
    
	// Draw a border around the image

	float rightx = LEFTX + _cols;
	float bottomy = TOPY + _rows;

	drawBorder(LEFTX, TOPY, rightx, TOPY);
	drawBorder(rightx, TOPY, rightx, bottomy);
	drawBorder(rightx, bottomy, LEFTX, bottomy);
	drawBorder(LEFTX, bottomy, LEFTX, TOPY);
}

void AVisionHUD::drawBorder(float lx, float uy, float rx, float by)
{
	DrawLine(lx, uy, rx, by, BORDER_COLOR, BORDER_WIDTH);
}
