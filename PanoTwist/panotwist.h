/* Copyright (c) 2018 Peter Kondratyuk. All Rights Reserved.
*
* You may use, distribute and modify the code in this file under the terms of the MIT Open Source license, however
* if this file is included as part of a larger project, the project as a whole may be distributed under a different
* license.
*
* MIT license:
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
* to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions
* of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#ifndef PANOTWIST_H
#define PANOTWIST_H

#include "Array.h"
#include "CvImageWidget.h"

#include "NadirZenithWidget.h"
#include "MaxSizeWidget.h"

#include <QtWidgets/QMainWindow>
#include "ui_panotwist.h"

class PanoTwist : public QMainWindow
{
	Q_OBJECT

public:
	PanoTwist(QWidget *parent = 0);
	~PanoTwist(){}

public slots:
	void OnOpenFolderClicked();
	void OnPrevFileClicked();
	void OnNextFileClicked();
	void OnApplyClicked();					//Save single file
	void OnApplyToAllClicked();				//Save all opened files (batch save) - does not rotate
	void OnMenuHelp();
	void OnMenuAbout();
	void OnMenuLicense();

	void OnImageMouseLeftPressed(cv::Point2d pos);
	void OnImageMouseLeftReleased(cv::Point2d pos);
	void OnImageMouseMoved(cv::Point2d pos);

	void OnNadirZenithChanged();

private:
	void Init();
	void UpdateInterface();												//Updates the button state and image based on curIndex
	bool FileSaveChecks();												//Some checks to make sure we can save the files
	void ShowImage();													//Show the current scaledImage with crosshairs at the center
	void ProcessImage(cv::Mat& image, bool fRotate, bool fRescale);		//Applies rotation (if requested) and nadir-zenith modifications to the image

	//Image transformation functions called by the ProcessImage function
	void InternalPatch(cv::Mat& image, NadirZenithWidget* nzWidget);
	void InternalRotate(cv::Mat& image);
	void InternalRescale(cv::Mat& image);

	//OpenCV-based functions for pixel operations
	cv::Vec3b InterpolatePixel(const cv::Mat& img, cv::Point2f pt, int borderX, int borderY);
	void HorizontalCyclicRotate(cv::Mat& mat, int numPixels);

	//Inserting exif tags into the processed files
	void InsertExifTags(const BString& filename, const cv::Mat& image);	//Fix exif tags after the rotation is done

private:
	CvImageWidget* imageWidget;
	NadirZenithWidget* zenithWidget;
	NadirZenithWidget* nadirWidget;
	MaxSizeWidget* maxSizeWidget;

private:
	CHArray<BString> fileArray;		//The full file names of equirectangular panorama files in the current directory
	CHArray<BString> nameOnlyArray;	//Same, but only the file names and not the paths
	BString curFolder;				//The currently opened folder
	BString saveSubfolderName;

	int curIndex;					//The index of the current file in the fileArray

	cv::Mat fullMat;
	cv::Mat scaledMat;
	cv::Size scaledSize;

	double rotationRad;				//Current accumulated rotation angle in radians

	cv::Point2d lastClickCoord;		//the coordinate of a mouse click when dragging the image
	double clickRotationRad;		//the rotation of the image when the user clicked the mouse to drag the image
	bool fDraggingImage;			//whether the image is being dragged

	const double Pi;

private:
	Ui::PanoTwistClass ui;
};

#endif // PANOTWIST_H
