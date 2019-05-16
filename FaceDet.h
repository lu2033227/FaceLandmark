#ifndef FaceDet_H
#define FaceDet_H

#include <windows.h>

/*
version 1.1    Date: 2013/12/13
1.fix some bugs.
2.add the component id and part positions to the output.

Version 1.0    Date: 2013/8/13
1.the first version. The basic framework and some codes are from Junjie's program in pedestrian detection.

This is a program for face detection in the wild, developed by Junjie Yan @ NLPR. (yanjunjie@outlook.com) 

Since this version does not care so much on speed, it has the following differences with the algorithm in [1]:
1. The gradient feature are calculated on three channels, which results a slight better accuracy with slight slower speed.
2. The default model includes 8 mixtures, include mixtures for yaw angle in [-90,-60],[-60,-30],[-30,-15],[-15,0] and their mirrors (the roll and pitch angles are between [-30,30]).
3. If you want to get the ROC curve on FDDB in the ICB2013 paper, please use linear regression as the post-processing step to refine the bounding box location. The reasons is that the annotation style of FDDB
is very different from the annotation style in our training model.

The following are some important notes in using the program.
1. The default model can detect faces larger than 72X72 pixels. To detect smaller faces, please specify the last parameter when calling FD_process.
2. The available parameters are mainly in the structure TFaceParams, please specify an instance of the structure before use the DLL.
3. The returned face detection results are in the structure TFaces. It has an limitation that the face number is smaller than 256 for an image, which should always hold in real applications.
4. The DLL uses some INTEL MKL and IPP functions for the speed, and was built by the Intel C++ compilers, OpenCV version 2.4.6. Where the opencv_core is used to read image and opencv_imgproc to resize image.
5. Please note the memory on your machine. It should be OK for images such as 2000*1500 with 4G memeory machine, but may be out of memory for larger images when 8 mixture model is used on a PC with 4GB memory.
6. The current release is a single thread version, parallel should be OK if you want to make it faster.
7. The program accepts 1-channel gray image. However, for best performance, please use 3-channel color image if possible.

This software is developed based on Junjie's papers. If you use this software in your paper, please kindly cite the following two papers:
1.
@article{junjie2014IVC,
title={Face Detection by Structural models},
author={Yan, J. and Zhang, X. and Lei, Z. and Li, S},
journal={Image and vision computing},
year={2014},
publisher={Elsevier}
}

2.
@article{yanreal,
title={Real-time High Performance Deformable Model for Face Detection in the Wild},
author={Yan, Junjie and Zhang, Xucong and Lei, Zhen and Li, Stan Z},
booktitle={6th IEEE International Conference on Biometrics},
year={2013},
publisher={IEEE}
}
*/

/**
 * Face detection handle.
 */
DECLARE_HANDLE(HFaceDetector);

/**
 * Face detection result.
 */

#define MAXPEDINIMAGE 256
typedef struct tagFaces {
	int _count;		/*< the count of faces in a image */
	RECT _pos[MAXPEDINIMAGE];		/*< the positions of faces in a image */
	double _score[MAXPEDINIMAGE];   /*< the scores of faces in a image */
	int _partnum;
	int _componentid[MAXPEDINIMAGE];
	RECT _partPos[MAXPEDINIMAGE*16];
} TFaces;

/**
 * Face detection parameters.
 */
typedef struct tagParams{
	float thresh; /*< the threshold in face detection. Tt can be any float value, but please specify it between [-0.5,0] in general application. Smaller value means more detections.  -0.18 is a prefer parameter. */
	int interval; /*< the interval number in building feature pyramid. It means the layer number between in an octave. please specify it in [4,10]. Larger value means more detections, slower speed and higher memory. 4 is a prefer parameter in general application.*/
	int padx; /*< number of padded cells in x direction to feature pyramid for truncated faces. If you don't need truncated faces, just set it to be 0. */
	int pady; /*< number of padded cells in y direction to feature pyramid for truncated faces. If you don't need truncated faces, just set it to be 0. */
	char modelfile1[200]; /*< It specifies the location of model file1. In current release, we provide three choices.*/
	char modelfile2[200]; /*< It specifies the location of model file2. */
} TFaceParams;

/**
 * Create a face detector.
 * @return if success return the handle of the detector, otherwise return NULL.
 */
HFaceDetector FD_Create(TFaceParams&);

/**
* Detect all Faces in a color image by a f ace detector.
* @param handle: the handle of the face detector
* @param bgr: image data in directshow (and opencv) format, 3 channel color image
* @param width: the width of the image
* @param height: the height of the image
* @param pitch: pitch of image data
* @param bpp: 8 for gray and 24 for bgr
* @param resizescale: resize scale of the original image. If your designed face can be smaller than 72*72, please set it above 1 based on your desgined size. For example, if the minial face size if 36*36, you can set it to be 0.5. For some high resolution images with large faces, set it below 1.0f would speed up the detection.
return detection result in Tfaces format.
 */

TFaces FD_Process(HFaceDetector handle, unsigned char* bgr, int width, int height,int pitch, int bpp, float resizescale);
/**
 * Free a created face detector.
 * @param handle the handle of a face detector
 */ 
void FD_Free(HFaceDetector handle);

#endif
