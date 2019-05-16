#ifndef AMG_H
#define AMG_H

#include <windows.h>

/**
 * The version of this SDK.
 */
#define AIM_VERSION "1.3.4.2"

/**
 * Feature types.
 */
#define COMPACT_FEATURE		0
#define NORMAL_FEATURE		1
#define EXPANDED_FEATURE	2

/** 
 * The length of face feature.
 */
#define COMPACT_FEATURE_SIZE  1004
#define NORMAL_FEATURE_SIZE   2004
#define EXPANDED_FEATURE_SIZE 4004

/**
 * Flags for feature conversion.
 */
#define FC_COMPACT_TO_NORMAL	0
#define FC_COMPACT_TO_EXPANDED  1
#define FC_NORMAL_TO_EXPANDED   2

/** 
 * The length of face signature.
 */
#define SIGNATURE_SIZE 332

/**
 * The max size of normalized face image.
 */
#define MAX_NFACE_WIDTH 140
#define MAX_NFACE_HEIGHT 160
#define MAX_NFACE_SIZE (MAX_NFACE_WIDTH * MAX_NFACE_HEIGHT * 3)

/**
 * The count of facial landmarks.
 */
#define LANDMARK_COUNT 68

/**
 * A facial landmark.
 */
typedef struct {
	float x;
	float y;
} TLandmark;

/**
 * The landmarking result.
 */
typedef struct {
	TLandmark pts[LANDMARK_COUNT];
} TLandmarks;

/**
 * Face engine object.
 */
DECLARE_HANDLE(HAIM);

/**
 * The result of processed faces.
 */
typedef struct {
	RECT rtFace;									/*< coordinate of face in the image */
	POINT ptLeftEye;								/*< coordinate of the left eye */ 
	POINT ptRightEye;								/*< coordinate of the right eye */
	float roll;										/*< the Euler angle around Z-axis */
	float yaw;										/*< the Euler angle around Y-axis */
	float pitch;									/*< the Euler angle around X-axis */
	float confidence;								/*< confidence of face detector */
	float quality;									/*< quality of face image, e.g., blur, noise, and so on */
	unsigned char feature[EXPANDED_FEATURE_SIZE];	/*< feature template */
} TFaceResult;

/**
 * The result of detected faces.
 */
typedef struct {
	RECT rtFace;								/*< coordinate of face in the image */
	POINT ptLeftEye;							/*< coordinate of the left eye */ 
	POINT ptRightEye;							/*< coordinate of the right eye */
	float roll;									/*< the Euler angle around Z-axis */
	float yaw;									/*< the Euler angle around Y-axis */
	float pitch;								/*< the Euler angle around X-axis */
	float confidence;							/*< confidence of face detector */
	float quality;								/*< quality of face image, e.g., blur, noise, and so on */
	unsigned char softb[SIGNATURE_SIZE];		/*< signature for soft biometrics */
	unsigned char face[MAX_NFACE_SIZE];			/*< normalized face image, BGR format */
	int width;									/*< the width of face image, <= MAX_NFACE_WIDTH*/
	int height;									/*< the height of face image, <= MAX_NFACE_HEIGHT*/
	unsigned char signature[SIGNATURE_SIZE];	/*< signature for feature extraction on remote machine */
} TFaceDetectionResult;

/**
 * The parameters for face detection, quality assessment and feature extraction.
 * Only the faces satisfy the following constrains will be output
 * 1. the eye distance > minEyeDistance;
 * 2. the eye distance < maxEyeDistance;
 * 3. -roll < the roll angle of face < roll;
 * 4. -yaw < the yaw angle of face < yaw;
 * 5. -pitch < the pitch angle of face < pitch;
 * 6. the confidence of face < confidence;
 * 7. the quality of face < quality (0 denotes the best quality, 100 denotes the worst quality).
 * 
 * feaType: the type of feature. The sizes of feature: compact < normal < expanded. 
 *								 The matching speed: compact > normal > expanded.							 
 */
typedef struct {
	float minEyeDistance;		/*< positive, default: 15 */
	float maxEyeDistance;		/*< positive, > minEyeDistance, default: 10000 */
	float roll;					/*< from 0 to 180 degree, default: 0 */
	float yaw;					/*< from 0 to 90 degree, default: 90 */
	float pitch;				/*< from 0 to 90 degree, default: 90 */
	float confidence;			/*< from 0 to 1, default: 0.2 */
	float quality;				/*< from 0 to 100,  default: 100 */
	int sortType;				/*< 0: sort by confidence, 1: by size, default: 0 */
	int feaType;				/*< 0: compact, 1: normal, 2: expanded, default: 1*/
} TFaceParams;

/**
 * Create face engine.
 * @return if success return the handle of face engine, else return NULL.
 */
typedef HAIM (*TAimCreate)();

/** 
 * Free face engine.
 * @param handle the handle of face engine.
 */
typedef void (*TAimFree)(HAIM handle);

/**
 * Set parameters for face detection and quality assessment.
 * @param handle the handle of face engine.
 * @param params the parameters for face detection.
 * @return if success return 0, else return the error code.
 */
typedef int (*TAimSetParams)(HAIM handle, const TFaceParams* params);

/**
 * Detect face and extract feature with parameters.
 * @param handle the handle of face engine.
 * @param image image data in BGR24 or Gray format.
 * @param width the width of image.
 * @param height the height of image.
 * @param pitch the byte offset between two lines of image.
 * @param bpp 8: gray image, 24: BGR image. 
 * @param maxCount the maximum number of faces we want to detect in the image.
 * @param results the properties of detected faces and their features.
 * @return the number of detected faces. 
 */
typedef int (*TAimProcess)(HAIM handle, const unsigned char* image, int width, int height, int pitch, int bpp,
		 int maxCount, TFaceResult* results);

/**
 * Detect face and extract feature for enrollment (high detection rate, X64 only).
 * @param handle the handle of face engine.
 * @param image image data in BGR24 or Gray format.
 * @param width the width of image.
 * @param height the height of image.
 * @param pitch the byte offset between two lines of image.
 * @param bpp 8: gray image, 24: BGR image. 
 * @param result the properties of detected face (maximum) and its feature.
 * @return the number of detected faces. 
 */
typedef int (*TAimEnroll)(HAIM handle, const unsigned char* image, int width, int height, int pitch, int bpp,
	TFaceResult* result);

/**
 * Detect face with parameters.
 * @param handle the handle of face engine.
 * @param image image data in BGR24 or Gray format.
 * @param width the width of image.
 * @param height the height of image.
 * @param pitch the byte offset between two lines of image.
 * @param bpp 8: gray image, 24: BGR image. 
 * @param maxCount the maximum number of faces we want to detect in the image.
 * @param results the properties of detected faces and their features.
 * @return the number of detected faces. 
 */
typedef int (*TAimDetect)(HAIM handle, const unsigned char* image, int width, int height, int pitch, int bpp,
		 int maxCount, TFaceDetectionResult* results);

/**
 * Extract feature from a face.
 * @param handle the handle of face engine.
 * @param image image data in BGR24 or Gray format.
 * @param width the width of image.
 * @param height the height of image.
 * @param pitch the byte offset between two lines of image.
 * @param bpp 8: gray image, 24: BGR image. 
 * @param maxCount the maximum number of faces we want to detect in the image.
 * @param results the properties of detected faces and their features.
 * @return the number of detected faces. 
 */
typedef int (*TAimExtract)(HAIM handle, TFaceDetectionResult* fdResult, unsigned char* feature);

/**
 * Match two features. 
 * @param handle the handle of face engine.
 * @param fea1 feature data of a face image.
 * @param fea2 feature data of another face image.
 * @return return the similarity score of two features (0 to 1).
 */
typedef float (*TAimMatch)(HAIM handle, const unsigned char* fea1, const unsigned char* fea2);

/**
 * Get the feature size (in byte). 
 * @param handle the handle of face engine.
 * @return return the size of feature.
 */
typedef int (*TAimGetFeatureSize)(HAIM handle);

/**
 * Extract feature based on manually labeled landmarks. (X86 only)
 * @param handle the handle of face engine.
 * @param image image data in BGR24 or Gray format.
 * @param width the width of image.
 * @param height the height of image.
 * @param pitch the byte offset between two lines of image.
 * @param bpp 8: gray image, 24: BGR image.
 * @param eyes the coordinates of two eyes.
 * @param feature extracted feature template.
 * @return if success return 0, else return the error code.
 */
typedef int (*TAimManualEnroll)(HAIM handle, const unsigned char* image, int width, int height, int pitch, int bpp,
	const POINT* eyes, unsigned char* feature);

/**
 * Convert feature between different types. (X86 only)
 * @param handle the handle of face engine.
 * @param feaSrc the source type of feature.
 * @param feaDst the destination type of feature.
 * @param
 * @return if success return 0, else return the error code.
 */
typedef int (*TAimConvert)(HAIM handle, const unsigned char* feaSrc, unsigned char* feaDst, int flag);

/**
 * Get the expiration time of CoderMeter Dog.
 * @return -2: fatal error, -1: expired, 0: unlimited, other: the hours to deadline
 */
typedef int (*TGetCodeMeterDate)();

/**
 * Localize the landmarks from a face image. (x64 only)
 * 
 */
typedef int (*TAimLandmark)(HAIM handle, const unsigned char* image, 
	int width, int height, int pitch, int bpp,
	TLandmarks* lms);

extern TAimCreate			IAimCreate;
extern TAimFree				IAimFree;
extern TAimSetParams		IAimSetParams;
extern TAimProcess			IAimProcess;
extern TAimEnroll			IAimEnroll;
extern TAimDetect			IAimDetect;
extern TAimExtract			IAimExtract;
extern TAimMatch			IAimMatch;
extern TAimGetFeatureSize	IAimGetFeatureSize;
extern TAimManualEnroll		IAimManualEnroll;
extern TAimConvert			IAimConvert;
extern TGetCodeMeterDate	IGetCodeMeterDate;
extern TAimLandmark			IAimLandmark;

HMODULE LoadAPI(const char* fileName);
void FreeAPI(HMODULE handle);

#endif