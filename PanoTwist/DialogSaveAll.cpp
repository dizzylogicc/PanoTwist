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

#include "DialogSaveAll.h"
#include <QDir>

DialogSaveAll::DialogSaveAll(
							QWidget* parent,
							const BString& theFolder,
							CHArray<BString>& theFileList,
							const BString& theSaveSubfolder,
							std::function<void(cv::Mat&)> theProcessingFunction,
							std::function<void(const BString&, const cv::Mat&)> theExifTagFunction) :
DialogSaveOrOpen(parent, theFolder, theFileList),
saveSubfolder(theSaveSubfolder),
processingFunction(theProcessingFunction),
exifTagFunction(theExifTagFunction)
{
	//No need to setup Ui, it's already set up in the base class

	setWindowTitle("Saving all panoramas");

	//Set label text
	ui.labelFolder->setText(QString("Saving to folder ") + folder);
	ui.labelCurFile->setText("Reading images...");

	//Start the worker thread
	std::thread worker(&DialogSaveAll::WorkerThread, this);
	worker.detach();
}

void DialogSaveAll::OnThreadFinished()
{
	ui.bnOk->setEnabled(true);
	ui.bnOk->setFocus();
	ui.bnCancel->setEnabled(false);
	fWorkerFinished = true;

	BString info;
	info.Format("Finished - saved %i panorama files.", numImagesProcessed);
	ui.labelCurFile->setText(info.c_str());
	ui.progressBar->setValue(100);

	if (fCloseRequested) { reject(); return; }
}

void DialogSaveAll::OnFileFinished()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	ui.progressBar->setValue(std::round(percentDone));

	BString info;
	info.Format("Saved file %s (%i files processed).", lastImageName, numImagesProcessed);
	ui.labelCurFile->setText(info.c_str());

	imageWidget->RescaleToParentAndShow(lastImage);
}

void DialogSaveAll::WorkerThread()
{
	//Folder where the results are going to be saved
	BString resultsFolder = folder + saveSubfolder;

	//For each file name in the list
	for (auto& name : fileList)
	{
		//Read matrix, process it, save it to the subfolder, set exif tags
		cv::Mat mat = cv::imread(folder + name, CV_LOAD_IMAGE_COLOR);

		if (mat.cols == 0 || mat.rows == 0) continue;	//Something went wrong - maybe the user moved the file

		processingFunction(mat);
		cv::imwrite(resultsFolder + name, mat);
		exifTagFunction(resultsFolder + name, mat);

		numImagesProcessed++;

		if (1)
		{
			//This block is guarded by a mutex
			std::lock_guard<std::recursive_mutex> lock(mutex);
			percentDone = 100.0 * double(numImagesProcessed) / double(fileList.Count());
			lastImageName = name;
			mat.copyTo(lastImage);
		}

		emit SignalFileFinished();
		if (fCloseRequested) break;
	}
	
	emit SignalThreadFinished();	
}