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

#include "panotwist.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QLayout>
#include <QStyleFactory>

#include "DialogOpeningFolder.h"
#include "DialogHelpOrLicence.h"
#include "DialogSaveAll.h"
#include "DialogAbout.h"

#include <exiv2/exiv2.hpp>

PanoTwist::PanoTwist(QWidget *parent):
	QMainWindow(parent),
	Pi(3.1415926535897932)
{
	ui.setupUi(this);

	QApplication::setStyle(QStyleFactory::create("Fusion"));

	setFixedSize(size());
	
	saveSubfolderName = "Panotwist output/";

	imageWidget = new CvImageWidget(ui.scrollAreaImage);

	QHBoxLayout* nadirLayout = new QHBoxLayout(this);
	nadirLayout->setMargin(0);
	ui.frameNadir->setLayout(nadirLayout);

	QHBoxLayout* zenithLayout = new QHBoxLayout(this);
	zenithLayout->setMargin(0);
	ui.frameZenith->setLayout(zenithLayout);

	nadirWidget = new NadirZenithWidget();
	nadirWidget->SetCheckText("Patch nadir");
	nadirWidget->SetColor("#AAAAAA");
	nadirWidget->SetFnadir(true);

	zenithWidget = new NadirZenithWidget();
	zenithWidget->SetCheckText("Patch zenith");
	zenithWidget->SetColor("#66d9ff");
	zenithWidget->SetFnadir(false);

	nadirLayout->addWidget(nadirWidget);
	zenithLayout->addWidget(zenithWidget);

	maxSizeWidget = new MaxSizeWidget();
	QHBoxLayout* maxSizeLayout = new QHBoxLayout(this);
	maxSizeLayout->setMargin(0);
	ui.frameMaxSize->setLayout(maxSizeLayout);
	maxSizeLayout->addWidget(maxSizeWidget);

	//Connect events from the zenith and nadir widgets
	QObject::connect(nadirWidget, &NadirZenithWidget::SignalSettingsChanged, this, &PanoTwist::OnNadirZenithChanged);
	QObject::connect(zenithWidget, &NadirZenithWidget::SignalSettingsChanged, this, &PanoTwist::OnNadirZenithChanged);

	//Connect events from the image widget
	QObject::connect(imageWidget, &CvImageWidget::MouseLeftPressed, this, &PanoTwist::OnImageMouseLeftPressed);
	QObject::connect(imageWidget, &CvImageWidget::MouseLeftReleased, this, &PanoTwist::OnImageMouseLeftReleased);
	QObject::connect(imageWidget, &CvImageWidget::MouseMoved, this, &PanoTwist::OnImageMouseMoved);
	
	fDraggingImage = false;

	Init();
}

void PanoTwist::Init()
{
	fileArray.Clear();
	nameOnlyArray.Clear();
	curFolder = "";

	curIndex = -1;

	scaledSize = cv::Size(imageWidget->AvailableWidth()+1,imageWidget->AvailableHeight()+1);
	scaledMat = cv::Mat::zeros(scaledSize, CV_8UC3);

	rotationRad = 0;

	ui.labelCurrentFolder->setText("");
	ui.labelCurrentFile->setText("");

	imageWidget->Clear();

	UpdateInterface();
}

//Update the interface - the buttons, the current file name, and the image - based on curIndex
void PanoTwist::UpdateInterface()
{
	//If no directory is selected, or no files in the current directory
	//Disable all buttons except for OpenDirectory, and enable them otherwise

	bool fFilesPresent = (curIndex >= 0 );

	ui.bnPrevFile->setEnabled(fFilesPresent);
	ui.bnNextFile->setEnabled(fFilesPresent);
	ui.bnApply->setEnabled(fFilesPresent);
	ui.bnApplyAll->setEnabled(fileArray.Count() > 1);
	ui.labelDragImage->setEnabled(fFilesPresent);
	nadirWidget->setEnabled(fFilesPresent);
	zenithWidget->setEnabled(fFilesPresent);
	maxSizeWidget->setEnabled(fFilesPresent);

	//Don't do anything if no files are present
	if(!fFilesPresent)	return;
	
	//If we do have files in the arrays, selectively enable the Prev/Next file buttons
	ui.bnPrevFile->setEnabled(curIndex != 0);
	ui.bnNextFile->setEnabled(curIndex < fileArray.Count() - 1);
	
	setCursor(Qt::WaitCursor);

	//Read the current file
	fullMat = cv::imread(fileArray[curIndex], CV_LOAD_IMAGE_COLOR);

	int interpMethod;
	if (fullMat.cols >= scaledMat.cols) interpMethod = cv::INTER_AREA;
	else interpMethod = cv::INTER_LINEAR;

	cv::resize(fullMat, scaledMat, scaledSize, 0, 0, interpMethod);

	setCursor(Qt::ArrowCursor);

	//Show the current name of the file
	BString fileString;
	fileString.Format("File %i of %i: %s (%i x %i)",
		curIndex + 1,
		fileArray.Count(),
		nameOnlyArray[curIndex],
		fullMat.cols,
		fullMat.rows);
	ui.labelCurrentFile->setText(fileString.c_str());

	//The rotation position is zero after loading
	rotationRad = 0;
	
	//Show the image
	ShowImage();
}

//Show image of the current rotated scaledImage with crosshairs drawn in the middle
void PanoTwist::ShowImage()
{
	if (curIndex == -1) return;

	cv::Mat temp;
	scaledMat.copyTo(temp);
	
	//Apply rotation and process nadir and zenith
	ProcessImage(temp, true, false);

	//Draw a cross in the center
	int xSize = scaledSize.width;
	int ySize = scaledSize.height;
	int lineLength = 40;
	int thickness = 2;
	int centerX = xSize / 2;
	int centerY = ySize / 2;
	cv::Scalar color(0,0,255);

	cv::line(temp, cv::Point(centerX, centerY - lineLength), cv::Point(centerX, centerY + lineLength), color, thickness);
	cv::line(temp, cv::Point(centerX - lineLength, centerY), cv::Point(centerX + lineLength, centerY), color, thickness);

	imageWidget->ShowImage(temp);
}

//Apply current rotation and patch nadir-zenith in the image provided
void PanoTwist::ProcessImage(cv::Mat& image, bool fRotate, bool fRescale)
{
	if (fRescale) InternalRescale(image);
	if(fRotate) InternalRotate(image);
	InternalPatch(image, nadirWidget);
	InternalPatch(image, zenithWidget);
}

//Rescale the image if the maxSizeWidget is enabled
//And image size is greater than allowed
void PanoTwist::InternalRescale(cv::Mat& image)
{
	if (!maxSizeWidget->IsEnabled()) return;

	int maxHeight = maxSizeWidget->MaxPermittedHeight();
	if (maxHeight >= image.rows) return;

	//Yes, the image is bigger than allowed - rescale it down
	cv::resize(image, image, cv::Size(maxHeight * 2, maxHeight), 0, 0, cv::INTER_AREA);
}

//Internal function to rotate the image in the azimuth angle
void PanoTwist::InternalRotate(cv::Mat& image)
{
	double rotAngle = fmod(rotationRad, 2*Pi);
	int rotationPixels = int(double(image.cols) * rotAngle / (2*Pi));

	HorizontalCyclicRotate(image, rotationPixels);
}

//Internal function called by ProcessNadirZenith()
void PanoTwist::InternalPatch(cv::Mat& image, NadirZenithWidget* nzWidget)
{
	if (!nzWidget->IsEnabled()) return;

	bool fNadir = nzWidget->IsNadir();
	double angleDeg = nzWidget->AngleDeg();
	double angleRad = angleDeg / 180.0 * Pi;

	int xSize = image.cols;
	int ySize = image.rows;

	//The height of the patch in equirectangular projection
	int nzHeight = int(double(ySize) * angleDeg / 180.0 / 2);

	if (nzHeight == 0) return;

	//The location of the patch - either at the top or bottom
	int yStartPoint;
	if (fNadir) yStartPoint = ySize - nzHeight;
	else yStartPoint = 0;
	
	cv::Mat dest(image, cv::Rect(0, yStartPoint, xSize, nzHeight));
	
	if (nzWidget->IsFillAverage())			//Filled by average color
	{
		cv::Vec3d bgr = cv::Vec3d(0,0,0);
		int numPixelsIncluded = 0;		//We will exclude black pixels from the calculation

		//Sum of all pixel values
		for (int i = 0; i < dest.cols; i++)
		{
			for (int j = 0; j < dest.rows; j++)
			{
				cv::Vec3b curPixel = dest.at<cv::Vec3b>(j, i);
				
				//Exclude black pixels
				if (curPixel(0) > 0 && curPixel(1) >0 && curPixel(2) > 0)
				{
					numPixelsIncluded++;
					bgr += curPixel;
				}
			}
		}

		cv::Vec3b avColor(0,0,0);
		if (numPixelsIncluded > 0)
		{
			double num = numPixelsIncluded;
			avColor = cv::Vec3b(bgr(0) / num, bgr(1) / num, bgr(2) / num);
		}

		cv::Mat patchImage(nzHeight, xSize, image.type(), avColor);
		patchImage.copyTo(dest);
	}

	else if (nzWidget->IsFillColor())		//Filled by specified color
	{
		BString colorString = nzWidget->ColorString();

		QColor color(colorString.c_str());

		cv::Mat patchImage( nzHeight, xSize, image.type(), cv::Vec3b(color.blue(), color.green(), color.red()) );
		patchImage.copyTo(dest);
	}

	else if (nzWidget->IsFillImage() && nzWidget->IsPatchImageLoaded())		//Filled by image
	{
		cv::Mat patchImage = nzWidget->PatchImage();

		//The size of the patch image, in distances between pixels
		int patchXsize = patchImage.cols-1;
		int patchYsize = patchImage.rows-1;

		double patchXcenter = double(patchXsize) / 2.;
		double patchYcenter = double(patchYsize) / 2.;
		
		//Diameter and radius of the circle from the patch image that we'll be using
		//In pixels
		double diameter = std::min(patchXsize, patchYsize);
		double radius = diameter / 2.;

		//Direction constant depending on whether this is nadir or zenith
		double dirConst = -1;
		if (fNadir) dirConst = 1;

		//For every pixel in the destination matrix, find an interpolated pixel from the patch image
		for (int i = 0; i < dest.rows; i++)				//polar angle coordinate
		{
			for (int j = 0; j < dest.cols; j++)			//azimuthal angle coordinate
			{
				//Polar coordinates in the patch image
				double r = double(i) / double(dest.rows - 1) * radius;
				double phi = double(j) / double(dest.cols) * 2. * Pi + Pi / 2.;

				//Cartesian coordinates in the patch image
				double x = patchXcenter + r * cos(phi);
				double y = patchYcenter + dirConst * r * sin(phi);

				cv::Vec3b interpPixelValue = InterpolatePixel(patchImage, cv::Point2f(float(x), float(y)),
												cv::BORDER_REFLECT_101, cv::BORDER_REFLECT_101);

				int rowCoord;
				if (fNadir) rowCoord = dest.rows - i - 1;
				else rowCoord = i;

				dest.at<cv::Vec3b>(rowCoord, j) = interpPixelValue;
			}
		}
	}
}

//User is opening a new folder
//May press cancel on the dialogs
void PanoTwist::OnOpenFolderClicked()
{
	QString response = QFileDialog::getOpenFileName(this,
		tr("Select a folder by picking any panorama image in it:"), "", tr("JPEG and TIFF images (*.jpg *.jpeg *.tif *.tiff)"));

	if (response == "") return;		//pressed cancel

	//Local vars until the user decides to go ahead
	BString newFolder = QFileInfo(response).absolutePath().toStdString() + "/";
	CHArray<BString> newNameOnlyArray;

	//Create the OpeningFolder dialog
	//That will search the directory for panorama files
	DialogOpeningFolder* dialog = new DialogOpeningFolder(this, newFolder, newNameOnlyArray);

	if (dialog->exec() != QDialog::Accepted)		//Pressed cancel - show message and return
	{
		ui.statusBar->showMessage("Opening folder cancelled.", 3000);
		return;
	}

	//If there are no files in the list, clear all data, show message and return
	if (newNameOnlyArray.Count() == 0)
	{
		ui.statusBar->showMessage("No panoramas found in selected folder.", 3000);
		Init();
		return;
	}

	//****** We are going ahead with swithching to new folder

	//Clear all the data
	Init();
	curFolder = newFolder;
	ui.labelCurrentFolder->setText(curFolder.c_str());
	ui.statusBar->showMessage("Opened new folder.", 3000);
	nameOnlyArray = newNameOnlyArray;

	//Create an array with full file names
	for (auto& name : nameOnlyArray) fileArray << curFolder + name;
	
	//Look for the selected file in fileArray
	//If it is there, move it to the beginning of the list
	BString selectedFile = response.toStdString();
	int pos = fileArray.PositionOf(selectedFile);
	if (pos > 0)
	{
		fileArray.SwitchElements(0, pos);
		nameOnlyArray.SwitchElements(0, pos);
	}

	curIndex = 0;
	UpdateInterface();
}

void PanoTwist::OnPrevFileClicked()
{
	if (curIndex == -1) return;

	if (curIndex == 0) return;
	
	curIndex--;
	UpdateInterface();
}

void PanoTwist::OnNextFileClicked()
{
	if (curIndex == -1) return;

	if (curIndex == fileArray.Count() - 1) return;
	
	curIndex++;
	UpdateInterface();
}

//Checks that the directory still exists and creates /Panotwist output/ subfolder, if it isn't there
bool PanoTwist::FileSaveChecks()
{
	if (curIndex == -1) return false;

	//Check that the current folder exists - if it does not, show error and return
	if (!QDir(curFolder.c_str()).exists())
	{
		QtUtils::ErrorBox("The opened folder, " + curFolder + ", no longer exists. Unable to save files there.");
		return false;
	}

	//Folder where the results are going to be saved
	BString resultsFolder = curFolder + saveSubfolderName;

	//If the folder does not exist, create it
	if (!QDir(resultsFolder.c_str()).exists()) QDir().mkdir(resultsFolder.c_str());

	//If it still does not exist, show error and return
	if (!QDir(resultsFolder.c_str()).exists())
	{
		QtUtils::ErrorBox("Unable to create the /Panotwist output/ folder in the current folder, " + curFolder + "."
			" The file could not be saved.");
		return false;
	}

	return true;
}

//Writes to a directory named "Panotwist output" inside the current directory
void PanoTwist::OnApplyClicked()
{
	if (!FileSaveChecks()) return;

	setCursor(Qt::WaitCursor);

	//Folder where the results are going to be saved
	BString resultsFolder = curFolder + saveSubfolderName;
	
	//Copy original full-size image and process it
	cv::Mat temp;
	fullMat.copyTo(temp);
	ProcessImage(temp, true, true);

	//Write it in the results folder
	BString newFileName = resultsFolder + nameOnlyArray[curIndex];
	imwrite(newFileName, temp);

	BString info = "Saved file " + nameOnlyArray[curIndex];
	ui.statusBar->showMessage(info.c_str(), 2000);

	//Add the panorama Exif tag
	InsertExifTags(newFileName, temp);

	setCursor(Qt::ArrowCursor);
}

//Batch save
//User wants to apply the nadir and zenith settings to all files
//Also rescales as needed, but does not rotate
void PanoTwist::OnApplyToAllClicked()
{
	using namespace std::placeholders;
	if (!FileSaveChecks()) return;

	//Create the SaveAll dialog that will save everything
	DialogSaveAll* dialog = new DialogSaveAll(	this,
												curFolder,
												nameOnlyArray,
												saveSubfolderName,
												//batch save does not rotate, but rescales if needed
												std::bind(&PanoTwist::ProcessImage, this, _1, false, true),
												std::bind(&PanoTwist::InsertExifTags, this, _1, _2)
												);

	if (dialog->exec() == QDialog::Accepted)
	{
		BString info;
		info.Format("%i panoramas saved.", fileArray.Count());
		ui.statusBar->showMessage(info.c_str(), 3000);
	}
}

//Fix exif tags after the rotation is done
void PanoTwist::InsertExifTags(const BString& filename, const cv::Mat& image)
{
	Exiv2::ExifData exifData;
	exifData["Exif.Image.ImageWidth"] = int32_t(image.cols);
	exifData["Exif.Image.ImageLength"] = int32_t(image.rows);

	Exiv2::XmpData xmpData;

	xmpData["Xmp.GPano.ProjectionType"] = Exiv2::XmpTextValue("equirectangular");
	xmpData["Xmp.GPano.UsePanoramaViewer"] = true;

	xmpData["Xmp.GPano.CroppedAreaLeftPixels"] = int32_t(0);
	xmpData["Xmp.GPano.CroppedAreaTopPixels"] = int32_t(0);

	xmpData["Xmp.GPano.CroppedAreaImageWidthPixels"] = int32_t(image.cols);
	xmpData["Xmp.GPano.CroppedAreaImageHeightPixels"] = int32_t(image.rows);

	xmpData["Xmp.GPano.FullPanoWidthPixels"] = int32_t(image.cols);
	xmpData["Xmp.GPano.FullPanoHeightPixels"] = int32_t(image.rows);

	Exiv2::Image::AutoPtr imageFile = Exiv2::ImageFactory::open(filename);
	if (!imageFile.get()) return;

	imageFile->setXmpData(xmpData);
	imageFile->setExifData(exifData);
	imageFile->writeMetadata();
}

void PanoTwist::OnNadirZenithChanged()
{
	ShowImage();
}

void PanoTwist::OnImageMouseLeftPressed(cv::Point2d pos)
{
	fDraggingImage = true;
	lastClickCoord = pos;
	clickRotationRad = rotationRad;
}

void PanoTwist::OnImageMouseLeftReleased(cv::Point2d pos)
{
	fDraggingImage = false;
}

void PanoTwist::OnImageMouseMoved(cv::Point2d pos)
{
	if (!fDraggingImage || curIndex == -1) return;

	double diff = pos.x - lastClickCoord.x;

	rotationRad = clickRotationRad + diff / double(scaledMat.cols) * 2. * Pi;

	ShowImage();
}

void PanoTwist::OnMenuAbout()
{
	DialogAbout* dialog = new DialogAbout(this);
	dialog->exec();
}

void PanoTwist::OnMenuHelp()
{
	DialogHelpOrLicence* dialog = new DialogHelpOrLicence(this);
	dialog->setWindowTitle("Pano Twist Help");
	dialog->SetTopLabelText("PanoTwist usage instructions:");
	dialog->SetMainText("DialogContentH.dld");

	dialog->exec();
}

void PanoTwist::OnMenuLicense()
{
	DialogHelpOrLicence* dialog = new DialogHelpOrLicence(this);
	dialog->setWindowTitle("License");
	dialog->SetTopLabelText("This software is distributed under the following License:");
	dialog->SetMainText("DialogContentL.dld");

	dialog->exec();
}

// Get the value of the interpolated pixel from the image
//Image must not be empty
//And its pixels should be cv::Vec3b
//Border types specify the handling of the points that are extrapolated outside the image
cv::Vec3b PanoTwist::InterpolatePixel(const cv::Mat& img, cv::Point2f pt, int borderX, int borderY)
{
	int x = (int)pt.x;
	int y = (int)pt.y;

	int x0 = cv::borderInterpolate(x, img.cols, borderX);
	int x1 = cv::borderInterpolate(x + 1, img.cols, borderX);
	int y0 = cv::borderInterpolate(y, img.rows, borderY);
	int y1 = cv::borderInterpolate(y + 1, img.rows, borderY);

	float a = pt.x - (float)x;
	float c = pt.y - (float)y;

	uchar b = (uchar)cvRound((img.at<cv::Vec3b>(y0, x0)[0] * (1.f - a) + img.at<cv::Vec3b>(y0, x1)[0] * a) * (1.f - c)
		+ (img.at<cv::Vec3b>(y1, x0)[0] * (1.f - a) + img.at<cv::Vec3b>(y1, x1)[0] * a) * c);
	uchar g = (uchar)cvRound((img.at<cv::Vec3b>(y0, x0)[1] * (1.f - a) + img.at<cv::Vec3b>(y0, x1)[1] * a) * (1.f - c)
		+ (img.at<cv::Vec3b>(y1, x0)[1] * (1.f - a) + img.at<cv::Vec3b>(y1, x1)[1] * a) * c);
	uchar r = (uchar)cvRound((img.at<cv::Vec3b>(y0, x0)[2] * (1.f - a) + img.at<cv::Vec3b>(y0, x1)[2] * a) * (1.f - c)
		+ (img.at<cv::Vec3b>(y1, x0)[2] * (1.f - a) + img.at<cv::Vec3b>(y1, x1)[2] * a) * c);

	return cv::Vec3b(b, g, r);
}

//Performs horizontal right cyclical shift of the matrix
void PanoTwist::HorizontalCyclicRotate(cv::Mat& mat, int numPixels)
{
	//Shift the value of numPixels into the interval [0,mat.cols) as needed
	numPixels = numPixels % mat.cols;
	if (numPixels < 0) numPixels += mat.cols;

	if (numPixels == 0) return;

	cv::Mat temp = cv::Mat::zeros(mat.size(), mat.type());

	cv::Rect r0 = cv::Rect(mat.cols - numPixels, 0, numPixels, mat.rows);
	cv::Rect r1 = cv::Rect(0, 0, numPixels, mat.rows);
	cv::Rect r2 = cv::Rect(0, 0, mat.cols - numPixels, mat.rows);
	cv::Rect r3 = cv::Rect(numPixels, 0, mat.cols - numPixels, mat.rows);

	mat(r0).copyTo(temp(r1));
	mat(r2).copyTo(temp(r3));
	temp.copyTo(mat);
}