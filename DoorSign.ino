#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>

#include <SPI.h>

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

const uint8_t lettersStripPixelCount = 66;
const uint8_t lettersStripPin = 2;

const uint8_t lampStripPixelCount = 3;
const uint8_t lampStripPin = 12;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> lettersStrip(lettersStripPixelCount, lettersStripPin);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> lampStrip(lampStripPixelCount, lampStripPin);

NeoPixelAnimator animations(2);
NeoPixelAnimator lampAnimations(lampStripPixelCount);

const RgbColor black(0, 0, 0);
RgbColor lettersColor = HslColor(random(360) / 360.0f, 1.0f, 255);

uint16_t lastLetter = 0;
int8_t moveDir = 1; // track the direction of movement

// what is stored for state is specific to the need, in this case, the colors.
// Basically what ever you need inside the animation update function
struct MyAnimationState
{
		RgbColor StartingColor;
		RgbColor EndingColor;
};

MyAnimationState animationState[lampStripPixelCount];

void SetRandomSeed()
{
		uint32_t seed;

		// random works best with a seed that can use 31 bits
		// analogRead on a unconnected pin tends toward less than four bits
		seed = analogRead(0);
		delay(1);

		for (int shifts = 3; shifts < 31; shifts += 3)
		{
				seed ^= analogRead(0) << shifts;
				delay(1);
		}

		// Serial.println(seed);
		randomSeed(seed);
}

void BlendAnimUpdate(const AnimationParam& param)
{
		// this gets called for each animation on every time step
		// progress will start at 0.0 and end at 1.0
		// we use the blend function on the RgbColor to mix
		// color based on the progress given to us in the animation
		RgbColor updatedColor = RgbColor::LinearBlend(
				animationState[param.index].StartingColor,
				animationState[param.index].EndingColor,
				param.progress);
		// apply the color to the strip
		lampStrip.SetPixelColor(param.index, updatedColor);
}

void PickRandom(float luminance)
{
		// pick random count of pixels to animate
		uint16_t count = random(lampStripPixelCount);
		while (count > 0)
		{
				// pick a random pixel
				uint16_t pixel = random(lampStripPixelCount);

				// pick random time and random color
				// we use HslColor object as it allows us to easily pick a color
				// with the same saturation and luminance
				uint16_t time = random(100, 400);
				animationState[pixel].StartingColor = lampStrip.GetPixelColor(pixel);
				animationState[pixel].EndingColor = HslColor(random(360) / 360.0f, 1.0f, luminance);

				lampAnimations.StartAnimation(pixel, time, BlendAnimUpdate);

				count--;
		}
}

void MoveAnimUpdate(const AnimationParam& param)
{
	const uint8_t letterCount = 13;
	const uint8_t ledPerLetter = 6;

	const uint16_t letters[letterCount][ledPerLetter] = {
		{ 0,  1,  2,  3,  4,  5}, //C
		{ 6,  7,  8,  9, 10, 11}, //R
		{12, 13, 14, 15, 16, 17}, //E
		{18, 19, 20, 21, 22, -1}, //A
		{23, 24, 25, 26, 27, -1}, //T
		{28, 29, 30, -1, -1, -1}, //I
		{31, 32, 33, 34, 35, -1}, //V
		{36, 37, 38, 39, 40, 41}, //E

		{42, 43, 44, 45, -1, -1}, //S
		{46, 47, 48, 49, 50, -1}, //P
		{51, 52, 53, 54, 55, -1}, //A
		{56, 57, 58, 59, -1, -1}, //C
		{60, 61, 62, 63, 64, 65}  //E
	};

	uint16_t nextLetter;

	if (moveDir > 0)
	{
		nextLetter = param.progress * letterCount;
	}
	else
	{
		nextLetter = (1.0f - param.progress) * letterCount;
	}

	for (uint8_t f = 0; f < ledPerLetter; f++) {
		if (letters[lastLetter][f] != -1) {
			lettersStrip.SetPixelColor(letters[lastLetter][f], black);
		}

		if (letters[nextLetter][f] != -1) {
			lettersStrip.SetPixelColor(letters[nextLetter][f], lettersColor);
		}
	}

	lastLetter = nextLetter;

	if (param.state == AnimationState_Completed)
	{
		moveDir *= -1;
		animations.RestartAnimation(param.index);

		if (moveDir > 0) {
			lettersColor = HslColor(random(360) / 360.0f, 1.0f, 255);
		}
	}
}

void SetupAnimations()
{
	animations.StartAnimation(0, 1000, MoveAnimUpdate);
}

void setup()
{

	lettersStrip.Begin();
	lettersStrip.Show();

	lampStrip.Begin();
	lampStrip.Show();

	SetRandomSeed();
	SetupAnimations();
}

void loop()
{
	animations.UpdateAnimations();
	lettersStrip.Show();

	if (lampAnimations.IsAnimating())
	{
		// the normal loop just needs these two to run the active animations
		lampAnimations.UpdateAnimations();
		lampStrip.Show();
	}
	else
	{
		// no animations runnning, start some
		//
		PickRandom(0.2f); // 0.0 = black, 0.25 is normal, 0.5 is bright
	}
}
