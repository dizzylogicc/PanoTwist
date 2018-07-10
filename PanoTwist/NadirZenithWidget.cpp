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

#include "NadirZenithWidget.h"
#include "QtUtils.h"
#include <QFileDialog>
#include <QRegExpValidator>

NadirZenithWidget::NadirZenithWidget(QWidget* parent) :
QWidget(parent)
{
	ui.setupUi(this);

	imageWidget = new CvImageWidget(ui.frameImage);
	fPatchImageLoaded = false;

	QRegExp colorRegExp("^#?([A-Fa-f0-9]{6})$");
	colorValidator = new QRegExpValidator(colorRegExp, this);
	//ui.editColor->setValidator(colorValidator);

	curColorString = "#AAAAAA";
	ui.editColor->setText(curColorString.c_str());

	ui.radioAverage->setChecked(true);
	HandleEnabling();
}

void NadirZenithWidget::SetColor(const BString& colorString)
{
	int pos = 0;
	QString str = colorString.c_str();
	if (colorValidator->validate(str, pos) == QValidator::Acceptable)
	{
		curColorString = colorString;
		curColorString.MakeUpper();
		ui.editColor->setText(curColorString.c_str());
		emit SignalSettingsChanged();
	}
	else ui.editColor->setText(curColorString.c_str());
};

void NadirZenithWidget::HandleEnabling()
{
	//Disable or enable everything depending on the state of the main check box
	bool fCheckEnabled = IsEnabled();

	ui.radioAverage->setEnabled(fCheckEnabled);
	ui.radioColor->setEnabled(fCheckEnabled);
	ui.radioImage->setEnabled(fCheckEnabled);

	ui.label1->setEnabled(fCheckEnabled);
	ui.label2->setEnabled(fCheckEnabled);
	ui.label3->setEnabled(fCheckEnabled);

	ui.editColor->setEnabled(fCheckEnabled);
	ui.frameImage->setEnabled(fCheckEnabled);
	ui.spinAngle->setEnabled(fCheckEnabled);
	ui.bnSelectImage->setEnabled(fCheckEnabled);

	if (!fCheckEnabled) return;

	//Handle radio buttons with fill options
	ui.editColor->setEnabled(ui.radioColor->isChecked());
	ui.label1->setEnabled(ui.radioColor->isChecked());

	ui.bnSelectImage->setEnabled(ui.radioImage->isChecked());
	ui.frameImage->setEnabled(ui.radioImage->isChecked());
}

void NadirZenithWidget::OnColorEditingFinished()
{
	BString colorText = ui.editColor->text().toStdString();
	SetColor(colorText);
}

void NadirZenithWidget::OnColorTextChanged()
{
	int pos = 0;
	QString colorText = ui.editColor->text();
	if (colorValidator->validate(colorText, pos) == QValidator::Acceptable)
	{
		SetColor(colorText.toStdString());
		ui.editColor->setStyleSheet("QLineEdit { background: #ffffff; }");
	}
	else ui.editColor->setStyleSheet("QLineEdit { background: #ff8080; }");
}

void NadirZenithWidget::OnSelectImageClicked()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open image to patch nadir or zenith"), "", tr("Image Files (*.png *.jpeg *.jpg *.tif *. tiff *.bmp)"));

	if (fileName == "") return;

	cv::Mat mat = cv::imread(fileName.toStdString(), CV_LOAD_IMAGE_COLOR);

	if (mat.cols == 0 || mat.rows == 0)
	{
		QtUtils::ErrorBox(BString("Failed to open image ") + fileName.toStdString().c_str());
		return;
	}

	//Save the opened image as patchImage
	patchImage = mat;

	//Then extract the central square from the image
	cv::Mat square;

	//If the image is wide
	if (mat.cols > mat.rows) cv::Mat(mat, cv::Rect((mat.cols - mat.rows) / 2, 0, mat.rows, mat.rows)).copyTo(square);

	//If the image is tall or square
	else cv::Mat(mat, cv::Rect(0, (mat.rows - mat.cols) / 2, mat.cols, mat.cols)).copyTo(square);

	//Iterate over the pixels of the square and grey out the pixels that are outside the central circle
	int squareSize = square.cols;
	double center = double(squareSize - 1.) / 2.;
	double center2 = center*center;
	cv::Vec3b gray(120, 120, 120);
	 
	for (int i = 0; i < squareSize; i++)
	{
		for (int j = 0; j < squareSize; j++)
		{
			double x = i;
			double y = j;

			double dist2 = (x - center)*(x - center) + (y - center)*(y - center);

			//one pixel margin in favor of including the pixel
			if (dist2 > (center2+1.)) square.at<cv::Vec3b>(i, j) = gray;
		}
	}
	
	imageWidget->RescaleToParentAndShow(square);
	fPatchImageLoaded = true;

	emit SignalSettingsChanged();
}