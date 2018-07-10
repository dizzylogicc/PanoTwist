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

//Base class for save folder and open folder dialogs in Pano Twist
#pragma once

#include <QDialog>
#include "ui_DialogOpeningFolder.h"

#include "CvImageWidget.h"
#include "BString.h"

#include <mutex>
#include <thread>
#include <atomic>

class DialogSaveOrOpen : public QDialog
{
	Q_OBJECT

public:
	DialogSaveOrOpen(QWidget* parent, const BString& theFolder, CHArray<BString>& theFileList);
	~DialogSaveOrOpen(){}

protected:
	//Redefine the QDialog::reject method
	void reject();

signals:
	void SignalThreadFinished();	//The thread has finished opening all files in the directory
	void SignalFileFinished();		//The thread has finished opening a single file in the directory

public slots:
	virtual void OnBnOKclicked();
	virtual void OnBnCancelClicked();
	virtual void OnThreadFinished() = 0;
	virtual void OnFileFinished() = 0;

protected:
	//The actual work is done in a separate thread
	//For interface responsiveness
	//The thread communicates to the interface thread through queued connection signals
	virtual void WorkerThread() = 0;

protected:
	//These are only accessed by the worker thread
	CHArray<BString>& fileList;
	BString folder;

	//The user has requested to close the dialog window - shut down the worker thread, then close
	std::atomic<bool> fCloseRequested;
	bool fWorkerFinished;

protected:
	//Access to these members is done from two threads
	//Protected by a mutex
	std::recursive_mutex mutex;

	CvImageWidget* imageWidget;
	int numImagesProcessed;
	BString lastImageName;
	double percentDone;
	cv::Mat lastImage;

protected:
	Ui::DialogOpeningFolderClass ui;
};