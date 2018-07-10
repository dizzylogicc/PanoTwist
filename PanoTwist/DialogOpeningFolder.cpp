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

#include "DialogOpeningFolder.h"
#include <QDir>

DialogOpeningFolder::DialogOpeningFolder(QWidget* parent, const BString& theFolder, CHArray<BString>& outNameOnlyArray) :
DialogSaveOrOpen(parent, theFolder, outNameOnlyArray)
{
	//No need to setup Ui, it's already set up in the base class

	setWindowTitle("Opening folder");
	
	fileList.Clear();

	//Set label text
	ui.labelFolder->setText(QString("Opening folder ") + folder);
	ui.labelCurFile->setText("Reading images...");

	//Start the worker thread
	std::thread worker(&DialogOpeningFolder::WorkerThread, this);
	worker.detach();
}

void DialogOpeningFolder::OnThreadFinished()
{
	ui.bnOk->setEnabled(true);
	ui.bnOk->setFocus();
	fWorkerFinished = true;

	BString info;
	if(fileList.Count() > 0) info.Format("Finished - found %i panoramas.",fileList.Count());
	else info = "Finished - no equirectangular panoramas found in this folder.";
	ui.labelCurFile->setText(info.c_str());
	ui.progressBar->setValue(100);

	if (fCloseRequested) { reject(); return; }
}

void DialogOpeningFolder::OnFileFinished()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	ui.progressBar->setValue(std::round(percentDone));

	BString info;
	info.Format("Opened %s. %i panoramas found.", lastImageName, fileList.Count());
	ui.labelCurFile->setText(info.c_str());

	imageWidget->RescaleToParentAndShow(lastImage);
}

void DialogOpeningFolder::WorkerThread()
{
	//Get all JPEG files in the directory
	QDir dir(folder.c_str());

	QStringList filters;
	filters << "*.jpg" << "*.jpeg" << "*.tif" << "*.tiff";
	dir.setNameFilters(filters);
	dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

	//Read the list of files in the directory
	QStringList dirList = dir.entryList();
	
	//Check each file and store file names into the array
	//For equirectangular panoramas (width == height * 2)
	int numFilesRead = 0;
	for (auto& fileName : dirList)
	{
		BString fullFileName = folder + fileName.toStdString().c_str();
		cv::Mat mat = cv::imread(fullFileName, CV_LOAD_IMAGE_COLOR);
		numFilesRead++;

		if(1)		//So that the compiler does not try to optimize out the block
		{
			//This block is guarded by a mutex
			std::lock_guard<std::recursive_mutex> lock(mutex);

			percentDone = 100.0 * double(numFilesRead) / double(dirList.count());
			lastImageName = fileName.toStdString();
			mat.copyTo(lastImage);

			if (mat.size().width > 0 && mat.size().height > 0 && mat.size().width == mat.size().height * 2) fileList << fileName.toStdString();

			emit SignalFileFinished();
		}

		if (fCloseRequested) break;
	}

	emit SignalThreadFinished();
}