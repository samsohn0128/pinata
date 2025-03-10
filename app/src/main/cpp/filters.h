#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "colour_space.h"
#include <cmath>
#include "enhance.h"
#undef PI
#define PI 3.1415926535897932f

#define min(x,y)  (x <= y) ? x : y
#define max(x,y)  (x >= y) ? x : y

#define ceilComponent(x) ((x > 255) ? 255 : x)

#define blackAndWhite(r, g, b) ((r * 0.59f) + (g * 0.39f) + (b * 0.12f))
#define hardLightLayerPixelComponent(mask, img) ((mask > 128) ? 255 - (((255 - (2 * (mask-128)) ) * (255-img) )/256) : (2*mask*img)/256)
#define sepiaLum(r, g, b) (r * 0.61f + g * 0.32f + b * 0.07f)
#define vinTan(x) (x + (x*x*x)/3)

extern "C" {


const uchar LUTsepiaRed[256] = {24, 24, 25, 26, 27, 28, 29, 30, 30, 30, 31, 32, 33, 34, 35, 36, 37, 37, 38, 38, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 47, 48, 49, 50, 50, 51, 52, 53, 54, 55, 56, 57, 57, 58, 58, 59, 60, 61, 62, 63, 64, 64, 65, 66, 67, 68, 69, 70, 71, 71, 72, 72, 73, 74, 75, 76, 77, 78, 78, 79, 80, 81, 82, 83, 84, 85, 85, 86, 87, 88, 89, 89, 90, 91, 92, 93, 93, 94, 95, 96, 97, 97, 98, 99, 100, 101, 102, 102, 103, 104, 105, 106, 107, 108, 109, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 146, 147, 148, 149, 150, 151, 152, 153, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 178, 180, 181, 182, 183, 184, 185, 186, 186, 187, 188, 189, 190, 191, 193, 194, 195, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 255};
const uchar LUTsepiaGreen[256] = {16, 16, 16, 17, 18, 18, 19, 20, 20, 20, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 31, 32, 33, 33, 34, 35, 36, 36, 36, 37, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 47, 48, 48, 49, 50, 51, 51, 52, 53, 54, 54, 55, 55, 56, 57, 58, 59, 60, 61, 61, 61, 62, 63, 64, 65, 66, 67, 67, 68, 68, 69, 70, 72, 73, 74, 75, 75, 76, 77, 78, 78, 79, 80, 81, 81, 82, 83, 84, 85, 86, 87, 88, 90, 90, 91, 92, 93, 94, 95, 96, 97, 97, 98, 99, 100, 101, 103, 104, 105, 106, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 122, 123, 123, 124, 125, 127, 128, 129, 130, 131, 132, 132, 134, 135, 136, 137, 138, 139, 141, 141, 142, 144, 145, 146, 147, 148, 149, 150, 151, 152, 154, 155, 156, 157, 158, 160, 160, 161, 162, 163, 165, 166, 167, 168, 169, 170, 171, 173, 174, 175, 176, 177, 178, 179, 180, 182, 183, 184, 185, 187, 188, 189, 189, 191, 192, 193, 194, 196, 197, 198, 198, 200, 201, 202, 203, 205, 206, 207, 208, 209, 210, 211, 212, 213, 215, 216, 217, 218, 219, 220, 221, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 255};
const uchar LUTsepiaBlue[256] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 11, 12, 12, 13, 13, 14, 14, 14, 14, 15, 15, 16, 16, 17, 17, 17, 18, 18, 19, 20, 20, 21, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 28, 28, 29, 29, 30, 31, 31, 31, 32, 33, 33, 34, 35, 36, 37, 38, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 47, 48, 49, 50, 51, 52, 53, 53, 54, 55, 56, 57, 58, 59, 60, 60, 61, 62, 63, 65, 66, 67, 67, 68, 69, 70, 72, 73, 74, 75, 75, 76, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 90, 91, 92, 93, 93, 95, 97, 98, 99, 100, 101, 102, 104, 104, 106, 107, 108, 109, 111, 112, 114, 115, 115, 117, 118, 120, 121, 122, 123, 124, 125, 127, 128, 129, 131, 132, 133, 135, 136, 137, 138, 139, 141, 142, 144, 145, 147, 147, 149, 150, 151, 153, 154, 156, 157, 159, 159, 161, 162, 164, 165, 167, 168, 169, 170, 172, 173, 174, 176, 177, 178, 180, 181, 182, 184, 185, 186, 188, 189, 191, 192, 193, 194, 196, 197, 198, 200, 201, 203, 204, 205, 206, 207, 209, 210, 211, 213, 214, 215, 216, 218, 219, 220, 221, 223, 224, 225, 226, 227, 229, 230, 231, 232, 234, 235, 236, 237, 238, 239, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 255};

const unsigned char LUTxproRed[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                             1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7,
                                             8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 14, 15,
                                             15, 16, 16, 17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 23, 24, 24, 25, 26,
                                             27, 27, 28, 29, 30, 30, 31, 32, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41,
                                             42, 44, 44, 45, 46, 47, 49, 50, 52, 53, 54, 56, 57, 58, 60, 61, 63, 64,
                                             66, 68, 69, 71, 73, 75, 76, 78, 80, 81, 83, 85, 87, 89, 91, 93, 95, 97,
                                             98, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125,
                                             127, 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 149, 151, 154,
                                             156, 157, 159, 161, 163, 165, 167, 169, 171, 173, 175, 177, 178, 180,
                                             182, 184, 185, 187, 188, 191, 192, 193, 195, 197, 198, 200, 202, 203,
                                             205, 206, 208, 209, 211, 212, 214, 215, 217, 219, 220, 221, 223, 224,
                                             225, 227, 228, 230, 231, 232, 234, 235, 236, 237, 239, 240, 241, 242,
                                             243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 255,
                                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                             255, 255, 255 };
const unsigned char LUTxproGreen[256] = { 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                               10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
                                               28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
                                               47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 58, 59, 60, 61, 62, 63, 64, 65,
                                               67, 69, 70, 71, 72, 73, 75, 76, 77, 78, 80, 81, 82, 83, 85, 86, 87, 88,
                                               90, 91, 92, 94, 95, 96, 97, 99, 100, 101, 103, 104, 105, 107, 108, 109,
                                               111, 112, 113, 115, 116, 117, 119, 120, 121, 123, 124, 125, 127, 129,
                                               130, 132, 133, 134, 136, 137, 138, 140, 141, 142, 144, 145, 146, 147,
                                               149, 149, 150, 152, 153, 154, 155, 157, 158, 159, 160, 162, 163, 164,
                                               166, 167, 168, 170, 171, 172, 173, 174, 176, 177, 178, 179, 181, 182,
                                               183, 184, 185, 187, 188, 189, 190, 191, 192, 193, 195, 196, 197, 198,
                                               199, 200, 201, 202, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213,
                                               213, 215, 216, 217, 217, 218, 219, 220, 221, 222, 222, 223, 224, 225,
                                               226, 226, 227, 228, 229, 229, 230, 231, 231, 232, 233, 233, 234, 235,
                                               235, 236, 236, 237, 238, 238, 239, 239, 240, 240, 241, 241, 242, 242,
                                               243, 243, 244, 244, 245, 245, 245, 246, 246, 247, 247, 248, 248, 248,
                                               249, 249, 250, 250, 250, 251, 251, 251, 252, 252, 253, 253, 254, 254,
                                               255, 255, 255 };
const unsigned char LUTxproBlue[256] = { 21, 21, 21, 22, 23, 24, 25, 26,
                                              27, 28, 29, 29, 30, 31, 32, 33, 34, 34, 35, 36, 37, 38, 39, 39, 40, 41,
                                              42, 43, 44, 44, 45, 46, 47, 48, 49, 49, 50, 51, 52, 53, 54, 54, 55, 56,
                                              57, 58, 59, 59, 60, 61, 62, 63, 64, 64, 65, 66, 67, 68, 69, 69, 70, 71,
                                              72, 73, 74, 74, 75, 76, 77, 78, 79, 80, 80, 81, 82, 83, 84, 84, 85, 86,
                                              87, 88, 89, 89, 91, 91, 92, 93, 94, 95, 95, 96, 97, 98, 99, 100, 101,
                                              101, 102, 103, 104, 105, 106, 106, 107, 108, 109, 110, 111, 111, 112,
                                              113, 114, 115, 115, 116, 117, 118, 119, 120, 121, 121, 122, 123, 124,
                                              125, 126, 126, 127, 128, 129, 130, 131, 132, 132, 133, 134, 135, 136,
                                              137, 137, 138, 139, 140, 141, 142, 142, 143, 144, 145, 146, 147, 147,
                                              148, 149, 150, 151, 152, 152, 153, 154, 155, 156, 157, 157, 158, 159,
                                              160, 160, 162, 162, 163, 164, 165, 166, 167, 167, 168, 169, 170, 171,
                                              172, 172, 173, 174, 175, 176, 176, 177, 178, 179, 180, 181, 181, 182,
                                              183, 184, 185, 186, 186, 187, 188, 189, 190, 191, 191, 192, 193, 194,
                                              195, 196, 197, 198, 198, 199, 200, 201, 202, 203, 203, 204, 205, 206,
                                              207, 208, 208, 209, 210, 211, 212, 213, 213, 214, 215, 216, 217, 218,
                                              218, 219, 220, 221, 222, 223, 223, 224, 225, 226, 227, 228, 228, 229,
                                              230, 231, 232, 232, 233 };

static uchar overlayComponents(uchar overlayComponent, uchar underlayComponent, float alpha) {
    float underlay = underlayComponent * alpha;
    return (uchar)((underlay / 255) * (underlay + ((2.0f * overlayComponent) / 255) * (255 - underlay)));
}

static uchar multiplyPixelComponentsWithAlpha(unsigned char overlayComponent, float alpha, unsigned char underlayComponent) {
    return (uchar)((underlayComponent * overlayComponent * alpha)/255);
}

static uchar multiplyPixelComponents(unsigned char overlayComponent, uchar underlayComponent) {
    return (uchar)((float)(underlayComponent * overlayComponent)/255);
}

static unsigned char darkenPixelsComponent(unsigned char overlay, unsigned char underlay) {
    return min(underlay, overlay);
}

static float applyBrightnessToPixelComponent(float colourComponent, float brightness) {
    float scaled = brightness/2;
    if (scaled < 0.0) {
        return colourComponent * ( 1.0f + scaled);
    } else {
        return colourComponent + ((1.0f - colourComponent) * scaled);
    }
}

static double applyContrastToPixelComponent(float pixelComponent, float contrast) {
    return min(1.0f, ((pixelComponent - 0.5f) * (tan ((contrast + 1) * PI/4) ) + 0.5f));
}


void applySajuno(cv::Mat &inp, cv::Mat &out, int val);
void applyManglow(cv::Mat &inp, cv::Mat &out, int val);
void applyPalacia(cv::Mat &inp, cv::Mat &out, int val);
void applyBW(cv::Mat &inp, cv::Mat &out, int val);
void applyAnsel(cv::Mat &inp, cv::Mat &out, int val);
void applySepia(cv::Mat &inp, cv::Mat &out, int val);
void applyAnax(cv::Mat &inp, cv::Mat &out, int val);
void applyThreshold(cv::Mat &inp, cv::Mat &out, int val);
void applyGrain(cv::Mat &inp, cv::Mat &out, int val);
void applyHistEq(cv::Mat &inp, cv::Mat &out, int val);
void applyCyano(cv::Mat &inp, cv::Mat &out, int val);
void applyBoostRedEffect(cv::Mat &inp, cv::Mat &out, int val);
void applyNegative(cv::Mat &src, cv::Mat &dst, int val);
void applyGreenBoostEffect(cv::Mat &src, cv::Mat &dst, int val);
void applyColorBoostEffect(cv::Mat &src, cv::Mat &dst, int val);
void applyBlueBoostEffect(cv::Mat &src, cv::Mat &dst, int val);
void applyCyanise(cv::Mat &src, cv::Mat &dst, int val);
void applyFade(cv::Mat &src, cv::Mat &dst, int val);
void applyCartoon(cv::Mat &src, cv::Mat &dst, int val);
void applyEdgify(cv::Mat &src, cv::Mat &dst, int val);
void applyPencilSketch(cv::Mat &src, cv::Mat &dst, int val);
void applyRedBlueEffect(cv::Mat &src, cv::Mat &dst, int val);
void applyRedGreenFilter(cv::Mat &src, cv::Mat &dst, int val);
void applyWhiteYellowTint(cv::Mat &src, cv::Mat &dst, int val);
}